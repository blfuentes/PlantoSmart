#include "c_system.h"

#include <driver/gpio.h>
#include <driver/i2c_master.h>
#include <stdlib.h>

#define LDR_GPIO        GPIO_NUM_2
#define HYGROMETER_GPIO GPIO_NUM_3

static const char* I2C_SCAN_TAG = "SYSTEM";

static SystemDevs system_devs = {.display_sda_pin = CONFIG_SDA_GPIO,
                                 .display_scl_pin = CONFIG_SCL_GPIO};
SystemDevs* system_init(void) {
    // Initialize display
    display_init(&system_devs.display);

    // Initialize sensors
    system_devs.ldr_pin        = LDR_GPIO;
    system_devs.hygrometer_pin = HYGROMETER_GPIO;

    return &system_devs;
}