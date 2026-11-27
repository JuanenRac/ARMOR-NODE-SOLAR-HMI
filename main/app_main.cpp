// ARMOR-NODE-SOLAR-HMI - the start of the firmware: it says what the board is, starts screen, touch and cable network, and shows the board check screen. The solar pages behind it have
// no server yet: they say "not set up", and no operation on a battery is offered. NOT VERIFIED ON A BOARD.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#include <cstdio>
#include <string>

#include "board.h"
#include "diag_ui.hpp"
#include "esp_app_desc.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "esp_psram.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "pin_gate.hpp"
#include "sdkconfig.h"
#include "ui.hpp"

namespace {

constexpr const char* TAG = "solar-hmi";

std::string g_chip, g_i2c = "-", g_touch = "not started", g_screen = "not started", g_ethernet_error;
armor::solarhmi::PinGate g_pin;   // no PIN is kept in flash yet: none is set, so nothing can be unlocked

const char* reset_name(esp_reset_reason_t reason) {
  switch (reason) {
    case ESP_RST_POWERON: return "power on";
    case ESP_RST_SW: return "restart";
    case ESP_RST_PANIC: return "crash";
    case ESP_RST_INT_WDT:
    case ESP_RST_TASK_WDT:
    case ESP_RST_WDT: return "watchdog";
    case ESP_RST_BROWNOUT: return "low supply";
    case ESP_RST_USB: return "USB";
    default: return "other";
  }
}

std::string uptime_text() {
  const long long seconds = esp_timer_get_time() / 1000000;
  char text[32];
  std::snprintf(text, sizeof text, "%lld h %02lld min %02lld s", seconds / 3600, (seconds / 60) % 60, seconds % 60);
  return text;
}

std::string memory_text() {
  std::uint32_t flash = 0;
  if (esp_flash_get_size(nullptr, &flash) != ESP_OK) flash = 0;
  const std::size_t psram = esp_psram_is_initialized() ? esp_psram_get_size() : 0;
  char text[128];
  std::snprintf(text, sizeof text, "flash %u MB, PSRAM %u MB, free internal %u KB, free PSRAM %u KB", static_cast<unsigned>(flash >> 20), static_cast<unsigned>(psram >> 20),
                static_cast<unsigned>(heap_caps_get_free_size(MALLOC_CAP_INTERNAL) >> 10), static_cast<unsigned>(heap_caps_get_free_size(MALLOC_CAP_SPIRAM) >> 10));
  return text;
}

std::string ethernet_text() {
  if (!g_ethernet_error.empty()) return g_ethernet_error;
  char text[32];
  board_ethernet_text(text, sizeof text);
  return text;
}

armor::solarhmi::diag::Info check_info() {
  armor::solarhmi::diag::Info info;
  info.chip = g_chip;
  info.memory = memory_text();
  info.screen = g_screen;
  info.touch = g_touch;
  info.i2c = g_i2c;
  info.ethernet = ethernet_text();
  info.wifi = "ESP32-C6: not started by this build";
  info.uptime = uptime_text();
  return info;
}

void show_check() { armor::solarhmi::diag::show(check_info, {}, "en"); }

armor::solarhmi::ui::State screen_state() {
  static std::string last_address;
  static std::uint32_t generation = 0;
  armor::solarhmi::ui::State state;
  state.version = esp_app_get_description()->version;
  char address[20];
  board_ethernet_address(address, sizeof address);
  state.address = address;
  state.pin_set = g_pin.record().set;
  if (state.address != last_address) {
    last_address = state.address;
    ++generation;
  }
  state.generation = generation;
  return state;
}

void start_screen() {
  esp_err_t error = board_display_start();
  if (error != ESP_OK) {
    ESP_LOGE(TAG, "the panel did not start: %s", esp_err_to_name(error));
    return;
  }
  lv_display_t* display = nullptr;
  error = board_lvgl_start(&display);
  if (error != ESP_OK) {
    ESP_LOGE(TAG, "LVGL did not start: %s", esp_err_to_name(error));
    return;
  }
  char panel[96];
#if CONFIG_SOLAR_LCD_PANEL_OLD
  const char* const kind = "old panel";
#else
  const char* const kind = "new panel (V2)";
#endif
  std::snprintf(panel, sizeof panel, "JD9165 1024x600, %s, %d MHz, h back porch %d", kind, CONFIG_SOLAR_LCD_DPI_CLOCK_MHZ, CONFIG_SOLAR_LCD_HBP);
  g_screen = panel;

  int address = 0;
  error = board_touch_start(display, &address);
  if (error == ESP_OK) {
    char touch[64];
    std::snprintf(touch, sizeof touch, "GT911 at 0x%02X, %d x %d", address, CONFIG_SOLAR_TOUCH_WIDTH, CONFIG_SOLAR_TOUCH_HEIGHT);
    g_touch = touch;
  } else {
    g_touch = std::string("no touch: ") + esp_err_to_name(error);
  }

  if (!lvgl_port_lock(0)) return;
  armor::solarhmi::ui::Actions actions;
  actions.diagnostics = show_check;
  if (!armor::solarhmi::ui::start(screen_state, actions)) ESP_LOGE(TAG, "the screen could not be built");
  show_check();
  lvgl_port_unlock();
  board_backlight(80);
}

}  // namespace

extern "C" void app_main(void) {
  const esp_app_desc_t* app = esp_app_get_description();
  esp_chip_info_t chip;
  esp_chip_info(&chip);
  char text[96];
  std::snprintf(text, sizeof text, "ESP32-P4 revision v%u.%u, %u cores, last reset: %s", chip.revision / 100, chip.revision % 100, chip.cores, reset_name(esp_reset_reason()));
  g_chip = text;
  ESP_LOGI(TAG, "ARMOR-NODE-SOLAR-HMI %s (built %s %s)", app->version, app->date, app->time);
  ESP_LOGI(TAG, "%s", g_chip.c_str());
  ESP_LOGI(TAG, "%s", memory_text().c_str());
  if (!esp_psram_is_initialized()) ESP_LOGE(TAG, "no external RAM: the screen will not start");

  char found[96];
  const esp_err_t bus = board_i2c_start(found, sizeof found);
  g_i2c = bus == ESP_OK ? std::string(found) : std::string("error: ") + esp_err_to_name(bus);

  start_screen();

  const esp_err_t network = board_ethernet_start();
  if (network != ESP_OK) {
    g_ethernet_error = std::string("not started: ") + esp_err_to_name(network);
    ESP_LOGW(TAG, "ethernet %s", g_ethernet_error.c_str());
  }

  // one line every ten seconds, so that a log taken at any moment says where the board is
  while (true) {
    vTaskDelay(pdMS_TO_TICKS(10000));
    ESP_LOGI(TAG, "up %s | %s | touch: %s | ethernet: %s", uptime_text().c_str(), memory_text().c_str(), g_touch.c_str(), ethernet_text().c_str());
  }
}
