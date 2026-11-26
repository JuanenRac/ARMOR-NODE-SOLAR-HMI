// ARMOR-NODE-SOLAR-HMI simulator - prints what a made-up installation would make ARMOR-SERVER answer.
//   solar_sim                     every scenario, one answer per line
//   solar_sim cell-high [step]    one scenario
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include "plant.hpp"

int main(int argc, char** argv) {
  using namespace armor::solarhmi::sim;
  if (argc < 2) {
    for (const Named& known : kScenarios) std::puts(server_answer(known.scenario).c_str());
    return 0;
  }
  Scenario scenario = Scenario::kNormal;
  if (!scenario_named(argv[1], scenario)) {
    std::fputs("unknown scenario; one of:", stderr);
    for (const Named& known : kScenarios) std::fprintf(stderr, " %s", known.name);
    std::fputs("\n", stderr);
    return 2;
  }
  const std::uint32_t step = argc > 2 ? static_cast<std::uint32_t>(std::strtoul(argv[2], nullptr, 10)) : 0u;
  std::puts(server_answer(scenario, step).c_str());
  return 0;
}
