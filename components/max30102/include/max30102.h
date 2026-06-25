#pragma once

#include "driver/i2c.h"
#include "esp_err.h"
#include "health_model.h"

typedef struct {
    i2c_port_t port;
    gpio_num_t sda_gpio;
    gpio_num_t scl_gpio;
    uint32_t clock_hz;
} max30102_config_t;

esp_err_t max30102_init(const max30102_config_t *config);
esp_err_t max30102_read_sample(health_sample_t *sample);
