#include "c_system.h"
#include "config.h"
#include "i2c_scan.h"
#include "sensors.h"
#include "telegram.h"
#include "wifi.h"

#include <esp_event.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <stdbool.h>

static const bool ENABLE_SCAN  = false;
static const bool ENABLE_DEBUG = true;

#define SENSOR_TASK_PERIOD_MS     500
#define DISPLAY_TASK_TIMEOUT_MS   1000
#define SENSOR_TASK_STACK_SIZE    3072
#define DISPLAY_TASK_STACK_SIZE   3072
#define TELEGRAM_TASK_STACK_SIZE  8192
#define SENSOR_TASK_PRIORITY      5
#define DISPLAY_TASK_PRIORITY     4
#define TELEGRAM_TASK_PRIORITY    3
#define TELEGRAM_REPORT_PERIOD_MS 5 * 60 * 1000  // 5 minutes

typedef struct {
    SensorData data;
} SensorMessage;

static QueueHandle_t g_sensor_queue;
static SystemDevs* g_sysdevs;
static TelegramBotClient g_telegram_client;
static bool g_telegram_ready = false;
static char g_telegram_chat_ids[TELEGRAM_MAX_RECIPIENTS][TELEGRAM_CHAT_ID_MAX_LEN];
static int g_telegram_chat_count = 0;

typedef struct {
    TelegramBotClient* client;
    const char* chat_id;
} TelegramTaskArg;

static void sensor_task(void* arg) {
    (void)arg;
    SensorMessage msg;

    ESP_LOGI("SENSOR_TASK", "Sensor task started");

    for (;;) {
        sensors_update(&msg.data);

        ESP_LOGI("SENSOR_TASK", "Read sensor data: light=%d%% humidity=%d%%",
                 (int)msg.data.light_percentage, (int)msg.data.humidity_level);

        // Keep only the latest sample so display is always up to date.
        BaseType_t ret = xQueueOverwrite(g_sensor_queue, &msg);
        if (ret != pdPASS) {
            ESP_LOGE("SENSOR_TASK", "Failed to write to queue");
        }

        vTaskDelay(pdMS_TO_TICKS(SENSOR_TASK_PERIOD_MS));
    }
}

static void telegram_report_task(void* arg) {
    (void)arg;
    SensorMessage msg;
    char text[128];
    char last_sent_text[sizeof(text)] = {0};
    bool has_last_sent_text           = false;
    int send_count                    = 0;

    ESP_LOGI("TELEGRAM_TASK", "Telegram report task started");

    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(TELEGRAM_REPORT_PERIOD_MS));

        if (!g_telegram_ready) {
            ESP_LOGD("TELEGRAM_TASK", "Telegram not ready, skipping");
            continue;
        }

        // Peek latest sensor data without consuming from queue.
        if (xQueuePeek(g_sensor_queue, &msg, 0) != pdPASS) {
            ESP_LOGW("TELEGRAM_TASK", "Failed to peek sensor data from queue");
            continue;
        }

        snprintf(text, sizeof(text), "~Los Veggies~\nLight: %d%%\nHumidity: %d%%",
                 (int)msg.data.light_percentage, (int)msg.data.humidity_level);

        // Avoid sending duplicate Telegram messages when sensor values did not change.
        if (has_last_sent_text && strcmp(text, last_sent_text) == 0) {
            ESP_LOGD("TELEGRAM_TASK", "No data change, skipping Telegram send");
            continue;
        }

        ESP_LOGI("TELEGRAM_TASK", "[%d] Broadcasting to %d chat(s): light=%d%% humidity=%d%%",
                 ++send_count, g_telegram_chat_count, (int)msg.data.light_percentage,
                 (int)msg.data.humidity_level);

        bool all_sent = true;
        for (int i = 0; i < g_telegram_chat_count; ++i) {
            esp_err_t ret =
                telegram_bot_send_message(&g_telegram_client, g_telegram_chat_ids[i], text);
            if (ret != ESP_OK) {
                all_sent = false;
                ESP_LOGE("TELEGRAM_TASK", "Send failed to chat[%d]=%s: %s", i,
                         g_telegram_chat_ids[i], esp_err_to_name(ret));
            }
        }

        if (all_sent) {
            strncpy(last_sent_text, text, sizeof(last_sent_text) - 1);
            last_sent_text[sizeof(last_sent_text) - 1] = '\0';
            has_last_sent_text                         = true;
            ESP_LOGI("TELEGRAM_TASK", "Broadcast #%d OK", send_count);
        }
    }
}

static void display_task(void* arg) {
    (void)arg;
    SensorMessage msg;

    ESP_LOGI("DISPLAY_TASK", "Display task started");

    for (;;) {
        BaseType_t ret = xQueuePeek(g_sensor_queue, &msg, pdMS_TO_TICKS(DISPLAY_TASK_TIMEOUT_MS));
        if (ret != pdPASS) {
            continue;  // Timeout, no new data
        }

        snprintf(g_sysdevs->display.lines[DISPLAY_LIGHT_LINE], DISPLAY_BUFFER_SIZE, "Light:%9d%%",
                 (int)msg.data.light_percentage);
        snprintf(g_sysdevs->display.lines[DISPLAY_HUMIDITY_LINE], DISPLAY_BUFFER_SIZE,
                 "Humidity:%6d%%", (int)msg.data.humidity_level);

        // Display WiFi SSID or LDR value in debug line
        const char* ssid = wifi_get_ssid();
        if (ssid) {
            snprintf(g_sysdevs->display.lines[DISPLAY_DEBUG_PAGE], DISPLAY_BUFFER_SIZE, "WiFi: %s",
                     ssid);
        } else {
            snprintf(g_sysdevs->display.lines[DISPLAY_DEBUG_PAGE], DISPLAY_BUFFER_SIZE, "LDR:%4d",
                     (int)msg.data.light_level);
        }

        g_sysdevs->display.debug_mode = ENABLE_DEBUG;
        display_update(&g_sysdevs->display);
    }
}

void app_main(void) {
    // Create default event loop for WiFi
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    if (ENABLE_SCAN) {
        scan_i2c_bus(CONFIG_SDA_GPIO, CONFIG_SCL_GPIO);
    }

    // Initialize WiFi with credentials from environment
    char wifi_ssid[WIFI_SSID_MAX_LEN];
    char wifi_password[WIFI_PASSWORD_MAX_LEN];

    if (config_load_wifi_credentials(wifi_ssid, wifi_password, WIFI_SSID_MAX_LEN,
                                     WIFI_PASSWORD_MAX_LEN)) {
        esp_err_t wifi_ret = wifi_init(wifi_ssid, wifi_password);
        if (wifi_ret != ESP_OK) {
            ESP_LOGE("MAIN", "WiFi initialization failed: %s", esp_err_to_name(wifi_ret));
        } else {
            ESP_LOGI("MAIN", "WiFi initialized successfully");
        }
    } else {
        ESP_LOGW("MAIN", "WiFi credentials not available");
    }

    char telegram_token[TELEGRAM_BOT_TOKEN_MAX_LEN];
    ESP_LOGI("MAIN", "Loading Telegram bot token...");
    if (config_load_telegram_bot_token(telegram_token, sizeof(telegram_token))) {
        ESP_LOGI("MAIN", "Telegram token loaded, length=%d", (int)strlen(telegram_token));
        if (telegram_bot_client_init(&g_telegram_client, telegram_token) == ESP_OK) {
            ESP_LOGI("MAIN", "Loading Telegram chat IDs...");
            g_telegram_ready = config_load_telegram_chat_ids(
                g_telegram_chat_ids, TELEGRAM_MAX_RECIPIENTS, &g_telegram_chat_count);
            if (g_telegram_ready) {
                ESP_LOGI("MAIN", "Telegram bot client initialized for %d chat(s)",
                         g_telegram_chat_count);
            } else {
                ESP_LOGW("MAIN",
                         "Telegram token OK but no chat ids found - set TELEGRAM_CHAT_IDS or "
                         "TELEGRAM_CHAT_ID in .env");
            }
        } else {
            ESP_LOGE("MAIN", "Failed to initialize Telegram bot client");
        }
    } else {
        ESP_LOGW("MAIN", "Telegram bot token not available");
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
        ESP_LOGE("MAIN", "Failed to create sensor queue");
        return;
    }
    ESP_LOGI("MAIN", "Sensor queue created");

    ESP_LOGI("MAIN", "Creating tasks...");
    xTaskCreate(sensor_task, "sensor_task", SENSOR_TASK_STACK_SIZE, NULL, SENSOR_TASK_PRIORITY,
                NULL);
    xTaskCreate(display_task, "display_task", DISPLAY_TASK_STACK_SIZE, NULL, DISPLAY_TASK_PRIORITY,
                NULL);
    xTaskCreate(telegram_report_task, "telegram_task", TELEGRAM_TASK_STACK_SIZE, NULL,
                TELEGRAM_TASK_PRIORITY, NULL);
    ESP_LOGI("MAIN", "All tasks created");

    // app_main can return once worker tasks are running.
}
