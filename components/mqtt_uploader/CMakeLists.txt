idf_component_register(
    SRCS "mqtt_uploader.c"
    INCLUDE_DIRS "include"
    REQUIRES mqtt health_model
)
