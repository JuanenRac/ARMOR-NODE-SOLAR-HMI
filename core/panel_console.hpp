// ARMOR-NODE-SOLAR-HMI - the lines of the serial console, the only place where the screen is set up for now (it has no keyboard for addresses and passwords). Whoever reaches the console
// holds the board and could write any firmware on it, so it asks for nothing; what it must never do is write a password or a PIN back. This file reads a line and changes the settings;
// what it asks of the board (save, restart, a PIN, an update) is answered as an action.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#pragma once
#include <cstddef>
#include <string>
#include <string_view>

#include "panel_config.hpp"

namespace armor::solarhmi {

enum class ConsoleAction { kNone, kSave, kHelp, kShow, kSetPin, kForgetPin, kForgetAll, kRestart, kUpdate };

struct ConsoleResult {
  ConsoleAction action = ConsoleAction::kNone;
  std::string argument;   // the digits of a new PIN, or the address of an update
  std::string answer;     // what to print; never holds a password or a PIN
};

constexpr std::size_t kMaxLine = 300;

constexpr const char* kConsoleHelp =
    "help | show | restart | forget\n"
    "set server.host NAME | set server.port N | set server.tls on|off | set server.fingerprint HEX|-\n"
    "set server.user NAME | set server.password TEXT | set language en|es|de|fr|it|ja|zh|pt\n"
    "set link on|off | set wifi on|off | set wifi.ssid NAME | set wifi.password TEXT | set update on|off\n"
    "pin set DIGITS | pin forget | update URL\n"
    "Settings are saved at once and used after 'restart'.";

namespace detail {

inline std::string_view trimmed(std::string_view text) {
  while (!text.empty() && (text.front() == ' ' || text.front() == '\t')) text.remove_prefix(1);
  while (!text.empty() && (text.back() == ' ' || text.back() == '\t' || text.back() == '\r' || text.back() == '\n')) text.remove_suffix(1);
  return text;
}

// The first word of a line; `rest` keeps what follows it, spaces inside included (a password may have them).
inline std::string_view first_word(std::string_view text, std::string_view& rest) {
  const std::size_t space = text.find(' ');
  if (space == std::string_view::npos) {
    rest = std::string_view();
    return text;
  }
  rest = trimmed(text.substr(space + 1));
  return text.substr(0, space);
}

inline bool switch_value(std::string_view value, bool& out) {
  if (value == "on") out = true;
  else if (value == "off") out = false;
  else return false;
  return true;
}

inline bool port_value(std::string_view value, int& out) {
  if (value.empty() || value.size() > 5) return false;
  int number = 0;
  for (const char c : value) {
    if (c < '0' || c > '9') return false;
    number = number * 10 + (c - '0');
  }
  if (number < 1 || number > 65535) return false;
  out = number;
  return true;
}

inline bool set_value(PanelConfig& config, std::string_view key, std::string_view value) {
  const auto text = [value](std::string& field, std::size_t most) {
    if (value.size() > most) return false;
    field = value == "-" ? std::string() : std::string(value);
    return true;
  };
  if (value.empty()) return false;
  if (key == "server.host") {
    if (value != "-" && !valid_host(value)) return false;
    return text(config.host, kMaxHost);
  }
  if (key == "server.port") return port_value(value, config.port);
  if (key == "server.tls") return switch_value(value, config.tls);
  if (key == "server.fingerprint") {
    if (value == "-") {
      config.fingerprint.clear();
      return true;
    }
    const std::string normal = server_pin::normalize(value);
    if (normal.empty()) return false;
    config.fingerprint = normal;
    return true;
  }
  if (key == "server.user") return text(config.user, kMaxUser);
  if (key == "server.password") return text(config.password, kMaxPassword);
  if (key == "language") {
    if (!known_language(value)) return false;
    config.language = std::string(value);
    return true;
  }
  if (key == "link") return switch_value(value, config.link_on);
  if (key == "wifi") return switch_value(value, config.wifi_on);
  if (key == "wifi.ssid") return text(config.ssid, kMaxSsid);
  if (key == "wifi.password") return text(config.wifi_password, kMaxWifiPassword);
  if (key == "update") return switch_value(value, config.update_on);
  return false;
}

}  // namespace detail

// The settings as the console shows them: a secret is only said to be set or not.
inline std::string config_summary(const PanelConfig& config, bool pin_set) {
  const auto on = [](bool value) { return value ? "on" : "off"; };
  const auto held = [](const std::string& secret) { return secret.empty() ? "not set" : "set"; };
  const auto said = [](const std::string& value) { return value.empty() ? std::string("-") : value; };
  std::string out;
  out += "server " + said(config.host) + " port " + std::to_string(config.port) + " tls " + on(config.tls) + " fingerprint " + said(config.fingerprint) + "\n";
  out += "user " + said(config.user) + " password " + held(config.password) + "\n";
  out += "language " + config.language + " link " + on(config.link_on) + " update " + on(config.update_on) + "\n";
  out += "wifi " + std::string(on(config.wifi_on)) + " ssid " + said(config.ssid) + " password " + held(config.wifi_password) + "\n";
  out += std::string("pin ") + (pin_set ? "set" : "not set");
  return out;
}

// One line. A line that is not understood changes nothing.
inline ConsoleResult console_line(PanelConfig& config, std::string_view line) {
  ConsoleResult result;
  line = detail::trimmed(line);
  if (line.empty()) return result;
  if (line.size() > kMaxLine) {
    result.answer = "line too long";
    return result;
  }
  std::string_view rest;
  const std::string_view command = detail::first_word(line, rest);
  if (command == "help" && rest.empty()) {
    result.action = ConsoleAction::kHelp;
    result.answer = kConsoleHelp;
  } else if (command == "show" && rest.empty()) {
    result.action = ConsoleAction::kShow;
  } else if (command == "restart" && rest.empty()) {
    result.action = ConsoleAction::kRestart;
    result.answer = "restarting";
  } else if (command == "forget" && rest.empty()) {
    config = PanelConfig{};
    result.action = ConsoleAction::kForgetAll;
    result.answer = "all settings and the PIN are forgotten";
  } else if (command == "pin") {
    std::string_view digits;
    const std::string_view what = detail::first_word(rest, digits);
    if (what == "forget" && digits.empty()) {
      result.action = ConsoleAction::kForgetPin;
    } else if (what == "set" && !digits.empty()) {
      result.action = ConsoleAction::kSetPin;
      result.argument = std::string(digits);
    } else {
      result.answer = "use: pin set DIGITS | pin forget";
    }
  } else if (command == "update") {
    if (!config.update_on) result.answer = "updating is off: set update on";
    else if (!(rest.substr(0, 7) == "http://" || rest.substr(0, 8) == "https://") || rest.find(' ') != std::string_view::npos) result.answer = "use: update http://... or https://...";
    else {
      result.action = ConsoleAction::kUpdate;
      result.argument = std::string(rest);
    }
  } else if (command == "set") {
    std::string_view value;
    const std::string_view key = detail::first_word(rest, value);
    if (detail::set_value(config, key, value)) {
      result.action = ConsoleAction::kSave;
      result.answer = "saved " + std::string(key);
    } else {
      result.answer = "not a setting or not a value it takes (help)";
    }
  } else {
    result.answer = "unknown command (help)";
  }
  return result;
}

}  // namespace armor::solarhmi
