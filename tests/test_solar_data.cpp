// ARMOR-NODE-SOLAR-HMI host tests - reading the server's answer: what is there, what is missing, what is refused.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#include <string>

#include "check.hpp"
#include "solar_data.hpp"

using namespace armor::solarhmi;

namespace {
std::string battery_entry(const std::string& reading, const std::string& extra = "\"stale\":false,") {
  return "{\"node_id\":\"n\",\"device\":\"d\",\"kind\":\"battery\"," + extra + "\"reading\":" + reading + "}";
}
std::string answer(const std::string& devices, const std::string& totals = "{\"inverters\":0,\"batteries\":1,\"stale\":0}") {
  return "{\"devices\":[" + devices + "],\"totals\":" + totals + "}";
}
}  // namespace

int main() {
  Snapshot snapshot;

  CHECK(read_snapshot("", snapshot) == ReadError::kNotJson);
  CHECK(read_snapshot("{", snapshot) == ReadError::kNotJson);
  CHECK(read_snapshot("[]", snapshot) == ReadError::kShape);
  CHECK(read_snapshot("{\"devices\":[]}", snapshot) == ReadError::kShape);
  CHECK(read_snapshot("{\"devices\":{},\"totals\":{}}", snapshot) == ReadError::kShape);
  CHECK(read_snapshot(std::string(70000, ' '), snapshot) == ReadError::kTooBig);

  CHECK(read_snapshot(answer(""), snapshot) == ReadError::kNone);
  CHECK(snapshot.batteries.empty() && snapshot.inverters.empty() && snapshot.skipped == 0);
  CHECK(snapshot.totals.batteries == 1);
  CHECK(!snapshot.totals.soc_percent.has_value());   // missing is missing, not zero

  CHECK(read_snapshot(answer("", "{\"soc_percent\":null,\"battery_w\":-120,\"pv_w\":\"12\"}"), snapshot) == ReadError::kNone);
  CHECK(!snapshot.totals.soc_percent.has_value());
  CHECK(!snapshot.totals.pv_w.has_value());
  CHECK(snapshot.totals.battery_w.has_value() && *snapshot.totals.battery_w == -120);

  const std::string reading =
      "{\"soc_percent\":88,\"voltage_v\":49.87,\"current_a\":-2.5,\"alarm\":true,\"unknown_field\":1,"
      "\"stack\":[{\"n\":1,\"present\":true,\"soc_percent\":130,\"cells_v\":[3.30,3.31,3.32]},{\"n\":2,\"present\":false},7,"
      "{\"n\":3,\"present\":true,\"cells_v\":[3.30,\"x\",3.32]},{\"n\":4,\"present\":true,\"cells_v\":[3.30,12]}]}";
  CHECK(read_snapshot(answer(battery_entry(reading)), snapshot) == ReadError::kNone);
  CHECK(snapshot.batteries.size() == 1);
  const Battery& battery = snapshot.batteries.front();
  CHECK(battery.node == "n" && battery.device == "d" && !battery.stale && !battery.example && battery.alarm && !battery.protecting);
  CHECK(battery.soc_percent && *battery.soc_percent == 88);
  CHECK(battery.stack.size() == 4);
  CHECK(!battery.stack[0].soc_percent.has_value());   // 130 % is not a state of charge
  CHECK(battery.stack[0].cells_v.size() == 3);
  CHECK(!battery.stack[1].present);
  CHECK(battery.stack[2].cells_v.empty());            // one bad cell drops the list: positions are cell numbers
  CHECK(battery.stack[3].cells_v.empty());

  // what does not say it is fresh is not taken as fresh
  CHECK(read_snapshot(answer(battery_entry("{}", "")), snapshot) == ReadError::kNone);
  CHECK(snapshot.batteries.size() == 1 && snapshot.batteries.front().stale);

  // entries that cannot be read are counted, and the others stay
  CHECK(read_snapshot(answer("{\"kind\":\"battery\"},{\"node_id\":\"n\",\"device\":\"d\",\"kind\":\"toaster\",\"reading\":{}},5," + battery_entry("{}")), snapshot) == ReadError::kNone);
  CHECK(snapshot.batteries.size() == 1 && snapshot.skipped == 3);

  std::string many;
  for (int i = 0; i < 35; ++i) many += (i ? "," : "") + battery_entry("{}");
  CHECK(read_snapshot(answer(many), snapshot) == ReadError::kNone);
  CHECK(snapshot.batteries.size() == kMaxDevices && snapshot.skipped == 35 - kMaxDevices);

  std::string modules = "{\"stack\":[";
  for (int i = 0; i < 20; ++i) modules += std::string(i ? "," : "") + "{\"present\":true}";
  modules += "]}";
  CHECK(read_snapshot(answer(battery_entry(modules)), snapshot) == ReadError::kNone);
  CHECK(snapshot.batteries.front().stack.size() == kMaxModules);

  const std::string inverter =
      "{\"node_id\":\"n\",\"device\":\"i\",\"kind\":\"inverter\",\"stale\":false,\"example\":true,"
      "\"reading\":{\"mode\":\"line\",\"grid_v\":231,\"pv_w\":-4,\"out_w\":500,\"warnings\":[\"fan\",3]}}";
  CHECK(read_snapshot(answer(inverter), snapshot) == ReadError::kNone);
  CHECK(snapshot.inverters.size() == 1);
  const Inverter& unit = snapshot.inverters.front();
  CHECK(unit.mode == "line" && unit.example && !unit.stale);
  CHECK(unit.grid_v && *unit.grid_v == 231);
  CHECK(!unit.pv_w.has_value());
  CHECK(unit.warnings.size() == 1 && unit.warnings.front() == "fan");

  // a failed read leaves nothing of the previous one behind
  CHECK(read_snapshot("nonsense", snapshot) == ReadError::kNotJson);
  CHECK(snapshot.inverters.empty() && snapshot.batteries.empty());

  // the maker and the states: only what is well formed is taken
  const std::string states =
      "{\"node_id\":\"a\",\"device\":\"b\",\"kind\":\"battery\",\"stale\":false,\"reading\":{\"manufacturer\":\"Daly\",\"manufacturer_source\":\"deduced\",\"stack\":[{\"n\":1,\"present\":true,"
      "\"flags\":[{\"column\":\"voltage\",\"word\":\"UV\",\"cell\":2},{\"column\":\"\",\"word\":\"x\"},7],\"cells_unread\":[1,99,-1],\"left_out\":[\"current\",4]}]}}";
  CHECK(read_snapshot(answer(states), snapshot) == ReadError::kNone);
  CHECK(snapshot.batteries.size() == 1);
  const Battery& said = snapshot.batteries.front();
  CHECK(said.maker.name == "Daly" && said.maker.deduced);
  CHECK(said.stack.size() == 1 && said.stack[0].states.size() == 1 && said.stack[0].states[0].cell == 2);
  CHECK(said.stack[0].cells_unread.size() == 1 && said.stack[0].left_out.size() == 1);

  FINISH("solar_data");
}
