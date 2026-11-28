// ARMOR-NODE-SOLAR-HMI - the settings in flash (see store.hpp). NOT VERIFIED ON A BOARD.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#include "store.hpp"

#include <string>

#include "bootloader_random.h"
#include "esp_log.h"
#include "esp_partition.h"
#include "esp_random.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "secret_seal.hpp"

namespace armor::solarhmi::store {
namespace {

constexpr const char* TAG = "solar-store";
constexpr const char* kSpace = "solarhmi";
constexpr std::size_t kMaxStored = 2048;

seal::Key g_key{};
bool g_ready = false;

bool load_key() {
  const esp_partition_t* part = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_NVS_KEYS, nullptr);
  if (part == nullptr) return false;
  std::uint8_t record[seal::kRecordBytes];
  if (esp_partition_read(part, 0, record, sizeof record) != ESP_OK) return false;
  if (seal::parse_key_record(record, sizeof record, g_key)) return true;
  random_bytes(g_key.data(), g_key.size());
  const auto made = seal::key_record(g_key);
  return esp_partition_erase_range(part, 0, part->size) == ESP_OK && esp_partition_write(part, 0, made.data(), made.size()) == ESP_OK;
}

// The stored text with its sealed values opened; false when there is none.
bool read_text(const char* name, std::string& out, bool& bad) {
  bad = false;
  if (!g_ready) return false;
  nvs_handle_t handle = 0;
  if (nvs_open(kSpace, NVS_READONLY, &handle) != ESP_OK) return false;
  std::size_t size = 0;
  std::string stored;
  bool found = nvs_get_str(handle, name, nullptr, &size) == ESP_OK && size > 1 && size <= kMaxStored;
  if (found) {
    stored.resize(size);
    found = nvs_get_str(handle, name, stored.data(), &size) == ESP_OK;
    stored.resize(size > 0 ? size - 1 : 0);
  }
  nvs_close(handle);
  if (!found) return false;
  if (seal::open_json(g_key, stored, out) == seal::Open::kBad) {
    ESP_LOGE(TAG, "the stored %s cannot be opened with the key of this board", name);
    bad = true;
    return false;
  }
  return true;
}

bool write_text(const char* name, const std::string& plain) {
  if (!g_ready) return false;
  const std::string stored = seal::seal_json(g_key, plain, [](std::uint8_t* nonce) { random_bytes(nonce, seal::kNonceBytes); });
  if (stored.size() >= kMaxStored) return false;
  nvs_handle_t handle = 0;
  if (nvs_open(kSpace, NVS_READWRITE, &handle) != ESP_OK) return false;
  const bool ok = nvs_set_str(handle, name, stored.c_str()) == ESP_OK && nvs_commit(handle) == ESP_OK;
  nvs_close(handle);
  return ok;
}

PinRecord unopenable() {
  PinRecord record;
  record.set = true;
  record.iterations = 1;   // no digits give the digest of zeros
  return record;
}

}  // namespace

void random_bytes(std::uint8_t* out, std::size_t size) {
  bootloader_random_enable();   // this chip has no radio of its own to feed its generator, and the screen uses no analogue input
  esp_fill_random(out, size);
  bootloader_random_disable();
}

bool start() {
  esp_err_t error = nvs_flash_init();
  if (error == ESP_ERR_NVS_NO_FREE_PAGES || error == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    if (nvs_flash_erase() == ESP_OK) error = nvs_flash_init();
  }
  g_ready = error == ESP_OK && load_key();
  if (!g_ready) ESP_LOGE(TAG, "the settings cannot be used: nothing will be saved");
  return g_ready;
}

PanelConfig load_config() {
  std::string text;
  bool bad = false;
  return read_text("config", text, bad) ? read_config(text) : PanelConfig{};
}

bool save_config(const PanelConfig& config) { return write_text("config", config_json(config)); }

PinRecord load_pin() {
  std::string text;
  bool bad = false;
  if (!read_text("pin", text, bad)) return bad ? unopenable() : PinRecord{};
  PinRecord record;
  return read_pin(text, record) ? record : unopenable();
}

bool save_pin(const PinRecord& record) { return write_text("pin", pin_json(record)); }

bool forget() {
  if (!g_ready) return false;
  nvs_handle_t handle = 0;
  if (nvs_open(kSpace, NVS_READWRITE, &handle) != ESP_OK) return false;
  const bool ok = nvs_erase_all(handle) == ESP_OK && nvs_commit(handle) == ESP_OK;
  nvs_close(handle);
  return ok;
}

}  // namespace armor::solarhmi::store
