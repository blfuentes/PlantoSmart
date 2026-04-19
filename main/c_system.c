#include "c_system.h"

#include <driver/gpio.h>
#include <driver/i2c_master.h>
#include <stdlib.h>

#define DISPLAY_GPIO_SDA_PIN GPIO_NUM_6
#define DISPLAY_GPIO_SCL_PIN GPIO_NUM_7

static SystemDevs system_devs = {.display_sda_pin = DISPLAY_GPIO_SDA_PIN,
                                 .display_scl_pin = DISPLAY_GPIO_SCL_PIN};
SystemDevs* system_init(void) {

    return NULL;
}