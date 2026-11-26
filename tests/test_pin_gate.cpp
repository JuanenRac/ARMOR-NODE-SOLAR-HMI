// ARMOR-NODE-SOLAR-HMI host tests - the PIN: the published PBKDF2 values, wrong tries and locks, and the end of a session.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#include <cstdint>
#include <string>

#include "check.hpp"
#include "pin_gate.hpp"

using namespace armor::solarhmi;

int main() {
  const auto* salt_text = reinterpret_cast<const std::uint8_t*>("salt");
  const auto one = pbkdf2_block("password", salt_text, 4, 1);
  CHECK(armor::hash::to_hex(one) == "120fb6cffcf8b32c43e7225256c4f837a86548c92ccc35480805987cb70be17b");
  const auto two = pbkdf2_block("password", salt_text, 4, 2);
  CHECK(armor::hash::to_hex(two) == "ae4d0c95af6b46d32d0adff928f06dd02a303f8ef3c251dfd6e2d85a95474c43");

  PinPolicy policy;
  policy.iterations = 10;   // the tests check the rules, not the cost
  Salt salt{};
  for (std::size_t i = 0; i < salt.size(); ++i) salt[i] = static_cast<std::uint8_t>(i * 7);

  PinGate gate(policy);
  CHECK(gate.enter("1234", 0) == PinResult::kNotSet);
  CHECK(!gate.set_pin("123", salt, 0));
  CHECK(!gate.set_pin("12345678901", salt, 0));
  CHECK(!gate.set_pin("12a4", salt, 0));
  CHECK(gate.set_pin("4821", salt, 0));
  CHECK(!gate.unlocked(0));                  // setting a PIN opens nothing
  CHECK(!gate.set_pin("1111", salt, 0));     // nor can it be changed from outside a session

  CHECK(gate.enter("12", 100) == PinResult::kBadFormat);
  CHECK(gate.record().failures == 0);
  CHECK(gate.enter("4821", 1000) == PinResult::kOk);
  CHECK(gate.unlocked(1000));
  CHECK(gate.set_pin("5000", salt, 1500));
  CHECK(gate.set_pin("4821", salt, 1500));

  // without a touch the session ends
  CHECK(gate.unlocked(60999));
  CHECK(!gate.unlocked(61000));
  gate.touch(61001);
  CHECK(!gate.unlocked(61001));              // a touch does not revive it

  // touches keep it open, but never past its longest time
  CHECK(gate.enter("4821", 0) == PinResult::kOk);
  for (std::int64_t now = 50000; now < 300000; now += 50000) {
    gate.touch(now);
    CHECK(gate.unlocked(now));
  }
  CHECK(!gate.unlocked(300000));

  CHECK(gate.enter("4821", 400000) == PinResult::kOk);
  gate.close();
  CHECK(!gate.unlocked(400000));

  // three wrong tries lock it; while locked even the right PIN is not tried
  CHECK(gate.enter("0000", 500000) == PinResult::kWrong);
  CHECK(gate.enter("0000", 500001) == PinResult::kWrong);
  CHECK(gate.enter("0000", 500002) == PinResult::kLocked);
  CHECK(gate.locked_for_ms(500002) == 60000);
  CHECK(gate.enter("4821", 510000) == PinResult::kLocked);
  CHECK(gate.record().failures == 3);
  CHECK(!gate.unlocked(510000));

  // the next lock is twice as long
  CHECK(gate.enter("0000", 560002) == PinResult::kWrong);
  CHECK(gate.enter("0000", 560003) == PinResult::kWrong);
  CHECK(gate.enter("0000", 560004) == PinResult::kLocked);
  CHECK(gate.locked_for_ms(560004) == 120000);

  // switching off and on again starts the lock again and keeps the count
  PinGate restarted(policy);
  restarted.restore(gate.record(), 0);
  CHECK(restarted.locked_for_ms(0) == 120000);
  CHECK(restarted.enter("4821", 1000) == PinResult::kLocked);
  CHECK(restarted.enter("4821", 120000) == PinResult::kOk);
  CHECK(restarted.record().failures == 0);

  // a record with some wrong tries but no full round is not locked at start
  PinRecord record = restarted.record();
  record.failures = 2;
  PinGate partly(policy);
  partly.restore(record, 0);
  CHECK(partly.locked_for_ms(0) == 0);
  CHECK(partly.enter("0000", 0) == PinResult::kLocked);

  // the lock has a ceiling
  record.failures = 300;
  PinGate long_locked(policy);
  long_locked.restore(record, 0);
  CHECK(long_locked.locked_for_ms(0) == policy.lock_max_ms);

  // the record does not hold the digits
  const PinRecord& kept = restarted.record();
  const std::string bytes(reinterpret_cast<const char*>(kept.digest.data()), kept.digest.size());
  CHECK(bytes.find("4821") == std::string::npos);
  CHECK(kept.iterations == 10);

  FINISH("pin_gate");
}
