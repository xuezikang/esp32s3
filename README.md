# ESP32-S3 便携式医疗健康监测仪

基于 ESP32-S3 的便携式健康监测设备复现项目，使用 ESP-IDF、FreeRTOS、LVGL 和 MQTT 搭建完整的 IoT 健康数据链路。

本项目对应的核心流程是：

`sensor acquisition -> local display -> cloud upload`

也就是：

`MAX30102 心率血氧采集 -> NV3030B 屏幕本地显示 -> MQTT 云端数据上传`

为了方便开源展示和快速复现，当前版本默认启用模拟健康数据。即使暂时没有 MAX30102 传感器和 NV3030B 显示屏，也可以查看 FreeRTOS 任务划分、LVGL 界面刷新、MQTT 数据发布和工程组织方式。有真实硬件后，只需要在 `idf.py menuconfig` 中关闭模拟模式并配置对应引脚即可切换到实机链路。

## 项目亮点

- 使用 ESP-IDF + CMake 搭建标准 ESP32-S3 工程，可直接用 Espressif VS Code 插件打开。
- 基于 FreeRTOS 拆分采集任务、界面任务和 MQTT 上传任务，模块职责清晰。
- 预留 MAX30102 I2C 驱动边界，负责心率、血氧原始数据读取和样本输出。
- 预留 NV3030B SPI 显示屏初始化边界，便于后续替换完整屏幕初始化序列。
- 使用 LVGL 构建本地健康数据看板，显示心率、血氧、数据来源和 MQTT 状态。
- 使用 MQTT 上传 JSON 格式健康数据，形成端到云的数据闭环。
- 通过 Kconfig 管理 Wi-Fi、MQTT、I2C、SPI 和模拟模式配置，避免把私密参数写死在代码中。

## 系统架构

```text
                 +------------------------+
                 |      ESP32-S3 MCU      |
                 +-----------+------------+
                             |
          +------------------+------------------+
          |                  |                  |
          v                  v                  v
  Acquisition Task       UI Task           MQTT Task
  采集/模拟健康数据      LVGL 界面刷新      云端遥测上传
          |                  |                  |
          v                  v                  v
  MAX30102 / Simulator   NV3030B Display   MQTT Broker
```

运行时数据流：

1. 采集任务周期性读取 MAX30102，或在默认配置下生成模拟心率/血氧数据。
2. 健康样本写入 FreeRTOS 队列，供 UI 和 MQTT 模块消费。
3. UI 任务刷新 LVGL 看板，在 NV3030B 屏幕上展示当前数据。
4. MQTT 任务将最新样本编码为 JSON，并发布到配置的主题。

## 硬件说明

| 模块 | 作用 | 默认配置 |
| --- | --- | --- |
| ESP32-S3 | 主控芯片，运行 FreeRTOS、LVGL 和 MQTT | Target: `esp32s3` |
| MAX30102 | 心率/血氧传感器 | I2C SDA GPIO 8, SCL GPIO 9 |
| NV3030B | 本地显示屏 | MOSI GPIO 11, SCLK GPIO 12, CS GPIO 10, DC GPIO 13, RST GPIO 14, BL GPIO 15 |

> 默认引脚用于快速复现，不代表所有开发板或屏幕模组的最终接线。请以实际 ESP32-S3 开发板、MAX30102 模块和 NV3030B 屏幕排线为准，并在 `idf.py menuconfig` 中调整。

## 软件环境

- ESP-IDF 5.x
- Espressif VS Code Extension
- Python 3.x
- 支持 ESP32-S3 的 USB 串口驱动

## 使用 Espressif VS Code 打开

1. 使用 VS Code 打开本仓库目录。
2. 安装并启用 Espressif VS Code 插件。
3. 选择本机 ESP-IDF 环境。
4. 设置芯片目标：

```powershell
idf.py set-target esp32s3
```

5. 打开配置菜单：

```powershell
idf.py menuconfig
```

6. 进入 `Portable Health Monitor`，根据需要配置 Wi-Fi、MQTT 和硬件引脚。
7. 编译、烧录并打开串口监视器：

```powershell
idf.py build
idf.py -p COM5 flash monitor
```

请把 `COM5` 替换为你电脑上实际的 ESP32-S3 串口号。

## 关键配置项

运行 `idf.py menuconfig` 后进入 `Portable Health Monitor` 菜单：

| 配置项 | 说明 |
| --- | --- |
| `HEALTH_MONITOR_SIMULATE_SENSOR` | 是否启用模拟数据。默认开启，便于无硬件演示。 |
| `HEALTH_MONITOR_WIFI_SSID` | Wi-Fi 名称，用于 MQTT 云端上传。 |
| `HEALTH_MONITOR_WIFI_PASSWORD` | Wi-Fi 密码。 |
| `HEALTH_MONITOR_MQTT_URI` | MQTT Broker 地址，例如 `mqtt://broker.emqx.io`。 |
| `HEALTH_MONITOR_MQTT_TOPIC` | 健康数据上传主题。 |
| `HEALTH_MONITOR_I2C_SDA_GPIO` | MAX30102 的 I2C SDA 引脚。 |
| `HEALTH_MONITOR_I2C_SCL_GPIO` | MAX30102 的 I2C SCL 引脚。 |
| `HEALTH_MONITOR_LCD_*` | NV3030B 的 SPI、复位、背光和数据/命令引脚。 |

## MQTT 数据格式

设备端会向 `CONFIG_HEALTH_MONITOR_MQTT_TOPIC` 发布 JSON 数据：

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

字段说明：

| 字段 | 含义 |
| --- | --- |
| `heart_rate_bpm` | 心率，单位 BPM。 |
| `spo2_percent` | 血氧饱和度，单位百分比。 |
| `raw_red` | MAX30102 红光通道原始值，模拟模式下为生成值。 |
| `raw_ir` | MAX30102 红外通道原始值，模拟模式下为生成值。 |
| `valid` | 当前样本是否有效。 |
| `source` | 数据来源，`simulated` 表示模拟数据，`max30102` 表示真实传感器数据。 |

## 工程结构

```text
main/
  app_main.c              系统初始化、FreeRTOS 任务创建和模块连接
  Kconfig.projbuild       Wi-Fi、MQTT、传感器和显示屏配置项
components/
  health_model/           健康样本结构体、数据归一化、模拟样本生成
  max30102/               MAX30102 I2C 驱动边界
  nv3030b/                NV3030B SPI 显示屏初始化边界
  app_ui/                 LVGL 健康数据看板
  mqtt_uploader/          MQTT 客户端和健康数据发布
tests/
  test_project_contract.py 仓库结构和文档合同测试
```

## 当前复现程度

当前版本重点复现“项目链路”和“工程能力”，适合用于 GitHub 开源展示、简历项目佐证和后续实机开发起点。

已经完成：

- ESP32-S3 ESP-IDF 工程骨架。
- FreeRTOS 多任务结构。
- 健康样本数据模型。
- 默认模拟心率/血氧数据。
- MAX30102 传感器驱动边界。
- NV3030B 显示屏驱动边界。
- LVGL 本地界面模块。
- MQTT JSON 上传模块。
- 中文 README 和可执行的仓库合同测试。

当前仍属于“可编译复现骨架 + 硬件接口预留”的版本。因为不同 NV3030B 屏幕模组的初始化参数、分辨率、偏移方向和背光控制方式可能不同，实际点亮屏幕时需要根据屏幕卖家资料补全初始化表。MAX30102 的心率/血氧算法也需要结合采样率、滤波窗口、手指接触状态和实际标定数据继续优化。

## 后续扩展路线

### 1. 完善 MAX30102 实测算法

当前 `max30102` 模块已经完成 I2C 初始化和 FIFO 数据读取边界。后续可以继续增加：

- DC 去除和滑动平均滤波，降低环境光和手指抖动影响。
- 红光/红外 AC 分量提取，用于计算血氧比值。
- 峰值检测算法，用于从 PPG 波形中估算心率。
- 手指接触检测，避免无效数据误显示。
- 多窗口平滑，提升心率和血氧显示稳定性。

建议先在串口中打印 `raw_red` 和 `raw_ir`，确认波形随手指接触变化明显，再逐步加入滤波和计算逻辑。

### 2. 补全 NV3030B 屏幕驱动

当前 `nv3030b` 模块保留了 SPI 总线初始化、复位、背光和基础命令发送流程。后续需要根据具体屏幕模组补齐：

- 完整 NV3030B 初始化命令表。
- 屏幕分辨率、横竖屏方向和显示偏移。
- LVGL flush 回调，将 LVGL 绘制缓冲区写入屏幕。
- DMA 传输和双缓冲，提高界面刷新效率。
- 背光 PWM 调光，支持低功耗显示。

如果屏幕无法点亮，优先检查供电、电平、SPI 模式、复位时序、背光引脚和初始化表。

### 3. 接入真实云端平台

当前 MQTT 上传模块使用通用 Broker URI 和主题配置。后续可以扩展为：

- EMQX、阿里云 IoT、腾讯云 IoT 或自建 Mosquitto。
- MQTT 用户名、密码、Client ID 配置。
- TLS 证书连接，提高公网传输安全性。
- 设备在线状态、遗嘱消息和重连策略。
- 云端数据库存储和健康趋势图展示。

建议先使用公共测试 Broker 验证发布链路，再迁移到正式云平台。

### 4. 优化设备交互和产品形态

在基本链路稳定后，可以继续补充：

- LVGL 多页面界面，例如实时数据页、历史趋势页、网络状态页。
- 按键或触摸交互，用于切换页面和重新配网。
- NVS 保存 Wi-Fi 和设备配置。
- 低电量提醒、电池电压采样和充电状态显示。
- 异常阈值报警，例如心率过高、血氧过低。

这些扩展能让项目从“功能演示”进一步接近真实便携式健康监测设备。

## 本地验证

仓库提供了轻量级合同测试，用来确认开源项目关键文件、配置项和 README 内容完整：

```powershell
python -m pytest tests -q
```

ESP-IDF 环境可用时，再执行：

```powershell
idf.py set-target esp32s3
idf.py build
```

## 免责声明

本项目用于嵌入式开发学习、作品集展示和工程复现，不属于医疗器械软件。心率和血氧数据仅用于技术演示，不能用于临床诊断、治疗决策或任何医疗用途。
