#include "display.h"

#include <esp_log.h>
#include <string.h>

static const char* SCREEN_TAG   = "DISPLAY";
static const int DISPLAY_WIDTH  = 128;
static const int DISPLAY_HEIGHT = 32;
static const char* TITLE        = "~ LOS VEGGIES ~";
static const int TITLE_PAGE     = 0;
static const int LIGHT_PAGE     = 2;
static const int HUMIDITY_PAGE  = 3;

void display_init(Display* display) {
    // OLED
    ESP_LOGI(SCREEN_TAG, "Initializing SSD1306 OLED display...");
    ESP_LOGI(SCREEN_TAG, "INTERFACE is i2c");
    ESP_LOGI(SCREEN_TAG, "CONFIG_SDA_GPIO=%d", CONFIG_SDA_GPIO);
    ESP_LOGI(SCREEN_TAG, "CONFIG_SCL_GPIO=%d", CONFIG_SCL_GPIO);
    ESP_LOGI(SCREEN_TAG, "CONFIG_RESET_GPIO=%d", CONFIG_RESET_GPIO);
    ESP_LOGI(SCREEN_TAG, "Panel is %dx%d", DISPLAY_WIDTH, DISPLAY_HEIGHT);

    i2c_master_init(&display->dev, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO, CONFIG_RESET_GPIO);

    memset(display->lines, 0, sizeof(display->lines));
    display->consecutive_errors = 0;

    ssd1306_init(&display->dev, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    ssd1306_clear_screen(&display->dev, false);
    ssd1306_contrast(&display->dev, 0xff);

    strncpy(display->lines[DISPLAY_TITLE_LINE], TITLE, DISPLAY_BUFFER_SIZE - 1);
    display->lines[DISPLAY_TITLE_LINE][DISPLAY_BUFFER_SIZE - 1] = '\0';
    ssd1306_display_text(&display->dev, TITLE_PAGE, display->lines[DISPLAY_TITLE_LINE], 18, false);

    ESP_LOGI(SCREEN_TAG, "SSD1306 initialized");
}

static void display_reinit(Display* display) {
    ESP_LOGW(SCREEN_TAG, "Re-initializing display after I2C errors");
    ssd1306_init(&display->dev, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    ssd1306_clear_screen(&display->dev, false);
    ssd1306_contrast(&display->dev, 0xff);
    ssd1306_display_text(&display->dev, TITLE_PAGE, display->lines[DISPLAY_TITLE_LINE], 18, false);
}

bool display_update(Display* display) {

    // Reset error count
    i2c_display_get_and_clear_error_count();

    ssd1306_display_text(&display->dev, LIGHT_PAGE, display->lines[DISPLAY_LIGHT_LINE], 18, false);
    ssd1306_display_text(&display->dev, HUMIDITY_PAGE, display->lines[DISPLAY_HUMIDITY_LINE], 18,
                         false);

    int errors = i2c_display_get_and_clear_error_count();
    if (errors == 0) {
        display->consecutive_errors = 0;
        return true;
    }

    display->consecutive_errors++;
    ESP_LOGW(SCREEN_TAG, "I2C errors during update (%d), consecutive=%d", errors,
             display->consecutive_errors);

    if (display->consecutive_errors >= DISPLAY_MAX_CONSECUTIVE_ERRORS) {
        // Attempt bus recovery first.
        esp_err_t err = i2c_display_recover_bus();
        if (err != ESP_OK) {
            ESP_LOGE(SCREEN_TAG, "I2C bus recovery failed: %s", esp_err_to_name(err));
        }
        // Re-initialize the display controller.
        display_reinit(display);
        display->consecutive_errors = 0;
    }

    return false;
}