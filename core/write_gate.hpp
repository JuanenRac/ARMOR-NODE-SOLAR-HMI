// ARMOR-NODE-SOLAR-HMI - whether the screen may even offer an operation on a battery. Only two exist from a wall: checking it (reading it again) and resetting its counters; every other
// operation of the server's catalogue stays with an administrator elsewhere. It only answers; it sends nothing. The order, when there is a way to send one, goes through ARMOR-SERVER,
// which checks the PIN, decides on its own and keeps the record. Everything starts closed.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#pragma once

#include "solar_view.hpp"

namespace armor::solarhmi {

// kOther stands for anything else: it is never offered by the screen, whatever the server allows.
enum class Operation { kOther, kCheck, kResetCounters };

struct WriteContext {
  Operation operation = Operation::kOther;
  bool server_allows = false;   // the server's switch for writing to batteries (off unless an administrator turned it on)
  bool account_may = false;     // the server says the account of this screen may ask for it
  Link link = Link::kNever;
  bool device_stale = true;
  bool example = false;         // an example device is never written to
  bool pin_session = false;
  bool confirmed = false;       // the person said yes on the screen, after reading what will happen
};

enum class WriteBlock { kNone, kNotFromScreen, kServerOff, kNoPermission, kNoServer, kOldData, kExample, kNoPin, kNotConfirmed };

// The first reason that stops it, in the order the screen explains them.
inline WriteBlock first_block(const WriteContext& context) {
  if (context.operation != Operation::kCheck && context.operation != Operation::kResetCounters) return WriteBlock::kNotFromScreen;
  if (!context.server_allows) return WriteBlock::kServerOff;
  if (!context.account_may) return WriteBlock::kNoPermission;
  if (read_only(context.link)) return WriteBlock::kNoServer;
  if (context.device_stale) return WriteBlock::kOldData;
  if (context.example) return WriteBlock::kExample;
  if (!context.pin_session) return WriteBlock::kNoPin;
  if (!context.confirmed) return WriteBlock::kNotConfirmed;
  return WriteBlock::kNone;
}

}  // namespace armor::solarhmi
