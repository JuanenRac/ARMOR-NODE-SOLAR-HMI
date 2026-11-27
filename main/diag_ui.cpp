// ARMOR-NODE-SOLAR-HMI - the board check screen (LVGL 9, 1024 x 600).
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#include "diag_ui.hpp"

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <utility>

#include "lvgl.h"
#include "screen_text.hpp"

LV_FONT_DECLARE(solar_font_latin_s)
LV_FONT_DECLARE(solar_font_latin_m)
LV_FONT_DECLARE(solar_font_ja_s)
LV_FONT_DECLARE(solar_font_ja_m)
LV_FONT_DECLARE(solar_font_zh_s)
LV_FONT_DECLARE(solar_font_zh_m)

namespace armor::solarhmi::diag {
namespace {

constexpr int kRows = 8;
const char* const kKeys[kRows] = {"diag_chip", "diag_memory", "diag_screen", "diag_touch", "diag_i2c", "diag_ethernet", "diag_wifi", "diag_uptime"};

std::function<Info()> g_info;
std::function<void()> g_open;
std::size_t g_language = 0;
lv_obj_t* g_screen = nullptr;
lv_obj_t* g_previous = nullptr;
lv_obj_t* g_values[kRows] = {};
lv_obj_t* g_point = nullptr;
lv_obj_t* g_dot = nullptr;
lv_timer_t* g_timer = nullptr;
int g_max_x = 0, g_max_y = 0;

void build();

std::string_view language() { return text::kLanguages[g_language]; }
std::string word(const char* key) { return std::string(text::word(key, language())); }

const lv_font_t* small_font() { return language() == "ja" ? &solar_font_ja_s : language() == "zh" ? &solar_font_zh_s : &solar_font_latin_s; }
const lv_font_t* normal_font() { return language() == "ja" ? &solar_font_ja_m : language() == "zh" ? &solar_font_zh_m : &solar_font_latin_m; }

lv_obj_t* box(int x, int y, int w, int h, std::uint32_t colour, int radius = 8) {
  lv_obj_t* object = lv_obj_create(g_screen);
  lv_obj_remove_style_all(object);
  lv_obj_set_pos(object, x, y);
  lv_obj_set_size(object, w, h);
  lv_obj_set_style_bg_color(object, lv_color_hex(colour), 0);
  lv_obj_set_style_bg_opa(object, LV_OPA_COVER, 0);
  lv_obj_set_style_radius(object, radius, 0);
  lv_obj_remove_flag(object, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_remove_flag(object, LV_OBJ_FLAG_CLICKABLE);   // a touch anywhere else reaches the screen itself
  return object;
}

lv_obj_t* label(lv_obj_t* parent, int x, int y, int width, const std::string& value, const lv_font_t* font, std::uint32_t colour) {
  lv_obj_t* object = lv_label_create(parent);
  lv_obj_set_pos(object, x, y);
  lv_obj_set_width(object, width);
  lv_obj_set_style_text_font(object, font, 0);
  lv_obj_set_style_text_color(object, lv_color_hex(colour), 0);
  lv_label_set_long_mode(object, LV_LABEL_LONG_MODE_DOTS);
  lv_obj_set_height(object, lv_font_get_line_height(font));
  lv_label_set_text(object, value.c_str());
  return object;
}

lv_obj_t* button(int x, int y, int w, int h, const std::string& value, lv_event_cb_t callback) {
  lv_obj_t* object = box(x, y, w, h, 0x38BDF8);
  lv_obj_add_flag(object, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(object, callback, LV_EVENT_CLICKED, nullptr);
  lv_obj_t* caption = label(object, 6, (h - lv_font_get_line_height(normal_font())) / 2, w - 12, value, normal_font(), 0x0B1220);
  lv_obj_set_style_text_align(caption, LV_TEXT_ALIGN_CENTER, 0);
  return object;
}

void refresh() {
  if (!g_info) return;
  const Info info = g_info();
  const std::string* const values[kRows] = {&info.chip, &info.memory, &info.screen, &info.touch, &info.i2c, &info.ethernet, &info.wifi, &info.uptime};
  for (int i = 0; i < kRows; ++i) lv_label_set_text(g_values[i], values[i]->c_str());
}

void tick(lv_timer_t*) { refresh(); }

// Where the finger is, and how far it has ever got: a controller set up for another size never reaches the far corners.
void on_touch(lv_event_t*) {
  lv_indev_t* input = lv_indev_active();
  if (input == nullptr) return;
  lv_point_t point;
  lv_indev_get_point(input, &point);
  if (point.x > g_max_x) g_max_x = point.x;
  if (point.y > g_max_y) g_max_y = point.y;
  char value[64];
  std::snprintf(value, sizeof value, "x %d  y %d   (max %d, %d)", static_cast<int>(point.x), static_cast<int>(point.y), g_max_x, g_max_y);
  lv_label_set_text(g_point, value);
  lv_obj_set_pos(g_dot, point.x - 12, point.y - 12);
  lv_obj_remove_flag(g_dot, LV_OBJ_FLAG_HIDDEN);
}

void on_language(lv_event_t*) {
  g_language = (g_language + 1) % text::kLanguages.size();
  lv_async_call([](void*) { build(); }, nullptr);
}

void on_open(lv_event_t*) {
  lv_async_call(
      [](void*) {
        close();
        if (g_open) g_open();
      },
      nullptr);
}

void build() {
  if (g_screen == nullptr) return;   // closed before a change of language was drawn
  lv_obj_clean(g_screen);
  label(g_screen, 80, 14, 600, word("diag_title"), normal_font(), 0x38BDF8);
  label(g_screen, 700, 18, 240, std::string(language()), small_font(), 0x9FB0C7);
  for (int i = 0; i < kRows; ++i) {
    const int y = 60 + i * 40;
    box(20, y, 984, 36, 0x162033, 6);
    label(g_screen, 32, y + 8, 220, word(kKeys[i]), small_font(), 0x9FB0C7);
    g_values[i] = label(g_screen, 260, y + 8, 736, "-", &solar_font_latin_s, 0xF1F5F9);
  }
  // red, green, blue, white, always in this order: another order on the glass means the colour order or the byte order is wrong
  const std::uint32_t colours[] = {0xFF0000, 0x00FF00, 0x0000FF, 0xFFFFFF};
  for (int i = 0; i < 4; ++i) box(80 + i * 216, 388, 216, 40, colours[i], 0);
  label(g_screen, 80, 438, 864, word("diag_hint"), small_font(), 0x9FB0C7);
  g_point = label(g_screen, 80, 470, 864, "x -  y -", &solar_font_latin_s, 0xFACC15);
  button(80, 510, 420, 70, word("diag_open"), on_open);
  button(524, 510, 420, 70, std::string(language()) + "  >", on_language);
  // a mark in each corner of the glass
  for (int i = 0; i < 4; ++i) box(i % 2 == 0 ? 0 : 1024 - 48, i / 2 == 0 ? 0 : 600 - 48, 48, 48, 0xF59E0B, 0);
  g_dot = box(0, 0, 24, 24, 0xFACC15, 12);
  lv_obj_add_flag(g_dot, LV_OBJ_FLAG_HIDDEN);
  lv_obj_remove_flag(g_dot, LV_OBJ_FLAG_CLICKABLE);
  refresh();
}

}  // namespace

void close() {
  if (g_screen == nullptr) return;
  if (g_timer != nullptr) lv_timer_delete(g_timer);
  g_timer = nullptr;
  if (g_previous != nullptr) lv_screen_load(g_previous);
  lv_obj_delete(g_screen);
  g_screen = nullptr;
}

void show(std::function<Info()> info, std::function<void()> open, const std::string& asked) {
  g_info = std::move(info);
  g_open = std::move(open);
  g_language = 0;
  for (std::size_t i = 0; i < text::kLanguages.size(); ++i)
    if (text::kLanguages[i] == asked) g_language = i;
  g_max_x = g_max_y = 0;
  if (g_screen == nullptr) {
    g_previous = lv_screen_active();
    g_screen = lv_obj_create(nullptr);
    lv_obj_remove_flag(g_screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(g_screen, lv_color_hex(0x0B1220), 0);
    lv_obj_add_event_cb(g_screen, on_touch, LV_EVENT_PRESSING, nullptr);
  }
  build();
  lv_screen_load(g_screen);
  if (g_timer == nullptr) g_timer = lv_timer_create(tick, 500, nullptr);
}

}  // namespace armor::solarhmi::diag
