// ARMOR-NODE-SOLAR-HMI - the start of the firmware: it says what the board is, starts screen, touch and cable network, and shows the board check screen. Set up from the serial console
// (core/panel_console.hpp), the solar pages show what ARMOR-SERVER says; everything starts off, and no operation on a battery is sent. NOT VERIFIED ON A BOARD.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#include <cstdio>
#include <mutex>
#include <string>
#include <utility>

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
#include "link.hpp"
#include "ota.hpp"
#include "panel_config.hpp"
#include "panel_console.hpp"
#include "pin_gate.hpp"
#include "sdkconfig.h"
#include "serial_console.hpp"
#include "store.hpp"
#include "ui.hpp"
#if CONFIG_SOLAR_WIFI
#include "solar_wifi.h"
#endif

namespace {

constexpr const char* TAG = "solar-hmi";

std::string g_chip, g_i2c = "-", g_touch = "not started", g_screen = "not started", g_ethernet_error, g_wifi = "not in this build";
bool g_screen_up = false;
armor::solarhmi::PanelConfig g_config;   // the settings of this start; the console changes the stored ones, used at the next start
armor::solarhmi::PanelConfig g_edited;   // only the console's task touches it
std::mutex g_pin_lock;
armor::solarhmi::PinGate g_pin;          // under g_pin_lock: the screen and the console both use it

std::int64_t now_ms() { return esp_timer_get_time() / 1000; }

std::string wifi_address() {
#if CONFIG_SOLAR_WIFI
  char text[20];
  solar_wifi_address(text, sizeof text);
  return text;
#else
  return "";
#endif
}

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
  const std::string wifi = wifi_address();
  info.wifi = "ESP32-C6: " + (wifi.empty() ? g_wifi : wifi);
  info.uptime = uptime_text();
  return info;
}

void show_check() { armor::solarhmi::diag::show(check_info, {}, "en"); }

armor::solarhmi::ui::State screen_state() {
  static std::string last_address;
  static std::uint32_t generation = 0;
  static std::uint32_t last_answer = 0;
  armor::solarhmi::ui::State state;
  state.version = esp_app_get_description()->version;
  state.language = g_config.language;
  char address[20];
  board_ethernet_address(address, sizeof address);
  state.address = address;
  if (state.address.empty()) state.address = wifi_address();
  {
    std::lock_guard<std::mutex> guard(g_pin_lock);
    state.pin_set = g_pin.record().set;
    state.pin_session = g_pin.unlocked(now_ms());
    state.pin_locked_s = static_cast<int>((g_pin.locked_for_ms(now_ms()) + 999) / 1000);
  }
  armor::solarhmi::net::View view = armor::solarhmi::net::view();
  state.link = view.link;
  state.snapshot = std::move(view.snapshot);
  if (state.address != last_address || view.generation != last_answer) {
    last_address = state.address;
    last_answer = view.generation;
    ++generation;
  }
  state.generation = generation;
  return state;
}

// The screen's PIN pad. The record, with its count of wrong tries, is written before the answer is shown.
armor::solarhmi::PinResult unlock(const std::string& digits) {
  std::lock_guard<std::mutex> guard(g_pin_lock);
  const armor::solarhmi::PinResult result = g_pin.enter(digits, now_ms());
  armor::solarhmi::store::save_pin(g_pin.record());
  return result;
}

// One line of the serial console. It runs in the console's task.
void on_console(const std::string& line) {
  using armor::solarhmi::ConsoleAction;
  namespace store = armor::solarhmi::store;
  const armor::solarhmi::ConsoleResult result = armor::solarhmi::console_line(g_edited, line);
  if (!result.answer.empty()) std::printf("%s\n", result.answer.c_str());
  switch (result.action) {
    case ConsoleAction::kSave:
      if (!store::save_config(g_edited)) std::printf("could not be saved\n");
      break;
    case ConsoleAction::kShow: {
      std::lock_guard<std::mutex> guard(g_pin_lock);
      std::printf("%s\nlink now: %s\n", armor::solarhmi::config_summary(g_edited, g_pin.record().set).c_str(), armor::solarhmi::net::last_result());
      break;
    }
    case ConsoleAction::kSetPin: {
      std::lock_guard<std::mutex> guard(g_pin_lock);
      armor::solarhmi::Salt salt{};
      store::random_bytes(salt.data(), salt.size());
      const armor::solarhmi::PinRecord before = g_pin.record();
      g_pin.restore(armor::solarhmi::PinRecord{}, now_ms());   // whoever types here holds the board: a new PIN replaces the old one
      if (!g_pin.set_pin(result.argument, salt, now_ms())) {
        g_pin.restore(before, now_ms());
        std::printf("the PIN was not set: 4 to 10 digits\n");
      } else if (!store::save_pin(g_pin.record())) {
        std::printf("the PIN is set until the next restart: it could not be saved\n");
      } else {
        std::printf("the PIN is set\n");
      }
      break;
    }
    case ConsoleAction::kForgetPin: {
      std::lock_guard<std::mutex> guard(g_pin_lock);
      g_pin.restore(armor::solarhmi::PinRecord{}, now_ms());
      std::printf("%s\n", store::save_pin(g_pin.record()) ? "the PIN is forgotten" : "could not be saved");
      break;
    }
    case ConsoleAction::kForgetAll: {
      std::lock_guard<std::mutex> guard(g_pin_lock);
      g_pin.restore(armor::solarhmi::PinRecord{}, now_ms());
      if (!store::forget()) std::printf("could not be erased\n");
      break;
    }
    case ConsoleAction::kUpdate: {
      const std::string error = armor::solarhmi::ota::install(result.argument);
      if (error.empty()) std::printf("installed: type restart to run it\n");
      else std::printf("not installed: %s\n", error.c_str());
      break;
    }
    case ConsoleAction::kRestart:
      vTaskDelay(pdMS_TO_TICKS(200));
      esp_restart();
      break;
    default:
      break;
  }
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
  actions.unlock = unlock;
  actions.lock = [] {
    std::lock_guard<std::mutex> guard(g_pin_lock);
    g_pin.close();
  };
  actions.touched = [] {
    std::lock_guard<std::mutex> guard(g_pin_lock);
    g_pin.touch(now_ms());
  };
  if (!armor::solarhmi::ui::start(screen_state, actions)) ESP_LOGE(TAG, "the screen could not be built");
  show_check();
  lvgl_port_unlock();
  board_backlight(80);
  g_screen_up = true;
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

  if (armor::solarhmi::store::start()) {
    g_config = armor::solarhmi::store::load_config();
    g_pin.restore(armor::solarhmi::store::load_pin(), now_ms());   // before any other task exists
  }
  g_edited = g_config;

  start_screen();

  const esp_err_t network = board_ethernet_start();
  if (network != ESP_OK) {
    g_ethernet_error = std::string("not started: ") + esp_err_to_name(network);
    ESP_LOGW(TAG, "ethernet %s", g_ethernet_error.c_str());
  }

#if CONFIG_SOLAR_WIFI
  if (g_config.wifi_on) {
    const esp_err_t wifi = solar_wifi_start(g_config.ssid.c_str(), g_config.wifi_password.c_str());
    g_wifi = wifi == ESP_OK ? std::string("joining") : std::string("error: ") + esp_err_to_name(wifi);
  } else {
    g_wifi = "off";
  }
#endif
  // the link needs the network layer, which the cable or the radio started
  if (!armor::solarhmi::net::start(g_config)) ESP_LOGI(TAG, "the link to the server is off or not set up (type help)");
  if (!armor::solarhmi::serial::start(on_console)) ESP_LOGW(TAG, "the serial console did not start");

  // one line every ten seconds, so that a log taken at any moment says where the board is
  bool confirmed = false;
  while (true) {
    vTaskDelay(pdMS_TO_TICKS(10000));
    if (!confirmed && g_screen_up && now_ms() > 60000) {
      armor::solarhmi::ota::confirm_running();   // an update that does not get this far is rolled back at the next start
      confirmed = true;
    }
    ESP_LOGI(TAG, "up %s | %s | touch: %s | ethernet: %s", uptime_text().c_str(), memory_text().c_str(), g_touch.c_str(), ethernet_text().c_str());
  }
}
