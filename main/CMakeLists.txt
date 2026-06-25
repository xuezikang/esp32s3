idf_component_register(
    SRCS "app_main.c"
    INCLUDE_DIRS "."
    REQUIRES
        app_ui
        esp_event
        esp_wifi
        health_model
        max30102
        mqtt_uploader
        nvs_flash
        nv3030b
)
