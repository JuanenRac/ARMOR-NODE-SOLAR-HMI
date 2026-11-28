// ARMOR-NODE-SOLAR-HMI host tests - old data, the levels of cells, modules and batteries, and the blinking.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#include <cstring>
#include <optional>

#include "check.hpp"
#include "solar_view.hpp"

using namespace armor::solarhmi;

int main() {
  CHECK(link_of(std::nullopt, 0) == Link::kNever);
  CHECK(link_of(1000, 1000) == Link::kOnline);
  CHECK(link_of(1000, 15999) == Link::kOnline);
  CHECK(link_of(1000, 16000) == Link::kStale);
  CHECK(link_of(1000, 500) == Link::kStale);   // a clock that went backwards is not trusted
  CHECK(link_of(0, 100, LinkPolicy{50}) == Link::kStale);
  CHECK(read_only(Link::kNever) && read_only(Link::kStale) && !read_only(Link::kOnline));

  const Thresholds limits;
  CHECK(!cell_stats({}).has_value());
  const auto stats = cell_stats({3.30, 3.25, 3.41, 3.33});
  CHECK(stats.has_value());
  CHECK(stats->min_index == 1 && stats->max_index == 2 && stats->delta_mv == 160);

  CHECK(cell_level(3.30, limits) == Level::kOk);
  CHECK(cell_level(3.55, limits) == Level::kAlarm);
  CHECK(cell_level(2.90, limits) == Level::kAlarm);

  Module module;
  CHECK(module_level(module, limits) == Level::kWarn);   // a module that is not there
  module.present = true;
  CHECK(module_level(module, limits) == Level::kOk);
  module.cells_v = {3.30, 3.31};
  CHECK(module_level(module, limits) == Level::kOk);
  module.cells_v = {3.30, 3.40};
  CHECK(module_level(module, limits) == Level::kWarn);
  module.cells_v = {3.30, 3.60};
  CHECK(module_level(module, limits) == Level::kAlarm);
  module.cells_v.clear();
  module.temperature_c = 55;
  CHECK(module_level(module, limits) == Level::kAlarm);
  module.temperature_c = 0;
  CHECK(module_level(module, limits) == Level::kAlarm);
  module.temperature_c = 20;
  module.soc_percent = 20;
  CHECK(module_level(module, limits) == Level::kWarn);

  Battery battery;
  CHECK(battery_level(battery, limits) == Level::kOk);
  battery.stale = true;
  CHECK(battery_level(battery, limits) == Level::kWarn);
  battery.protecting = true;
  CHECK(battery_level(battery, limits) == Level::kAlarm);
  battery.protecting = false;
  battery.alarm = true;
  CHECK(battery_level(battery, limits) == Level::kAlarm);
  battery = Battery{};
  battery.soc_percent = 10;
  CHECK(battery_level(battery, limits) == Level::kWarn);
  battery.stack.push_back(module);
  battery.stack.back().cells_v = {2.50};
  CHECK(battery_level(battery, limits) == Level::kAlarm);

  CHECK(std::strcmp(mark_of(Level::kAlarm).icon, "alert") == 0 && std::strcmp(mark_of(Level::kAlarm).word, "level_alarm") == 0);
  CHECK(std::strcmp(mark_of(Level::kWarn).icon, "warning") == 0);
  CHECK(std::strcmp(mark_of(Level::kOk).icon, "check") == 0);

  CHECK(lit(Level::kOk, 700) && lit(Level::kWarn, 700));
  CHECK(lit(Level::kAlarm, 0) && !lit(Level::kAlarm, 500) && lit(Level::kAlarm, 1000));
  // a half period shorter than a quarter of a second is not used: at most two blinks a second
  CHECK(lit(Level::kAlarm, 249, 10) && !lit(Level::kAlarm, 250, 10) && lit(Level::kAlarm, 500, 10));

  CHECK(state_level({"base", "SysError", -1}) == Level::kFault);
  CHECK(state_level({"voltage", "High", -1}) == Level::kWarn && state_level({"current", "DOCAlm", -1}) == Level::kWarn);
  CHECK(state_level({"system", "High", -1}) == Level::kAlarm);
  CHECK(state_level({"voltage", "OV", 3}) == Level::kAlarm && state_level({"voltage", "NeverSeen", -1}) == Level::kAlarm);
  CHECK(worst(Level::kAlarm, Level::kFault) == Level::kFault);
  CHECK(lit(Level::kFault, 500));   // a fault never blinks
  CHECK(std::strcmp(mark_of(Level::kFault).word, "level_fault") == 0 && std::strcmp(mark_of(Level::kFault).icon, "fault") == 0);

  Module said;
  said.present = true;
  said.cells_v = {3.30, 3.31, 0.0};
  said.cells_unread = {2};
  CHECK(module_level(said, limits) == Level::kFault);
  const auto only_read = cell_stats(said.cells_v, said.cells_unread);
  CHECK(only_read && only_read->min_index == 0 && only_read->delta_mv == 10);
  CHECK(!cell_stats({0.0}, {0}).has_value());
  said.cells_unread.clear();
  said.cells_v = {3.30, 3.31};
  said.states = {{"voltage", "High", -1}};
  CHECK(module_level(said, limits) == Level::kWarn);
  said.states = {{"voltage", "OV", 1}};
  CHECK(module_level(said, limits) == Level::kAlarm);
  said.temperature_c = 80.0;
  said.states = {{"base", "SysError", -1}};
  CHECK(module_level(said, limits) == Level::kFault);   // a hot module does not lower a fault to an alarm

  FINISH("solar_view");
}
