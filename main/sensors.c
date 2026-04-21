#include "sensors.h"

#include <esp_adc/adc_oneshot.h>
#include <esp_log.h>

static const char* SENSORS_TAG = "SENSORS";

#define LDR_MIN 2090  // dark
#define LDR_MAX 4095  // bright light

static adc_oneshot_unit_handle_t adc_handle;

void sensors_init(const SensorConfig* config) {
    // Initialize ADC for LDR
    adc_oneshot_unit_init_cfg_t unit_cfg = {
        .unit_id = ADC_UNIT_1,
    };
    adc_oneshot_new_unit(&unit_cfg, &adc_handle);

    adc_oneshot_chan_cfg_t chan_cfg = {
        .atten    = ADC_ATTEN_DB_12,  // Full range ~0-3.3V
        .bitwidth = ADC_BITWIDTH_12,  // 0-4095
    };
    adc_oneshot_config_channel(adc_handle, ADC_CHANNEL_3, &chan_cfg);
}

void sensors_update(SensorData* data) {
    int ldr_value = 0;
    adc_oneshot_read(adc_handle, ADC_CHANNEL_3, &ldr_value);
    if (ldr_value < LDR_MIN)
        ldr_value = LDR_MIN;
    float light_pct = (float)(ldr_value - LDR_MIN) / (LDR_MAX - LDR_MIN) * 100.0f;

    data->light_level      = (float)ldr_value;  // 0-4095
    data->light_percentage = light_pct;         // 0-100%
    ESP_LOGI(SENSORS_TAG, "LDR value: %d, %f%%", ldr_value, light_pct);

    data->humidity_level = 0.0f;
}