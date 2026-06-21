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

/**
 * @brief Check for a reset command in recent Telegram updates.
 * @param client Initialized bot client
 * @param chat_ids List of authorized Telegram chat IDs as strings
 * @param chat_count Number of authorized chat IDs
 * @param reset_requested Set to true when a /reset command is received from an authorized chat
 * @return ESP_OK on success, otherwise an ESP-IDF error code
 */
esp_err_t telegram_bot_check_reset_command(const TelegramBotClient* client,
                                            const char chat_ids[][TELEGRAM_CHAT_ID_MAX_LEN],
                                            int chat_count, bool* reset_requested);