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

    SensorData data;
    for (;;) {
        sensors_update(&data);

        snprintf(sysdevs->display.lines[1], DISPLAY_BUFFER_SIZE, "Light: %d%%",
                 (int)data.light_percentage);
        display_update(&sysdevs->display);

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
