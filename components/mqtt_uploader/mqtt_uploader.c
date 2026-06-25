#include "mqtt_uploader.h"

#include <stdio.h>

#include "esp_log.h"
#include "mqtt_client.h"

static const char *TAG = "mqtt_uploader";
static esp_mqtt_client_handle_t s_client;
static const char *s_topic;
static bool s_connected;

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    (void)handler_args;
    (void)base;
    (void)event_data;

    if (event_id == MQTT_EVENT_CONNECTED) {
        s_connected = true;
        ESP_LOGI(TAG, "MQTT connected");
    } else if (event_id == MQTT_EVENT_DISCONNECTED) {
        s_connected = false;
        ESP_LOGW(TAG, "MQTT disconnected");
    }
}

esp_err_t mqtt_uploader_start(const mqtt_uploader_config_t *config)
{
    if (config == 0 || config->broker_uri == 0 || config->topic == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    s_topic = config->topic;
    esp_mqtt_client_config_t mqtt_config = {
        .broker.address.uri = config->broker_uri,
    };
    s_client = esp_mqtt_client_init(&mqtt_config);
    if (s_client == 0) {
        return ESP_FAIL;
    }

    ESP_ERROR_CHECK(esp_mqtt_client_register_event(s_client, ESP_EVENT_ANY_ID, mqtt_event_handler, 0));
    return esp_mqtt_client_start(s_client);
}

bool mqtt_uploader_is_connected(void)
{
    return s_connected;
}

esp_err_t mqtt_uploader_publish_sample(const health_sample_t *sample)
{
    if (sample == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    if (s_client == 0 || !s_connected) {
        return ESP_ERR_INVALID_STATE;
    }

    char payload[192];
    int len = snprintf(
        payload,
        sizeof(payload),
        "{\"heart_rate_bpm\":%d,\"spo2_percent\":%d,\"raw_red\":%u,\"raw_ir\":%u,\"valid\":%s,\"source\":\"%s\"}",
        sample->heart_rate_bpm,
        sample->spo2_percent,
        sample->raw_red,
        sample->raw_ir,
        sample->valid ? "true" : "false",
        sample->simulated ? "simulated" : "max30102");

    if (len <= 0 || len >= (int)sizeof(payload)) {
        return ESP_ERR_NO_MEM;
    }

    int msg_id = esp_mqtt_client_publish(s_client, s_topic, payload, 0, 1, 0);
    return msg_id >= 0 ? ESP_OK : ESP_FAIL;
}
