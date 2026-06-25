#pragma once

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_err.h"

typedef struct {
    spi_host_device_t host;
    gpio_num_t mosi_gpio;
    gpio_num_t sclk_gpio;
    gpio_num_t cs_gpio;
    gpio_num_t dc_gpio;
    gpio_num_t rst_gpio;
    gpio_num_t bl_gpio;
    int pixel_clock_hz;
} nv3030b_config_t;

esp_err_t nv3030b_init(const nv3030b_config_t *config);
