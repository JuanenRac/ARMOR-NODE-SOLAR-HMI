// The settings of the screen, the lines of its serial console and its session in the server.
#include <string>
#include <vector>

#include "check.hpp"
#include "panel_config.hpp"
#include "panel_console.hpp"
#include "panel_session.hpp"
#include "update_policy.hpp"

using namespace armor::solarhmi;

namespace {

void test_config() {
  const PanelConfig empty;
  CHECK(!empty.link_on && !empty.wifi_on && !empty.update_on && !empty.server_ready());
  PanelConfig config;
  config.host = "armor.local";
  config.port = 8443;
  config.tls = true;
  config.fingerprint = std::string(64, 'a');
  config.user = "wall";
  config.password = "a password with spaces";
  config.language = "pt";
  config.link_on = true;
  config.ssid = "home";
  config.wifi_password = "secret";
  const PanelConfig back = read_config(config_json(config));
  CHECK(back.host == config.host && back.port == 8443 && back.tls && back.fingerprint == config.fingerprint);
  CHECK(back.user == "wall" && back.password == config.password && back.language == "pt" && back.link_on && !back.wifi_on && back.server_ready());
  CHECK(back.ssid == "home" && back.wifi_password == "secret" && !back.update_on);
  CHECK(read_config("not json").host.empty());
  const PanelConfig bad = read_config("{\"host\":\"a/b\",\"port\":99999,\"language\":\"xx\",\"fingerprint\":\"12\"}");
  CHECK(bad.host.empty() && bad.port == 8080 && bad.language == "en" && bad.fingerprint.empty());
}

void test_pin_record() {
  PinGate gate;
  Salt salt{};
  salt[0] = 7;
  CHECK(gate.set_pin("4821", salt, 0));
  PinRecord back;
  CHECK(read_pin(pin_json(gate.record()), back));
  CHECK(back.set && back.salt == salt && back.digest == gate.record().digest && back.iterations == gate.record().iterations);
  PinGate restored;
  restored.restore(back, 0);
  CHECK(restored.enter("4821", 1) == PinResult::kOk);
  CHECK(read_pin(pin_json(PinRecord{}), back) && !back.set);
  CHECK(!read_pin("{\"set\":true,\"iterations\":10,\"pin_salt\":\"00\",\"pin_hash\":\"00\"}", back));
  CHECK(!read_pin("", back));
}

void test_console() {
  PanelConfig config;
  CHECK(console_line(config, "set server.host 192.168.1.10").action == ConsoleAction::kSave);
  CHECK(console_line(config, "set server.port 8443").action == ConsoleAction::kSave);
  CHECK(console_line(config, "set server.tls on").action == ConsoleAction::kSave);
  CHECK(console_line(config, "set server.user wall\r\n").action == ConsoleAction::kSave);
  const ConsoleResult password = console_line(config, "set server.password two words");
  CHECK(password.action == ConsoleAction::kSave && password.answer.find("two words") == std::string::npos);
  CHECK(config.host == "192.168.1.10" && config.port == 8443 && config.tls && config.user == "wall" && config.password == "two words");
  CHECK(config_summary(config, false).find("two words") == std::string::npos);
  for (const char* wrong : {"set server.host a/b", "set server.port 0", "set server.port 70000", "set server.tls yes", "set language xx", "set server.fingerprint 12", "set nothing 1", "set link"}) {
    const PanelConfig before = config;
    CHECK(console_line(config, wrong).action == ConsoleAction::kNone);
    CHECK(config_json(before) == config_json(config));
  }
  CHECK(console_line(config, "update http://host/app.bin").action == ConsoleAction::kNone);   // off by default
  CHECK(console_line(config, "set update on").action == ConsoleAction::kSave);
  CHECK(console_line(config, "update ftp://host/app.bin").action == ConsoleAction::kNone);
  const ConsoleResult update = console_line(config, "update http://host/app.bin");
  CHECK(update.action == ConsoleAction::kUpdate && update.argument == "http://host/app.bin");
  const ConsoleResult pin = console_line(config, "pin set 4821");
  CHECK(pin.action == ConsoleAction::kSetPin && pin.argument == "4821" && pin.answer.empty());
  CHECK(console_line(config, "pin forget").action == ConsoleAction::kForgetPin);
  CHECK(console_line(config, "pin").action == ConsoleAction::kNone);
  CHECK(console_line(config, std::string(400, 'x')).action == ConsoleAction::kNone);
  CHECK(console_line(config, "   ").answer.empty());
  CHECK(console_line(config, "forget").action == ConsoleAction::kForgetAll && config.host.empty());
}

void test_session() {
  int sign_ins = 0, reads = 0;
  bool accept = false, expired = false;
  PanelSession session(
      [&](bool post, const std::string& path, const std::string& body, const std::string& cookie) {
        RawAnswer answer;
        if (post) {
          ++sign_ins;
          CHECK(path == kSessionPath && body.find("\"username\":\"wall\"") != std::string::npos && cookie.empty());
          answer.status = accept ? 201 : 401;
          if (accept) answer.cookie = "armor=1";
          return answer;
        }
        ++reads;
        CHECK(cookie == "armor=1");
        answer.status = expired ? 401 : 200;
        expired = false;
        return answer;
      },
      "wall", "secret");
  CHECK(session.get("/x", 0).status == 401 && sign_ins == 1 && reads == 0);
  CHECK(session.get("/x", 1000).status == 401 && sign_ins == 1);                       // it waits
  CHECK(session.waiting_ms(1000) == kSignInWaitMs - 1000);
  CHECK(session.get("/x", kSignInWaitMs).status == 401 && sign_ins == 2);
  CHECK(session.waiting_ms(kSignInWaitMs) == 2 * kSignInWaitMs);                         // and longer each time
  accept = true;
  CHECK(session.get("/x", 10 * kSignInWaitMs).status == 200 && sign_ins == 3 && reads == 1 && session.signed_in());
  expired = true;
  CHECK(session.get("/x", 10 * kSignInWaitMs + 1).status == 200 && sign_ins == 4 && reads == 3);   // a session that ended is opened again once

  PanelSession silent([](bool, const std::string&, const std::string&, const std::string&) { return RawAnswer{}; }, "wall", "secret");
  CHECK(silent.get("/x", 0).status == 0);
  CHECK(silent.waiting_ms(0) == 0);   // nothing answered: the next turn tries again
}

void test_update() {
  CHECK(board_name(false, false) == "jc1060-rev1-newpanel" && board_name(true, true) == "jc1060-rev3-oldpanel");
  const std::string hash(64, 'a');
  CHECK(published_hash(std::string(64, 'A') + "  app.bin
") == hash);
  CHECK(published_hash(hash) == hash);
  CHECK(published_hash("abc").empty() && published_hash(hash + "f").empty() && published_hash(std::string(64, 'g')).empty());
  ImageFacts facts{"armor_solar_hmi", "armor_solar_hmi", "jc1060-rev1-newpanel", "0.0.9", "0.0.3", hash, hash, std::string(128, '0')};
  CHECK(refusal(facts) == "bad_signature");                 // nobody but the owner of the key can make a signature that passes
  facts.version = "0.0.3";
  CHECK(refusal(facts) == "not_newer");
  facts.version = "0.0.9";
  facts.project = "armor_hmi";
  CHECK(refusal(facts) == "other_program");
  facts.project = "armor_solar_hmi";
  facts.hash = std::string(64, 'b');
  CHECK(refusal(facts) == "hash_differs");
  facts.published.clear();
  CHECK(refusal(facts) == "no_published_hash");
}

}  // namespace

int main() {
  test_config();
  test_pin_record();
  test_console();
  test_session();
  test_update();
  FINISH("test_panel");
}
