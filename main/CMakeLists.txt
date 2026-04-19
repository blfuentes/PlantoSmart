FILE(GLOB_RECURSE app_sources ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

idf_component_register(
	SRCS ${app_sources}
	INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/include"
	REQUIRES ssd1306
)

# idf_component_register(SRCS "main.c" "c_system.c" "display.c"
#                     INCLUDE_DIRS "." "../include"
#                     REQUIRES driver esp_driver_gpio esp_driver_i2c ssd1306)
