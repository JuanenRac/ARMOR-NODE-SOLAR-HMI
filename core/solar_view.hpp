// ARMOR-NODE-SOLAR-HMI - the decisions behind what is drawn: when the data is old, how worried a cell, a module or a battery looks, and how a mark blinks.
// Nothing here draws. An alarm is never said with colour or blinking alone: every level has an icon and a word.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#pragma once
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

#include "solar_data.hpp"

namespace armor::solarhmi {

// ---- the link to the server ----
struct LinkPolicy {
  std::int64_t stale_after_ms = 15000;
};

enum class Link { kNever, kOnline, kStale };

inline Link link_of(std::optional<std::int64_t> last_answer_ms, std::int64_t now_ms, const LinkPolicy& policy = {}) {
  if (!last_answer_ms.has_value()) return Link::kNever;
  const std::int64_t age = now_ms - *last_answer_ms;
  return age >= 0 && age < policy.stale_after_ms ? Link::kOnline : Link::kStale;
}

// Without a fresh answer the screen only reads: what it shows is marked as old and no order is offered.
inline bool read_only(Link link) { return link != Link::kOnline; }

// ---- thresholds (settings of the screen; the starting values are the ones of the server's energy alarms where it has one) ----
struct Thresholds {
  double cell_high_v = 3.55;      // lithium iron phosphate
  double cell_low_v = 2.90;
  int spread_warn_mv = 100;
  double temp_high_c = 55;
  double temp_low_c = 0;
  double soc_low_percent = 20;
};

struct CellStats {
  double min_v = 0, max_v = 0;
  int delta_mv = 0;
  std::size_t min_index = 0, max_index = 0;
};

inline std::optional<CellStats> cell_stats(const std::vector<double>& cells) {
  if (cells.empty()) return std::nullopt;
  CellStats stats;
  stats.min_v = stats.max_v = cells.front();
  for (std::size_t i = 1; i < cells.size(); ++i) {
    if (cells[i] < stats.min_v) { stats.min_v = cells[i]; stats.min_index = i; }
    if (cells[i] > stats.max_v) { stats.max_v = cells[i]; stats.max_index = i; }
  }
  stats.delta_mv = static_cast<int>(std::lround((stats.max_v - stats.min_v) * 1000.0));
  return stats;
}

enum class Level { kOk = 0, kWarn = 1, kAlarm = 2 };

inline Level worst(Level a, Level b) { return static_cast<int>(a) >= static_cast<int>(b) ? a : b; }

inline Level cell_level(double volts, const Thresholds& limits) {
  return volts >= limits.cell_high_v || volts <= limits.cell_low_v ? Level::kAlarm : Level::kOk;
}

inline Level module_level(const Module& module, const Thresholds& limits) {
  if (!module.present) return Level::kWarn;
  Level level = Level::kOk;
  for (const double cell : module.cells_v) level = worst(level, cell_level(cell, limits));
  if (module.temperature_c && (*module.temperature_c >= limits.temp_high_c || *module.temperature_c <= limits.temp_low_c)) level = Level::kAlarm;
  const std::optional<CellStats> stats = cell_stats(module.cells_v);
  if (stats && stats->delta_mv >= limits.spread_warn_mv) level = worst(level, Level::kWarn);
  if (module.soc_percent && *module.soc_percent <= limits.soc_low_percent) level = worst(level, Level::kWarn);
  return level;
}

inline Level battery_level(const Battery& battery, const Thresholds& limits) {
  Level level = battery.alarm || battery.protecting ? Level::kAlarm : Level::kOk;
  for (const Module& module : battery.stack) level = worst(level, module_level(module, limits));
  if (battery.soc_percent && *battery.soc_percent <= limits.soc_low_percent) level = worst(level, Level::kWarn);
  if (battery.stale) level = worst(level, Level::kWarn);
  return level;
}

// ---- marks and blinking ----
struct Mark {
  const char* icon;   // name of a symbol of the screen
  const char* word;   // key of the word in the eight languages
};

inline Mark mark_of(Level level) {
  switch (level) {
    case Level::kAlarm: return {"alert", "level_alarm"};
    case Level::kWarn: return {"warning", "level_warning"};
    default: return {"check", "level_ok"};
  }
}

constexpr std::int64_t kMinBlinkHalfPeriodMs = 250;   // never more than two blinks a second

// Whether a mark is lit now. Only an alarm blinks; the others stay lit.
inline bool lit(Level level, std::int64_t now_ms, std::int64_t half_period_ms = 500) {
  if (level != Level::kAlarm) return true;
  const std::int64_t half = half_period_ms < kMinBlinkHalfPeriodMs ? kMinBlinkHalfPeriodMs : half_period_ms;
  return (now_ms / half) % 2 == 0;
}

}  // namespace armor::solarhmi
