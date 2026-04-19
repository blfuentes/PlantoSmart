#ifndef C_SYSTEM_H__
#define C_SYSTEM_H__

#include <driver/i2c_master.h>
#include <ssd1306.h>

typedef struct {
    // SSD1306_t oled_dev;
    gpio_num_t display_sda_pin;
    gpio_num_t display_scl_pin;

} SystemDevs;

SystemDevs* system_init(void);

SystemDevs* system_sleep(void);

#endif  // C_SYSTEM_H__