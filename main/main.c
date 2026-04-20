#include "c_system.h"

#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>

void app_main(void) {
    SystemDevs* sysdevs = system_init();
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
