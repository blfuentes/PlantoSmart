#include "sensors.h"

#include <esp_adc/adc_cali.h>
#include <esp_adc/adc_cali_scheme.h>
#include <esp_adc/adc_oneshot.h>
#include <esp_err.h>
#include <esp_log.h>

static const char* SENSORS_TAG = "SENSORS";

#define ADC_SATURATION_RAW 4090
#define LDR_MIN            1300  // dark (~20k LDR with 10k divider)
#define LDR_MAX            4050  // bright (~0.1k LDR with 10k divider)
#define LDR_SAMPLES        8
#define HYGROMETER_WET_RAW 2100  // under water
#define HYGROMETER_DRY_RAW 3950  // in open air
#define HYGROMETER_SAMPLES 8

static adc_oneshot_unit_handle_t adc_handle;
static adc_channel_t ldr_channel        = ADC_CHANNEL_2;
static adc_channel_t hygrometer_channel = ADC_CHANNEL_3;
static adc_cali_handle_t adc_cali_handle;
static bool adc_cali_enabled = false;
static bool s_has_ldr        = false;

static bool adc_calibration_init(void) {
    esp_err_t ret = ESP_FAIL;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id  = ADC_UNIT_1,
        .atten    = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
    };
    ret = adc_cali_create_scheme_curve_fitting(&cali_config, &adc_cali_handle);
#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    adc_cali_line_fitting_config_t cali_config = {
        .unit_id      = ADC_UNIT_1,
        .atten        = ADC_ATTEN_DB_12,
        .bitwidth     = ADC_BITWIDTH_12,
        .default_vref = 1100,
    };
    ret = adc_cali_create_scheme_line_fitting(&cali_config, &adc_cali_handle);
#endif

    if (ret == ESP_OK) {
        ESP_LOGI(SENSORS_TAG, "ADC calibration enabled");
        return true;
    }

    ESP_LOGW(SENSORS_TAG, "ADC calibration not available: %s", esp_err_to_name(ret));
    return false;
}

static uint8_t ldr_raw_to_percent(int raw_value) {
    int clamped_raw = raw_value;
    if (clamped_raw < LDR_MIN) {
        clamped_raw = LDR_MIN;
    }
    if (clamped_raw > LDR_MAX) {
        clamped_raw = LDR_MAX;
    }

    int pct = (clamped_raw - LDR_MIN) * 100 / (LDR_MAX - LDR_MIN);
    if (pct < 0) {
        pct = 0;
    }
    if (pct > 100) {
        pct = 100;
    }

    return (uint8_t)pct;
}

static uint8_t hygrometer_raw_to_percent(int raw_value) {
    int clamped_raw = raw_value;
    if (clamped_raw < HYGROMETER_WET_RAW) {
        clamped_raw = HYGROMETER_WET_RAW;
    }
    if (clamped_raw > HYGROMETER_DRY_RAW) {
        clamped_raw = HYGROMETER_DRY_RAW;
    }

    // Many resistive probes report lower voltage when wetter.
    int pct = (HYGROMETER_DRY_RAW - clamped_raw) * 100 / (HYGROMETER_DRY_RAW - HYGROMETER_WET_RAW);
    if (pct < 0) {
        pct = 0;
    }
    if (pct > 100) {
        pct = 100;
    }
    return (uint8_t)pct;
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
    if (config != NULL) {
        s_has_ldr = config->has_ldr;
    }

    if (s_has_ldr && (config != NULL) && !adc_channel_from_gpio(config->ldr_pin, &ldr_channel)) {
        ESP_LOGW(SENSORS_TAG, "Invalid LDR GPIO for ADC: %d. Using ADC_CHANNEL_2.",
                 (int)config->ldr_pin);
        ldr_channel = ADC_CHANNEL_2;
    }

    if ((config != NULL) && !adc_channel_from_gpio(config->hygrometer_pin, &hygrometer_channel)) {
        ESP_LOGW(SENSORS_TAG, "Invalid hygrometer GPIO for ADC: %d. Using ADC_CHANNEL_3.",
                 (int)config->hygrometer_pin);
        hygrometer_channel = ADC_CHANNEL_3;
    }

    // Initialize ADC unit (shared by all channels)
    adc_oneshot_unit_init_cfg_t unit_cfg = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit_cfg, &adc_handle));

    adc_oneshot_chan_cfg_t chan_cfg = {
        .atten    = ADC_ATTEN_DB_12,  // Full range ~0-3.3V
        .bitwidth = ADC_BITWIDTH_12,  // 0-4095
    };
    if (s_has_ldr) {
        adc_oneshot_config_channel(adc_handle, ldr_channel, &chan_cfg);
    } else {
        ESP_LOGI(SENSORS_TAG, "LDR sensor absent, skipping LDR channel configuration");
    }

    // Initialize ADC for hygrometer
    adc_oneshot_chan_cfg_t hygrometer_chan_cfg = {
        .atten    = ADC_ATTEN_DB_12,  // Full range ~0-3.3V
        .bitwidth = ADC_BITWIDTH_12,  // 0-4095
    };
    adc_oneshot_config_channel(adc_handle, hygrometer_channel, &hygrometer_chan_cfg);

    adc_cali_enabled = adc_calibration_init();

    ESP_LOGI(SENSORS_TAG, "ADC channels: LDR=%s Hygrometer=%d", s_has_ldr ? "enabled" : "disabled",
             (int)hygrometer_channel);
}

void sensors_update(SensorData* data) {
    if (s_has_ldr) {
        // Read and average LDR value to reduce ADC noise.
        int ldr_sum = 0;
        for (int i = 0; i < LDR_SAMPLES; ++i) {
            int sample = 0;
            if (adc_oneshot_read(adc_handle, ldr_channel, &sample) != ESP_OK) {
                ESP_LOGW(SENSORS_TAG, "LDR ADC read failed on sample %d", i);
            }
            ldr_sum += sample;
        }
        int ldr_value = ldr_sum / LDR_SAMPLES;
        int ldr_mv    = 0;
        if (adc_cali_enabled) {
            adc_cali_raw_to_voltage(adc_cali_handle, ldr_value, &ldr_mv);
        }

        if (ldr_value >= ADC_SATURATION_RAW) {
            ESP_LOGW(SENSORS_TAG,
                     "LDR ADC near saturation (%d). The divider is overdriving the ADC in bright "
                     "light; use a smaller fixed resistor or reverse the divider.",
                     ldr_value);
        }

        int ldr_clamped = ldr_value;
        if (ldr_clamped < LDR_MIN) {
            ldr_clamped = LDR_MIN;
        }
        if (ldr_clamped > LDR_MAX) {
            ldr_clamped = LDR_MAX;
        }

        uint8_t light_pct      = ldr_raw_to_percent(ldr_value);
        data->light_level      = (uint16_t)ldr_value;
        data->light_percentage = light_pct;
        if (adc_cali_enabled) {
            ESP_LOGI(SENSORS_TAG, "LDR raw: %d, %dmV, clamped: %d, %d%%", ldr_value, ldr_mv,
                     ldr_clamped, light_pct);
        } else {
            ESP_LOGI(SENSORS_TAG, "LDR raw: %d, clamped: %d, %d%%", ldr_value, ldr_clamped,
                     light_pct);
        }
    } else {
        data->light_level      = 0;
        data->light_percentage = 0;
    }

    // Read and average hygrometer value to reduce ADC noise.
    int hygrometer_sum = 0;
    for (int i = 0; i < HYGROMETER_SAMPLES; ++i) {
        int sample = 0;
        if (adc_oneshot_read(adc_handle, hygrometer_channel, &sample) != ESP_OK) {
            ESP_LOGW(SENSORS_TAG, "Hygrometer ADC read failed on sample %d", i);
        }
        hygrometer_sum += sample;
    }
    int hygrometer_value = hygrometer_sum / HYGROMETER_SAMPLES;

    if (hygrometer_value >= 4090) {
        ESP_LOGW(SENSORS_TAG,
                 "Hygrometer ADC near saturation (%d). AO voltage may be above ADC input range.",
                 hygrometer_value);
    }

    data->humidity_level = hygrometer_raw_to_percent(hygrometer_value);
    ESP_LOGI(SENSORS_TAG, "Hygrometer raw: %d, moisture: %d%%", hygrometer_value,
             data->humidity_level);
}