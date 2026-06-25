#include "nv3030b.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "nv3030b";
static spi_device_handle_t s_lcd;

static esp_err_t send_cmd(uint8_t cmd)
{
    spi_transaction_t trans = {
        .length = 8,
        .tx_buffer = &cmd,
    };
    gpio_set_level(CONFIG_HEALTH_MONITOR_LCD_DC_GPIO, 0);
    return spi_device_transmit(s_lcd, &trans);
}

static esp_err_t send_data(const uint8_t *data, int len)
{
    spi_transaction_t trans = {
        .length = len * 8,
        .tx_buffer = data,
    };
    gpio_set_level(CONFIG_HEALTH_MONITOR_LCD_DC_GPIO, 1);
    return spi_device_transmit(s_lcd, &trans);
}

esp_err_t nv3030b_init(const nv3030b_config_t *config)
{
    if (config == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    gpio_config_t output_config = {
        .pin_bit_mask = (1ULL << config->dc_gpio) | (1ULL << config->rst_gpio) | (1ULL << config->bl_gpio),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&output_config));

    spi_bus_config_t bus_config = {
        .mosi_io_num = config->mosi_gpio,
        .miso_io_num = GPIO_NUM_NC,
        .sclk_io_num = config->sclk_gpio,
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
        .max_transfer_sz = 240 * 320 * 2,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(config->host, &bus_config, SPI_DMA_CH_AUTO));

    spi_device_interface_config_t dev_config = {
        .clock_speed_hz = config->pixel_clock_hz,
        .mode = 0,
        .spics_io_num = config->cs_gpio,
        .queue_size = 7,
    };
    ESP_ERROR_CHECK(spi_bus_add_device(config->host, &dev_config, &s_lcd));

    gpio_set_level(config->rst_gpio, 0);
    vTaskDelay(pdMS_TO_TICKS(20));
    gpio_set_level(config->rst_gpio, 1);
    vTaskDelay(pdMS_TO_TICKS(120));

    ESP_ERROR_CHECK(send_cmd(0x11));
    vTaskDelay(pdMS_TO_TICKS(120));
    ESP_ERROR_CHECK(send_cmd(0x3A));
    const uint8_t color_mode = 0x55;
    ESP_ERROR_CHECK(send_data(&color_mode, 1));
    ESP_ERROR_CHECK(send_cmd(0x29));

    gpio_set_level(config->bl_gpio, 1);
    ESP_LOGI(TAG, "NV3030B display boundary initialized");
    return ESP_OK;
}
