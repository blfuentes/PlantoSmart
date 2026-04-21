FILE(GLOB_RECURSE app_sources ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

idf_component_register(
	SRCS ${app_sources}
	INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/include"
	REQUIRES ssd1306 esp_timer esp_adc
)
