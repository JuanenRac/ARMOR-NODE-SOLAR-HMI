// ARMOR-NODE-SOLAR-HMI host tests - an operation on a battery is offered only when every condition holds.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#include "check.hpp"
#include "write_gate.hpp"

using namespace armor::solarhmi;

namespace {
WriteContext all_good() {
  WriteContext context;
  context.operation = Operation::kResetCounters;
  context.server_allows = context.account_may = context.pin_session = context.confirmed = true;
  context.link = Link::kOnline;
  context.device_stale = false;
  return context;
}
}  // namespace

int main() {
  CHECK(first_block(WriteContext{}) == WriteBlock::kNotFromScreen);
  CHECK(first_block(all_good()) == WriteBlock::kNone);

  WriteContext context = all_good();
  context.operation = Operation::kCheck;
  CHECK(first_block(context) == WriteBlock::kNone);
  context.operation = Operation::kOther;
  CHECK(first_block(context) == WriteBlock::kNotFromScreen);   // everything else stays out, however good the rest is
  context = all_good();
  context.server_allows = false;
  CHECK(first_block(context) == WriteBlock::kServerOff);
  context = all_good();
  context.account_may = false;
  CHECK(first_block(context) == WriteBlock::kNoPermission);
  context = all_good();
  context.link = Link::kStale;
  CHECK(first_block(context) == WriteBlock::kNoServer);
  context.link = Link::kNever;
  CHECK(first_block(context) == WriteBlock::kNoServer);
  context = all_good();
  context.device_stale = true;
  CHECK(first_block(context) == WriteBlock::kOldData);
  context = all_good();
  context.example = true;
  CHECK(first_block(context) == WriteBlock::kExample);
  context = all_good();
  context.pin_session = false;
  CHECK(first_block(context) == WriteBlock::kNoPin);
  context = all_good();
  context.confirmed = false;
  CHECK(first_block(context) == WriteBlock::kNotConfirmed);

  FINISH("write_gate");
}
