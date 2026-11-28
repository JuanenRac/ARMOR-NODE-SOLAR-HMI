// ARMOR-NODE-SOLAR-HMI - reads lines from the serial port (UART0) and from the USB of the chip, in a task of its own, and hands each one over. Nothing typed is written back, so a password
// never shows in the log. NOT VERIFIED ON A BOARD.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#pragma once
#include <functional>
#include <string>

namespace armor::solarhmi::serial {

bool start(std::function<void(const std::string& line)> on_line);

}  // namespace armor::solarhmi::serial
