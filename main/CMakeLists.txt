# Load WiFi credentials from .env file and generate header
set(WIFI_SSID_VALUE "")
set(WIFI_PASSWORD_VALUE "")
set(WIFI_CONFIGURED 0)
set(TELEGRAM_BOT_TOKEN_VALUE "")
set(TELEGRAM_TOKEN_CONFIGURED 0)
set(TELEGRAM_CHAT_ID_VALUE "")
set(TELEGRAM_CHAT_ID_CONFIGURED 0)
set(TELEGRAM_CHAT_IDS_VALUE "")
set(TELEGRAM_CHAT_IDS_CONFIGURED 0)

if(EXISTS "${CMAKE_SOURCE_DIR}/.env")
	file(STRINGS "${CMAKE_SOURCE_DIR}/.env" env_lines)
	foreach(line ${env_lines})
		if(line MATCHES "^WIFI_SSID=(.*)$")
			set(WIFI_SSID_VALUE "${CMAKE_MATCH_1}")
			# Remove surrounding quotes
			string(REGEX REPLACE "^\"(.*)\"$" "\\1" WIFI_SSID_VALUE "${WIFI_SSID_VALUE}")
		elseif(line MATCHES "^WIFI_PASSWORD=(.*)$")
			set(WIFI_PASSWORD_VALUE "${CMAKE_MATCH_1}")
			# Remove surrounding quotes
			string(REGEX REPLACE "^\"(.*)\"$" "\\1" WIFI_PASSWORD_VALUE "${WIFI_PASSWORD_VALUE}")
		elseif(line MATCHES "^TELGRAM_BOT_TOKEN=(.*)$")
			set(TELEGRAM_BOT_TOKEN_VALUE "${CMAKE_MATCH_1}")
			string(REGEX REPLACE "^\"(.*)\"$" "\\1" TELEGRAM_BOT_TOKEN_VALUE "${TELEGRAM_BOT_TOKEN_VALUE}")
		elseif(line MATCHES "^TELEGRAM_BOT_TOKEN=(.*)$")
			set(TELEGRAM_BOT_TOKEN_VALUE "${CMAKE_MATCH_1}")
			string(REGEX REPLACE "^\"(.*)\"$" "\\1" TELEGRAM_BOT_TOKEN_VALUE "${TELEGRAM_BOT_TOKEN_VALUE}")
		elseif(line MATCHES "^TELEGRAM_CHAT_ID=(.*)$")
			set(TELEGRAM_CHAT_ID_VALUE "${CMAKE_MATCH_1}")
			string(REGEX REPLACE "^\"(.*)\"$" "\\1" TELEGRAM_CHAT_ID_VALUE "${TELEGRAM_CHAT_ID_VALUE}")
		elseif(line MATCHES "^TELEGRAM_CHAT_IDS=(.*)$")
			set(TELEGRAM_CHAT_IDS_VALUE "${CMAKE_MATCH_1}")
			string(REGEX REPLACE "^\"(.*)\"$" "\\1" TELEGRAM_CHAT_IDS_VALUE "${TELEGRAM_CHAT_IDS_VALUE}")
		endif()
	endforeach()
	
	if(WIFI_SSID_VALUE AND WIFI_PASSWORD_VALUE)
		message(STATUS "WiFi credentials loaded from .env: ${WIFI_SSID_VALUE}")
		set(WIFI_CONFIGURED 1)
	endif()

	if(TELEGRAM_BOT_TOKEN_VALUE)
		message(STATUS "Telegram bot token loaded from .env")
		set(TELEGRAM_TOKEN_CONFIGURED 1)
	endif()

	if(TELEGRAM_CHAT_ID_VALUE)
		message(STATUS "Telegram chat id loaded from .env")
		set(TELEGRAM_CHAT_ID_CONFIGURED 1)
	endif()

	if(TELEGRAM_CHAT_IDS_VALUE)
		message(STATUS "Telegram chat ids loaded from .env")
		set(TELEGRAM_CHAT_IDS_CONFIGURED 1)
	endif()
endif()

FILE(GLOB_RECURSE app_sources ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

# Generate config header with WiFi credentials
set(CONFIG_HEADER_PATH "${CMAKE_BINARY_DIR}/wifi_config_gen.h")
file(WRITE "${CONFIG_HEADER_PATH}" "#pragma once\n\n")
if(WIFI_CONFIGURED)
	file(APPEND "${CONFIG_HEADER_PATH}" "#define WIFI_SSID_CONFIGURED 1\n")
	file(APPEND "${CONFIG_HEADER_PATH}" "#define WIFI_SSID \"${WIFI_SSID_VALUE}\"\n")
	file(APPEND "${CONFIG_HEADER_PATH}" "#define WIFI_PASSWORD \"${WIFI_PASSWORD_VALUE}\"\n")
else()
	file(APPEND "${CONFIG_HEADER_PATH}" "// WiFi credentials not configured\n")
endif()

if(TELEGRAM_TOKEN_CONFIGURED)
	file(APPEND "${CONFIG_HEADER_PATH}" "#define TELEGRAM_BOT_TOKEN_CONFIGURED 1\n")
	file(APPEND "${CONFIG_HEADER_PATH}" "#define TELEGRAM_BOT_TOKEN \"${TELEGRAM_BOT_TOKEN_VALUE}\"\n")
else()
	file(APPEND "${CONFIG_HEADER_PATH}" "// Telegram bot token not configured\n")
endif()

if(TELEGRAM_CHAT_ID_CONFIGURED)
	file(APPEND "${CONFIG_HEADER_PATH}" "#define TELEGRAM_CHAT_ID_CONFIGURED 1\n")
	file(APPEND "${CONFIG_HEADER_PATH}" "#define TELEGRAM_CHAT_ID \"${TELEGRAM_CHAT_ID_VALUE}\"\n")
else()
	file(APPEND "${CONFIG_HEADER_PATH}" "// Telegram chat id not configured\n")
endif()

if(TELEGRAM_CHAT_IDS_CONFIGURED)
	file(APPEND "${CONFIG_HEADER_PATH}" "#define TELEGRAM_CHAT_IDS_CONFIGURED 1\n")
	file(APPEND "${CONFIG_HEADER_PATH}" "#define TELEGRAM_CHAT_IDS \"${TELEGRAM_CHAT_IDS_VALUE}\"\n")
else()
	file(APPEND "${CONFIG_HEADER_PATH}" "// Telegram chat ids not configured\n")
endif()

idf_component_register(
	SRCS ${app_sources}
	INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/include" "${CMAKE_BINARY_DIR}"
	REQUIRES ssd1306 esp_timer esp_adc esp_wifi esp_netif nvs_flash esp_event esp_http_client mbedtls
	PRIV_REQUIRES esp_common
)
