// ARMOR-NODE-SOLAR-HMI - the screen, 1024 x 600 (LVGL 9). A header with the state of the link, five pages chosen at the bottom, and an overlay for the PIN pad and the question before an
// operation. An alarm is said with an icon and a word as well as with colour, and only its icon blinks (core/solar_view.hpp keeps it at two blinks a second or fewer).
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#include "ui.hpp"

#include <cstdint>
#include <utility>
#include <vector>

#include "lvgl.h"
#include "screen_model.hpp"
#include "screen_text.hpp"

LV_FONT_DECLARE(solar_font_latin_s)
LV_FONT_DECLARE(solar_font_latin_m)
LV_FONT_DECLARE(solar_font_latin_l)
LV_FONT_DECLARE(solar_font_ja_s)
LV_FONT_DECLARE(solar_font_ja_m)
LV_FONT_DECLARE(solar_font_ja_l)
LV_FONT_DECLARE(solar_font_zh_s)
LV_FONT_DECLARE(solar_font_zh_m)
LV_FONT_DECLARE(solar_font_zh_l)

namespace armor::solarhmi::ui {
namespace {

constexpr int kWidth = 1024, kHeight = 600, kHeader = 56, kNav = 76;
constexpr int kContent = kHeight - kHeader - kNav;

constexpr std::uint32_t kBack = 0x0B1220, kPanel = 0x162033, kPanelHigh = 0x22304A, kInk = 0xF1F5F9, kSoft = 0x9FB0C7, kAccent = 0x38BDF8, kOk = 0x22C55E, kWarn = 0xF59E0B,
                        kAlarm = 0xEF4444, kSun = 0xFACC15;

enum class Size { kSmall, kNormal, kLarge };
enum class Overlay { kNone, kPin, kAsk, kNotice };

std::function<State()> g_source;
Actions g_actions;
State g_state;
Page g_page = Page::kSummary;
std::size_t g_battery = 0;
int g_module = 1;
Overlay g_overlay = Overlay::kNone;
Operation g_operation = Operation::kOther;
std::string g_digits;
const char* g_notice = "";
std::uint32_t g_drawn_generation = 0;
std::string g_drawn_signature;
std::vector<lv_obj_t*> g_blinking;
lv_obj_t* g_header = nullptr;
lv_obj_t* g_content = nullptr;
lv_obj_t* g_nav = nullptr;
lv_obj_t* g_layer = nullptr;

void rebuild();

const lv_font_t* font_of(Size size) {
  const std::string_view language = text::effective_language(g_state.language);
  const int index = static_cast<int>(size);
  static const lv_font_t* const latin[] = {&solar_font_latin_s, &solar_font_latin_m, &solar_font_latin_l};
  static const lv_font_t* const japanese[] = {&solar_font_ja_s, &solar_font_ja_m, &solar_font_ja_l};
  static const lv_font_t* const chinese[] = {&solar_font_zh_s, &solar_font_zh_m, &solar_font_zh_l};
  if (language == "ja") return japanese[index];
  if (language == "zh") return chinese[index];
  return latin[index];
}

std::string word(std::string_view key) { return std::string(text::word(key, text::effective_language(g_state.language))); }

std::uint32_t colour_of(Level level) { return level >= Level::kAlarm ? kAlarm : level == Level::kWarn ? kWarn : kOk; }
const char* symbol_of(Level level) { return level == Level::kFault ? LV_SYMBOL_CLOSE : level == Level::kAlarm ? LV_SYMBOL_BELL : level == Level::kWarn ? LV_SYMBOL_WARNING : LV_SYMBOL_OK; }

// ---- the building blocks ----
lv_obj_t* box(lv_obj_t* parent, int x, int y, int w, int h, std::uint32_t colour, int radius = 12) {
  lv_obj_t* object = lv_obj_create(parent);
  lv_obj_remove_style_all(object);
  lv_obj_set_pos(object, x, y);
  lv_obj_set_size(object, w, h);
  lv_obj_set_style_bg_color(object, lv_color_hex(colour), 0);
  lv_obj_set_style_bg_opa(object, LV_OPA_COVER, 0);
  lv_obj_set_style_radius(object, radius, 0);
  lv_obj_remove_flag(object, LV_OBJ_FLAG_SCROLLABLE);
  return object;
}

lv_obj_t* label(lv_obj_t* parent, int x, int y, int width, const std::string& text, Size size = Size::kNormal, std::uint32_t colour = kInk, lv_text_align_t align = LV_TEXT_ALIGN_LEFT) {
  lv_obj_t* object = lv_label_create(parent);
  lv_obj_set_pos(object, x, y);
  lv_obj_set_width(object, width);
  lv_obj_set_style_text_font(object, font_of(size), 0);
  lv_obj_set_style_text_color(object, lv_color_hex(colour), 0);
  lv_obj_set_style_text_align(object, align, 0);
  lv_label_set_long_mode(object, LV_LABEL_LONG_MODE_DOTS);
  lv_obj_set_height(object, lv_font_get_line_height(font_of(size)));
  lv_label_set_text(object, text.c_str());
  return object;
}

lv_obj_t* paragraph(lv_obj_t* parent, int x, int y, int width, const std::string& text, Size size, std::uint32_t colour) {
  lv_obj_t* object = label(parent, x, y, width, "", size, colour, LV_TEXT_ALIGN_CENTER);
  lv_label_set_long_mode(object, LV_LABEL_LONG_MODE_WRAP);
  lv_obj_set_height(object, LV_SIZE_CONTENT);
  lv_label_set_text(object, text.c_str());
  return object;
}

// The mark of a level: its icon (the one thing that blinks, and only for an alarm) and its word.
void mark(lv_obj_t* parent, int x, int y, int width, Level level, bool with_word = true) {
  lv_obj_t* icon = lv_label_create(parent);
  lv_obj_set_pos(icon, x, y);
  lv_obj_set_style_text_font(icon, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(icon, lv_color_hex(colour_of(level)), 0);
  lv_label_set_text(icon, symbol_of(level));
  if (level == Level::kAlarm) g_blinking.push_back(icon);
  if (with_word) label(parent, x + 34, y + 2, width - 34, word(mark_of(level).word), Size::kSmall, colour_of(level));
}

lv_obj_t* button(lv_obj_t* parent, int x, int y, int w, int h, const std::string& text, std::uint32_t colour, lv_event_cb_t callback, std::intptr_t data, Size size = Size::kNormal,
                 std::uint32_t ink = kInk) {
  lv_obj_t* object = box(parent, x, y, w, h, colour);
  lv_obj_add_flag(object, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(object, callback, LV_EVENT_CLICKED, reinterpret_cast<void*>(data));
  lv_obj_set_style_bg_opa(object, LV_OPA_70, LV_STATE_PRESSED);
  const int line = lv_font_get_line_height(font_of(size));
  label(object, 6, (h - line) / 2, w - 12, text, size, ink, LV_TEXT_ALIGN_CENTER);
  return object;
}

void later() { lv_async_call([](void*) { rebuild(); }, nullptr); }
void touched() {
  if (g_actions.touched) g_actions.touched();
}

// ---- events ----
void on_nav(lv_event_t* event) {
  touched();
  g_page = static_cast<Page>(reinterpret_cast<std::intptr_t>(lv_event_get_user_data(event)));
  later();
}

void on_module(lv_event_t* event) {
  touched();
  const auto data = reinterpret_cast<std::intptr_t>(lv_event_get_user_data(event));
  g_battery = static_cast<std::size_t>(data / 100);
  g_module = static_cast<int>(data % 100);
  g_page = Page::kCells;
  later();
}

WriteContext write_context(Operation operation) {
  WriteContext context;
  context.operation = operation;
  context.server_allows = g_state.server_allows;
  context.account_may = g_state.account_may;
  context.link = g_state.link;
  const bool known = g_battery < g_state.snapshot.batteries.size();
  context.device_stale = !known || g_state.snapshot.batteries[g_battery].stale;
  context.example = known && g_state.snapshot.batteries[g_battery].example;
  context.pin_session = g_state.pin_session;
  return context;
}

void on_operation(lv_event_t* event) {
  touched();
  const auto operation = static_cast<Operation>(reinterpret_cast<std::intptr_t>(lv_event_get_user_data(event)));
  const WriteBlock block = first_block(write_context(operation));
  g_operation = operation;
  if (block == WriteBlock::kNoPin) {
    g_digits.clear();
    g_notice = "";
    g_overlay = Overlay::kPin;
  } else if (block == WriteBlock::kNotConfirmed) {
    g_overlay = Overlay::kAsk;
  } else {
    g_notice = block_key(block);
    g_overlay = Overlay::kNotice;
  }
  later();
}

void on_key(lv_event_t* event) {
  touched();
  const auto key = static_cast<int>(reinterpret_cast<std::intptr_t>(lv_event_get_user_data(event)));
  if (key >= 0 && key <= 9) {
    if (g_digits.size() < PinPolicy{}.max_digits) g_digits.push_back(static_cast<char>('0' + key));
  } else if (key == 10) {
    g_digits.clear();
  } else if (key == 12) {
    g_digits.clear();
    g_overlay = Overlay::kNone;
  } else {
    const PinResult result = g_actions.unlock ? g_actions.unlock(g_digits) : PinResult::kNotSet;
    g_digits.clear();
    if (result == PinResult::kOk) {
      g_overlay = Overlay::kNone;
    } else {
      g_notice = result == PinResult::kLocked ? "pin_locked" : result == PinResult::kNotSet ? "pin_none" : "pin_wrong";
    }
  }
  later();
}

void on_answer(lv_event_t* event) {
  touched();
  const bool yes = reinterpret_cast<std::intptr_t>(lv_event_get_user_data(event)) == 1;
  g_overlay = Overlay::kNone;
  if (yes && g_battery < g_state.snapshot.batteries.size()) {
    WriteContext context = write_context(g_operation);
    context.confirmed = true;
    const WriteBlock block = first_block(context);   // asked again: the answer may have come in while the question was open
    if (block == WriteBlock::kNone) {
      const Battery& battery = g_state.snapshot.batteries[g_battery];
      if (g_actions.confirmed) g_actions.confirmed(g_operation, battery.node, battery.device, g_module);
      g_notice = "order_not_sent";
    } else {
      g_notice = block_key(block);
    }
    g_overlay = Overlay::kNotice;
  }
  later();
}

void on_close(lv_event_t*) {
  touched();
  g_overlay = Overlay::kNone;
  later();
}

void on_lock(lv_event_t*) {
  if (g_actions.lock) g_actions.lock();
  later();
}

void on_diagnostics(lv_event_t*) {
  touched();
  if (g_actions.diagnostics) g_actions.diagnostics();
}

void on_back(lv_event_t*) {
  touched();
  g_page = Page::kBatteries;
  later();
}

// ---- the pages ----
void tile(lv_obj_t* parent, int x, int y, int w, int h, const std::string& title, const std::string& value, std::uint32_t colour, const std::string& note = "") {
  lv_obj_t* card = box(parent, x, y, w, h, kPanel);
  label(card, 16, 12, w - 32, title, Size::kSmall, kSoft);
  label(card, 16, 38, w - 32, value, Size::kLarge, colour);
  if (!note.empty()) label(card, 16, h - 30, w - 32, note, Size::kSmall, kSoft);
}

Level inverter_level(const Inverter& inverter) {
  if (!inverter.stale && inverter.mode == "fault") return Level::kAlarm;
  return inverter.stale || !inverter.warnings.empty() ? Level::kWarn : Level::kOk;
}

void page_summary() {
  const Totals& totals = g_state.snapshot.totals;
  tile(g_content, 12, 8, 328, 118, word("soc"), format_percent(totals.soc_percent), totals.soc_percent && *totals.soc_percent <= g_state.limits.soc_low_percent ? kWarn : kOk,
       word("stored_energy") + ": " + format_value(totals.energy_kwh, 2, "kWh"));
  tile(g_content, 348, 8, 218, 118, word("solar_power"), format_power(totals.pv_w), kSun);
  tile(g_content, 574, 8, 218, 118, word("load_power"), format_power(totals.load_w), kInk);
  tile(g_content, 800, 8, 212, 118, word("battery_power"), format_power(totals.battery_w), kAccent, word(flow_key(totals.battery_w)));

  int y = 136;
  const auto row = [&y](const std::string& name, const std::string& what, const std::string& value, Level level) {
    if (y + 52 > kContent) return;
    lv_obj_t* line = box(g_content, 12, y, 1000, 48, kPanel, 8);
    mark(line, 14, 11, 200, level);
    label(line, 220, 11, 300, name, Size::kSmall);
    label(line, 530, 11, 250, what, Size::kSmall, kSoft);
    label(line, 786, 11, 200, value, Size::kSmall, kInk, LV_TEXT_ALIGN_RIGHT);
    y += 54;
  };
  for (const Battery& battery : g_state.snapshot.batteries)
    row(battery.node + " / " + battery.device, battery.stale ? word("device_old") : word("stack") + ": " + format_value(battery.voltage_v, 2, "V"), format_percent(battery.soc_percent),
        battery_level(battery, g_state.limits));
  for (const Inverter& inverter : g_state.snapshot.inverters)
    row(inverter.node + " / " + inverter.device, inverter.stale ? word("device_old") : word(mode_key(inverter.mode)), format_power(inverter.pv_w), inverter_level(inverter));
  if (g_state.snapshot.skipped > 0 && y + 30 <= kContent) label(g_content, 16, y, 980, word("not_shown"), Size::kSmall, kWarn);
  if (g_state.snapshot.batteries.empty() && g_state.snapshot.inverters.empty())
    paragraph(g_content, 112, 220, 800, word(g_state.link == Link::kNever ? "waiting" : "no_devices"), Size::kNormal, kSoft);
}

void page_batteries() {
  lv_obj_add_flag(g_content, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_scroll_dir(g_content, LV_DIR_VER);
  int cell = 0;
  for (std::size_t b = 0; b < g_state.snapshot.batteries.size(); ++b) {
    const Battery& battery = g_state.snapshot.batteries[b];
    for (const Module& module : battery.stack) {
      const int x = 12 + (cell % 4) * 252, y = 8 + (cell / 4) * 222;
      ++cell;
      const Level level = battery.stale ? Level::kWarn : module_level(module, g_state.limits);
      lv_obj_t* card = box(g_content, x, y, 244, 214, kPanel);
      lv_obj_set_style_border_color(card, lv_color_hex(colour_of(level)), 0);
      lv_obj_set_style_border_width(card, level == Level::kOk ? 0 : 3, 0);
      lv_obj_add_flag(card, LV_OBJ_FLAG_CLICKABLE);
      lv_obj_add_event_cb(card, on_module, LV_EVENT_CLICKED, reinterpret_cast<void*>(static_cast<std::intptr_t>(b * 100 + static_cast<std::size_t>(module.n))));
      label(card, 14, 8, 216, word("module") + " " + std::to_string(module.n), Size::kNormal);
      const std::string maker = maker_text(module.maker.name.empty() ? stack_maker(battery) : module.maker, word("deduced"));
      label(card, 14, 40, 216, maker.empty() ? battery.device : battery.device + " - " + maker, Size::kSmall, kSoft);
      label(card, 14, 64, 216, format_percent(module.soc_percent), Size::kLarge, colour_of(level));
      label(card, 14, 118, 216, format_value(module.voltage_v, 2, "V") + "   " + format_value(module.temperature_c, 1, "°C"), Size::kSmall);
      const std::optional<CellStats> stats = cell_stats(module.cells_v, module.cells_unread);
      label(card, 14, 146, 216, word("spread") + ": " + (stats ? std::to_string(stats->delta_mv) + " mV" : std::string("--")), Size::kSmall, kSoft);
      mark(card, 14, 176, 216, level);
    }
  }
  if (cell == 0) paragraph(g_content, 112, 180, 800, word(g_state.link == Link::kNever ? "waiting" : "no_devices"), Size::kNormal, kSoft);
}

void page_cells() {
  if (g_battery >= g_state.snapshot.batteries.size()) return page_batteries();
  const Battery& battery = g_state.snapshot.batteries[g_battery];
  const Module* module = nullptr;
  for (const Module& candidate : battery.stack)
    if (candidate.n == g_module) module = &candidate;
  if (module == nullptr) return page_batteries();

  const Level level = battery.stale ? Level::kWarn : module_level(*module, g_state.limits);
  button(g_content, 12, 8, 130, 52, word("back"), kPanelHigh, on_back, 0);
  const std::string maker = maker_text(module->maker.name.empty() ? stack_maker(battery) : module->maker, word("deduced"));
  label(g_content, 160, 8, 460, word("module") + " " + std::to_string(module->n) + " - " + battery.node + " / " + battery.device, Size::kNormal);
  const std::string health = battery.stack_health_percent ? word("stack_health") + " " + format_percent(battery.stack_health_percent) : std::string();
  label(g_content, 160, 40, 460, maker.empty() ? health : health.empty() ? maker : maker + " - " + health, Size::kSmall, kSoft);
  mark(g_content, 640, 22, 170, level);
  label(g_content, 820, 22, 192, format_value(module->temperature_c, 1, "°C") + "  " + format_percent(module->soc_percent), Size::kSmall, kInk, LV_TEXT_ALIGN_RIGHT);

  const std::optional<CellStats> stats = cell_stats(module->cells_v, module->cells_unread);
  lv_obj_t* chart = box(g_content, 12, 68, 1000, 292, kPanel);
  if (!stats) {
    paragraph(chart, 100, 130, 800, word("no_cells"), Size::kNormal, kSoft);
  } else {
    const int count = static_cast<int>(module->cells_v.size());
    const int step = 984 / (count < 15 ? 15 : count);
    for (int c = 0; c < count; ++c) {
      const double volts = module->cells_v[static_cast<std::size_t>(c)];
      const bool unread = is_unread(module->cells_unread, static_cast<std::size_t>(c));
      Level own = unread ? Level::kFault : cell_level(volts, g_state.limits);
      for (const BatteryState& state : module->states)
        if (state.cell == c) own = worst(own, state_level(state));
      const int x = 8 + c * step, full = 206, high = unread ? 0 : 6 + static_cast<int>(cell_bar(volts) * (full - 6));
      label(chart, x, 6, step, std::to_string(c + 1), Size::kSmall, kSoft, LV_TEXT_ALIGN_CENTER);
      box(chart, x + 8, 32, step - 16, full, kPanelHigh, 4);
      if (high > 0) box(chart, x + 8, 32 + full - high, step - 16, high, own >= Level::kAlarm ? kAlarm : own == Level::kWarn ? kWarn : kAccent, 4);
      if (own == Level::kFault) mark(chart, x + step / 2 - 12, 40, 40, own, false);
      if (own == Level::kAlarm) mark(chart, x + step / 2 - 12, 40, 40, own, false);
      // three decimals do not fit under sixteen bars, so the figure is written in millivolts when the bars are narrow
      label(chart, x, 244, step, unread ? std::string("--") : step >= 62 ? format_number(volts, 3) : format_number(volts * 1000.0, 0), Size::kSmall, own >= Level::kAlarm ? kAlarm : kInk, LV_TEXT_ALIGN_CENTER);
    }
    label(chart, 16, 266, 968,
          word("min") + " " + format_number(stats->min_v, 3) + " V (" + std::to_string(stats->min_index + 1) + ")     " + word("max") + " " + format_number(stats->max_v, 3) + " V (" +
              std::to_string(stats->max_index + 1) + ")     " + word("spread") + " " + std::to_string(stats->delta_mv) + " mV",
          Size::kSmall, kSoft, LV_TEXT_ALIGN_CENTER);
  }
  button(g_content, 12, 374, 300, 70, word("check"), kPanelHigh, on_operation, static_cast<std::intptr_t>(Operation::kCheck));
  button(g_content, 326, 374, 300, 70, word("reset_counters"), kPanelHigh, on_operation, static_cast<std::intptr_t>(Operation::kResetCounters));
  const WriteBlock block = first_block(write_context(Operation::kResetCounters));
  if (block != WriteBlock::kNotConfirmed) paragraph(g_content, 640, 380, 372, word(block_key(block)), Size::kSmall, kSoft);
  if (g_state.pin_session) button(g_content, 660, 374, 352, 70, word("lock_now"), kPanelHigh, on_lock, 0);
}

void page_inverters() {
  lv_obj_add_flag(g_content, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_scroll_dir(g_content, LV_DIR_VER);
  int y = 8;
  for (const Inverter& inverter : g_state.snapshot.inverters) {
    lv_obj_t* card = box(g_content, 12, y, 1000, 330, kPanel);
    const std::string maker = maker_text(inverter.maker, word("deduced"));
    label(card, 16, 12, 760, inverter.node + " / " + inverter.device + (maker.empty() ? "" : " - " + maker), Size::kNormal);
    mark(card, 800, 16, 190, inverter_level(inverter));
    label(card, 16, 48, 970, word("mode") + ": " + (inverter.stale ? word("device_old") : word(mode_key(inverter.mode))), Size::kSmall, inverter.mode == "fault" ? kAlarm : kSoft);
    tile(card, 16, 80, 236, 104, word("grid"), format_value(inverter.grid_v, 0, "V"), kInk);
    tile(card, 260, 80, 236, 104, word("output"), format_power(inverter.out_w), kInk);
    tile(card, 504, 80, 236, 104, word("pv"), format_power(inverter.pv_w), kSun);
    tile(card, 748, 80, 236, 104, word("battery_power"), format_value(inverter.battery_v, 1, "V"), kAccent);
    label(card, 16, 198, 320, word("load_percent") + ": " + format_percent(inverter.load_percent), Size::kSmall);
    label(card, 340, 198, 320, word("current") + ": " + format_value(inverter.battery_a, 1, "A"), Size::kSmall);
    label(card, 664, 198, 320, word("heatsink") + ": " + format_value(inverter.heatsink_c, 0, "°C"), Size::kSmall);
    std::string warnings;
    for (const std::string& warning : inverter.warnings) warnings += (warnings.empty() ? "" : ", ") + warning;
    label(card, 16, 234, 970, word("warnings") + ": " + (warnings.empty() ? word("none") : warnings), Size::kSmall, warnings.empty() ? kSoft : kWarn);
    // what the inverter counts itself, only when it said it
    const auto counted = [](const std::string& name, const Reading& today, const Reading& total) {
      return name + ": " + word("energy_today") + " " + format_energy(today) + ", " + word("energy_total") + " " + format_energy(total);
    };
    if (inverter.pv_today_wh || inverter.pv_total_wh) label(card, 16, 262, 970, counted(word("solar_power"), inverter.pv_today_wh, inverter.pv_total_wh), Size::kSmall, kSun);
    if (inverter.load_today_wh || inverter.load_total_wh) label(card, 16, 290, 970, counted(word("load_power"), inverter.load_today_wh, inverter.load_total_wh), Size::kSmall, kSoft);
    y += 340;
  }
  if (g_state.snapshot.inverters.empty()) paragraph(g_content, 112, 180, 800, word(g_state.link == Link::kNever ? "waiting" : "no_devices"), Size::kNormal, kSoft);
}

void page_alarms() {
  lv_obj_add_flag(g_content, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_scroll_dir(g_content, LV_DIR_VER);
  const std::vector<AlarmItem> alarms = alarms_of(g_state.snapshot, g_state.limits);
  int y = 8;
  for (const AlarmItem& item : alarms) {
    if (y > 40 * 56) break;
    lv_obj_t* line = box(g_content, 12, y, 1000, 52, kPanel, 8);
    mark(line, 14, 13, 170, item.level);
    label(line, 196, 13, 400, word(item.key), Size::kSmall);
    std::string where = item.device;
    if (item.module > 0) where += " - " + word("module") + " " + std::to_string(item.module);
    if (item.cell > 0) where += " (" + std::to_string(item.cell) + ")";
    label(line, 606, 4, 380, where, Size::kSmall, kSoft, LV_TEXT_ALIGN_RIGHT);
    label(line, 606, 26, 380, item.detail, Size::kSmall, colour_of(item.level), LV_TEXT_ALIGN_RIGHT);
    y += 58;
  }
  if (alarms.empty()) paragraph(g_content, 112, 180, 800, word(g_state.link == Link::kNever ? "waiting" : "all_quiet"), Size::kNormal, kSoft);
}

void page_settings() {
  lv_obj_t* card = box(g_content, 12, 8, 1000, 240, kPanel);
  label(card, 20, 20, 300, word("language"), Size::kNormal, kSoft);
  label(card, 330, 20, 640, std::string(text::effective_language(g_state.language)), Size::kNormal);
  label(card, 20, 70, 300, word("address"), Size::kNormal, kSoft);
  label(card, 330, 70, 640, g_state.address.empty() ? word("no_address") : g_state.address, Size::kNormal);
  label(card, 20, 120, 300, word("version"), Size::kNormal, kSoft);
  label(card, 330, 120, 640, g_state.version, Size::kNormal);
  label(card, 20, 180, 960, g_state.pin_set ? (g_state.pin_session ? "PIN: " + word("level_ok") : "PIN: " + word("block_no_pin")) : word("pin_none"), Size::kSmall, kSoft);
  if (g_actions.diagnostics) button(g_content, 12, 262, 340, 72, word("diag_button"), kPanelHigh, on_diagnostics, 0);
}

void draw_overlay() {
  lv_obj_clean(g_layer);
  if (g_overlay == Overlay::kNone) {
    lv_obj_add_flag(g_layer, LV_OBJ_FLAG_HIDDEN);
    return;
  }
  lv_obj_remove_flag(g_layer, LV_OBJ_FLAG_HIDDEN);
  if (g_overlay == Overlay::kPin) {
    lv_obj_t* card = box(g_layer, 262, 20, 500, 560, kPanel);
    label(card, 20, 14, 460, word("pin_title"), Size::kNormal, kInk, LV_TEXT_ALIGN_CENTER);
    // the digits are never shown, only how many there are
    label(card, 20, 52, 460, std::string(g_digits.size(), '*'), Size::kLarge, kAccent, LV_TEXT_ALIGN_CENTER);
    if (g_notice[0] != '\0') label(card, 20, 106, 460, word(g_notice), Size::kSmall, kAlarm, LV_TEXT_ALIGN_CENTER);
    for (int key = 1; key <= 9; ++key) button(card, 40 + ((key - 1) % 3) * 144, 136 + ((key - 1) / 3) * 86, 132, 76, std::to_string(key), kPanelHigh, on_key, key, Size::kNormal);
    button(card, 40, 394, 132, 76, word("clear"), kPanelHigh, on_key, 10, Size::kSmall);
    button(card, 184, 394, 132, 76, "0", kPanelHigh, on_key, 0);
    button(card, 328, 394, 132, 76, word("ok"), kAccent, on_key, 11, Size::kSmall, kBack);
    button(card, 40, 480, 420, 66, word("cancel"), kPanelHigh, on_key, 12);
    return;
  }
  lv_obj_t* card = box(g_layer, 162, 130, 700, 330, kPanel);
  if (g_overlay == Overlay::kAsk) {
    const bool reset = g_operation == Operation::kResetCounters;
    label(card, 20, 18, 660, word("module") + " " + std::to_string(g_module) + (g_battery < g_state.snapshot.batteries.size() ? " - " + g_state.snapshot.batteries[g_battery].device : std::string()),
          Size::kNormal, kSoft, LV_TEXT_ALIGN_CENTER);
    paragraph(card, 30, 72, 640, word(reset ? "confirm_reset" : "confirm_check"), Size::kNormal, reset ? kWarn : kInk);
    button(card, 40, 230, 300, 76, word("no"), kPanelHigh, on_answer, 0);
    button(card, 360, 230, 300, 76, word("yes"), reset ? kAlarm : kAccent, on_answer, 1, Size::kNormal, reset ? kInk : kBack);
  } else {
    paragraph(card, 30, 90, 640, word(g_notice), Size::kNormal, kInk);
    button(card, 200, 230, 300, 76, word("back"), kPanelHigh, on_close, 0);
  }
}

void draw_header() {
  lv_obj_clean(g_header);
  label(g_header, 16, 14, 300, "A.R.M.O.R. Solar", Size::kNormal, kAccent);
  const bool online = g_state.link == Link::kOnline;
  if (g_state.link == Link::kStale) {
    lv_obj_t* banner = box(g_header, 330, 10, 420, 36, kWarn, 8);
    label(banner, 8, 6, 404, word("old_data"), Size::kSmall, kBack, LV_TEXT_ALIGN_CENTER);
  }
  const Level level = online ? Level::kOk : Level::kWarn;
  mark(g_header, 780, 16, 30, level, false);
  label(g_header, 816, 17, 196, word(online ? "link_online" : g_state.link == Link::kStale ? "link_stale" : "link_never"), Size::kSmall, colour_of(level));
}

void draw_nav() {
  lv_obj_clean(g_nav);
  static const char* const keys[] = {"nav_summary", "nav_batteries", "nav_inverters", "nav_alarms", "nav_settings"};
  static const Page pages[] = {Page::kSummary, Page::kBatteries, Page::kInverters, Page::kAlarms, Page::kSettings};
  const std::vector<AlarmItem> alarms = alarms_of(g_state.snapshot, g_state.limits);
  for (int i = 0; i < 5; ++i) {
    const bool here = g_page == pages[i] || (g_page == Page::kCells && pages[i] == Page::kBatteries);
    lv_obj_t* tab = button(g_nav, 8 + i * 203, 8, 196, 60, word(keys[i]), here ? kAccent : kPanel, on_nav, static_cast<std::intptr_t>(pages[i]), Size::kSmall, here ? kBack : kInk);
    if (pages[i] == Page::kAlarms && !alarms.empty()) mark(tab, 6, 4, 30, alarms.front().level, false);
  }
}

void rebuild() {
  if (g_content == nullptr) return;
  g_blinking.clear();
  lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(kBack), 0);
  draw_header();
  lv_obj_clean(g_content);
  lv_obj_scroll_to_y(g_content, 0, LV_ANIM_OFF);
  lv_obj_remove_flag(g_content, LV_OBJ_FLAG_SCROLLABLE);
  switch (g_page) {
    case Page::kSummary: page_summary(); break;
    case Page::kBatteries: page_batteries(); break;
    case Page::kCells: page_cells(); break;
    case Page::kInverters: page_inverters(); break;
    case Page::kAlarms: page_alarms(); break;
    case Page::kSettings: page_settings(); break;
  }
  draw_nav();
  draw_overlay();
}

std::string signature(const State& state) {
  return state.language + '|' + state.address + '|' + std::to_string(static_cast<int>(state.link)) + std::to_string(state.server_allows) + std::to_string(state.account_may) +
         std::to_string(state.pin_set) + std::to_string(state.pin_session) + '|' + std::to_string(state.pin_locked_s > 0);
}

void tick(lv_timer_t*) {
  if (!g_source) return;
  g_state = g_source();
  const std::string now = signature(g_state);
  // the page is drawn again only when something changed, so that a finger on a button does not lose it
  if (now != g_drawn_signature || g_state.generation != g_drawn_generation) {
    if (!g_state.pin_session && g_overlay == Overlay::kAsk) g_overlay = Overlay::kNone;   // the session ended while the question was open
    g_drawn_signature = now;
    g_drawn_generation = g_state.generation;
    rebuild();
  }
}

void blink(lv_timer_t*) {
  const bool on = lit(Level::kAlarm, static_cast<std::int64_t>(lv_tick_get()));
  for (lv_obj_t* object : g_blinking) lv_obj_set_style_opa(object, on ? LV_OPA_COVER : LV_OPA_20, 0);
}

}  // namespace

bool start(std::function<State()> state, Actions actions) {
  if (lv_screen_active() == nullptr || !state) return false;
  g_source = std::move(state);
  g_actions = std::move(actions);
  g_state = g_source();
  lv_obj_t* screen = lv_screen_active();
  lv_obj_remove_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(screen, lv_color_hex(kBack), 0);
  g_header = box(screen, 0, 0, kWidth, kHeader, kBack, 0);
  g_content = box(screen, 0, kHeader, kWidth, kContent, kBack, 0);
  g_nav = box(screen, 0, kHeader + kContent, kWidth, kNav, kBack, 0);
  g_layer = box(screen, 0, 0, kWidth, kHeight, 0x000000, 0);
  lv_obj_set_style_bg_opa(g_layer, LV_OPA_80, 0);
  lv_obj_add_flag(g_layer, LV_OBJ_FLAG_CLICKABLE);   // nothing behind an overlay can be touched
  g_drawn_signature = signature(g_state);
  g_drawn_generation = g_state.generation;
  rebuild();
  lv_timer_create(tick, 500, nullptr);
  lv_timer_create(blink, 100, nullptr);
  return true;
}

void refresh() {
  if (!g_source) return;
  g_state = g_source();
  g_drawn_signature = signature(g_state);
  g_drawn_generation = g_state.generation;
  rebuild();
}

void show(Page page, std::size_t battery, int module) {
  g_page = page;
  g_battery = battery;
  g_module = module;
  g_overlay = Overlay::kNone;
  refresh();
}

void open_pin() {
  g_digits.clear();
  g_notice = "";
  g_overlay = Overlay::kPin;
  refresh();
}

void type_pin(const std::string& digits) {
  g_digits = digits;
  refresh();
}

void ask(Operation operation) {
  g_operation = operation;
  g_overlay = Overlay::kAsk;
  refresh();
}

void close_overlay() {
  g_overlay = Overlay::kNone;
  refresh();
}

}  // namespace armor::solarhmi::ui
