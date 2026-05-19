#pragma once

#include "config.h"
#include <esp_err.h>

/**
 * @brief Holds bot token and runtime state for Telegram API requests.
 */
typedef struct {
    /** Bot token provided by Telegram BotFather. */
    char token[TELEGRAM_BOT_TOKEN_MAX_LEN];
} TelegramBotClient;

/**
 * @brief Initialize Telegram bot client with token
 * @param client Bot client instance
 * @param token Bot token from @BotFather
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t telegram_bot_client_init(TelegramBotClient* client, const char* token);

/**
 * @brief Send a plain text message to a chat
 * @param client Initialized bot client
 * @param chat_id Telegram chat id as string
 * @param text Message text
 * @return ESP_OK on successful HTTP 200 response
 */
esp_err_t telegram_bot_send_message(const TelegramBotClient* client, const char* chat_id,
                                    const char* text);