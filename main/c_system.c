#include "c_system.h"

#include <driver/gpio.h>
#include <driver/i2c_master.h>
#include <stdlib.h>

static SystemDevs system_devs = {.display_sda_pin = CONFIG_SDA_GPIO,
                                 .display_scl_pin = CONFIG_SCL_GPIO};
SystemDevs* system_init(void) {
    // Initialize display
    display_init(&system_devs.display);

    return &system_devs;
}