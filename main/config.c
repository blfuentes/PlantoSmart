#include "config.h"
#include "wifi_config_gen.h"
#include <esp_log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char* TAG = "CONFIG";

bool config_load_wifi_credentials(char* ssid, char* password, int ssid_len, int password_len) {
    if (!ssid || !password) {
        ESP_LOGE(TAG, "Invalid parameters");
        return false;
    }

#ifdef WIFI_SSID_CONFIGURED
    // Use compile-time credentials from .env file
    const char* default_ssid = WIFI_SSID;
    const char* default_password = WIFI_PASSWORD;
    
    if (default_ssid && default_password) {
        strncpy(ssid, default_ssid, ssid_len - 1);
        ssid[ssid_len - 1] = '\0';
        strncpy(password, default_password, password_len - 1);
        password[password_len - 1] = '\0';
        ESP_LOGI(TAG, "WiFi credentials loaded from configuration");
        return true;
    }
#endif

    // Fallback: try to read from environment variables
    const char* env_ssid = getenv("WIFI_SSID");
    const char* env_password = getenv("WIFI_PASSWORD");

    if (env_ssid && env_password) {
        strncpy(ssid, env_ssid, ssid_len - 1);
        ssid[ssid_len - 1] = '\0';
        strncpy(password, env_password, password_len - 1);
        password[password_len - 1] = '\0';
        ESP_LOGI(TAG, "WiFi credentials loaded from environment");
        return true;
    }

    // If nothing is set, return false
    ESP_LOGE(TAG, "WiFi credentials not found. Please configure WIFI_SSID and WIFI_PASSWORD in .env file.");
    return false;
}

bool config_load_telegram_bot_token(char* token, int token_len) {
    if (!token || token_len <= 0) {
        ESP_LOGE(TAG, "Invalid Telegram token parameters");
        return false;
    }

#ifdef TELEGRAM_BOT_TOKEN_CONFIGURED
    const char* default_token = TELEGRAM_BOT_TOKEN;
    if (default_token && default_token[0] != '\0') {
        strncpy(token, default_token, token_len - 1);
        token[token_len - 1] = '\0';
        ESP_LOGI(TAG, "Telegram bot token loaded from configuration");
        return true;
    }
#endif

    // Keep compatibility with both names to avoid breaking existing .env files.
    const char* env_token = getenv("TELGRAM_BOT_TOKEN");
    if (!env_token || env_token[0] == '\0') {
        env_token = getenv("TELEGRAM_BOT_TOKEN");
    }

    if (env_token && env_token[0] != '\0') {
        strncpy(token, env_token, token_len - 1);
        token[token_len - 1] = '\0';
        ESP_LOGI(TAG, "Telegram bot token loaded from environment");
        return true;
    }

    ESP_LOGE(TAG, "Telegram bot token not found. Please configure TELGRAM_BOT_TOKEN in .env file.");
    return false;
}

bool config_load_telegram_chat_id(char* chat_id, int chat_id_len) {
    if (!chat_id || chat_id_len <= 0) {
        ESP_LOGE(TAG, "Invalid Telegram chat_id parameters");
        return false;
    }

#ifdef TELEGRAM_CHAT_ID_CONFIGURED
    const char* default_id = TELEGRAM_CHAT_ID;
    if (default_id && default_id[0] != '\0') {
        strncpy(chat_id, default_id, chat_id_len - 1);
        chat_id[chat_id_len - 1] = '\0';
        ESP_LOGI(TAG, "Telegram chat id loaded from configuration");
        return true;
    }
#endif

    const char* env_id = getenv("TELEGRAM_CHAT_ID");
    if (env_id && env_id[0] != '\0') {
        strncpy(chat_id, env_id, chat_id_len - 1);
        chat_id[chat_id_len - 1] = '\0';
        ESP_LOGI(TAG, "Telegram chat id loaded from environment");
        return true;
    }

    ESP_LOGE(TAG, "Telegram chat id not found. Please configure TELEGRAM_CHAT_ID in .env file.");
    return false;
}
