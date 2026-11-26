// ARMOR-NODE-SOLAR-HMI - whether the screen may even offer an operation on a battery (check it, or reset it). It only answers; it sends nothing. The order, when there is a way to
// send one, goes through ARMOR-SERVER, which decides on its own and keeps the record. Everything starts closed.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#pragma once

#include "solar_view.hpp"

namespace armor::solarhmi {

struct WriteContext {
  bool server_allows = false;   // the server's switch for writing to batteries (off unless an administrator turned it on)
  bool account_may = false;     // the server says the account of this screen may ask for it
  Link link = Link::kNever;
  bool device_stale = true;
  bool example = false;         // an example device is never written to
  bool pin_session = false;
  bool confirmed = false;       // the person said yes on the screen, after reading what will happen
};

enum class WriteBlock { kNone, kServerOff, kNoPermission, kNoServer, kOldData, kExample, kNoPin, kNotConfirmed };

// The first reason that stops it, in the order the screen explains them.
inline WriteBlock first_block(const WriteContext& context) {
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
