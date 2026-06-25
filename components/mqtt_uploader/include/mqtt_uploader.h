#pragma once

#include <stdbool.h>

#include "esp_err.h"
#include "health_model.h"

typedef struct {
    const char *broker_uri;
    const char *topic;
} mqtt_uploader_config_t;

esp_err_t mqtt_uploader_start(const mqtt_uploader_config_t *config);
bool mqtt_uploader_is_connected(void);
esp_err_t mqtt_uploader_publish_sample(const health_sample_t *sample);
