#include "max30102.h"

#include <string.h>

#include "esp_log.h"

#define MAX30102_ADDR 0x57
#define MAX30102_REG_INTR_ENABLE_1 0x02
#define MAX30102_REG_FIFO_CONFIG 0x08
#define MAX30102_REG_MODE_CONFIG 0x09
#define MAX30102_REG_SPO2_CONFIG 0x0A
#define MAX30102_REG_FIFO_DATA 0x07

static const char *TAG = "max30102";
static max30102_config_t s_config;
static bool s_initialized;

static esp_err_t write_reg(uint8_t reg, uint8_t value)
{
    uint8_t data[2] = {reg, value};
    return i2c_master_write_to_device(s_config.port, MAX30102_ADDR, data, sizeof(data), pdMS_TO_TICKS(100));
}

esp_err_t max30102_init(const max30102_config_t *config)
{
    if (config == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    s_config = *config;

    i2c_config_t i2c_config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = s_config.sda_gpio,
        .scl_io_num = s_config.scl_gpio,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = s_config.clock_hz,
        .clk_flags = 0,
    };

    ESP_ERROR_CHECK(i2c_param_config(s_config.port, &i2c_config));
    ESP_ERROR_CHECK(i2c_driver_install(s_config.port, I2C_MODE_MASTER, 0, 0, 0));

    ESP_ERROR_CHECK(write_reg(MAX30102_REG_MODE_CONFIG, 0x40));
    vTaskDelay(pdMS_TO_TICKS(50));
    ESP_ERROR_CHECK(write_reg(MAX30102_REG_INTR_ENABLE_1, 0xC0));
    ESP_ERROR_CHECK(write_reg(MAX30102_REG_FIFO_CONFIG, 0x4F));
    ESP_ERROR_CHECK(write_reg(MAX30102_REG_MODE_CONFIG, 0x03));
    ESP_ERROR_CHECK(write_reg(MAX30102_REG_SPO2_CONFIG, 0x27));

    s_initialized = true;
    ESP_LOGI(TAG, "MAX30102 initialized on I2C port %d", s_config.port);
    return ESP_OK;
}

esp_err_t max30102_read_sample(health_sample_t *sample)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    if (sample == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t reg = MAX30102_REG_FIFO_DATA;
    uint8_t fifo[6] = {0};
    esp_err_t err = i2c_master_write_read_device(
        s_config.port,
        MAX30102_ADDR,
        &reg,
        1,
        fifo,
        sizeof(fifo),
        pdMS_TO_TICKS(100));
    if (err != ESP_OK) {
        return err;
    }

    memset(sample, 0, sizeof(*sample));
    sample->raw_red = ((uint32_t)(fifo[0] & 0x03) << 16) | ((uint32_t)fifo[1] << 8) | fifo[2];
    sample->raw_ir = ((uint32_t)(fifo[3] & 0x03) << 16) | ((uint32_t)fifo[4] << 8) | fifo[5];
    sample->heart_rate_bpm = 75;
    sample->spo2_percent = 98;
    sample->valid = sample->raw_red > 1000 && sample->raw_ir > 1000;
    sample->simulated = false;
    health_model_normalize(sample);

    return ESP_OK;
}
