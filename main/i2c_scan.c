#include "i2c_scan.h"

#include <driver/i2c_master.h>
#include <esp_err.h>
#include <esp_log.h>

#if CONFIG_I2C_PORT_1
#define APP_I2C_PORT I2C_NUM_1
#else
#define APP_I2C_PORT I2C_NUM_0
#endif

static const char* I2C_SCAN_TAG = "I2C_SCAN";

void scan_i2c_bus(int sda_gpio, int scl_gpio) {
    i2c_master_bus_config_t bus_config = {
        .clk_source                   = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt            = 7,
        .i2c_port                     = APP_I2C_PORT,
        .scl_io_num                   = scl_gpio,
        .sda_io_num                   = sda_gpio,
        .flags.enable_internal_pullup = true,
    };

    i2c_master_bus_handle_t bus_handle;
    esp_err_t err = i2c_new_master_bus(&bus_config, &bus_handle);
    if (err != ESP_OK) {
        ESP_LOGE(I2C_SCAN_TAG, "Failed to create I2C bus on SDA=%d SCL=%d: %s", sda_gpio, scl_gpio,
                 esp_err_to_name(err));
        return;
    }

    ESP_LOGI(I2C_SCAN_TAG, "Scanning I2C bus on SDA=%d SCL=%d", sda_gpio, scl_gpio);

    int found = 0;
    for (int addr = 0x03; addr <= 0x77; addr++) {
        err = i2c_master_probe(bus_handle, addr, 50);
        if (err == ESP_OK) {
            ESP_LOGI(I2C_SCAN_TAG, "Found I2C device at 0x%02X", addr);
            found++;
        }
    }

    if (found == 0) {
        ESP_LOGW(I2C_SCAN_TAG, "No I2C devices found");
    }

    err = i2c_del_master_bus(bus_handle);
    if (err != ESP_OK) {
        ESP_LOGW(I2C_SCAN_TAG, "Failed to delete I2C scan bus: %s", esp_err_to_name(err));
    }
}
