from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


def read(path: str) -> str:
    return (ROOT / path).read_text(encoding="utf-8")


def test_esp_idf_project_files_exist():
    required = [
        "CMakeLists.txt",
        "main/CMakeLists.txt",
        "main/Kconfig.projbuild",
        "main/app_config.h",
        "main/app_main.c",
        "components/health_model/CMakeLists.txt",
        "components/health_model/include/health_model.h",
        "components/health_model/health_model.c",
        "components/max30102/CMakeLists.txt",
        "components/max30102/include/max30102.h",
        "components/max30102/max30102.c",
        "components/nv3030b/CMakeLists.txt",
        "components/nv3030b/include/nv3030b.h",
        "components/nv3030b/nv3030b.c",
        "components/app_ui/CMakeLists.txt",
        "components/app_ui/include/app_ui.h",
        "components/app_ui/app_ui.c",
        "components/mqtt_uploader/CMakeLists.txt",
        "components/mqtt_uploader/include/mqtt_uploader.h",
        "components/mqtt_uploader/mqtt_uploader.c",
        "README.md",
        ".gitignore",
        "LICENSE",
    ]

    missing = [path for path in required if not (ROOT / path).exists()]

    assert missing == []


def test_kconfig_exposes_hardware_and_cloud_settings():
    kconfig = read("main/Kconfig.projbuild")

    expected_symbols = [
        "HEALTH_MONITOR_SIMULATE_SENSOR",
        "HEALTH_MONITOR_WIFI_SSID",
        "HEALTH_MONITOR_WIFI_PASSWORD",
        "HEALTH_MONITOR_MQTT_URI",
        "HEALTH_MONITOR_MQTT_TOPIC",
        "HEALTH_MONITOR_I2C_SDA_GPIO",
        "HEALTH_MONITOR_I2C_SCL_GPIO",
        "HEALTH_MONITOR_LCD_MOSI_GPIO",
        "HEALTH_MONITOR_LCD_SCLK_GPIO",
        "HEALTH_MONITOR_LCD_CS_GPIO",
        "HEALTH_MONITOR_LCD_DC_GPIO",
        "HEALTH_MONITOR_LCD_RST_GPIO",
        "HEALTH_MONITOR_LCD_BL_GPIO",
    ]

    for symbol in expected_symbols:
        assert symbol in kconfig


def test_application_wires_expected_runtime_modules():
    app = read("main/app_main.c")

    for token in [
        "xTaskCreate",
        "health_model_next_simulated_sample",
        "max30102_read_sample",
        "app_ui_update",
        "mqtt_uploader_publish_sample",
        "nv3030b_init",
    ]:
        assert token in app


def test_readme_documents_resume_aligned_chain():
    readme = read("README.md")

    for phrase in [
        "ESP32-S3",
        "MAX30102",
        "NV3030B",
        "FreeRTOS",
        "LVGL",
        "MQTT",
        "idf.py set-target esp32s3",
        "Espressif VS Code",
        "sensor acquisition",
        "local display",
        "cloud upload",
    ]:
        assert phrase in readme
