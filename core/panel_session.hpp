// ARMOR-NODE-SOLAR-HMI - the session of the panel account in ARMOR-SERVER, the same one the other panel and the apps use: POST /api/v1/studio/session with the user and the password gives a
// cookie that goes with every later request. A session that ended is opened again once. A sign-in the server refuses is not tried again at once: the wait doubles up to fifteen minutes, so
// that a wrong password never locks the account by insisting. The HTTP itself is given by the caller.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <utility>

#include "json.hpp"
#include "server_link.hpp"

namespace armor::solarhmi {

struct RawAnswer {
  int status = 0;         // 0: nothing came back
  std::string body;
  std::string cookie;     // "name=value" of a Set-Cookie
};

// `post`: a POST with `body` as JSON; otherwise a GET. `cookie` is sent when it is not empty.
using RawHttp = std::function<RawAnswer(bool post, const std::string& path, const std::string& body, const std::string& cookie)>;

constexpr const char* kSessionPath = "/api/v1/studio/session";
constexpr std::int64_t kSignInWaitMs = 30000;
constexpr std::int64_t kSignInWaitMaxMs = 900000;

class PanelSession {
 public:
  PanelSession(RawHttp http, std::string user, std::string password) : http_(std::move(http)), user_(std::move(user)), password_(std::move(password)) {}

  HttpAnswer get(const std::string& path, std::int64_t now_ms) {
    if (cookie_.empty()) {
      const int signed_in = sign_in(now_ms);
      if (signed_in != 200) return HttpAnswer{signed_in, ""};
      return asked(path);
    }
    HttpAnswer answer = asked(path);
    if (answer.status != 401) return answer;
    cookie_.clear();
    const int signed_in = sign_in(now_ms);
    if (signed_in != 200) return HttpAnswer{signed_in, ""};
    return asked(path);
  }

  bool signed_in() const { return !cookie_.empty(); }
  std::int64_t waiting_ms(std::int64_t now_ms) const { return now_ms < retry_at_ ? retry_at_ - now_ms : 0; }

 private:
  HttpAnswer asked(const std::string& path) {
    RawAnswer answer = http_(false, path, "", cookie_);
    return HttpAnswer{answer.status, std::move(answer.body)};
  }

  // 200 when there is a session, 0 when nothing answered, 401 otherwise (also while it waits after a refusal).
  int sign_in(std::int64_t now_ms) {
    if (now_ms < retry_at_) return 401;
    json::Writer w;
    w.begin_object().field("username", user_).field("password", password_).end_object();
    const RawAnswer answer = http_(true, kSessionPath, w.str(), "");
    if (answer.status == 0) return 0;
    if ((answer.status == 200 || answer.status == 201) && !answer.cookie.empty()) {
      cookie_ = answer.cookie;
      wait_ms_ = 0;
      retry_at_ = 0;
      return 200;
    }
    wait_ms_ = wait_ms_ == 0 ? kSignInWaitMs : (wait_ms_ * 2 > kSignInWaitMaxMs ? kSignInWaitMaxMs : wait_ms_ * 2);
    retry_at_ = now_ms + wait_ms_;
    return 401;
  }

  RawHttp http_;
  std::string user_, password_, cookie_;
  std::int64_t wait_ms_ = 0, retry_at_ = 0;
};

}  // namespace armor::solarhmi
