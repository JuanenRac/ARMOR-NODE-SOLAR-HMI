// ARMOR-NODE-SOLAR-HMI host tests - the reading link to the server, against the made-up installation: the list and each battery, cuts, refusals and answers that cannot be read.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#include <map>
#include <string>
#include <vector>

#include "check.hpp"
#include "plant.hpp"
#include "server_link.hpp"

using namespace armor::solarhmi;

namespace {
struct FakeServer {
  std::map<std::string, HttpAnswer> answers;
  std::vector<std::string> asked;
  Transport transport() {
    return [this](const std::string& path) {
      asked.push_back(path);
      const auto found = answers.find(path);
      return found == answers.end() ? HttpAnswer{404, ""} : found->second;
    };
  }
};
}  // namespace

int main() {
  const std::string whole = sim::server_answer(sim::Scenario::kBmsFault);
  const std::string battery = device_path("solar-sim", "stack-1");

  FakeServer server;
  server.answers[kPanelSolarPath] = {200, whole};
  server.answers[battery] = {200, whole};
  ServerLink link(server.transport());
  CHECK(link.link(0) == Link::kNever);
  CHECK(link.poll(1000) == LinkResult::kOk);
  CHECK(server.asked.size() == 2 && server.asked[0] == kPanelSolarPath && server.asked[1] == battery);
  CHECK(link.link(1000) == Link::kOnline);
  CHECK(link.link(16000) == Link::kStale);
  CHECK(link.snapshot().batteries.size() == 1 && link.snapshot().inverters.size() == 1);
  CHECK(link.snapshot().batteries[0].stack.size() == 4 && !link.snapshot().batteries[0].stale);
  CHECK(link.snapshot().batteries[0].maker.name == "Pylontech" && !link.snapshot().batteries[0].maker.deduced);

  // a cut: what was read stays, and it grows old
  server.answers[kPanelSolarPath] = {0, ""};
  CHECK(link.poll(2000) == LinkResult::kNoServer);
  CHECK(link.snapshot().batteries.size() == 1);
  CHECK(link.link(20000) == Link::kStale);

  server.answers[kPanelSolarPath] = {401, ""};
  CHECK(link.poll(3000) == LinkResult::kSignIn);
  server.answers[kPanelSolarPath] = {403, ""};
  CHECK(link.poll(3000) == LinkResult::kNotAllowed);
  server.answers[kPanelSolarPath] = {500, ""};
  CHECK(link.poll(3000) == LinkResult::kRefused);
  server.answers[kPanelSolarPath] = {200, "{not json"};
  CHECK(link.poll(3000) == LinkResult::kBadAnswer);
  CHECK(link.link(3000) == Link::kOnline);   // still the answer of the first turn, not yet old

  // a battery whose own answer fails has no modules and counts as old: the figures of before are not kept as new
  server.answers[kPanelSolarPath] = {200, whole};
  server.answers[battery] = {500, ""};
  CHECK(link.poll(4000) == LinkResult::kOk);
  CHECK(link.snapshot().batteries[0].stack.empty() && link.snapshot().batteries[0].stale);
  server.answers[battery] = {401, ""};
  CHECK(link.poll(5000) == LinkResult::kSignIn);

  // an answer about another device is not taken for this one
  server.answers[battery] = {200, "{\"devices\":[{\"node_id\":\"other\",\"device\":\"stack-9\",\"kind\":\"battery\",\"stale\":false,\"reading\":{}}],\"totals\":{}}"};
  CHECK(link.poll(6000) == LinkResult::kOk);
  CHECK(link.snapshot().batteries[0].device == "stack-1" && link.snapshot().batteries[0].stale);

  // a name that is not one of the contract is never put in a path
  CHECK(safe_name("solar-sim") && !safe_name("../x") && !safe_name("a b") && !safe_name("") && !safe_name(std::string(65, 'a')));
  server.asked.clear();
  server.answers[kPanelSolarPath] = {200, "{\"devices\":[{\"node_id\":\"../admin\",\"device\":\"x\",\"kind\":\"battery\",\"stale\":false,\"reading\":{}}],\"totals\":{}}"};
  CHECK(link.poll(7000) == LinkResult::kOk);
  CHECK(server.asked.size() == 1);
  FINISH("test_server_link");
}
