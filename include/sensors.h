#ifndef SENSORS_H__
#define SENSORS_H__

#include <driver/i2c_master.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief GPIO configuration used to bind ADC channels for sensors.
 */
typedef struct {
    /** ADC-capable GPIO for light sensor (LDR) analog output. */
    gpio_num_t ldr_pin;
    /** ADC-capable GPIO for hygrometer analog output. */
    gpio_num_t hygrometer_pin;
    /** Set false to skip all LDR reads when the sensor is not physically present. */
    bool has_ldr;
} SensorConfig;

/**
 * @brief Normalized and raw sensor readings produced by the sensor module.
 */
typedef struct {
    /** Averaged raw ADC value for LDR channel (0-4095). */
    uint16_t light_level;
    /** Computed ambient light percentage in range 0-100. */
    uint8_t light_percentage;
    /** Computed soil moisture percentage in range 0-100. */
    uint8_t humidity_level;
} SensorData;

/**
 * @brief Initialize ADC channels and calibration for configured sensors.
 * @param config Optional pin configuration; defaults are used when NULL.
 */
void sensors_init(const SensorConfig* config);

/**
 * @brief Acquire fresh sensor samples and populate output data.
 * @param data Output struct receiving latest readings.
 */
void sensors_update(SensorData* data);

#endif  // SENSORS_H__