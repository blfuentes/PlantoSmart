#ifndef C_SYSTEM_H__
#define C_SYSTEM_H__

#include <driver/i2c_master.h>

#include "display.h"

/**
 * @brief Aggregates GPIO assignments and initialized peripherals used by the app.
 */
typedef struct {
    /** SDA GPIO used by the OLED I2C bus. */
    gpio_num_t display_sda_pin;
    /** SCL GPIO used by the OLED I2C bus. */
    gpio_num_t display_scl_pin;
    /** ADC-capable GPIO connected to the hygrometer output. */
    gpio_num_t hygrometer_pin;
    /** ADC-capable GPIO connected to the LDR divider output. */
    gpio_num_t ldr_pin;
    /** Display driver state and frame buffer lines. */
    Display display;

} SystemDevs;

/**
 * @brief Initialize board-level devices and populate the shared system descriptor.
 * @return Pointer to a static SystemDevs instance.
 */
SystemDevs* system_init(void);

/**
 * @brief Put the system into low-power state.
 * @return Pointer to SystemDevs instance when supported.
 * @note Declared for future use; implementation may be platform-specific.
 */
SystemDevs* system_sleep(void);

#endif  // C_SYSTEM_H__