// ARMOR-NODE-SOLAR-HMI - the download and the installation of an update (see ota.hpp). NOT VERIFIED ON A BOARD.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#include "ota.hpp"

#include <algorithm>
#include <memory>

#include "esp_app_desc.h"
#include "esp_crt_bundle.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "mbedtls/sha256.h"
#include "sdkconfig.h"
#include "server_pin.hpp"
#include "update_policy.hpp"

namespace armor::solarhmi::ota {
namespace {

constexpr const char* TAG = "solar-ota";
constexpr std::size_t kChunk = 4096;

#if CONFIG_ESP32P4_SELECTS_REV_LESS_V3
constexpr bool kRevision3 = false;
#else
constexpr bool kRevision3 = true;
#endif
#if CONFIG_SOLAR_LCD_PANEL_OLD
constexpr bool kOldPanel = true;
#else
constexpr bool kOldPanel = false;
#endif

esp_http_client_handle_t open(const std::string& url, std::int64_t& length) {
  esp_http_client_config_t config{};
  config.url = url.c_str();
  config.timeout_ms = 15000;
  config.crt_bundle_attach = esp_crt_bundle_attach;
  config.disable_auto_redirect = true;
  esp_http_client_handle_t client = esp_http_client_init(&config);
  if (client == nullptr) return nullptr;
  if (esp_http_client_open(client, 0) != ESP_OK) {
    esp_http_client_cleanup(client);
    return nullptr;
  }
  length = esp_http_client_fetch_headers(client);
  if (esp_http_client_get_status_code(client) != 200) {
    esp_http_client_cleanup(client);
    return nullptr;
  }
  return client;
}

// A small text file next to the image; "" when it cannot be read.
std::string text_of(const std::string& url) {
  std::int64_t length = 0;
  esp_http_client_handle_t client = open(url, length);
  if (client == nullptr) return "";
  char buffer[256];
  int total = 0;
  while (total < static_cast<int>(sizeof buffer)) {
    const int got = esp_http_client_read(client, buffer + total, static_cast<int>(sizeof buffer) - total);
    if (got <= 0) break;
    total += got;
  }
  esp_http_client_cleanup(client);
  return std::string(buffer, static_cast<std::size_t>(total));
}

}  // namespace

std::string install(const std::string& url) {
  ImageFacts facts;
  facts.published = published_hash(text_of(url + ".sha256"));
  if (facts.published.empty()) return "no_published_hash";
  facts.signature = text_of(url + ".sig2");
  if (facts.signature.empty()) return "no_signature";

  const esp_partition_t* target = esp_ota_get_next_update_partition(nullptr);
  if (target == nullptr) return "no_slot";
  std::int64_t length = 0;
  esp_http_client_handle_t client = open(url, length);
  if (client == nullptr) return "no_answer";
  if (length > static_cast<std::int64_t>(target->size)) {
    esp_http_client_cleanup(client);
    return "too_big";
  }
  esp_ota_handle_t handle = 0;
  if (esp_ota_begin(target, OTA_SIZE_UNKNOWN, &handle) != ESP_OK) {
    esp_http_client_cleanup(client);
    return "ota_begin";
  }
  std::unique_ptr<char[]> chunk(new char[kChunk]);
  mbedtls_sha256_context sha;
  mbedtls_sha256_init(&sha);
  mbedtls_sha256_starts(&sha, 0);
  std::size_t total = 0;
  std::string error;
  while (true) {
    const int got = esp_http_client_read(client, chunk.get(), static_cast<int>(kChunk));
    if (got < 0) { error = "cut"; break; }
    if (got == 0) {
      if (!esp_http_client_is_complete_data_received(client)) error = "cut";
      break;
    }
    total += static_cast<std::size_t>(got);
    if (total > target->size) { error = "too_big"; break; }
    mbedtls_sha256_update(&sha, reinterpret_cast<const unsigned char*>(chunk.get()), static_cast<std::size_t>(got));
    if (esp_ota_write(handle, chunk.get(), static_cast<std::size_t>(got)) != ESP_OK) { error = "ota_write"; break; }
  }
  unsigned char digest[32];
  mbedtls_sha256_finish(&sha, digest);
  mbedtls_sha256_free(&sha);
  esp_http_client_cleanup(client);
  if (!error.empty()) {
    esp_ota_abort(handle);
    return error;
  }
  if (esp_ota_end(handle) != ESP_OK) return "image_invalid";

  esp_app_desc_t description{};
  if (esp_ota_get_partition_description(target, &description) != ESP_OK) return "image_invalid";
  const esp_app_desc_t* running = esp_app_get_description();
  facts.hash = server_pin::to_hex(digest, sizeof digest);
  facts.project = description.project_name;
  facts.running_project = running->project_name;
  facts.version = description.version;
  facts.running_version = running->version;
  facts.board = board_name(kRevision3, kOldPanel);
  const std::string reason = refusal(facts);
  if (!reason.empty()) return reason;   // the slot was written but is never set to start
  if (esp_ota_set_boot_partition(target) != ESP_OK) return "ota_boot";
  ESP_LOGI(TAG, "version %s will start at the next restart", description.version);
  return "";
}

void confirm_running() {
  esp_ota_img_states_t state;
  if (esp_ota_get_state_partition(esp_ota_get_running_partition(), &state) == ESP_OK && state == ESP_OTA_IMG_PENDING_VERIFY) {
    esp_ota_mark_app_valid_cancel_rollback();
    ESP_LOGI(TAG, "the new version is confirmed");
  }
}

}  // namespace armor::solarhmi::ota
