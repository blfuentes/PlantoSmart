#include "display.h"

#include <esp_log.h>

static const char* SCREEN_TAG = "DISPLAY";

void display_init(Display* display) {
    // OLED
    ESP_LOGI(SCREEN_TAG, "Initializing SSD1306 OLED display...");
    ESP_LOGI(SCREEN_TAG, "INTERFACE is i2c");
    ESP_LOGI(SCREEN_TAG, "CONFIG_SDA_GPIO=%d", CONFIG_SDA_GPIO);
    ESP_LOGI(SCREEN_TAG, "CONFIG_SCL_GPIO=%d", CONFIG_SCL_GPIO);
    ESP_LOGI(SCREEN_TAG, "CONFIG_RESET_GPIO=%d", CONFIG_RESET_GPIO);
    ESP_LOGI(SCREEN_TAG, "Panel is 128x64");

    i2c_master_init(&display->dev, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO, CONFIG_RESET_GPIO);

    ssd1306_init(&display->dev, 128, 64);
    ssd1306_clear_screen(&display->dev, false);
    ssd1306_contrast(&display->dev, 0xff);
    // char starting_text[] = "LOS VEGGIES...";
    char starting_text[] = "STARTING...";

    ssd1306_display_text(&display->dev, 0, starting_text, 18, false);

    ESP_LOGI(SCREEN_TAG, "SSD1306 initialized");
}