#pragma once

#include <stdbool.h>

/**
 * @brief Probe an I2C bus and log discovered device addresses.
 * @param sda_gpio SDA GPIO number for the temporary scan bus.
 * @param scl_gpio SCL GPIO number for the temporary scan bus.
 */
void scan_i2c_bus(int sda_gpio, int scl_gpio);
