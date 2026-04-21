#include "c_system.h"
#include "i2c_scan.h"
#include "sensors.h"

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <stdbool.h>

static const bool ENABLE_SCAN = false;

#define SENSOR_TASK_PERIOD_MS   500
#define DISPLAY_TASK_TIMEOUT_MS 1000
#define SENSOR_TASK_STACK_SIZE  3072
#define DISPLAY_TASK_STACK_SIZE 3072
#define SENSOR_TASK_PRIORITY    5
#define DISPLAY_TASK_PRIORITY   4

typedef struct {
    SensorData data;
} SensorMessage;

static QueueHandle_t g_sensor_queue;
static SystemDevs* g_sysdevs;

static void sensor_task(void* arg) {
    (void)arg;
    SensorMessage msg;

    for (;;) {
        sensors_update(&msg.data);

        // Keep only the latest sample so display is always up to date.
        xQueueOverwrite(g_sensor_queue, &msg);
        vTaskDelay(pdMS_TO_TICKS(SENSOR_TASK_PERIOD_MS));
    }
}

static void display_task(void* arg) {
    (void)arg;
    SensorMessage msg;

    for (;;) {
        if (xQueueReceive(g_sensor_queue, &msg, pdMS_TO_TICKS(DISPLAY_TASK_TIMEOUT_MS)) == pdPASS) {
            snprintf(g_sysdevs->display.lines[1], DISPLAY_BUFFER_SIZE, "Light: %d%%",
                     (int)msg.data.light_percentage);
            snprintf(g_sysdevs->display.lines[2], DISPLAY_BUFFER_SIZE, "Humidity: %d%%",
                     (int)msg.data.humidity_level);
            display_update(&g_sysdevs->display);
        }
    }
}

void app_main(void) {
    if (ENABLE_SCAN) {
        scan_i2c_bus(CONFIG_SDA_GPIO, CONFIG_SCL_GPIO);
    }

    g_sysdevs = system_init();

    SensorConfig sensor_config = {
        .ldr_pin        = g_sysdevs->ldr_pin,
        .hygrometer_pin = g_sysdevs->hygrometer_pin,
    };

    sensors_init(&sensor_config);

    // Queue length 1 with overwrite semantics: latest sensor reading wins.
    g_sensor_queue = xQueueCreate(1, sizeof(SensorMessage));
    if (g_sensor_queue == NULL) {
        return;
    }

    xTaskCreate(sensor_task, "sensor_task", SENSOR_TASK_STACK_SIZE, NULL, SENSOR_TASK_PRIORITY,
                NULL);
    xTaskCreate(display_task, "display_task", DISPLAY_TASK_STACK_SIZE, NULL, DISPLAY_TASK_PRIORITY,
                NULL);

    // app_main can return once worker tasks are running.
}
