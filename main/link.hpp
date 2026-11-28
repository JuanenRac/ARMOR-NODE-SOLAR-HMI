// ARMOR-NODE-SOLAR-HMI - the reading link of the screen to ARMOR-SERVER, in a task of its own: it signs in with the panel account (core/panel_session.hpp) and reads the solar equipment
// every few seconds (core/server_link.hpp). It only starts when the option "link" is on and the server is set up; it writes nothing to the server. NOT VERIFIED ON A BOARD.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#pragma once
#include <cstdint>

#include "panel_config.hpp"
#include "solar_data.hpp"
#include "solar_view.hpp"

namespace armor::solarhmi::net {

struct View {
  Snapshot snapshot;
  Link link = Link::kNever;
  std::uint32_t generation = 0;   // goes up with every answer
};

bool start(const PanelConfig& config);
View view();
// "off", or what the last turn ended with, for the console.
const char* last_result();

}  // namespace armor::solarhmi::net
