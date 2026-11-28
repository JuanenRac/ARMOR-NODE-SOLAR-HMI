// ARMOR-NODE-SOLAR-HMI screen simulator - draws the real screen (main/ui.cpp, LVGL 9, 1024x600) on a computer and writes one picture per language and scene as a PPM file.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
//
//   solar_screens OUTPUT_DIR [language ...]       writes OUTPUT_DIR/<language>_<scene>.ppm (all eight languages when none is named)
//
// Evidence: simulator. The pictures show what the code draws with a made-up installation; nothing here has run on a screen. The server's answer is the text of simulator/plant.hpp,
// read by the same reader as the firmware (core/solar_data.hpp), so a field the reader loses is missing in the picture too.
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "diag_ui.hpp"
#include "lvgl.h"
#include "plant.hpp"
#include "screen_text.hpp"
#include "ui.hpp"

namespace {
using namespace armor::solarhmi;

constexpr int kWidth = 1024, kHeight = 600;
std::uint16_t g_buffer[kWidth * kHeight];   // what LVGL draws into
std::uint16_t g_frame[kWidth * kHeight];    // the last full picture
std::uint32_t g_tick_ms = 0;
ui::State g_state;
int g_failures = 0;

std::uint32_t tick_cb() { return g_tick_ms; }

void flush_cb(lv_display_t* display, const lv_area_t*, std::uint8_t* pixels) {
  std::memcpy(g_frame, pixels, sizeof g_frame);
  lv_display_flush_ready(display);
}

void advance(int ms) {
  for (int spent = 0; spent < ms; spent += 10) {
    g_tick_ms += 10;
    lv_timer_handler();
  }
}

void load(sim::Scenario scenario) {
  if (read_snapshot(sim::server_answer(scenario), g_state.snapshot) != ReadError::kNone) {
    std::fprintf(stderr, "the simulator's answer could not be read\n");
    ++g_failures;
  }
  ++g_state.generation;
}

void save(const std::string& directory, const std::string& language, const char* scene) {
  // at the start of the lit half of a blink, so that the picture is taken with the marks of an alarm lit
  advance(100);
  while (lit(Level::kAlarm, static_cast<std::int64_t>(g_tick_ms))) advance(10);
  while (!lit(Level::kAlarm, static_cast<std::int64_t>(g_tick_ms))) advance(10);
  advance(150);
  lv_obj_invalidate(lv_screen_active());
  lv_refr_now(nullptr);
  const std::string path = directory + "/" + language + "_" + scene + ".ppm";
  std::FILE* file = std::fopen(path.c_str(), "wb");
  if (file == nullptr) {
    std::fprintf(stderr, "cannot write %s\n", path.c_str());
    ++g_failures;
    return;
  }
  std::fprintf(file, "P6\n%d %d\n255\n", kWidth, kHeight);
  std::vector<std::uint8_t> row(kWidth * 3);
  for (int y = 0; y < kHeight; ++y) {
    for (int x = 0; x < kWidth; ++x) {
      const std::uint16_t p = g_frame[y * kWidth + x];
      row[x * 3 + 0] = static_cast<std::uint8_t>(((p >> 11) & 0x1F) * 255 / 31);
      row[x * 3 + 1] = static_cast<std::uint8_t>(((p >> 5) & 0x3F) * 255 / 63);
      row[x * 3 + 2] = static_cast<std::uint8_t>((p & 0x1F) * 255 / 31);
    }
    std::fwrite(row.data(), 1, row.size(), file);
  }
  std::fclose(file);
}

void take_language(const std::string& directory, const std::string& language) {
  const std::uint32_t generation = g_state.generation;
  g_state = ui::State{};
  g_state.generation = generation + 1;
  g_state.language = language;
  g_state.version = "0.0.0";

  // the board check screen, with figures that say they are made up
  diag::show(
      [] {
        diag::Info info;
        info.chip = "simulator (no chip)";
        info.memory = "simulator";
        info.screen = "1024x600 simulator";
        info.touch = "simulator";
        info.i2c = "-";
        info.ethernet = "-";
        info.wifi = "-";
        info.uptime = "0 s";
        return info;
      },
      {}, language);
  save(directory, language, "board_check");
  diag::close();

  // before the first answer
  ui::show(ui::Page::kSummary);
  save(directory, language, "waiting");

  g_state.link = Link::kOnline;
  g_state.address = "192.168.1.60";
  load(sim::Scenario::kNormal);
  ui::show(ui::Page::kSummary);
  save(directory, language, "summary");
  ui::show(ui::Page::kBatteries);
  save(directory, language, "batteries");
  ui::show(ui::Page::kInverters);
  save(directory, language, "inverters");
  ui::show(ui::Page::kSettings);
  save(directory, language, "settings");

  load(sim::Scenario::kCellHigh);
  ui::show(ui::Page::kCells, 0, 2);
  save(directory, language, "cells_alarm");
  ui::show(ui::Page::kAlarms);
  save(directory, language, "alarms");

  // the electronics of a battery fail: cells not read, its own words, and a figure left out
  load(sim::Scenario::kBmsFault);
  ui::show(ui::Page::kCells, 0, 3);
  save(directory, language, "bms_fault");

  // the operations: offered here as a server that allows them would
  load(sim::Scenario::kNormal);
  g_state.server_allows = true;
  g_state.account_may = true;
  g_state.pin_set = true;
  ui::show(ui::Page::kCells, 0, 1);
  ui::open_pin();
  ui::type_pin("12");
  save(directory, language, "pin");
  ui::close_overlay();
  g_state.pin_session = true;
  ui::show(ui::Page::kCells, 0, 1);
  ui::ask(Operation::kResetCounters);
  save(directory, language, "confirm_reset");
  ui::close_overlay();
  g_state.pin_session = false;
  g_state.server_allows = false;
  g_state.account_may = false;

  // the server stops answering: what is shown is marked as old
  load(sim::Scenario::kNodeSilent);
  g_state.link = Link::kStale;
  ui::show(ui::Page::kSummary);
  save(directory, language, "old_data");

  g_state.link = Link::kOnline;
  load(sim::Scenario::kInverterFault);
  ui::show(ui::Page::kInverters);
  save(directory, language, "inverter_fault");
}
}  // namespace

int main(int argc, char** argv) {
  if (argc < 2) {
    std::fprintf(stderr, "usage: solar_screens OUTPUT_DIR [language ...]\n");
    return 2;
  }
  const std::string directory = argv[1];
  std::vector<std::string> languages;
  for (int i = 2; i < argc; ++i) languages.emplace_back(argv[i]);
  if (languages.empty())
    for (const std::string_view known : text::kLanguages) languages.emplace_back(known);

  lv_init();
  lv_tick_set_cb(tick_cb);
  lv_display_t* display = lv_display_create(kWidth, kHeight);
  lv_display_set_color_format(display, LV_COLOR_FORMAT_RGB565);
  lv_display_set_buffers(display, g_buffer, nullptr, sizeof g_buffer, LV_DISPLAY_RENDER_MODE_FULL);
  lv_display_set_flush_cb(display, flush_cb);

  if (!ui::start([] { return g_state; }, ui::Actions{})) {
    std::fprintf(stderr, "the screen could not be built\n");
    return 1;
  }
  for (const std::string& language : languages) take_language(directory, language);
  std::printf("SCREENS=%s languages=%zu\n", g_failures == 0 ? "PASS" : "FAIL", languages.size());
  return g_failures == 0 ? 0 : 1;
}
