// ARMOR-NODE-SOLAR-HMI - the HTTP side of the reading link (see link.hpp). Over https:// the server's certificate must be the one whose fingerprint was set, or be vouched for by the usual
// certificate authorities (core/server_pin.hpp): it is never accepted unchecked. NOT VERIFIED ON A BOARD.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#include "link.hpp"

#include <algorithm>
#include <atomic>
#include <mutex>
#include <optional>
#include <string>

#include "esp_crt_bundle.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mbedtls/sha256.h"
#include "mbedtls/ssl.h"
#include "mbedtls/x509_crt.h"
#include "panel_session.hpp"
#include "server_link.hpp"
#include "server_pin.hpp"

namespace armor::solarhmi::net {
namespace {

constexpr const char* TAG = "solar-link";
constexpr std::size_t kMaxAnswer = 64 * 1024;   // the most the JSON reader takes
constexpr int kTimeoutMs = 6000;
constexpr int kTurnMs = 5000;

std::mutex g_lock;
View g_view;                              // under g_lock
std::int64_t g_last_answer_ms = -1;       // under g_lock
std::atomic<const char*> g_result{"off"};
PanelConfig g_config;                     // written once, before the task starts
server_pin::PinContext g_pin;

std::int64_t now_ms() { return esp_timer_get_time() / 1000; }

esp_err_t on_event(esp_http_client_event_t* event) {
  auto* answer = static_cast<RawAnswer*>(event->user_data);
  if (answer == nullptr) return ESP_OK;
  if (event->event_id == HTTP_EVENT_ON_HEADER && event->header_key != nullptr && event->header_value != nullptr) {
    std::string key = event->header_key;
    for (char& c : key) c = c >= 'A' && c <= 'Z' ? static_cast<char>(c - 'A' + 'a') : c;
    if (key == "set-cookie" && answer->cookie.empty()) {
      const std::string value = event->header_value;
      answer->cookie = value.substr(0, value.find(';'));
    }
  } else if (event->event_id == HTTP_EVENT_ON_DATA && event->data != nullptr) {
    // one byte more than the reader takes is kept, so that a longer answer is refused instead of being read cut
    const std::size_t room = kMaxAnswer + 1 > answer->body.size() ? kMaxAnswer + 1 - answer->body.size() : 0;
    answer->body.append(static_cast<const char*>(event->data), std::min(room, static_cast<std::size_t>(event->data_len)));
  }
  return ESP_OK;
}

int pin_verify(void*, mbedtls_x509_crt* certificate, int depth, uint32_t* flags) {
  if (depth != 0) {
    *flags = 0;   // the certificate of the server is the one compared, whoever signed it
    return 0;
  }
  std::uint8_t digest[server_pin::kDigestBytes];
  if (mbedtls_sha256(certificate->raw.p, certificate->raw.len, digest, 0) != 0) return MBEDTLS_ERR_X509_CERT_VERIFY_FAILED;
  if (server_pin::matches(digest, g_pin.want)) {
    *flags = 0;
    return 0;
  }
  return MBEDTLS_ERR_X509_CERT_VERIFY_FAILED;
}

esp_err_t pin_attach(void* configuration) {
  const esp_err_t attached = esp_crt_bundle_attach(configuration);
  if (attached != ESP_OK) return attached;
  mbedtls_ssl_conf_verify(static_cast<mbedtls_ssl_config*>(configuration), pin_verify, nullptr);
  return ESP_OK;
}

RawAnswer http(bool post, const std::string& path, const std::string& body, const std::string& cookie) {
  RawAnswer answer;
  const std::string url = std::string(g_config.tls ? "https://" : "http://") + g_config.host + ":" + std::to_string(g_config.port) + path;
  esp_http_client_config_t config{};
  config.url = url.c_str();
  config.method = post ? HTTP_METHOD_POST : HTTP_METHOD_GET;
  config.timeout_ms = kTimeoutMs;
  config.event_handler = on_event;
  config.user_data = &answer;
  config.disable_auto_redirect = true;
  const server_pin::Trust trust = server_pin::trust_for(g_config.tls, g_config.fingerprint, "");
  std::optional<server_pin::PinContext::Scope> scope;
  if (trust == server_pin::Trust::kFingerprint) {
    scope.emplace(g_pin, g_config.fingerprint, false);
    config.crt_bundle_attach = pin_attach;
  } else if (trust == server_pin::Trust::kAuthorities) {
    config.crt_bundle_attach = esp_crt_bundle_attach;
  }
  esp_http_client_handle_t client = esp_http_client_init(&config);
  if (client == nullptr) return answer;
  if (!body.empty()) {
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_header(client, "X-Requested-With", "armor");
    esp_http_client_set_post_field(client, body.c_str(), static_cast<int>(body.size()));
  }
  if (!cookie.empty()) esp_http_client_set_header(client, "Cookie", cookie.c_str());
  if (esp_http_client_perform(client) == ESP_OK) answer.status = esp_http_client_get_status_code(client);
  esp_http_client_cleanup(client);
  return answer;
}

const char* name_of(LinkResult result) {
  switch (result) {
    case LinkResult::kOk: return "reading";
    case LinkResult::kNoServer: return "no answer from the server";
    case LinkResult::kSignIn: return "the sign-in is refused";
    case LinkResult::kNotAllowed: return "not allowed: is panel.solar on in the server?";
    case LinkResult::kBadAnswer: return "the answer cannot be read";
    default: return "the server refuses";
  }
}

void task(void*) {
  PanelSession session(http, g_config.user, g_config.password);
  ServerLink link([&session](const std::string& path) { return session.get(path, now_ms()); });
  while (true) {
    const std::int64_t started = now_ms();
    const LinkResult result = link.poll(started);
    if (name_of(result) != g_result.load()) ESP_LOGI(TAG, "%s", name_of(result));
    g_result.store(name_of(result));
    if (result == LinkResult::kOk) {
      std::lock_guard<std::mutex> guard(g_lock);
      g_view.snapshot = link.snapshot();
      ++g_view.generation;
      g_last_answer_ms = now_ms();
    }
    vTaskDelay(pdMS_TO_TICKS(kTurnMs));
  }
}

}  // namespace

bool start(const PanelConfig& config) {
  if (!config.link_on || !config.server_ready()) return false;
  g_config = config;
  g_result.store("starting");
  return xTaskCreate(task, "solar-link", 12288, nullptr, 4, nullptr) == pdPASS;
}

View view() {
  std::lock_guard<std::mutex> guard(g_lock);
  View out = g_view;
  out.link = link_of(g_last_answer_ms < 0 ? std::optional<std::int64_t>() : std::optional<std::int64_t>(g_last_answer_ms), now_ms());
  return out;
}

const char* last_result() { return g_result.load(); }

}  // namespace armor::solarhmi::net
