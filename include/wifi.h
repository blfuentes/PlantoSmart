#pragma once

#include <esp_err.h>

/**
 * @brief Initialize WiFi in station mode and connect to the configured network
 * @param ssid The SSID of the WiFi network to connect to
 * @param password The password for the WiFi network
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t wifi_init(const char* ssid, const char* password);

/**
 * @brief Stop WiFi and clean up resources
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t wifi_stop(void);

/**
 * @brief Check if WiFi is currently connected
 * @return true if connected, false otherwise
 */
bool wifi_is_connected(void);

/**
 * @brief Get the local IP address when connected
 * @return Pointer to IP address string, or NULL if not connected
 */
const char* wifi_get_ip(void);

/**
 * @brief Get the WiFi SSID name
 * @return Pointer to SSID string, or NULL if not set
 */
const char* wifi_get_ssid(void);
