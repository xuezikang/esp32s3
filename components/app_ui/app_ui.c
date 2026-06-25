#include "app_ui.h"

#include <stdio.h>

#include "lvgl.h"

static lv_obj_t *s_hr_label;
static lv_obj_t *s_spo2_label;
static lv_obj_t *s_source_label;
static lv_obj_t *s_mqtt_label;

esp_err_t app_ui_init(void)
{
    lv_init();

    lv_obj_t *screen = lv_scr_act();
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x101820), 0);

    lv_obj_t *title = lv_label_create(screen);
    lv_label_set_text(title, "ESP32-S3 Health Monitor");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 16);

    s_hr_label = lv_label_create(screen);
    lv_obj_set_style_text_color(s_hr_label, lv_color_hex(0xFF5A5F), 0);
    lv_obj_align(s_hr_label, LV_ALIGN_CENTER, 0, -45);

    s_spo2_label = lv_label_create(screen);
    lv_obj_set_style_text_color(s_spo2_label, lv_color_hex(0x35C2FF), 0);
    lv_obj_align(s_spo2_label, LV_ALIGN_CENTER, 0, 0);

    s_source_label = lv_label_create(screen);
    lv_obj_set_style_text_color(s_source_label, lv_color_hex(0xD6E4E5), 0);
    lv_obj_align(s_source_label, LV_ALIGN_CENTER, 0, 45);

    s_mqtt_label = lv_label_create(screen);
    lv_obj_set_style_text_color(s_mqtt_label, lv_color_hex(0xD6E4E5), 0);
    lv_obj_align(s_mqtt_label, LV_ALIGN_BOTTOM_MID, 0, -18);

    return ESP_OK;
}

void app_ui_update(const health_sample_t *sample, bool mqtt_connected)
{
    if (sample == 0 || s_hr_label == 0) {
        return;
    }

    char line[64];
    snprintf(line, sizeof(line), "HR: %d bpm", sample->heart_rate_bpm);
    lv_label_set_text(s_hr_label, line);

    snprintf(line, sizeof(line), "SpO2: %d%%", sample->spo2_percent);
    lv_label_set_text(s_spo2_label, line);

    snprintf(line, sizeof(line), "Source: %s", sample->simulated ? "simulated" : "MAX30102");
    lv_label_set_text(s_source_label, line);

    snprintf(line, sizeof(line), "MQTT: %s", mqtt_connected ? "connected" : "offline");
    lv_label_set_text(s_mqtt_label, line);
}

void app_ui_tick(void)
{
    lv_timer_handler();
}
