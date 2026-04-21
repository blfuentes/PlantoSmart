#include "c_system.h"
#include "i2c_scan.h"
#include "sensors.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdbool.h>

static const bool ENABLE_SCAN = false;

void app_main(void) {
    if (ENABLE_SCAN) {
        scan_i2c_bus(CONFIG_SDA_GPIO, CONFIG_SCL_GPIO);
    }

    SystemDevs* sysdevs = system_init();

    SensorConfig sensor_config = {
        .hygrometer_pin = sysdevs->hygrometer_pin,
        .ldr_pin        = sysdevs->ldr_pin,
    };

    sensors_init(&sensor_config);
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
