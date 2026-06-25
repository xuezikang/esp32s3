# ESP32-S3 Portable Health Monitor

An ESP-IDF demo project for a portable medical health monitor based on ESP32-S3. It recreates the resume project chain:

`sensor acquisition -> local display -> cloud upload`

The default firmware uses simulated health samples, so the FreeRTOS tasks, LVGL dashboard, and MQTT payload path can be reviewed without hardware. When hardware is available, disable simulation in `menuconfig` and connect a MAX30102 sensor plus an NV3030B SPI display.

## Features

- ESP32-S3 application using ESP-IDF and CMake.
- FreeRTOS tasks for acquisition, UI refresh, and MQTT upload.
- MAX30102 driver boundary over I2C for heart-rate and SpO2 samples.
- NV3030B display initialization boundary over SPI.
- LVGL dashboard showing heart rate, SpO2, sample mode, and upload status.
- MQTT JSON telemetry publisher for cloud upload.
- Kconfig options for Wi-Fi, MQTT, I2C pins, SPI display pins, and simulation mode.

## Hardware

| Module | Purpose | Default pins |
| --- | --- | --- |
| ESP32-S3 | Main MCU | Target: `esp32s3` |
| MAX30102 | Heart-rate and blood-oxygen sensor | SDA GPIO 8, SCL GPIO 9 |
| NV3030B | Local display | MOSI GPIO 11, SCLK GPIO 12, CS GPIO 10, DC GPIO 13, RST GPIO 14, BL GPIO 15 |

Adjust all pins with `idf.py menuconfig` under `Portable Health Monitor`.

## Open With Espressif VS Code

1. Open this folder in VS Code.
2. Install or enable the Espressif VS Code ESP-IDF extension.
3. Select an ESP-IDF environment.
4. Set the target:

```powershell
idf.py set-target esp32s3
```

5. Configure project options:

```powershell
idf.py menuconfig
```

6. Build, flash, and monitor:

```powershell
idf.py build
idf.py -p COM5 flash monitor
```

Use your actual serial port instead of `COM5`.

## MQTT Payload

The uploader publishes JSON to `CONFIG_HEALTH_MONITOR_MQTT_TOPIC`:

```json
{
  "heart_rate_bpm": 76,
  "spo2_percent": 98,
  "raw_red": 50123,
  "raw_ir": 54567,
  "valid": true,
  "source": "simulated"
}
```

## Configuration

Run `idf.py menuconfig` and open `Portable Health Monitor`.

- `HEALTH_MONITOR_SIMULATE_SENSOR`: keep enabled for a no-hardware demo.
- `HEALTH_MONITOR_WIFI_SSID`: Wi-Fi SSID for cloud upload.
- `HEALTH_MONITOR_WIFI_PASSWORD`: Wi-Fi password.
- `HEALTH_MONITOR_MQTT_URI`: MQTT broker URI, for example `mqtt://broker.emqx.io`.
- `HEALTH_MONITOR_MQTT_TOPIC`: telemetry topic.
- `HEALTH_MONITOR_I2C_*`: MAX30102 I2C pins.
- `HEALTH_MONITOR_LCD_*`: NV3030B SPI and control pins.

## Project Layout

```text
main/
  app_main.c              FreeRTOS task wiring and system initialization
components/
  health_model/           Health sample type and simulated sample generator
  max30102/               MAX30102 I2C driver boundary
  nv3030b/                NV3030B SPI display boundary
  app_ui/                 LVGL dashboard boundary
  mqtt_uploader/          MQTT upload boundary
tests/
  test_project_contract.py Repository contract tests
```

## Current Reproduction Level

This repository is intentionally small and interview-friendly. It proves the complete architecture and integration chain while keeping hardware-specific risk isolated behind driver boundaries. For a physical product build, the next step is to tune the MAX30102 filtering algorithm and replace the NV3030B initialization table with the exact vendor sequence for your screen module.

## Local Checks

Run the repository contract tests:

```powershell
python -m pytest tests -q
```
