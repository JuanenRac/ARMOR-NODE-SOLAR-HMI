// ARMOR-NODE-SOLAR-HMI - what the pages of the screen list, worked out from one answer of the server: the alarms to show and how numbers are written. Nothing here draws.
// A value that is missing is written "--", never as a zero.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#pragma once
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <string>
#include <vector>

#include "solar_data.hpp"
#include "solar_view.hpp"
#include "write_gate.hpp"

namespace armor::solarhmi {

struct AlarmItem {
  Level level = Level::kWarn;
  const char* key = "";     // a word of the table of the screen
  std::string device;       // "node/device"
  int module = 0;           // 0: the whole device
  int cell = 0;             // 0: not a cell
  std::string detail;       // a number or the text the equipment sent
};

inline std::string format_number(double value, int places) {
  char text[32];
  std::snprintf(text, sizeof text, "%.*f", places, value);
  return text;
}

// "--" for a missing value; watts above a thousand are written in kilowatts.
inline std::string format_power(const Reading& watts) {
  if (!watts) return "--";
  if (std::fabs(*watts) >= 1000.0) return format_number(*watts / 1000.0, 2) + " kW";
  return format_number(*watts, 0) + " W";
}

inline std::string format_value(const Reading& value, int places, const char* unit) {
  if (!value) return "--";
  return format_number(*value, places) + " " + unit;
}

inline std::string format_percent(const Reading& value) { return value ? format_number(*value, 0) + " %" : std::string("--"); }

// "Pylon", or "Daly (deduced)" with the word given; empty when the maker is not known.
inline std::string maker_text(const Maker& maker, const std::string& deduced_word) {
  if (maker.name.empty()) return "";
  return maker.deduced ? maker.name + " (" + deduced_word + ")" : maker.name;
}

// The maker of a stack: its own, or that of its first module that says one.
inline Maker stack_maker(const Battery& battery) {
  if (!battery.maker.name.empty()) return battery.maker;
  for (const Module& module : battery.stack)
    if (!module.maker.name.empty()) return module.maker;
  return Maker{};
}

// The key of the word for the mode of an inverter; a mode the screen does not know says "unknown".
inline std::string mode_key(const std::string& mode) {
  for (const char* known : {"power_on", "standby", "line", "battery", "fault", "power_saving", "shutdown"})
    if (mode == known) return "mode_" + mode;
  return "mode_unknown";
}

inline const char* flow_key(const Reading& watts) {
  if (!watts) return "idle";
  if (*watts > 20.0) return "charging";
  if (*watts < -20.0) return "discharging";
  return "idle";
}

inline const char* block_key(WriteBlock block) {
  switch (block) {
    case WriteBlock::kNotFromScreen: return "block_not_from_screen";
    case WriteBlock::kServerOff: return "block_server_off";
    case WriteBlock::kNoPermission: return "block_no_permission";
    case WriteBlock::kNoServer: return "block_no_server";
    case WriteBlock::kOldData: return "block_old_data";
    case WriteBlock::kExample: return "block_example";
    case WriteBlock::kNoPin: return "block_no_pin";
    default: return "";
  }
}

namespace detail {

inline void add_module_alarms(std::vector<AlarmItem>& out, const std::string& device, const Module& module, const Thresholds& limits) {
  if (!module.present) {
    out.push_back({Level::kWarn, "module_missing", device, module.n, 0, ""});
    return;
  }
  // what the battery itself says: the cells its electronics do not read, then each state that is not normal
  for (const int cell : module.cells_unread) out.push_back({Level::kFault, "bms_fault", device, module.n, cell + 1, "0 V"});
  for (const BatteryState& state : module.states) {
    const Level level = state_level(state);
    out.push_back({level, level == Level::kFault ? "bms_fault" : level == Level::kAlarm ? "bms_protect" : "bms_notice", device, module.n, state.cell + 1, state.column + " " + state.word});
  }
  if (!module.left_out.empty()) {
    std::string names;
    for (const std::string& name : module.left_out) names += (names.empty() ? "" : ", ") + name;
    out.push_back({Level::kWarn, "left_out", device, module.n, 0, names});
  }
  for (std::size_t c = 0; c < module.cells_v.size(); ++c) {
    const double volts = module.cells_v[c];
    if (is_unread(module.cells_unread, c) || cell_level(volts, limits) != Level::kAlarm) continue;
    out.push_back({Level::kAlarm, volts >= limits.cell_high_v ? "cell_high" : "cell_low", device, module.n, static_cast<int>(c) + 1, format_number(volts, 3) + " V"});
  }
  const std::optional<CellStats> stats = cell_stats(module.cells_v, module.cells_unread);
  if (stats && stats->delta_mv >= limits.spread_warn_mv) out.push_back({Level::kWarn, "spread_high", device, module.n, 0, format_number(stats->delta_mv, 0) + " mV"});
  if (module.temperature_c && (*module.temperature_c >= limits.temp_high_c || *module.temperature_c <= limits.temp_low_c))
    out.push_back({Level::kAlarm, "temp_out", device, module.n, 0, format_number(*module.temperature_c, 1) + " °C"});
}

}  // namespace detail

// The alarms of one answer, the worst first and otherwise in the order of the equipment. A silent device says only that it is silent: its old figures raise nothing new.
inline std::vector<AlarmItem> alarms_of(const Snapshot& snapshot, const Thresholds& limits) {
  std::vector<AlarmItem> out;
  for (const Battery& battery : snapshot.batteries) {
    const std::string device = battery.node + "/" + battery.device;
    if (battery.stale) {
      out.push_back({Level::kWarn, "device_old", device, 0, 0, ""});
      continue;
    }
    const std::size_t before = out.size();
    for (const Module& module : battery.stack) detail::add_module_alarms(out, device, module, limits);
    if (battery.soc_percent && *battery.soc_percent <= limits.soc_low_percent) out.push_back({Level::kWarn, "charge_low", device, 0, 0, format_percent(battery.soc_percent)});
    // the battery's own alarm is listed when nothing above already says what is wrong
    const bool said = std::any_of(out.begin() + static_cast<std::ptrdiff_t>(before), out.end(), [](const AlarmItem& item) { return item.level >= Level::kAlarm; });
    if ((battery.alarm || battery.protecting) && !said) out.push_back({Level::kAlarm, "battery_alarm", device, 0, 0, ""});
  }
  for (const Inverter& inverter : snapshot.inverters) {
    const std::string device = inverter.node + "/" + inverter.device;
    if (inverter.stale) {
      out.push_back({Level::kWarn, "device_old", device, 0, 0, ""});
      continue;
    }
    if (inverter.mode == "fault") out.push_back({Level::kAlarm, "inverter_fault", device, 0, 0, ""});
    for (const std::string& warning : inverter.warnings) out.push_back({Level::kWarn, "inverter_warning", device, 0, 0, warning});
  }
  std::stable_sort(out.begin(), out.end(), [](const AlarmItem& a, const AlarmItem& b) { return static_cast<int>(a.level) > static_cast<int>(b.level); });
  return out;
}

// How high the bar of a cell is drawn, from 0 to 1, between the two voltages of the scale.
inline double cell_bar(double volts, double lowest = 2.80, double highest = 3.70) {
  const double share = (volts - lowest) / (highest - lowest);
  return share < 0 ? 0 : share > 1 ? 1 : share;
}

}  // namespace armor::solarhmi
