// ARMOR-NODE-SOLAR-HMI - what the screen reads of the solar equipment: the devices and the sums that ARMOR-SERVER answers with (the same names as GET /api/v1/solar).
// The screen reads no battery by wire. A value that is missing, of another type or out of the range of the contract is simply not there: it is never drawn as a zero.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#pragma once
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "json.hpp"

namespace armor::solarhmi {

using Reading = std::optional<double>;

// The limits of the contract (ARMOR-COMMON schemas solar_battery and solar_inverter).
constexpr std::size_t kMaxDevices = 32;
constexpr std::size_t kMaxModules = 16;
constexpr std::size_t kMaxCells = 32;
constexpr std::size_t kMaxWarnings = 32;
constexpr std::size_t kMaxStates = 48;
constexpr std::size_t kMaxLeftOut = 12;

// Who made it, as the node told the server; `deduced` when its protocol or model tells it and the equipment did not say it. Empty: not known, and never made up.
struct Maker {
  std::string name;
  bool deduced = false;
};

// A state of a module or of one of its cells that is not the normal one, with the battery's own word. `cell` counts from 0; -1: the module.
struct BatteryState {
  std::string column, word;
  int cell = -1;
};

struct Module {
  int n = 0;
  bool present = false;
  Reading soc_percent, voltage_v, current_a, temperature_c;
  std::vector<double> cells_v;   // empty when the module does not say its cells, or says one that cannot be a cell voltage
  Maker maker;
  std::vector<BatteryState> states;
  std::vector<int> cells_unread;       // positions (from 0) of the cells the battery's electronics do not read
  std::vector<std::string> left_out;   // figures the node read as impossible and did not send
};

struct Battery {
  std::string node, device, model, state;
  bool stale = false, example = false, alarm = false, protecting = false;
  Reading soc_percent, voltage_v, current_a, power_w, capacity_ah, full_capacity_ah, energy_kwh;
  std::vector<Module> stack;
  Maker maker;
};

struct Inverter {
  std::string node, device, mode;
  bool stale = false, example = false;
  Reading grid_v, out_w, load_percent, battery_v, battery_a, battery_percent, pv_w, heatsink_c;
  std::vector<std::string> warnings;
  Maker maker;
};

// The sums are the server's: the screen does not add up the equipment itself, so that it says what Studio and the phone say.
struct Totals {
  int inverters = 0, batteries = 0, stale = 0;
  Reading pv_w, load_w, battery_w, soc_percent, energy_kwh;
};

struct Snapshot {
  std::vector<Battery> batteries;
  std::vector<Inverter> inverters;
  Totals totals;
  std::size_t skipped = 0;   // entries that could not be read (or beyond the limit): the screen says that something is not shown
};

enum class ReadError { kNone, kTooBig, kNotJson, kShape };

namespace detail {

inline Reading number_in(const json::Value& object, std::string_view name, double lowest, double highest) {
  const json::Value* member = object.get(name);
  if (member == nullptr || !member->is_number() || member->number < lowest || member->number > highest) return std::nullopt;
  return member->number;
}

inline Maker maker_of(const json::Value& object) {
  Maker maker;
  maker.name = object.string_or("manufacturer", "");
  if (maker.name.size() > 32) maker.name.clear();
  maker.deduced = !maker.name.empty() && object.string_or("manufacturer_source", "") == "deduced";
  return maker;
}

inline Module module_of(const json::Value& value) {
  Module module;
  module.n = static_cast<int>(value.integer_or("n", 0, 0, static_cast<long long>(kMaxModules)));
  module.present = value.bool_or("present", false);
  module.soc_percent = number_in(value, "soc_percent", 0, 100);
  module.voltage_v = number_in(value, "voltage_v", 0, 1000);
  module.current_a = number_in(value, "current_a", -1000, 1000);
  module.temperature_c = number_in(value, "temperature_c", -50, 200);
  const json::Value* cells = value.get("cells_v");
  if (cells != nullptr && cells->is_array() && cells->items.size() <= kMaxCells) {
    for (const json::Value& cell : cells->items) {
      // the position of a cell is its number: one bad value drops the whole list instead of shifting the others
      if (!cell.is_number() || cell.number < 0 || cell.number > 10) { module.cells_v.clear(); break; }
      module.cells_v.push_back(cell.number);
    }
  }
  module.maker = maker_of(value);
  const json::Value* flags = value.get("flags");
  if (flags != nullptr && flags->is_array()) {
    for (const json::Value& flag : flags->items) {
      if (module.states.size() == kMaxStates) break;
      if (!flag.is_object()) continue;
      BatteryState state{flag.string_or("column", ""), flag.string_or("word", ""), static_cast<int>(flag.integer_or("cell", -1, -1, static_cast<long long>(kMaxCells) - 1))};
      if (!state.column.empty() && !state.word.empty() && state.column.size() <= 24 && state.word.size() <= 16) module.states.push_back(std::move(state));
    }
  }
  const json::Value* unread = value.get("cells_unread");
  if (unread != nullptr && unread->is_array()) {
    for (const json::Value& entry : unread->items) {
      if (module.cells_unread.size() == kMaxCells) break;
      if (entry.is_number() && entry.number >= 0 && entry.number < static_cast<double>(kMaxCells)) module.cells_unread.push_back(static_cast<int>(entry.number));
    }
  }
  const json::Value* left_out = value.get("left_out");
  if (left_out != nullptr && left_out->is_array()) {
    for (const json::Value& entry : left_out->items) {
      if (module.left_out.size() == kMaxLeftOut) break;
      if (entry.is_string() && entry.text.size() <= 16) module.left_out.push_back(entry.text);
    }
  }
  return module;
}

inline Battery battery_of(const json::Value& reading) {
  Battery battery;
  battery.model = reading.string_or("model", "");
  battery.maker = maker_of(reading);
  battery.state = reading.string_or("state", "");
  battery.alarm = reading.bool_or("alarm", false);
  battery.protecting = reading.bool_or("protecting", false);
  battery.soc_percent = number_in(reading, "soc_percent", 0, 100);
  battery.voltage_v = number_in(reading, "voltage_v", 0, 1000);
  battery.current_a = number_in(reading, "current_a", -1000, 1000);
  battery.power_w = number_in(reading, "power_w", -1000000, 1000000);
  battery.capacity_ah = number_in(reading, "capacity_ah", 0, 100000);
  battery.full_capacity_ah = number_in(reading, "full_capacity_ah", 0, 100000);
  battery.energy_kwh = number_in(reading, "energy_kwh", 0, 10000);
  const json::Value* stack = reading.get("stack");
  if (stack != nullptr && stack->is_array()) {
    for (const json::Value& entry : stack->items) {
      if (battery.stack.size() == kMaxModules) break;
      if (entry.is_object()) battery.stack.push_back(module_of(entry));
    }
  }
  return battery;
}

inline Inverter inverter_of(const json::Value& reading) {
  Inverter inverter;
  inverter.mode = reading.string_or("mode", "unknown");
  inverter.maker = maker_of(reading);
  inverter.grid_v = number_in(reading, "grid_v", 0, 600);
  inverter.out_w = number_in(reading, "out_w", 0, 100000);
  inverter.load_percent = number_in(reading, "load_percent", 0, 200);
  inverter.battery_v = number_in(reading, "battery_v", 0, 1000);
  inverter.battery_a = number_in(reading, "battery_a", -1000, 1000);
  inverter.battery_percent = number_in(reading, "battery_percent", 0, 100);
  inverter.pv_w = number_in(reading, "pv_w", 0, 100000);
  inverter.heatsink_c = number_in(reading, "heatsink_c", -50, 200);
  const json::Value* warnings = reading.get("warnings");
  if (warnings != nullptr && warnings->is_array()) {
    for (const json::Value& warning : warnings->items) {
      if (inverter.warnings.size() == kMaxWarnings) break;
      if (warning.is_string()) inverter.warnings.push_back(warning.text);
    }
  }
  return inverter;
}

}  // namespace detail

// Reads one answer. On any error `out` is left empty: half an answer is never shown.
// The shared JSON reader takes at most 64 KB and 4096 values, so the answer has to be the compact one meant for a small screen.
inline ReadError read_snapshot(std::string_view text, Snapshot& out) {
  out = Snapshot{};
  if (text.size() > json::kMaxText) return ReadError::kTooBig;
  json::Value document;
  if (!json::parse(text, document)) return ReadError::kNotJson;
  const json::Value* devices = document.get("devices");
  const json::Value* totals = document.get("totals");
  if (devices == nullptr || !devices->is_array() || totals == nullptr || !totals->is_object()) return ReadError::kShape;

  for (const json::Value& entry : devices->items) {
    const json::Value* reading = entry.get("reading");
    const std::string node = entry.string_or("node_id", ""), device = entry.string_or("device", ""), kind = entry.string_or("kind", "");
    const bool known = kind == "battery" || kind == "inverter";
    if (node.empty() || device.empty() || !known || reading == nullptr || !reading->is_object() || out.batteries.size() + out.inverters.size() == kMaxDevices) {
      ++out.skipped;
      continue;
    }
    if (kind == "battery") {
      Battery battery = detail::battery_of(*reading);
      battery.node = node;
      battery.device = device;
      battery.stale = entry.bool_or("stale", true);   // what does not say it is fresh is not taken as fresh
      battery.example = entry.bool_or("example", false);
      out.batteries.push_back(std::move(battery));
    } else {
      Inverter inverter = detail::inverter_of(*reading);
      inverter.node = node;
      inverter.device = device;
      inverter.stale = entry.bool_or("stale", true);
      inverter.example = entry.bool_or("example", false);
      out.inverters.push_back(std::move(inverter));
    }
  }

  out.totals.inverters = static_cast<int>(totals->integer_or("inverters", 0, 0, 1000));
  out.totals.batteries = static_cast<int>(totals->integer_or("batteries", 0, 0, 1000));
  out.totals.stale = static_cast<int>(totals->integer_or("stale", 0, 0, 1000));
  out.totals.pv_w = detail::number_in(*totals, "pv_w", 0, 100000000);
  out.totals.load_w = detail::number_in(*totals, "load_w", 0, 100000000);
  out.totals.battery_w = detail::number_in(*totals, "battery_w", -100000000, 100000000);
  out.totals.soc_percent = detail::number_in(*totals, "soc_percent", 0, 100);
  out.totals.energy_kwh = detail::number_in(*totals, "energy_kwh", 0, 1000000);
  return ReadError::kNone;
}

}  // namespace armor::solarhmi
