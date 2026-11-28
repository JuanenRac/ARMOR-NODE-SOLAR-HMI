// ARMOR-NODE-SOLAR-HMI - what the screen keeps in flash: the server and the account it reads with, the language, the Wi-Fi network, the two switches (reading the server, updating) and the
// record of the PIN. Everything starts empty and off. Written as one JSON document; its passwords and the salt and hash of the PIN are sealed by the board (core/secret_seal.hpp) before it
// is stored. A document that cannot be read gives the empty settings, never half of them.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

#include "json.hpp"
#include "pin_gate.hpp"
#include "server_pin.hpp"

namespace armor::solarhmi {

struct PanelConfig {
  std::string host;                 // "" until set up
  int port = 18080;
  bool tls = false;
  std::string fingerprint;          // 64 hexadecimal digits of the server's certificate, or ""
  std::string user;
  std::string password;
  std::string language = "en";
  bool link_on = false;             // read the server
  bool wifi_on = false;             // only in an image built with Wi-Fi
  std::string ssid;
  std::string wifi_password;
  bool update_on = false;           // accept the "update" command of the serial console

  bool server_ready() const { return !host.empty() && !user.empty() && !password.empty(); }
};

constexpr std::size_t kMaxHost = 128, kMaxUser = 64, kMaxPassword = 128, kMaxSsid = 32, kMaxWifiPassword = 63;

inline bool known_language(std::string_view code) {
  for (const char* known : {"en", "es", "de", "fr", "it", "ja", "zh", "pt"})
    if (code == known) return true;
  return false;
}

// A host name or an address: nothing that could change the meaning of the address it is put into.
inline bool valid_host(std::string_view host) {
  if (host.empty() || host.size() > kMaxHost) return false;
  for (const char c : host)
    if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '.' || c == '-')) return false;
  return true;
}

inline std::string config_json(const PanelConfig& config) {
  json::Writer w;
  w.begin_object()
      .field("host", config.host)
      .field("port", config.port)
      .field("tls", config.tls)
      .field("fingerprint", config.fingerprint)
      .field("user", config.user)
      .field("password", config.password)
      .field("language", config.language)
      .field("link_on", config.link_on)
      .field("wifi_on", config.wifi_on)
      .field("ssid", config.ssid)
      .field("wifi_password", config.wifi_password)
      .field("update_on", config.update_on)
      .end_object();
  return w.str();
}

inline PanelConfig read_config(std::string_view text) {
  PanelConfig config;
  json::Value document;
  if (!json::parse(text, document) || !document.is_object()) return config;
  const auto bounded = [&document](std::string_view name, std::size_t most) {
    std::string value = document.string_or(name, "");
    if (value.size() > most) value.clear();
    return value;
  };
  config.host = bounded("host", kMaxHost);
  if (!config.host.empty() && !valid_host(config.host)) config.host.clear();
  config.port = static_cast<int>(document.integer_or("port", 18080, 1, 65535));
  config.tls = document.bool_or("tls", false);
  config.fingerprint = server_pin::normalize(document.string_or("fingerprint", ""));
  config.user = bounded("user", kMaxUser);
  config.password = bounded("password", kMaxPassword);
  config.language = document.string_or("language", "en");
  if (!known_language(config.language)) config.language = "en";
  config.link_on = document.bool_or("link_on", false);
  config.wifi_on = document.bool_or("wifi_on", false);
  config.ssid = bounded("ssid", kMaxSsid);
  config.wifi_password = bounded("wifi_password", kMaxWifiPassword);
  config.update_on = document.bool_or("update_on", false);
  return config;
}

namespace detail {

template <std::size_t N>
std::string hex_of(const std::array<std::uint8_t, N>& bytes) {
  return server_pin::to_hex(bytes.data(), bytes.size());
}

template <std::size_t N>
bool bytes_of(std::string_view hex, std::array<std::uint8_t, N>& out) {
  if (hex.size() != 2 * N) return false;
  for (std::size_t i = 0; i < N; ++i) {
    const int high = server_pin::hex_value(hex[2 * i]), low = server_pin::hex_value(hex[2 * i + 1]);
    if (high < 0 || low < 0) return false;
    out[i] = static_cast<std::uint8_t>((high << 4) | low);
  }
  return true;
}

}  // namespace detail

inline std::string pin_json(const PinRecord& record) {
  json::Writer w;
  w.begin_object().field("set", record.set);
  if (record.set)
    w.field("pin_salt", detail::hex_of(record.salt))
        .field("iterations", static_cast<long long>(record.iterations))
        .field("pin_hash", detail::hex_of(record.digest))
        .field("failures", static_cast<long long>(record.failures));
  w.end_object();
  return w.str();
}

// False when a record says there is a PIN and it cannot be read: the caller then keeps the screen locked instead of taking it for a screen without a PIN.
inline bool read_pin(std::string_view text, PinRecord& out) {
  out = PinRecord{};
  json::Value document;
  if (!json::parse(text, document) || !document.is_object()) return false;
  if (!document.bool_or("set", false)) return true;
  PinRecord record;
  record.set = true;
  record.iterations = static_cast<std::uint32_t>(document.integer_or("iterations", 0, 1, 1000000));
  record.failures = static_cast<unsigned>(document.integer_or("failures", 0, 0, 1000000));
  if (record.iterations == 0 || !detail::bytes_of(document.string_or("pin_salt", ""), record.salt) || !detail::bytes_of(document.string_or("pin_hash", ""), record.digest)) return false;
  out = record;
  return true;
}

}  // namespace armor::solarhmi
