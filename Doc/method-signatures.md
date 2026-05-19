# PlantoSmart method signatures

This document lists the project function signatures by module.

## Main application APIs

### Config module
Defined in include/config.h and implemented in main/config.c.

```c
bool config_load_wifi_credentials(char* ssid, char* password, int ssid_len, int password_len);
bool config_load_telegram_bot_token(char* token, int token_len);
bool config_load_telegram_chat_id(char* chat_id, int chat_id_len);
bool config_load_telegram_chat_ids(char chat_ids[][TELEGRAM_CHAT_ID_MAX_LEN], int max_chat_ids, int* loaded_count);
```

### System module
Declared in include/c_system.h.

```c
SystemDevs* system_init(void);
SystemDevs* system_sleep(void);
```

Notes:
- `system_init` is implemented in main/c_system.c.
- `system_sleep` is declared but currently has no implementation in this repository.

### Display module
Defined in include/display.h and implemented in main/display.c.

```c
void display_init(Display* display);
bool display_update(Display* display);
```

### Sensors module
Defined in include/sensors.h and implemented in main/sensors.c.

```c
void sensors_init(const SensorConfig* config);
void sensors_update(SensorData* data);
```

### Telegram module
Defined in include/telegram.h and implemented in main/telegram.c.

```c
esp_err_t telegram_bot_client_init(TelegramBotClient* client, const char* token);
esp_err_t telegram_bot_send_message(const TelegramBotClient* client, const char* chat_id, const char* text);
```

### I2C scan module
Defined in include/i2c_scan.h and implemented in main/i2c_scan.c.

```c
void scan_i2c_bus(int sda_gpio, int scl_gpio);
```

### WiFi module
Defined in include/wifi.h and implemented in main/wifi.c.

```c
esp_err_t wifi_init(const char* ssid, const char* password);
esp_err_t wifi_stop(void);
bool wifi_is_connected(void);
const char* wifi_get_ip(void);
const char* wifi_get_ssid(void);
```

### App entry point
Defined in main/main.c.

```c
void app_main(void);
```

## Internal application methods

These are internal static functions that are not exposed via headers.

### main/main.c

```c
static void sensor_task(void* arg);
static void telegram_report_task(void* arg);
static void display_task(void* arg);
```

### main/display.c

```c
static void display_reinit(Display* display);
```

### main/sensors.c

```c
static bool adc_calibration_init(void);
static float ldr_raw_to_percent(int raw_value);
static float hygrometer_raw_to_percent(int raw_value);
static bool adc_channel_from_gpio(gpio_num_t gpio, adc_channel_t* channel);
```

### main/telegram.c

```c
static bool is_unreserved_char(char c);
static size_t url_encode(const char* src, char* dst, size_t dst_len);
```

### main/wifi.c

```c
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
```

## SSD1306 library APIs

Defined in lib/ssd1306/ssd1306.h.

```c
void ssd1306_init(SSD1306_t* dev, int width, int height);
int ssd1306_get_width(SSD1306_t* dev);
int ssd1306_get_height(SSD1306_t* dev);
int ssd1306_get_pages(SSD1306_t* dev);
void ssd1306_show_buffer(SSD1306_t* dev);
void ssd1306_set_buffer(SSD1306_t* dev, uint8_t* buffer);
void ssd1306_get_buffer(SSD1306_t* dev, uint8_t* buffer);
void ssd1306_display_image(SSD1306_t* dev, int page, int seg, uint8_t* images, int width);
void ssd1306_display_text(SSD1306_t* dev, int page, char* text, int text_len, bool invert);
void ssd1306_display_text_x3(SSD1306_t* dev, int page, char* text, int text_len, bool invert);
void ssd1306_clear_screen(SSD1306_t* dev, bool invert);
void ssd1306_clear_line(SSD1306_t* dev, int page, bool invert);
void ssd1306_contrast(SSD1306_t* dev, int contrast);
void ssd1306_software_scroll(SSD1306_t* dev, int start, int end);
void ssd1306_scroll_text(SSD1306_t* dev, char* text, int text_len, bool invert);
void ssd1306_scroll_clear(SSD1306_t* dev);
void ssd1306_hardware_scroll(SSD1306_t* dev, ssd1306_scroll_type_t scroll);
void ssd1306_wrap_arround(SSD1306_t* dev, ssd1306_scroll_type_t scroll, int start, int end, int8_t delay);
void ssd1306_bitmaps(SSD1306_t* dev, int xpos, int ypos, uint8_t* bitmap, int width, int height, bool invert);
void _ssd1306_pixel(SSD1306_t* dev, int xpos, int ypos, bool invert);
void _ssd1306_line(SSD1306_t* dev, int x1, int y1, int x2, int y2, bool invert);
void _ssd1306_circle(SSD1306_t* dev, int x0, int y0, int r, bool invert);
void ssd1306_invert(uint8_t* buf, size_t blen);
void ssd1306_flip(uint8_t* buf, size_t blen);
uint8_t ssd1306_copy_bit(uint8_t src, int srcBits, uint8_t dst, int dstBits);
uint8_t ssd1306_rotate_byte(uint8_t ch1);
void ssd1306_fadeout(SSD1306_t* dev);
void ssd1306_dump(SSD1306_t dev);
void ssd1306_dump_page(SSD1306_t* dev, int page, int seg);

void i2c_master_init(SSD1306_t* dev, int16_t sda, int16_t scl, int16_t reset);
void i2c_init(SSD1306_t* dev, int width, int height);
void i2c_display_image(SSD1306_t* dev, int page, int seg, uint8_t* images, int width);
void i2c_contrast(SSD1306_t* dev, int contrast);
void i2c_hardware_scroll(SSD1306_t* dev, ssd1306_scroll_type_t scroll);
int i2c_display_get_and_clear_error_count(void);
esp_err_t i2c_display_recover_bus(void);

void spi_master_init(SSD1306_t* dev, int16_t GPIO_MOSI, int16_t GPIO_SCLK, int16_t GPIO_CS, int16_t GPIO_DC, int16_t GPIO_RESET);
bool spi_master_write_byte(spi_device_handle_t SPIHandle, const uint8_t* Data, size_t DataLength);
bool spi_master_write_command(SSD1306_t* dev, uint8_t Command);
bool spi_master_write_data(SSD1306_t* dev, const uint8_t* Data, size_t DataLength);
void spi_init(SSD1306_t* dev, int width, int height);
void spi_display_image(SSD1306_t* dev, int page, int seg, uint8_t* images, int width);
void spi_contrast(SSD1306_t* dev, int contrast);
void spi_hardware_scroll(SSD1306_t* dev, ssd1306_scroll_type_t scroll);

#if CONFIG_SPI_INTERFACE
void spi_clock_speed(int speed);
#endif
```

## Implementation-specific notes

- There are two I2C backend implementations for SSD1306:
  - lib/ssd1306/ssd1306_i2c_new.c
  - lib/ssd1306/ssd1306_i2c_legacy.c
- Both expose the same I2C function signatures (`i2c_init`, `i2c_display_image`, `i2c_contrast`, `i2c_hardware_scroll`).
- SPI transport signatures are declared in ssd1306.h and implemented in lib/ssd1306/ssd1306_spi.c.
