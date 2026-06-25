#include <string.h>

#include "app_config.h"
#include "app_ui.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "health_model.h"
#include "max30102.h"
#include "mqtt_uploader.h"
#include "nvs_flash.h"
#include "nv3030b.h"

static const char *TAG = "health_monitor";
static QueueHandle_t s_sample_queue;
static EventGroupHandle_t s_wifi_events;
static const int WIFI_CONNECTED_BIT = BIT0;

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    (void)arg;
    (void)event_data;

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        xEventGroupClearBits(s_wifi_events, WIFI_CONNECTED_BIT);
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        xEventGroupSetBits(s_wifi_events, WIFI_CONNECTED_BIT);
    }
}

static void wifi_start(void)
{
    if (strlen(CONFIG_HEALTH_MONITOR_WIFI_SSID) == 0) {
        ESP_LOGW(TAG, "Wi-Fi SSID is empty; MQTT upload will stay offline");
        return;
    }

    s_wifi_events = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&init_config));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, 0, 0));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, 0, 0));

    wifi_config_t wifi_config = {0};
    strlcpy((char *)wifi_config.sta.ssid, CONFIG_HEALTH_MONITOR_WIFI_SSID, sizeof(wifi_config.sta.ssid));
    strlcpy((char *)wifi_config.sta.password, CONFIG_HEALTH_MONITOR_WIFI_PASSWORD, sizeof(wifi_config.sta.password));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

static void acquisition_task(void *arg)
{
    (void)arg;

    for (;;) {
        health_sample_t sample;
#if CONFIG_HEALTH_MONITOR_SIMULATE_SENSOR
        sample = health_model_next_simulated_sample();
#else
        esp_err_t err = max30102_read_sample(&sample);
        if (err != ESP_OK) {
            ESP_LOGW(TAG, "MAX30102 read failed: %s", esp_err_to_name(err));
            sample = health_model_next_simulated_sample();
            sample.valid = false;
        }
#endif
        xQueueOverwrite(s_sample_queue, &sample);
        vTaskDelay(pdMS_TO_TICKS(HEALTH_MONITOR_SAMPLE_PERIOD_MS));
    }
}

static void ui_task(void *arg)
{
    (void)arg;
    health_sample_t sample = health_model_next_simulated_sample();

    for (;;) {
        health_sample_t next;
        if (xQueuePeek(s_sample_queue, &next, 0) == pdTRUE) {
            sample = next;
        }
        app_ui_update(&sample, mqtt_uploader_is_connected());
        app_ui_tick();
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

static void mqtt_task(void *arg)
{
    (void)arg;
    health_sample_t sample;

    for (;;) {
        if (xQueueReceive(s_sample_queue, &sample, portMAX_DELAY) == pdTRUE) {
            esp_err_t err = mqtt_uploader_publish_sample(&sample);
            if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
                ESP_LOGW(TAG, "MQTT publish failed: %s", esp_err_to_name(err));
            }
        }
    }
}

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());

    s_sample_queue = xQueueCreate(HEALTH_MONITOR_QUEUE_DEPTH, sizeof(health_sample_t));
    if (s_sample_queue == 0) {
        ESP_LOGE(TAG, "Failed to create sample queue");
        return;
    }

    nv3030b_config_t display_config = {
        .host = HEALTH_MONITOR_SPI_HOST,
        .mosi_gpio = CONFIG_HEALTH_MONITOR_LCD_MOSI_GPIO,
        .sclk_gpio = CONFIG_HEALTH_MONITOR_LCD_SCLK_GPIO,
        .cs_gpio = CONFIG_HEALTH_MONITOR_LCD_CS_GPIO,
        .dc_gpio = CONFIG_HEALTH_MONITOR_LCD_DC_GPIO,
        .rst_gpio = CONFIG_HEALTH_MONITOR_LCD_RST_GPIO,
        .bl_gpio = CONFIG_HEALTH_MONITOR_LCD_BL_GPIO,
        .pixel_clock_hz = 20 * 1000 * 1000,
    };
    ESP_ERROR_CHECK(nv3030b_init(&display_config));
    ESP_ERROR_CHECK(app_ui_init());

#if !CONFIG_HEALTH_MONITOR_SIMULATE_SENSOR
    max30102_config_t sensor_config = {
        .port = HEALTH_MONITOR_I2C_PORT,
        .sda_gpio = CONFIG_HEALTH_MONITOR_I2C_SDA_GPIO,
        .scl_gpio = CONFIG_HEALTH_MONITOR_I2C_SCL_GPIO,
        .clock_hz = 400000,
    };
    ESP_ERROR_CHECK(max30102_init(&sensor_config));
#endif

    wifi_start();
    if (strlen(CONFIG_HEALTH_MONITOR_WIFI_SSID) > 0) {
        mqtt_uploader_config_t mqtt_config = {
            .broker_uri = CONFIG_HEALTH_MONITOR_MQTT_URI,
            .topic = CONFIG_HEALTH_MONITOR_MQTT_TOPIC,
        };
        ESP_ERROR_CHECK(mqtt_uploader_start(&mqtt_config));
    }

    xTaskCreate(acquisition_task, "health_acquisition", 4096, 0, 5, 0);
    xTaskCreate(ui_task, "health_ui", 4096, 0, 4, 0);
    xTaskCreate(mqtt_task, "health_mqtt", 4096, 0, 4, 0);
}
