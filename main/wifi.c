#include "wifi.h"
#include <esp_log.h>
#include <esp_netif.h>
#include <esp_wifi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <nvs_flash.h>
#include <string.h>

static const char* TAG = "WIFI";

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static EventGroupHandle_t s_wifi_event_group = NULL;
static esp_netif_t* s_netif_sta = NULL;
static char s_ip_addr[16] = {0};
static char s_ssid[32] = {0};
static bool s_wifi_initialized = false;

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id,
                               void* event_data) {
    (void)arg;

    if (event_base == WIFI_EVENT) {
        switch (event_id) {
        case WIFI_EVENT_STA_START:
            ESP_LOGI(TAG, "WiFi started, attempting to connect...");
            esp_wifi_connect();
            break;

        case WIFI_EVENT_STA_DISCONNECTED: {
            wifi_event_sta_disconnected_t* disconn = (wifi_event_sta_disconnected_t*)event_data;
            ESP_LOGW(TAG, "WiFi disconnected. Reason: %d, Retrying connection...", disconn->reason);
            xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
            esp_wifi_connect();
            break;
        }

        default:
            break;
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*)event_data;
        esp_ip4_addr_t ip = event->ip_info.ip;

        snprintf(s_ip_addr, sizeof(s_ip_addr), IPSTR, IP2STR(&ip));
        ESP_LOGI(TAG, "Got IP address: %s", s_ip_addr);

        xEventGroupClearBits(s_wifi_event_group, WIFI_FAIL_BIT);
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

esp_err_t wifi_init(const char* ssid, const char* password) {
    if (s_wifi_initialized) {
        ESP_LOGW(TAG, "WiFi already initialized");
        return ESP_OK;
    }

    if (!ssid || !password) {
        ESP_LOGE(TAG, "SSID and password must not be NULL");
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "Initializing WiFi with SSID: %s", ssid);

    // Create event group for WiFi events
    s_wifi_event_group = xEventGroupCreate();
    if (s_wifi_event_group == NULL) {
        ESP_LOGE(TAG, "Failed to create event group");
        return ESP_ERR_NO_MEM;
    }

    // Initialize NVS (required for WiFi)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS flash initialization failed: %s", esp_err_to_name(ret));
        return ret;
    }

    // Initialize the TCP/IP stack
    ESP_ERROR_CHECK(esp_netif_init());

    // Create default WiFi station interface
    s_netif_sta = esp_netif_create_default_wifi_sta();
    if (s_netif_sta == NULL) {
        ESP_LOGE(TAG, "Failed to create WiFi station interface");
        return ESP_FAIL;
    }

    // Initialize WiFi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ret = esp_wifi_init(&cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "WiFi initialization failed: %s", esp_err_to_name(ret));
        return ret;
    }

    // Register event handlers
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler, NULL, &instance_got_ip));

    // Configure WiFi
    wifi_config_t wifi_config = {
        .sta = {
            .scan_method = WIFI_ALL_CHANNEL_SCAN,
            .bssid_set = 0,
            .channel = 0,
            .listen_interval = 0,
            .sort_method = WIFI_CONNECT_AP_BY_SIGNAL,
            .threshold = {
                .rssi = -127,
                .authmode = WIFI_AUTH_OPEN,
            },
            .pmf_cfg = {.capable = true, .required = false},
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
            .sae_h2e_identifier = {0},
        },
    };

    strncpy((char*)wifi_config.sta.ssid, (const char*)ssid, sizeof(wifi_config.sta.ssid) - 1);
    wifi_config.sta.ssid[sizeof(wifi_config.sta.ssid) - 1] = '\0';
    strncpy((char*)wifi_config.sta.password, (const char*)password, sizeof(wifi_config.sta.password) - 1);
    wifi_config.sta.password[sizeof(wifi_config.sta.password) - 1] = '\0';
    
    // Store SSID for later retrieval
    strncpy(s_ssid, ssid, sizeof(s_ssid) - 1);
    s_ssid[sizeof(s_ssid) - 1] = '\0';

    ret = esp_wifi_set_mode(WIFI_MODE_STA);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set WiFi mode: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure WiFi: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_wifi_start();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start WiFi: %s", esp_err_to_name(ret));
        return ret;
    }

    s_wifi_initialized = true;
    ESP_LOGI(TAG, "WiFi initialization complete");

    return ESP_OK;
}

esp_err_t wifi_stop(void) {
    if (!s_wifi_initialized) {
        return ESP_OK;
    }

    esp_err_t ret = esp_wifi_stop();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to stop WiFi: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_wifi_deinit();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to deinit WiFi: %s", esp_err_to_name(ret));
        return ret;
    }

    if (s_netif_sta != NULL) {
        esp_netif_destroy_default_wifi(s_netif_sta);
        s_netif_sta = NULL;
    }

    if (s_wifi_event_group != NULL) {
        vEventGroupDelete(s_wifi_event_group);
        s_wifi_event_group = NULL;
    }

    memset(s_ip_addr, 0, sizeof(s_ip_addr));
    s_wifi_initialized = false;

    return ESP_OK;
}

bool wifi_is_connected(void) {
    if (s_wifi_event_group == NULL) {
        return false;
    }
    EventBits_t bits = xEventGroupGetBits(s_wifi_event_group);
    return (bits & WIFI_CONNECTED_BIT) != 0;
}

const char* wifi_get_ip(void) {
    if (!wifi_is_connected()) {
        return NULL;
    }
    return s_ip_addr;
}

const char* wifi_get_ssid(void) {
    if (s_ssid[0] == '\0') {
        return NULL;
    }
    return s_ssid;
}
