// ARMOR-NODE-SOLAR-HMI - the board code of the Guition JC1060P470C. NOT VERIFIED ON A BOARD: it follows the maker's documents and the interfaces of the components of Espressif.
// The start-up commands of the panel are the default ones of the component esp_lcd_jd9165; the maker's own table is not copied here.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#include "board.h"

#include <stdio.h>
#include <string.h>

#include "board_p4_jc1060.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "driver/ledc.h"
#include "esp_check.h"
#include "esp_eth.h"
#include "esp_event.h"
#include "esp_lcd_jd9165.h"
#include "esp_lcd_mipi_dsi.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_touch_gt911.h"
#include "esp_ldo_regulator.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

static const char* TAG = "board";

static esp_lcd_panel_handle_t s_panel = NULL;
static esp_lcd_panel_io_handle_t s_panel_io = NULL;
static i2c_master_bus_handle_t s_i2c = NULL;
static volatile bool s_link = false;
static char s_address[20] = "";
static portMUX_TYPE s_lock = portMUX_INITIALIZER_UNLOCKED;

// ---- backlight ----
void board_backlight(int percent) {
#if CONFIG_SOLAR_LCD_BACKLIGHT_GPIO >= 0
  static bool ready = false;
  if (!ready) {
    const ledc_timer_config_t timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE, .duty_resolution = LEDC_TIMER_10_BIT, .timer_num = LEDC_TIMER_0, .freq_hz = 5000, .clk_cfg = LEDC_AUTO_CLK};
    const ledc_channel_config_t channel = {
        .gpio_num = CONFIG_SOLAR_LCD_BACKLIGHT_GPIO, .speed_mode = LEDC_LOW_SPEED_MODE, .channel = LEDC_CHANNEL_0, .timer_sel = LEDC_TIMER_0, .duty = 0, .hpoint = 0};
    if (ledc_timer_config(&timer) != ESP_OK || ledc_channel_config(&channel) != ESP_OK) {
      ESP_LOGE(TAG, "backlight: the PWM on GPIO%d could not be set up", CONFIG_SOLAR_LCD_BACKLIGHT_GPIO);
      return;
    }
    ready = true;
  }
  if (percent < 0) percent = 0;
  if (percent > 100) percent = 100;
#if CONFIG_SOLAR_LCD_BACKLIGHT_INVERT
  percent = 100 - percent;
#endif
  ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, (1023 * percent) / 100);
  ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
#else
  (void)percent;
#endif
}

// ---- screen ----
esp_err_t board_display_start(void) {
  esp_ldo_channel_handle_t power = NULL;
  const esp_ldo_channel_config_t ldo = {.chan_id = SOLAR_DSI_LDO_CHANNEL, .voltage_mv = SOLAR_DSI_LDO_MV};
  ESP_RETURN_ON_ERROR(esp_ldo_acquire_channel(&ldo, &power), TAG, "the regulator of the display interface");

  esp_lcd_dsi_bus_handle_t bus = NULL;
  const esp_lcd_dsi_bus_config_t bus_config = {.bus_id = 0, .num_data_lanes = 2, .phy_clk_src = 0, .lane_bit_rate_mbps = CONFIG_SOLAR_LCD_LANE_MBPS};
  ESP_RETURN_ON_ERROR(esp_lcd_new_dsi_bus(&bus_config, &bus), TAG, "the MIPI-DSI bus");

  const esp_lcd_dbi_io_config_t io_config = {.virtual_channel = 0, .lcd_cmd_bits = 8, .lcd_param_bits = 8};
  ESP_RETURN_ON_ERROR(esp_lcd_new_panel_io_dbi(bus, &io_config, &s_panel_io), TAG, "the command channel of the panel");

  static esp_lcd_dpi_panel_config_t dpi = {
      .dpi_clk_src = MIPI_DSI_DPI_CLK_SRC_DEFAULT,
      .dpi_clock_freq_mhz = CONFIG_SOLAR_LCD_DPI_CLOCK_MHZ,
      .virtual_channel = 0,
      .pixel_format = LCD_COLOR_PIXEL_FORMAT_RGB565,
      .num_fbs = 1,
      .video_timing =
          {
              .h_size = SOLAR_SCREEN_WIDTH,
              .v_size = SOLAR_SCREEN_HEIGHT,
              .hsync_pulse_width = CONFIG_SOLAR_LCD_HSYNC,
              .hsync_back_porch = CONFIG_SOLAR_LCD_HBP,
              .hsync_front_porch = CONFIG_SOLAR_LCD_HFP,
              .vsync_pulse_width = CONFIG_SOLAR_LCD_VSYNC,
              .vsync_back_porch = CONFIG_SOLAR_LCD_VBP,
              .vsync_front_porch = CONFIG_SOLAR_LCD_VFP,
          },
      .flags.use_dma2d = true,
  };
  static jd9165_vendor_config_t vendor;
  vendor.init_cmds = NULL;   // the default table of the component
  vendor.init_cmds_size = 0;
  vendor.mipi_config.dsi_bus = bus;
  vendor.mipi_config.dpi_config = &dpi;
  const esp_lcd_panel_dev_config_t panel_config = {
      .reset_gpio_num = CONFIG_SOLAR_LCD_RST_GPIO, .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB, .bits_per_pixel = 16, .vendor_config = &vendor};
  ESP_RETURN_ON_ERROR(esp_lcd_new_panel_jd9165(s_panel_io, &panel_config, &s_panel), TAG, "the panel driver");
  ESP_RETURN_ON_ERROR(esp_lcd_panel_reset(s_panel), TAG, "the reset of the panel");
  ESP_RETURN_ON_ERROR(esp_lcd_panel_init(s_panel), TAG, "the start of the panel");

  ESP_LOGI(TAG, "panel started: %d x %d, clock %d MHz, lanes %d Mbps, h %d/%d/%d, v %d/%d/%d, reset GPIO%d", SOLAR_SCREEN_WIDTH, SOLAR_SCREEN_HEIGHT, CONFIG_SOLAR_LCD_DPI_CLOCK_MHZ,
           CONFIG_SOLAR_LCD_LANE_MBPS, CONFIG_SOLAR_LCD_HSYNC, CONFIG_SOLAR_LCD_HBP, CONFIG_SOLAR_LCD_HFP, CONFIG_SOLAR_LCD_VSYNC, CONFIG_SOLAR_LCD_VBP, CONFIG_SOLAR_LCD_VFP,
           CONFIG_SOLAR_LCD_RST_GPIO);
#if CONFIG_SOLAR_LCD_TEST_BARS_MS > 0
  if (esp_lcd_dpi_panel_set_pattern(s_panel, MIPI_DSI_PATTERN_BAR_VERTICAL) == ESP_OK) {
    board_backlight(80);
    ESP_LOGI(TAG, "colour bars of the interface for %d ms: if the glass stays dark or shows noise here, the panel table or the timing is wrong", CONFIG_SOLAR_LCD_TEST_BARS_MS);
    vTaskDelay(pdMS_TO_TICKS(CONFIG_SOLAR_LCD_TEST_BARS_MS));
    esp_lcd_dpi_panel_set_pattern(s_panel, MIPI_DSI_PATTERN_NONE);
  }
#endif
  return ESP_OK;
}

esp_err_t board_lvgl_start(lv_display_t** display) {
  if (s_panel == NULL) return ESP_ERR_INVALID_STATE;
  lvgl_port_cfg_t port = ESP_LVGL_PORT_INIT_CONFIG();
  port.task_stack = 16384;
  ESP_RETURN_ON_ERROR(lvgl_port_init(&port), TAG, "the LVGL task");
  const lvgl_port_display_cfg_t config = {
      .io_handle = s_panel_io,
      .panel_handle = s_panel,
      .buffer_size = SOLAR_SCREEN_WIDTH * 100,
      .double_buffer = true,
      .hres = SOLAR_SCREEN_WIDTH,
      .vres = SOLAR_SCREEN_HEIGHT,
      .color_format = LV_COLOR_FORMAT_RGB565,
      .flags = {.buff_spiram = true},
  };
  const lvgl_port_display_dsi_cfg_t dsi = {.flags = {.avoid_tearing = false}};
  *display = lvgl_port_add_disp_dsi(&config, &dsi);
  return *display != NULL ? ESP_OK : ESP_FAIL;
}

// ---- I2C and touch ----
esp_err_t board_i2c_start(char* found, size_t size) {
  snprintf(found, size, "none");
  const i2c_master_bus_config_t config = {
      .i2c_port = I2C_NUM_0, .sda_io_num = SOLAR_PIN_I2C_SDA, .scl_io_num = SOLAR_PIN_I2C_SCL, .clk_source = I2C_CLK_SRC_DEFAULT, .glitch_ignore_cnt = 7, .flags.enable_internal_pullup = true};
  ESP_RETURN_ON_ERROR(i2c_new_master_bus(&config, &s_i2c), TAG, "the I2C bus");
  size_t used = 0;
  for (int address = 0x08; address < 0x78; ++address) {
    if (i2c_master_probe(s_i2c, (uint16_t)address, 20) != ESP_OK) continue;
    ESP_LOGI(TAG, "I2C: a device answers at 0x%02X", address);
    const int written = snprintf(found + used, size - used, "%s0x%02X", used ? " " : "", address);
    if (written < 0 || (size_t)written >= size - used) break;
    used += (size_t)written;
  }
  if (used == 0) ESP_LOGW(TAG, "I2C: nothing answers on SDA %d / SCL %d", SOLAR_PIN_I2C_SDA, SOLAR_PIN_I2C_SCL);
  return ESP_OK;
}

esp_err_t board_touch_start(lv_display_t* display, int* address) {
  if (s_i2c == NULL || display == NULL) return ESP_ERR_INVALID_STATE;
  int found = 0;
  if (i2c_master_probe(s_i2c, ESP_LCD_TOUCH_IO_I2C_GT911_ADDRESS, 50) == ESP_OK) found = ESP_LCD_TOUCH_IO_I2C_GT911_ADDRESS;
  else if (i2c_master_probe(s_i2c, ESP_LCD_TOUCH_IO_I2C_GT911_ADDRESS_BACKUP, 50) == ESP_OK) found = ESP_LCD_TOUCH_IO_I2C_GT911_ADDRESS_BACKUP;
  if (found == 0) {
    ESP_LOGW(TAG, "touch: no GT911 at 0x5D or 0x14");
    return ESP_ERR_NOT_FOUND;
  }
  esp_lcd_panel_io_i2c_config_t io_config = ESP_LCD_TOUCH_IO_I2C_GT911_CONFIG();
  io_config.dev_addr = (uint32_t)found;
  io_config.scl_speed_hz = 400000;
  esp_lcd_panel_io_handle_t io = NULL;
  ESP_RETURN_ON_ERROR(esp_lcd_new_panel_io_i2c(s_i2c, &io_config, &io), TAG, "the touch channel");
  const esp_lcd_touch_config_t config = {
      .x_max = CONFIG_SOLAR_TOUCH_WIDTH,
      .y_max = CONFIG_SOLAR_TOUCH_HEIGHT,
      .rst_gpio_num = CONFIG_SOLAR_TOUCH_RST_GPIO,
      .int_gpio_num = CONFIG_SOLAR_TOUCH_INT_GPIO,
      .flags = {
#ifdef CONFIG_SOLAR_TOUCH_SWAP_XY
          .swap_xy = 1,
#endif
#ifdef CONFIG_SOLAR_TOUCH_MIRROR_X
          .mirror_x = 1,
#endif
#ifdef CONFIG_SOLAR_TOUCH_MIRROR_Y
          .mirror_y = 1,
#endif
      },
  };
  esp_lcd_touch_handle_t touch = NULL;
  ESP_RETURN_ON_ERROR(esp_lcd_touch_new_i2c_gt911(io, &config, &touch), TAG, "the touch controller");
  const lvgl_port_touch_cfg_t port = {.disp = display, .handle = touch};
  if (lvgl_port_add_touch(&port) == NULL) return ESP_FAIL;
  *address = found;
  ESP_LOGI(TAG, "touch: GT911 at 0x%02X, set up for %d x %d", found, CONFIG_SOLAR_TOUCH_WIDTH, CONFIG_SOLAR_TOUCH_HEIGHT);
  return ESP_OK;
}

// ---- cable network ----
static void set_address(const char* text) {
  portENTER_CRITICAL(&s_lock);
  strlcpy(s_address, text, sizeof s_address);
  portEXIT_CRITICAL(&s_lock);
}

static void on_eth_event(void* arg, esp_event_base_t base, int32_t id, void* data) {
  (void)arg;
  (void)base;
  (void)data;
  if (id == ETHERNET_EVENT_CONNECTED) {
    s_link = true;
    ESP_LOGI(TAG, "ethernet: cable connected");
  } else if (id == ETHERNET_EVENT_DISCONNECTED) {
    s_link = false;
    set_address("");
    ESP_LOGI(TAG, "ethernet: cable removed");
  }
}

static void on_ip_event(void* arg, esp_event_base_t base, int32_t id, void* data) {
  (void)arg;
  (void)base;
  if (id != IP_EVENT_ETH_GOT_IP) return;
  const ip_event_got_ip_t* event = (const ip_event_got_ip_t*)data;
  char text[20];
  snprintf(text, sizeof text, IPSTR, IP2STR(&event->ip_info.ip));
  set_address(text);
  ESP_LOGI(TAG, "ethernet: address %s", text);
}

esp_err_t board_ethernet_start(void) {
#if CONFIG_SOLAR_ETHERNET
  ESP_RETURN_ON_ERROR(esp_netif_init(), TAG, "the network layer");
  ESP_RETURN_ON_ERROR(esp_event_loop_create_default(), TAG, "the event loop");
  const esp_netif_config_t netif_config = ESP_NETIF_DEFAULT_ETH();
  esp_netif_t* netif = esp_netif_new(&netif_config);
  if (netif == NULL) return ESP_ERR_NO_MEM;

  eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
  eth_esp32_emac_config_t emac = ETH_ESP32_EMAC_DEFAULT_CONFIG();
  emac.smi_gpio.mdc_num = SOLAR_PIN_ETH_MDC;
  emac.smi_gpio.mdio_num = SOLAR_PIN_ETH_MDIO;
  emac.clock_config.rmii.clock_mode = EMAC_CLK_EXT_IN;
  emac.clock_config.rmii.clock_gpio = SOLAR_PIN_ETH_REF_CLK;
  esp_eth_mac_t* mac = esp_eth_mac_new_esp32(&emac, &mac_config);
  eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
  phy_config.phy_addr = SOLAR_ETH_PHY_ADDR;
  phy_config.reset_gpio_num = SOLAR_PIN_ETH_PHY_RST;
  esp_eth_phy_t* phy = esp_eth_phy_new_ip101(&phy_config);
  if (mac == NULL || phy == NULL) return ESP_FAIL;

  const esp_eth_config_t config = ETH_DEFAULT_CONFIG(mac, phy);
  esp_eth_handle_t handle = NULL;
  ESP_RETURN_ON_ERROR(esp_eth_driver_install(&config, &handle), TAG, "the IP101 (no answer from the PHY?)");
  ESP_RETURN_ON_ERROR(esp_netif_attach(netif, esp_eth_new_netif_glue(handle)), TAG, "the network interface");
  ESP_RETURN_ON_ERROR(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, on_eth_event, NULL), TAG, "the cable events");
  ESP_RETURN_ON_ERROR(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, on_ip_event, NULL), TAG, "the address events");
  return esp_eth_start(handle);
#else
  return ESP_ERR_NOT_SUPPORTED;
#endif
}

void board_ethernet_address(char* out, size_t size) {
  portENTER_CRITICAL(&s_lock);
  strlcpy(out, s_address, size);
  portEXIT_CRITICAL(&s_lock);
}

void board_ethernet_text(char* out, size_t size) {
  char address[20];
  board_ethernet_address(address, sizeof address);
  if (address[0] != '\0') strlcpy(out, address, size);
  else strlcpy(out, s_link ? "cable, no address" : "no cable", size);
}
