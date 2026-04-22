#include "sensors.h"

#include <esp_adc/adc_oneshot.h>
#include <esp_log.h>

static const char* SENSORS_TAG = "SENSORS";

#define LDR_MIN            3100  // dark
#define LDR_MAX            4095  // bright light
#define HYGROMETER_WET_RAW 2100  // under water
#define HYGROMETER_DRY_RAW 3950  // in open air
#define HYGROMETER_SAMPLES 8

static adc_oneshot_unit_handle_t adc_handle;
static adc_channel_t ldr_channel        = ADC_CHANNEL_2;
static adc_channel_t hygrometer_channel = ADC_CHANNEL_3;

static float hygrometer_raw_to_percent(int raw_value) {
    int clamped_raw = raw_value;
    if (clamped_raw < HYGROMETER_WET_RAW) {
        clamped_raw = HYGROMETER_WET_RAW;
    }
    if (clamped_raw > HYGROMETER_DRY_RAW) {
        clamped_raw = HYGROMETER_DRY_RAW;
    }

    // Many resistive probes report lower voltage when wetter.
    return (float)(HYGROMETER_DRY_RAW - clamped_raw) * 100.0f /
           (float)(HYGROMETER_DRY_RAW - HYGROMETER_WET_RAW);
}

static bool adc_channel_from_gpio(gpio_num_t gpio, adc_channel_t* channel) {
    switch (gpio) {
        case GPIO_NUM_0:
            *channel = ADC_CHANNEL_0;
            return true;
        case GPIO_NUM_1:
            *channel = ADC_CHANNEL_1;
            return true;
        case GPIO_NUM_2:
            *channel = ADC_CHANNEL_2;
            return true;
        case GPIO_NUM_3:
            *channel = ADC_CHANNEL_3;
            return true;
        case GPIO_NUM_4:
            *channel = ADC_CHANNEL_4;
            return true;
        default:
            return false;
    }
}

void sensors_init(const SensorConfig* config) {
    if ((config != NULL) && !adc_channel_from_gpio(config->ldr_pin, &ldr_channel)) {
        ESP_LOGW(SENSORS_TAG, "Invalid LDR GPIO for ADC: %d. Using ADC_CHANNEL_2.",
                 (int)config->ldr_pin);
        ldr_channel = ADC_CHANNEL_2;
    }

    if ((config != NULL) && !adc_channel_from_gpio(config->hygrometer_pin, &hygrometer_channel)) {
        ESP_LOGW(SENSORS_TAG, "Invalid hygrometer GPIO for ADC: %d. Using ADC_CHANNEL_3.",
                 (int)config->hygrometer_pin);
        hygrometer_channel = ADC_CHANNEL_3;
    }

    // Initialize ADC for LDR
    adc_oneshot_unit_init_cfg_t unit_cfg = {
        .unit_id = ADC_UNIT_1,
    };
    adc_oneshot_new_unit(&unit_cfg, &adc_handle);

    adc_oneshot_chan_cfg_t chan_cfg = {
        .atten    = ADC_ATTEN_DB_12,  // Full range ~0-3.3V
        .bitwidth = ADC_BITWIDTH_12,  // 0-4095
    };
    adc_oneshot_config_channel(adc_handle, ldr_channel, &chan_cfg);

    // Initialize ADC for hygrometer
    adc_oneshot_chan_cfg_t hygrometer_chan_cfg = {
        .atten    = ADC_ATTEN_DB_12,  // Full range ~0-3.3V
        .bitwidth = ADC_BITWIDTH_12,  // 0-4095
    };
    adc_oneshot_config_channel(adc_handle, hygrometer_channel, &hygrometer_chan_cfg);

    ESP_LOGI(SENSORS_TAG, "ADC channels configured: LDR=%d Hygrometer=%d", (int)ldr_channel,
             (int)hygrometer_channel);
}

void sensors_update(SensorData* data) {
    // Read LDR value
    int ldr_value = 0;
    adc_oneshot_read(adc_handle, ldr_channel, &ldr_value);
    if (ldr_value < LDR_MIN)
        ldr_value = LDR_MIN;
    float light_pct = (float)(ldr_value - LDR_MIN) / (LDR_MAX - LDR_MIN) * 100.0f;

    data->light_level      = (float)ldr_value;  // 0-4095
    data->light_percentage = light_pct;         // 0-100%
    ESP_LOGI(SENSORS_TAG, "LDR value: %d, %f%%", ldr_value, light_pct);

    // Read and average hygrometer value to reduce ADC noise.
    int hygrometer_sum = 0;
    for (int i = 0; i < HYGROMETER_SAMPLES; ++i) {
        int sample = 0;
        adc_oneshot_read(adc_handle, hygrometer_channel, &sample);
        hygrometer_sum += sample;
    }
    int hygrometer_value = hygrometer_sum / HYGROMETER_SAMPLES;

    if (hygrometer_value >= 4090) {
        ESP_LOGW(SENSORS_TAG,
                 "Hygrometer ADC near saturation (%d). AO voltage may be above ADC input range.",
                 hygrometer_value);
    }

    data->humidity_level = hygrometer_raw_to_percent(hygrometer_value);  // 0-100%
    ESP_LOGI(SENSORS_TAG, "Hygrometer raw: %d, moisture: %.1f%%", hygrometer_value,
             data->humidity_level);
}