// ARMOR-NODE-SOLAR-HMI host tests - the words of the screen: every key in the eight languages, no key twice.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#include <set>
#include <string>

#include "check.hpp"
#include "screen_model.hpp"
#include "screen_text.hpp"

using namespace armor::solarhmi;

int main() {
  std::set<std::string> keys;
  for (const text::Row& row : text::kRows) {
    CHECK(keys.insert(std::string(row.key)).second);
    for (const std::string_view value : row.text) CHECK(!value.empty());
  }
  CHECK(text::kRowCount == keys.size());
  CHECK(text::effective_language("pt") == "pt");
  CHECK(text::effective_language("xx") == "en");
  CHECK(text::word("back", "es") == "Volver");
  CHECK(text::word("back", "nothing") == "Back");
  CHECK(text::word("no_such_key", "es") == "no_such_key");

  // every key the model can answer has a row
  for (const char* mode : {"power_on", "standby", "line", "battery", "fault", "power_saving", "shutdown", "something else"}) CHECK(text::known_key(mode_key(mode)));
  for (const WriteBlock block : {WriteBlock::kNotFromScreen, WriteBlock::kServerOff, WriteBlock::kNoPermission, WriteBlock::kNoServer, WriteBlock::kOldData, WriteBlock::kExample,
                                 WriteBlock::kNoPin})
    CHECK(text::known_key(block_key(block)));
  for (const double watts : {-100.0, 0.0, 100.0}) CHECK(text::known_key(flow_key(watts)));
  CHECK(text::known_key(flow_key(std::nullopt)));
  for (const Level level : {Level::kOk, Level::kWarn, Level::kAlarm, Level::kFault}) CHECK(text::known_key(mark_of(level).word));
  for (const char* key : {"bms_fault", "bms_protect", "bms_notice", "left_out", "deduced"}) CHECK(text::known_key(key));
  FINISH("test_screen_text");
}
