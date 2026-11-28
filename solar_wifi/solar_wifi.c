// ARMOR-NODE-SOLAR-HMI - Wi-Fi through the ESP32-C6 (see include/solar_wifi.h). NOT VERIFIED ON A BOARD.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#include "solar_wifi.h"

#include <stdio.h>
#include <string.h>

#include "esp_check.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"

static const char* TAG = "solar-wifi";
static char s_address[20];
static portMUX_TYPE s_lock = portMUX_INITIALIZER_UNLOCKED;

static void set_address(const char* text) {
  portENTER_CRITICAL(&s_lock);
  strlcpy(s_address, text, sizeof s_address);
  portEXIT_CRITICAL(&s_lock);
}

static void on_event(void* arg, esp_event_base_t base, int32_t id, void* data) {
  (void)arg;
  if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) {
    esp_wifi_connect();
  } else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
    set_address("");
    esp_wifi_connect();
  } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
    const ip_event_got_ip_t* event = (const ip_event_got_ip_t*)data;
    char text[20];
    snprintf(text, sizeof text, IPSTR, IP2STR(&event->ip_info.ip));
    set_address(text);
    ESP_LOGI(TAG, "address %s", text);
  }
}

esp_err_t solar_wifi_start(const char* ssid, const char* password) {
  if (ssid == NULL || ssid[0] == '\0') return ESP_ERR_INVALID_ARG;
  esp_err_t error = esp_netif_init();
  if (error != ESP_OK && error != ESP_ERR_INVALID_STATE) return error;
  error = esp_event_loop_create_default();
  if (error != ESP_OK && error != ESP_ERR_INVALID_STATE) return error;
  // below the priority of the cable network, so that the cable keeps the default route while it has an address
  esp_netif_inherent_config_t base = ESP_NETIF_INHERENT_DEFAULT_WIFI_STA();
  base.route_prio = 20;
  if (esp_netif_create_wifi(WIFI_IF_STA, &base) == NULL) return ESP_ERR_NO_MEM;
  ESP_RETURN_ON_ERROR(esp_wifi_set_default_wifi_sta_handlers(), TAG, "the station handlers");

  const wifi_init_config_t init = WIFI_INIT_CONFIG_DEFAULT();
  ESP_RETURN_ON_ERROR(esp_wifi_init(&init), TAG, "the ESP32-C6 does not answer");
  ESP_RETURN_ON_ERROR(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, on_event, NULL), TAG, "the radio events");
  ESP_RETURN_ON_ERROR(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, on_event, NULL), TAG, "the address events");
  ESP_RETURN_ON_ERROR(esp_wifi_set_storage(WIFI_STORAGE_RAM), TAG, "the storage");
  ESP_RETURN_ON_ERROR(esp_wifi_set_mode(WIFI_MODE_STA), TAG, "station mode");
  wifi_config_t station = {0};
  // the two fields need no final zero when they are full
  memcpy(station.sta.ssid, ssid, strnlen(ssid, sizeof station.sta.ssid));
  if (password != NULL) memcpy(station.sta.password, password, strnlen(password, sizeof station.sta.password));
  ESP_RETURN_ON_ERROR(esp_wifi_set_config(WIFI_IF_STA, &station), TAG, "the network");
  return esp_wifi_start();
}

void solar_wifi_address(char* out, size_t size) {
  portENTER_CRITICAL(&s_lock);
  strlcpy(out, s_address, size);
  portEXIT_CRITICAL(&s_lock);
}
