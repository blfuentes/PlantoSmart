#include "config.h"
#include "telegram.h"

#include <ctype.h>
#include <esp_crt_bundle.h>
#include <esp_http_client.h>
#include <esp_log.h>
#include <stdio.h>
#include <string.h>

static const char* TAG = "TELEGRAM";

static bool is_unreserved_char(char c) {
    return isalnum((unsigned char)c) || c == '-' || c == '_' || c == '.' || c == '~';
}

static size_t url_encode(const char* src, char* dst, size_t dst_len) {
    static const char* hex = "0123456789ABCDEF";
    size_t di = 0;

    for (size_t si = 0; src[si] != '\0'; ++si) {
        unsigned char ch = (unsigned char)src[si];
        if (is_unreserved_char((char)ch)) {
            if (di + 1 >= dst_len) {
                return 0;
            }
            dst[di++] = (char)ch;
        } else {
            if (di + 3 >= dst_len) {
                return 0;
            }
            dst[di++] = '%';
            dst[di++] = hex[ch >> 4];
            dst[di++] = hex[ch & 0x0F];
        }
    }

    if (di >= dst_len) {
        return 0;
    }
    dst[di] = '\0';
    return di;
}

esp_err_t telegram_bot_client_init(TelegramBotClient* client, const char* token) {
    if (!client || !token || token[0] == '\0') {
        return ESP_ERR_INVALID_ARG;
    }

    memset(client, 0, sizeof(*client));
    strncpy(client->token, token, sizeof(client->token) - 1);
    return ESP_OK;
}

esp_err_t telegram_bot_send_message(const TelegramBotClient* client, const char* chat_id,
                                    const char* text) {
    if (!client || client->token[0] == '\0' || !chat_id || !text) {
        ESP_LOGE(TAG, "Invalid argument: client=%p, token_empty=%d, chat_id=%p, text=%p",
                 client, (client ? (int)client->token[0] == '\0' : -1), chat_id, text);
        return ESP_ERR_INVALID_ARG;
    }

    char url[256];
    int written = snprintf(url, sizeof(url), "https://api.telegram.org/bot%s/sendMessage",
                           client->token);
    if (written <= 0 || written >= (int)sizeof(url)) {
        ESP_LOGE(TAG, "URL construction failed");
        return ESP_ERR_INVALID_SIZE;
    }
    char chat_id_encoded[128];
    char text_encoded[320];
    if (url_encode(chat_id, chat_id_encoded, sizeof(chat_id_encoded)) == 0 ||
        url_encode(text, text_encoded, sizeof(text_encoded)) == 0) {
        ESP_LOGE(TAG, "Failed to encode Telegram payload");
        return ESP_ERR_INVALID_SIZE;
    }
    char payload[512];
    written = snprintf(payload, sizeof(payload), "chat_id=%s&text=%s", chat_id_encoded,
                       text_encoded);
    if (written <= 0 || written >= (int)sizeof(payload)) {
        ESP_LOGE(TAG, "Payload construction failed: written=%d", written);
        return ESP_ERR_INVALID_SIZE;
    }
    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_POST,
        .timeout_ms = 5000,
        .crt_bundle_attach = esp_crt_bundle_attach,
    };
    esp_http_client_handle_t client_http = esp_http_client_init(&config);
    if (client_http == NULL) {
        ESP_LOGE(TAG, "Failed to init HTTP client");
        return ESP_FAIL;
    }
    esp_http_client_set_header(client_http, "Content-Type", "application/x-www-form-urlencoded");
    esp_http_client_set_post_field(client_http, payload, (int)strlen(payload));
    esp_err_t err = esp_http_client_perform(client_http);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "HTTP request failed: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client_http);
        return err;
    }
    int status = esp_http_client_get_status_code(client_http);
    esp_http_client_cleanup(client_http);

    if (status != 200) {
        ESP_LOGE(TAG, "Telegram sendMessage returned HTTP status %d", status);
        return ESP_FAIL;
    }

    return ESP_OK;
}