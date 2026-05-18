#pragma once

/**
 * @brief Load WiFi credentials from .env file
 * @param ssid Buffer to store SSID (must be at least WIFI_SSID_MAX_LEN bytes)
 * @param password Buffer to store password (must be at least WIFI_PASSWORD_MAX_LEN bytes)
 * @return true if credentials were loaded successfully, false otherwise
 */
bool config_load_wifi_credentials(char* ssid, char* password, int ssid_len, int password_len);

/**
 * @brief Load Telegram bot token from generated config or environment
 * @param token Buffer to store token
 * @param token_len Token buffer size
 * @return true if token was loaded successfully, false otherwise
 */
bool config_load_telegram_bot_token(char* token, int token_len);

/**
 * @brief Load Telegram chat id from generated config or environment
 * @param chat_id Buffer to store chat id
 * @param chat_id_len Buffer size
 * @return true if chat id was loaded successfully, false otherwise
 */
bool config_load_telegram_chat_id(char* chat_id, int chat_id_len);

#define WIFI_SSID_MAX_LEN 32
#define WIFI_PASSWORD_MAX_LEN 64
#define TELEGRAM_BOT_TOKEN_MAX_LEN 64
#define TELEGRAM_CHAT_ID_MAX_LEN 32