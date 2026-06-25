#pragma once

#include "esp_err.h"
#include "health_model.h"

esp_err_t app_ui_init(void);
void app_ui_update(const health_sample_t *sample, bool mqtt_connected);
void app_ui_tick(void);
