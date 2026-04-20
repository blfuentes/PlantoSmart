#ifndef SENSORS_H__
#define SENSORS_H__

#include <driver/i2c_master.h>

typedef struct {
    gpio_num_t hygrometer_pin;
    gpio_num_t ldr_pin;
} SensorConfig;

typedef struct {
    float light_level;
    float humidity_level;
} SensorData;

void sensors_init(const SensorConfig* config);

#endif  // SENSORS_H__