# ESP32-S3 Portable Health Monitor Design

## Goal

Recreate the resume project as an open-source ESP-IDF project that demonstrates the full "sensor acquisition - local display - MQTT upload" path for an ESP32-S3 portable health monitor.

## Scope

The first public version must build a credible project skeleton that can be opened by Espressif's VS Code ESP-IDF extension. It defaults to simulated heart-rate and SpO2 data so the application flow is visible without hardware. Real hardware integration points are present for the MAX30102 sensor and NV3030B display, with board pins controlled through Kconfig.

## Architecture

- `main`: initializes NVS, Wi-Fi, display, LVGL UI, MQTT, and FreeRTOS tasks.
- `components/health_model`: owns health sample data, filtering-friendly structures, and simulated sample generation.
- `components/max30102`: I2C driver boundary for the MAX30102 heart-rate and blood-oxygen sensor.
- `components/nv3030b`: SPI display panel boundary for the NV3030B screen.
- `components/app_ui`: LVGL dashboard with heart rate, SpO2, connection status, and sample freshness.
- `components/mqtt_uploader`: MQTT publishing boundary for health samples.

## Data Flow

`MAX30102 or simulator -> acquisition task -> queue -> UI task + MQTT task`

The acquisition task periodically produces `health_sample_t`. The UI task renders the latest sample to LVGL. The MQTT task publishes JSON telemetry when Wi-Fi and MQTT are enabled.

## Configuration

Kconfig exposes simulation mode, Wi-Fi SSID/password, MQTT broker URI/topic, MAX30102 I2C pins, and NV3030B SPI pins. Defaults are chosen to compile safely and avoid hard-coded private credentials.

## Verification

Because the current machine may not have ESP-IDF installed, the repository includes Python structure tests that verify the expected source files, project metadata, config options, and README content. When ESP-IDF is available, the README documents `idf.py set-target esp32s3 build flash monitor`.
