// ARMOR-NODE-SOLAR-HMI - the serial console (see serial_console.hpp). NOT VERIFIED ON A BOARD.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#include "serial_console.hpp"

#include <cstdint>
#include <utility>

#include "driver/uart.h"
#include "driver/usb_serial_jtag.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "panel_console.hpp"

namespace armor::solarhmi::serial {
namespace {

std::function<void(const std::string&)> g_on_line;
bool g_uart = false, g_usb = false;

// One byte of one of the two sources: a line ends with CR or LF; a line that grows too long is dropped to its end.
struct Line {
  std::string text;
  bool dropped = false;
  void take(char c) {
    if (c == '\r' || c == '\n') {
      if (!dropped && !text.empty()) g_on_line(text);
      text.clear();
      dropped = false;
    } else if (c == '\b' || c == 127) {
      if (!text.empty()) text.pop_back();
    } else if (!dropped) {
      text += c;
      if (text.size() > kMaxLine) {
        text.clear();
        dropped = true;
      }
    }
  }
};

void task(void*) {
  Line uart, usb;
  std::uint8_t buffer[64];
  while (true) {
    if (g_uart) {
      const int got = uart_read_bytes(UART_NUM_0, buffer, sizeof buffer, pdMS_TO_TICKS(20));
      for (int i = 0; i < got; ++i) uart.take(static_cast<char>(buffer[i]));
    }
    if (g_usb) {
      const int got = usb_serial_jtag_read_bytes(buffer, sizeof buffer, pdMS_TO_TICKS(20));
      for (int i = 0; i < got; ++i) usb.take(static_cast<char>(buffer[i]));
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

}  // namespace

bool start(std::function<void(const std::string& line)> on_line) {
  g_on_line = std::move(on_line);
  g_uart = uart_driver_install(UART_NUM_0, 1024, 0, 0, nullptr, 0) == ESP_OK;
  usb_serial_jtag_driver_config_t usb = USB_SERIAL_JTAG_DRIVER_CONFIG_DEFAULT();
  g_usb = usb_serial_jtag_driver_install(&usb) == ESP_OK;
  if (!g_uart && !g_usb) return false;
  return xTaskCreate(task, "solar-console", 16384, nullptr, 3, nullptr) == pdPASS;
}

}  // namespace armor::solarhmi::serial
