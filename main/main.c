#include "c_system.h"
#include "sensors.h"

#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>

void app_main(void) {
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
