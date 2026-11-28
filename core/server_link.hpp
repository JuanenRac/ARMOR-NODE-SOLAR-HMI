// ARMOR-NODE-SOLAR-HMI - the reading link to ARMOR-SERVER: the list of GET /api/v1/panel/solar and then each battery with its modules and cells, one device at a time.
// The transport is given by the caller (the account of the wall panel and its session are its business): it answers a GET of a path with a status and a text. Nothing here writes to
// the server, and the firmware does not use it yet: it is tried on a computer against the made-up installation of the simulator.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#pragma once
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <utility>

#include "solar_view.hpp"

namespace armor::solarhmi {

struct HttpAnswer {
  int status = 0;   // 0: nothing came back
  std::string body;
};

using Transport = std::function<HttpAnswer(const std::string& path)>;

// Why the last turn ended: the screen says which, so that a person knows whether to sign in, to turn the option on in the server or to look at the network.
enum class LinkResult {
  kOk,
  kNoServer,      // nothing came back
  kSignIn,        // 401: the session of the panel account is missing or over
  kNotAllowed,    // 403: the option panel.solar is off, or the account may not read
  kRefused,       // any other status
  kBadAnswer,     // an answer that cannot be read
};

constexpr const char* kPanelSolarPath = "/api/v1/panel/solar";

// A name that can be put in a path as it is: the ones the contract allows for a node and a device.
inline bool safe_name(const std::string& name) {
  if (name.empty() || name.size() > 64) return false;
  for (const char c : name)
    if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '-')) return false;
  return true;
}

inline std::string device_path(const std::string& node, const std::string& device) { return std::string(kPanelSolarPath) + "/devices/" + node + "/" + device; }

inline LinkResult result_of(int status) {
  if (status == 200) return LinkResult::kOk;
  if (status == 0) return LinkResult::kNoServer;
  if (status == 401) return LinkResult::kSignIn;
  if (status == 403) return LinkResult::kNotAllowed;
  return LinkResult::kRefused;
}

class ServerLink {
 public:
  explicit ServerLink(Transport transport, LinkPolicy policy = {}) : transport_(std::move(transport)), policy_(policy) {}

  // One turn of reading. What is kept only changes when the list is read; a battery whose own answer fails stays without modules and is marked as old, never filled with what was read before.
  LinkResult poll(std::int64_t now_ms) {
    const HttpAnswer list = transport_(kPanelSolarPath);
    const LinkResult listed = result_of(list.status);
    if (listed != LinkResult::kOk) return listed;
    Snapshot next;
    if (read_snapshot(list.body, next) != ReadError::kNone) return LinkResult::kBadAnswer;
    for (Battery& battery : next.batteries) {
      if (!safe_name(battery.node) || !safe_name(battery.device)) continue;
      const HttpAnswer one = transport_(device_path(battery.node, battery.device));
      if (one.status == 401) return LinkResult::kSignIn;
      Snapshot detail;
      if (one.status == 200 && read_snapshot(one.body, detail) == ReadError::kNone) {
        bool found = false;
        for (Battery& whole : detail.batteries) {
          if (whole.node != battery.node || whole.device != battery.device) continue;
          battery = std::move(whole);
          found = true;
          break;
        }
        if (found) continue;
      }
      battery.stack.clear();
      battery.stale = true;
    }
    snapshot_ = std::move(next);
    last_answer_ms_ = now_ms;
    return LinkResult::kOk;
  }

  const Snapshot& snapshot() const { return snapshot_; }
  Link link(std::int64_t now_ms) const { return link_of(last_answer_ms_, now_ms, policy_); }

 private:
  Transport transport_;
  LinkPolicy policy_;
  Snapshot snapshot_;
  std::optional<std::int64_t> last_answer_ms_;
};

}  // namespace armor::solarhmi
