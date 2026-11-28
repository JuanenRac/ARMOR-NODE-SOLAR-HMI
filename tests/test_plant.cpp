// ARMOR-NODE-SOLAR-HMI host tests - every scenario of the simulator is read by the screen and looks the way its name says.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#include <string>

#include "check.hpp"
#include "plant.hpp"
#include "solar_view.hpp"

using namespace armor::solarhmi;

namespace {
Snapshot read(sim::Scenario scenario, std::uint32_t step = 0) {
  Snapshot snapshot;
  CHECK(read_snapshot(sim::server_answer(scenario, step), snapshot) == ReadError::kNone);
  CHECK(snapshot.skipped == 0);
  CHECK(snapshot.batteries.size() == 1 && snapshot.inverters.size() == 1);
  return snapshot;
}
}  // namespace

int main() {
  const Thresholds limits;
  for (const sim::Named& known : sim::kScenarios) {
    sim::Scenario scenario = sim::Scenario::kNormal;
    CHECK(sim::scenario_named(known.name, scenario) && scenario == known.scenario);
    for (std::uint32_t step = 0; step < 20; ++step) {
      const Snapshot snapshot = read(known.scenario, step);
      const Battery& battery = snapshot.batteries.front();
      CHECK(battery.stack.size() == sim::kModules);
      for (const Module& module : battery.stack) CHECK(module.cells_v.size() == sim::kCells);
    }
  }
  sim::Scenario unknown = sim::Scenario::kNormal;
  CHECK(!sim::scenario_named("storm", unknown));

  CHECK(sim::server_answer(sim::Scenario::kNormal, 3) == sim::server_answer(sim::Scenario::kNormal, 3));
  CHECK(sim::server_answer(sim::Scenario::kNormal, 3) != sim::server_answer(sim::Scenario::kNormal, 4));

  const Snapshot normal = read(sim::Scenario::kNormal);
  CHECK(battery_level(normal.batteries.front(), limits) == Level::kOk);
  CHECK(normal.totals.soc_percent && *normal.totals.soc_percent == 76);
  CHECK(normal.totals.energy_kwh.has_value());

  const Snapshot low = read(sim::Scenario::kLowCharge);
  CHECK(battery_level(low.batteries.front(), limits) == Level::kWarn);
  CHECK(low.totals.battery_w && *low.totals.battery_w < 0);

  const Snapshot high = read(sim::Scenario::kCellHigh);
  CHECK(battery_level(high.batteries.front(), limits) == Level::kAlarm);
  CHECK(module_level(high.batteries.front().stack[0], limits) == Level::kOk);
  CHECK(module_level(high.batteries.front().stack[1], limits) == Level::kAlarm);
  const auto stats = cell_stats(high.batteries.front().stack[1].cells_v);
  CHECK(stats && stats->max_index == 6);

  const Snapshot fault = read(sim::Scenario::kInverterFault);
  CHECK(fault.inverters.front().mode == "fault" && fault.inverters.front().warnings.size() == 1);

  const Snapshot silent = read(sim::Scenario::kNodeSilent);
  CHECK(silent.batteries.front().stale && silent.inverters.front().stale);
  CHECK(silent.totals.stale == 2 && !silent.totals.soc_percent.has_value() && !silent.totals.battery_w.has_value());

  // three cells not read: a fault of the electronics, red and still, and not the lowest cells of the pack
  const Snapshot bms = read(sim::Scenario::kBmsFault);
  const Module& third = bms.batteries.front().stack[2];
  CHECK(third.cells_unread.size() == 3 && third.cells_unread.front() == 3);
  CHECK(module_level(third, limits) == Level::kFault && battery_level(bms.batteries.front(), limits) == Level::kFault);
  const auto read_cells = cell_stats(third.cells_v, third.cells_unread);
  CHECK(read_cells && read_cells->min_v > 3.0 && read_cells->delta_mv < limits.spread_warn_mv);
  CHECK(module_level(bms.batteries.front().stack[3], limits) == Level::kWarn);   // a figure left out
  CHECK(bms.batteries.front().maker.name == "Pylontech" && !bms.batteries.front().maker.deduced);
  CHECK(bms.inverters.front().maker.name == "Voltronic" && bms.inverters.front().maker.deduced);

  FINISH("plant");
}
