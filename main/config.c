#include "config.h"
#include "wifi_config_gen.h"
#include <ctype.h>
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

bool config_load_telegram_chat_ids(char chat_ids[][TELEGRAM_CHAT_ID_MAX_LEN], int max_chat_ids,
                                   int* loaded_count) {
    if (!chat_ids || max_chat_ids <= 0 || !loaded_count) {
        ESP_LOGE(TAG, "Invalid Telegram chat ids parameters");
        return false;
    }

    *loaded_count = 0;

    const char* ids_source = NULL;

#ifdef TELEGRAM_CHAT_IDS_CONFIGURED
    ids_source = TELEGRAM_CHAT_IDS;
#endif

    if (!ids_source || ids_source[0] == '\0') {
        ids_source = getenv("TELEGRAM_CHAT_IDS");
    }

    if (ids_source && ids_source[0] != '\0') {
        char ids_copy[TELEGRAM_MAX_RECIPIENTS * TELEGRAM_CHAT_ID_MAX_LEN] = {0};
        strncpy(ids_copy, ids_source, sizeof(ids_copy) - 1);

        char* saveptr = NULL;
        char* token = strtok_r(ids_copy, ",", &saveptr);
        while (token && *loaded_count < max_chat_ids) {
            while (*token != '\0' && isspace((unsigned char)*token)) {
                ++token;
            }

            size_t len = strlen(token);
            while (len > 0 && isspace((unsigned char)token[len - 1])) {
                token[--len] = '\0';
            }

            if (len > 0) {
                strncpy(chat_ids[*loaded_count], token, TELEGRAM_CHAT_ID_MAX_LEN - 1);
                chat_ids[*loaded_count][TELEGRAM_CHAT_ID_MAX_LEN - 1] = '\0';
                (*loaded_count)++;
            }

            token = strtok_r(NULL, ",", &saveptr);
        }

        if (*loaded_count > 0) {
            ESP_LOGI(TAG, "Loaded %d Telegram chat id(s) from TELEGRAM_CHAT_IDS", *loaded_count);
            return true;
        }
    }

    // Backward compatibility with single chat id config.
    if (config_load_telegram_chat_id(chat_ids[0], TELEGRAM_CHAT_ID_MAX_LEN)) {
        *loaded_count = 1;
        ESP_LOGI(TAG, "Loaded 1 Telegram chat id from TELEGRAM_CHAT_ID");
        return true;
    }

    ESP_LOGE(TAG,
             "Telegram chat ids not found. Configure TELEGRAM_CHAT_IDS (comma-separated) or TELEGRAM_CHAT_ID in .env file.");
    return false;
}
