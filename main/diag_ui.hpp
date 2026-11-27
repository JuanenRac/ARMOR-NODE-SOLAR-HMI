// ARMOR-NODE-SOLAR-HMI - the first screen of a new board: what it found (chip, memory, screen, touch, bus, network), colour bars in a known order and a mark in each corner to check
// the touch. It calls nothing of the board: the figures come as texts, so the simulator draws it too.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#pragma once
#include <functional>
#include <string>

namespace armor::solarhmi::diag {

struct Info {
  std::string chip;       // model and revision, reset reason
  std::string memory;     // flash, external and internal RAM
  std::string screen;     // the panel timing this image was built for
  std::string touch;      // the controller found
  std::string i2c;        // the addresses that answered on the bus
  std::string ethernet;   // link and address
  std::string wifi;       // the state of the radio chip
  std::string uptime;
};

// Builds the check screen and makes it the one shown. `info` is asked twice a second. `open` is called when the button to leave is touched (the screen shown before comes back by itself).
// `language` may be changed by touching its button: the eight languages follow each other, to see that every font is on the board.
void show(std::function<Info()> info, std::function<void()> open, const std::string& language);
// Shows again the screen there was before and frees the check screen. Not to be called from one of its own events.
void close();

}  // namespace armor::solarhmi::diag
