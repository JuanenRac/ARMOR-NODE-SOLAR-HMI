// ARMOR-NODE-SOLAR-HMI simulator - a made-up installation (one stack of four modules of fifteen cells and one inverter) that answers as ARMOR-SERVER would, in a few scenarios.
// The same scenario and step give the same text on every computer. No real equipment is behind any of these figures.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

#include "json.hpp"

namespace armor::solarhmi::sim {

enum class Scenario { kNormal, kLowCharge, kCellHigh, kInverterFault, kNodeSilent, kBmsFault };

struct Named {
  Scenario scenario;
  const char* name;
};

inline constexpr Named kScenarios[] = {{Scenario::kNormal, "normal"},
                                       {Scenario::kLowCharge, "low-charge"},
                                       {Scenario::kCellHigh, "cell-high"},
                                       {Scenario::kInverterFault, "inverter-fault"},
                                       {Scenario::kNodeSilent, "node-silent"},
                                       {Scenario::kBmsFault, "bms-fault"}};

constexpr int kModules = 4;
constexpr int kCells = 15;
constexpr double kModuleAh = 74.0;

inline bool scenario_named(std::string_view name, Scenario& out) {
  for (const Named& known : kScenarios)
    if (name == known.name) { out = known.scenario; return true; }
  return false;
}

// A small repeatable generator: a value from -3 to 3.
inline int jitter(std::uint32_t seed) {
  std::uint32_t x = seed * 2654435761u + 12345u;
  x ^= x >> 13;
  return static_cast<int>(x % 7u) - 3;
}

inline std::string server_answer(Scenario scenario, std::uint32_t step = 0) {
  const bool silent = scenario == Scenario::kNodeSilent;
  const bool low = scenario == Scenario::kLowCharge;
  const bool fault = scenario == Scenario::kInverterFault;
  const int soc = low ? 12 : 76;
  const double base_cell = low ? 3.150 : 3.310;
  const double current = low ? -18.0 : 12.5;
  const long long stamp = 1000LL * (static_cast<long long>(step) + 1);

  double cells[kModules][kCells];
  double cell_min = 10, cell_max = 0, module_v[kModules];
  for (int m = 0; m < kModules; ++m) {
    module_v[m] = 0;
    for (int c = 0; c < kCells; ++c) {
      double volts = base_cell + 0.002 * jitter(step * 97u + static_cast<std::uint32_t>(m * kCells + c));
      if (scenario == Scenario::kCellHigh && m == 1 && c == 6) volts = 3.640;
      const bool unread = scenario == Scenario::kBmsFault && m == 2 && c >= 3 && c <= 5;   // three cells the electronics do not read
      cells[m][c] = unread ? 0.0 : volts;
      module_v[m] += volts;
      if (volts < cell_min) cell_min = volts;
      if (volts > cell_max) cell_max = volts;
    }
  }
  const double stack_v = (module_v[0] + module_v[1] + module_v[2] + module_v[3]) / kModules;
  const double full_ah = kModuleAh * kModules;
  const double left_ah = full_ah * soc / 100.0;
  const double energy = left_ah * stack_v / 1000.0;
  const double pv = fault || low ? 0.0 : 1850.0;
  const double load = fault ? 0.0 : 640.0;

  json::Writer out;
  out.begin_object().key("devices").begin_array();

  out.begin_object().field("node_id", "solar-sim").field("device", "inverter-1").field("kind", "inverter");
  out.field("stale", silent).field("example", false).field("received_at", "1970-01-01T00:00:00.000Z");
  out.key("reading").begin_object().field("kind", "inverter").field("node_id", "solar-sim").field("device", "inverter-1").field("timestamp_ms", stamp);
  out.field("mode", fault ? "fault" : "battery");
  out.key("grid_v").number(0.0, 1).key("grid_hz").number(0.0, 1).key("out_v").number(fault ? 0.0 : 230.1, 1).key("out_hz").number(fault ? 0.0 : 50.0, 1);
  out.key("out_va").number(load, 0).key("out_w").number(load, 0).key("load_percent").number(load / 50.0, 0);
  out.key("battery_v").number(stack_v, 2).key("battery_a").number(current, 1).key("battery_percent").number(soc, 0);
  out.key("pv_v").number(pv > 0 ? 312.0 : 0.0, 1).key("pv_a").number(pv > 0 ? pv / 312.0 : 0.0, 1).key("pv_w").number(pv, 0);
  out.key("heatsink_c").number(fault ? 88.0 : 41.0, 0);
  out.field("ac_charging", false).field("pv_charging", pv > 0).field("load_on", !fault);
  out.field("manufacturer", "Voltronic").field("manufacturer_source", "deduced");
  out.key("warnings").begin_array();
  if (fault) out.string("over_temperature");
  out.end_array().end_object().end_object();

  out.begin_object().field("node_id", "solar-sim").field("device", "stack-1").field("kind", "battery");
  out.field("stale", silent).field("example", false).field("received_at", "1970-01-01T00:00:00.000Z");
  out.key("reading").begin_object().field("kind", "battery").field("node_id", "solar-sim").field("device", "stack-1").field("timestamp_ms", stamp);
  out.field("modules", kModules).field("state", current < 0 ? "discharging" : "charging");
  out.field("manufacturer", "Pylontech").field("manufacturer_source", "reported");
  out.key("voltage_v").number(stack_v, 2).key("current_a").number(current, 1);
  out.key("temperature_min_c").number(21.0, 1).key("temperature_max_c").number(24.5, 1);
  out.key("cell_min_v").number(cell_min, 3).key("cell_max_v").number(cell_max, 3);
  out.field("soc_percent", soc).field("alarm", scenario == Scenario::kCellHigh);
  out.key("capacity_ah").number(left_ah, 1).key("full_capacity_ah").number(full_ah, 1).key("energy_kwh").number(energy, 2);
  out.key("stack").begin_array();
  for (int m = 0; m < kModules; ++m) {
    out.begin_object().field("n", m + 1).field("present", true);
    out.key("voltage_v").number(module_v[m], 3).key("current_a").number(current / kModules, 2).key("temperature_c").number(21.0 + m, 1);
    out.field("soc_percent", soc).field("state", current < 0 ? "Dischg" : "Charge");
    out.key("cells_v").begin_array();
    for (int c = 0; c < kCells; ++c) out.number(cells[m][c], 3);
    out.end_array();
    if (scenario == Scenario::kBmsFault && m == 2) {
      out.key("cells_unread").begin_array().number(3, 0).number(4, 0).number(5, 0).end_array();
      out.key("flags").begin_array();
      out.begin_object().field("column", "voltage").field("word", "UV").field("cell", 3).end_object();
      out.begin_object().field("column", "voltage").field("word", "High").end_object();
      out.end_array();
    }
    if (scenario == Scenario::kBmsFault && m == 3) out.key("left_out").begin_array().string("current").end_array();
    out.end_object();
  }
  out.end_array().end_object().end_object();

  out.end_array().key("totals").begin_object();
  out.field("inverters", 1).field("batteries", 1).field("stale", silent ? 2 : 0);
  out.key("pv_w").number(silent ? 0.0 : pv, 0).key("load_w").number(silent ? 0.0 : load, 0);
  if (silent) {
    out.key("battery_w").null().key("soc_percent").null().key("energy_kwh").null();
  } else {
    out.key("battery_w").number(stack_v * current, 0).field("soc_percent", soc).key("energy_kwh").number(energy, 2);
  }
  out.end_object().end_object();
  return out.str();
}

}  // namespace armor::solarhmi::sim
