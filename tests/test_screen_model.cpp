// ARMOR-NODE-SOLAR-HMI host tests - what the pages list: the alarms of each scenario of the simulator, how numbers are written, the bar of a cell, and the pins of the board.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#include <set>
#include <string>

#include "board_p4_jc1060.h"
#include "check.hpp"
#include "plant.hpp"
#include "screen_model.hpp"
#include "screen_text.hpp"

using namespace armor::solarhmi;

static std::vector<AlarmItem> alarms_in(sim::Scenario scenario) {
  Snapshot snapshot;
  CHECK(read_snapshot(sim::server_answer(scenario), snapshot) == ReadError::kNone);
  const std::vector<AlarmItem> out = alarms_of(snapshot, Thresholds{});
  for (const AlarmItem& item : out) CHECK(text::known_key(item.key));
  return out;
}

static bool has(const std::vector<AlarmItem>& items, const std::string& key, Level level) {
  for (const AlarmItem& item : items)
    if (key == item.key && item.level == level) return true;
  return false;
}

int main() {
  CHECK(alarms_in(sim::Scenario::kNormal).empty());

  const std::vector<AlarmItem> high = alarms_in(sim::Scenario::kCellHigh);
  CHECK(!high.empty());
  CHECK(high.front().level == Level::kAlarm);
  CHECK(has(high, "cell_high", Level::kAlarm));
  CHECK(high.front().module == 2 && high.front().cell == 7);
  CHECK(has(high, "spread_high", Level::kWarn));
  CHECK(!has(high, "battery_alarm", Level::kAlarm));   // the cell already says what is wrong

  CHECK(has(alarms_in(sim::Scenario::kLowCharge), "charge_low", Level::kWarn));

  const std::vector<AlarmItem> fault = alarms_in(sim::Scenario::kInverterFault);
  CHECK(has(fault, "inverter_fault", Level::kAlarm));
  CHECK(has(fault, "inverter_warning", Level::kWarn));

  const std::vector<AlarmItem> silent = alarms_in(sim::Scenario::kNodeSilent);
  CHECK(silent.size() == 2);
  CHECK(has(silent, "device_old", Level::kWarn));

  // the battery's own alarm is listed when nothing else says what is wrong; a battery that is missing is a warning
  Snapshot own;
  Battery battery;
  battery.node = "n";
  battery.device = "d";
  battery.alarm = true;
  Module gone;
  gone.n = 3;
  battery.stack.push_back(gone);
  own.batteries.push_back(battery);
  const std::vector<AlarmItem> items = alarms_of(own, Thresholds{});
  CHECK(has(items, "battery_alarm", Level::kAlarm));
  CHECK(has(items, "module_missing", Level::kWarn));
  CHECK(items.front().level == Level::kAlarm);

  CHECK(format_power(std::nullopt) == "--");
  CHECK(format_power(640.4) == "640 W");
  CHECK(format_power(1850.0) == "1.85 kW");
  CHECK(format_power(-2000.0) == "-2.00 kW");
  CHECK(format_percent(std::nullopt) == "--");
  CHECK(format_percent(76.0) == "76 %");
  CHECK(format_value(3.31, 3, "V") == "3.310 V");
  CHECK(format_value(std::nullopt, 1, "V") == "--");
  CHECK(cell_bar(2.0) == 0.0);
  CHECK(cell_bar(4.0) == 1.0);
  CHECK(cell_bar(3.25) > 0.49 && cell_bar(3.25) < 0.51);

  // no pin of the board is used twice
  const int pins[] = {SOLAR_PIN_I2C_SDA,     SOLAR_PIN_I2C_SCL,     SOLAR_PIN_ETH_MDC,   SOLAR_PIN_ETH_MDIO,   SOLAR_PIN_ETH_PHY_RST, SOLAR_PIN_ETH_REF_CLK, SOLAR_PIN_ETH_TXD0,
                      SOLAR_PIN_ETH_TXD1,    SOLAR_PIN_ETH_TX_EN,   SOLAR_PIN_ETH_RXD0,  SOLAR_PIN_ETH_RXD1,   SOLAR_PIN_ETH_CRS_DV,  SOLAR_PIN_C6_CLK,      SOLAR_PIN_C6_CMD,
                      SOLAR_PIN_C6_D0,       SOLAR_PIN_C6_D1,       SOLAR_PIN_C6_D2,     SOLAR_PIN_C6_D3,      SOLAR_PIN_C6_RESET};
  std::set<int> used;
  for (const int pin : pins) {
    CHECK(pin >= 0 && pin <= 54);
    CHECK(used.insert(pin).second);
  }
  FINISH("test_screen_model");
}
