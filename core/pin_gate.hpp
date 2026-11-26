// ARMOR-NODE-SOLAR-HMI - the PIN of the screen: kept only as a salted, iterated hash (PBKDF2-HMAC-SHA256), with a growing lock after wrong tries and a short session that also
// ends when nobody touches the screen. The digits are never stored, shown or logged.
// A hash of a few digits does not resist somebody who can read the flash: the board has to encrypt it, and the server still decides every order.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

#include "hmac_sha256.hpp"

namespace armor::solarhmi {

struct PinPolicy {
  std::size_t min_digits = 4;
  std::size_t max_digits = 10;
  std::uint32_t iterations = 20000;
  unsigned attempts = 3;                   // wrong tries before each lock
  std::int64_t lock_ms = 60000;            // the first lock; each following one doubles
  std::int64_t lock_max_ms = 900000;
  std::int64_t session_ms = 300000;        // a session never lasts longer than this
  std::int64_t idle_ms = 60000;            // and ends after this long without a touch
};

using Salt = std::array<std::uint8_t, 16>;

// What is written in flash. The count of wrong tries is part of it, so that switching the screen off does not give new tries.
struct PinRecord {
  bool set = false;
  Salt salt{};
  std::uint32_t iterations = 0;
  hash::Digest digest{};
  unsigned failures = 0;
};

inline hash::Digest pbkdf2_block(std::string_view secret, const std::uint8_t* salt, std::size_t salt_size, std::uint32_t iterations) {
  const auto* key = reinterpret_cast<const std::uint8_t*>(secret.data());
  std::string message(reinterpret_cast<const char*>(salt), salt_size);
  message.append("\0\0\0\1", 4);
  hash::Digest step = hash::hmac_sha256(key, secret.size(), message);
  hash::Digest out = step;
  for (std::uint32_t i = 1; i < iterations; ++i) {
    step = hash::hmac_sha256(key, secret.size(), std::string_view(reinterpret_cast<const char*>(step.data()), step.size()));
    for (std::size_t j = 0; j < out.size(); ++j) out[j] = static_cast<std::uint8_t>(out[j] ^ step[j]);
  }
  return out;
}

enum class PinResult { kOk, kWrong, kLocked, kNotSet, kBadFormat };

class PinGate {
 public:
  explicit PinGate(const PinPolicy& policy = {}) : policy_(policy) {}

  bool well_formed(std::string_view digits) const {
    if (digits.size() < policy_.min_digits || digits.size() > policy_.max_digits) return false;
    for (const char c : digits)
      if (c < '0' || c > '9') return false;
    return true;
  }

  // Sets the first PIN, or a new one inside an open session. `salt` comes from the random source of the board.
  bool set_pin(std::string_view digits, const Salt& salt, std::int64_t now_ms) {
    if (!well_formed(digits) || (record_.set && !unlocked(now_ms))) return false;
    record_.set = true;
    record_.salt = salt;
    record_.iterations = policy_.iterations;
    record_.digest = pbkdf2_block(digits, salt.data(), salt.size(), record_.iterations);
    record_.failures = 0;
    return true;
  }

  // What was in flash at start. A record that was locked starts its lock again.
  void restore(const PinRecord& record, std::int64_t now_ms) {
    record_ = record;
    close();
    locked_until_ = lock_length() > 0 && record_.failures % policy_.attempts == 0 ? now_ms + lock_length() : 0;
  }

  // The caller writes record() to flash after every call, before it shows the answer.
  PinResult enter(std::string_view digits, std::int64_t now_ms) {
    close();
    if (!record_.set) return PinResult::kNotSet;
    if (now_ms < locked_until_) return PinResult::kLocked;
    if (!well_formed(digits)) return PinResult::kBadFormat;
    const hash::Digest typed = pbkdf2_block(digits, record_.salt.data(), record_.salt.size(), record_.iterations);
    unsigned char difference = 0;
    for (std::size_t i = 0; i < typed.size(); ++i) difference = static_cast<unsigned char>(difference | (typed[i] ^ record_.digest[i]));
    if (difference != 0) {
      ++record_.failures;
      if (policy_.attempts > 0 && record_.failures % policy_.attempts == 0) {
        locked_until_ = now_ms + lock_length();
        return PinResult::kLocked;
      }
      return PinResult::kWrong;
    }
    record_.failures = 0;
    open_ = true;
    opened_at_ = touched_at_ = now_ms;
    return PinResult::kOk;
  }

  bool unlocked(std::int64_t now_ms) {
    if (open_ && (now_ms - opened_at_ >= policy_.session_ms || now_ms - touched_at_ >= policy_.idle_ms)) close();
    return open_;
  }

  // A touch keeps an open session from ending for lack of use; it never revives one that has ended.
  void touch(std::int64_t now_ms) {
    if (unlocked(now_ms)) touched_at_ = now_ms;
  }

  // Also called when the screen goes to sleep.
  void close() { open_ = false; }

  std::int64_t locked_for_ms(std::int64_t now_ms) const { return now_ms < locked_until_ ? locked_until_ - now_ms : 0; }
  const PinRecord& record() const { return record_; }

 private:
  std::int64_t lock_length() const {
    if (policy_.attempts == 0 || record_.failures < policy_.attempts) return 0;
    std::int64_t length = policy_.lock_ms;
    for (unsigned lock = 1; lock < record_.failures / policy_.attempts && length < policy_.lock_max_ms; ++lock) length *= 2;
    return length < policy_.lock_max_ms ? length : policy_.lock_max_ms;
  }

  PinPolicy policy_;
  PinRecord record_;
  bool open_ = false;
  std::int64_t opened_at_ = 0, touched_at_ = 0, locked_until_ = 0;
};

}  // namespace armor::solarhmi
