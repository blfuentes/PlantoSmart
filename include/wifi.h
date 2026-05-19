#pragma once

#include <esp_err.h>

/**
 * @brief Initialize WiFi in station mode and connect to the configured network
 * @param ssid SSID of the WiFi network.
 * @param password Password for the WiFi network.
 * @return ESP_OK on success, otherwise an ESP-IDF error code.
 */
esp_err_t wifi_init(const char* ssid, const char* password);

/**
 * @brief Stop WiFi and clean up resources
 * @return ESP_OK on success, otherwise an ESP-IDF error code.
 */
esp_err_t wifi_stop(void);

/**
 * @brief Check if WiFi is currently connected
 * @return true when station has an active connection, false otherwise.
 */
bool wifi_is_connected(void);

/**
 * @brief Get the local IP address when connected
 * @return Pointer to internal IP string buffer, or NULL if not connected.
 */
const char* wifi_get_ip(void);

/**
 * @brief Get the WiFi SSID name
 * @return Pointer to internal SSID string buffer, or NULL if unset.
 */
const char* wifi_get_ssid(void);
