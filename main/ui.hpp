// ARMOR-NODE-SOLAR-HMI - the screen, 1024 x 600, drawn with LVGL 9. It calls nothing of the board: what it shows comes from `State` and what a person does leaves through `Actions`,
// so the simulator on a computer draws the very same screen as the firmware.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#pragma once
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>

#include "pin_gate.hpp"
#include "solar_data.hpp"
#include "solar_view.hpp"
#include "write_gate.hpp"

namespace armor::solarhmi::ui {

struct State {
  std::string language = "en";
  std::string version;
  std::string address;            // "" until the screen has one
  Snapshot snapshot;              // the last answer of the server
  std::uint32_t generation = 0;   // goes up with every new answer, so that the screen knows when to draw again
  Link link = Link::kNever;
  Thresholds limits;
  bool server_allows = false;     // the server's switch for battery operations from a screen
  bool account_may = false;       // the server says this account may ask
  bool pin_set = false;
  bool pin_session = false;
  int pin_locked_s = 0;
};

struct Actions {
  std::function<PinResult(const std::string&)> unlock;   // the digits typed; absent: the PIN pad refuses everything
  std::function<void()> lock;
  std::function<void()> diagnostics;                     // present on a board: the settings page offers the board check
  std::function<void()> touched;                          // any touch, so that an idle session ends
  // A confirmed operation on one battery of a stack. Sending an order is not written yet.
  std::function<void(Operation, const std::string& node, const std::string& device, int module)> confirmed;
};

enum class Page { kSummary, kBatteries, kCells, kInverters, kAlarms, kSettings };

// Builds the screen on the active display and starts refreshing it twice a second from `state`.
bool start(std::function<State()> state, Actions actions);
// Goes to a page; `battery` is the index of a stack and `module` the number of a battery in it (the cells page).
void show(Page page, std::size_t battery = 0, int module = 1);
void open_pin();
void type_pin(const std::string& digits);   // as if the keys were touched, without pressing OK
void ask(Operation operation);              // the question before an operation on the battery shown by the cells page
void close_overlay();
void refresh();

}  // namespace armor::solarhmi::ui
