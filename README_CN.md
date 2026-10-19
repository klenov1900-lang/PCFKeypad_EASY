# ⌨️ PCFKeypad_EASY

**语言：** [Русский](README.md) · 中文

[![Version](https://img.shields.io/badge/version-1.0.0-blue)](https://github.com/klenov1900-lang/PCFKeypad_EASY/releases)
[![License](https://img.shields.io/badge/license-MIT-green)](LICENSE)
[![Arduino](https://img.shields.io/badge/Arduino-Nano%20%7C%20Uno%20%7C%20Pro%20Mini-teal)](https://www.arduino.cc/)
[![Platform](https://img.shields.io/badge/platform-AVR-orange)](https://www.microchip.com/)
[![Status](https://img.shields.io/badge/status-stable-brightgreen)](https://github.com/klenov1900-lang/PCFKeypad_EASY)
[![Library Manager](https://img.shields.io/badge/Arduino%20Library%20Manager-approved-blue)](https://www.arduino.cc/reference/en/libraries/)

通过 **PCF8574** I2C 扩展器连接 **4×4** 矩阵键盘，专为 ATmega328P 设计。使用 `I2C_EASY` 作为底层 TWI 驱动。**不依赖 `Wire.h`。** 属于 `*_EASY` 系列。

## ✨ 特点

- ✅ **使用 `I2C_EASY`** — 不依赖 `Wire.h`
- ✅ **4×4 矩阵扫描** — 支持全部 16 个按键
- ✅ **按键防抖** — 可配置（默认 50 毫秒）
- ✅ **自动连发** — 支持长按及连发
- ✅ **省电模式** — 空闲超时后自动休眠
- ✅ **防鬼键** — 忽略虚假按键触发
- ✅ **多键屏蔽** — 仅返回单一明确的按键
- ✅ **I2C 错误处理** — 提供错误代码
- ✅ **自定义引脚和布局** — 支持非标准键盘
- ✅ **中断支持** — 可选
- ✅ **C 语言兼容** — `extern "C"`

## ⚙️ 要求与兼容性

- **微控制器：** ATmega328P（8 位 AVR）。
- **开发板：** Arduino Uno, Nano, Pro Mini。
- **扩展芯片：** PCF8574（默认地址 0x20）。
- **依赖项：** [`I2C_EASY`](https://github.com/klenov1900-lang/I2C_EASY)。
- **其他平台：** ESP8266, ESP32, STM32 — **不支持**（无法直接访问 AVR 寄存器）。

## 📦 安装

### 方法 1：通过 Arduino IDE 库管理器

1. 打开 Arduino IDE
2. 转到 **项目 → 加载库 → 管理库…**
3. 搜索 `PCFKeypad_EASY`
4. 点击 **安装**

### 方法 2：手动安装 (ZIP)

1. 下载 [最新版本](https://github.com/klenov1900-lang/PCFKeypad_EASY/archive/refs/heads/main.zip) 的 ZIP 压缩包
2. Arduino IDE：**项目 → 加载库 → 添加 .ZIP 库…**
3. 选择下载的压缩包

### 方法 3：通过 Git

```bash
cd ~/Arduino/libraries/
git clone https://github.com/klenov1900-lang/I2C_EASY.git
git clone https://github.com/klenov1900-lang/PCFKeypad_EASY.git
```

## 🔌 连接

### 🧩 接线图

| 🔌 Arduino Nano | ↔️ | 🧠 PCF8574 | ↔️ | ⌨️ 键盘 4x4 |
| :--- | :---: | :--- | :---: | :--- |
| 📍 **A4 (SDA)** | → | **SDA** | | |
| 📍 **A5 (SCL)** | → | **SCL** | | |
| ⚡ **5V** | → | **VCC** | | |
| ⏚ **GND** | → | **GND** | | |
| | | **P4** | → | 🔴 **R1** (行 1) |
| | | **P5** | → | 🔴 **R2** (行 2) |
| | | **P6** | → | 🔴 **R3** (行 3) |
| | | **P7** | → | 🔴 **R4** (行 4) |
| | | **P0** | → | 🔵 **C1** (列 1) |
| | | **P1** | → | 🔵 **C2** (列 2) |
| | | **P2** | → | 🔵 **C3** (列 3) |
| | | **P3** | → | 🔵 **C4** (列 4) |

### 📏 键盘排线定义

| 🔢 线号 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
| :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: |
| **📍 引脚** | 🟥 R1 | 🟥 R2 | 🟥 R3 | 🟥 R4 | 🟦 C1 | 🟦 C2 | 🟦 C3 | 🟦 C4 |

### 🏷️ PCF8574 地址

- 默认：**0x20**
- 如果短接 A0-A2 焊盘：地址会改变
- 可以使用 I2C 扫描器检查地址 (`i2c_scan()` 来自 [`I2C_EASY`](https://github.com/klenov1900-lang/I2C_EASY))

## 🚀 快速开始

```cpp
#include <I2C_EASY.h>
#include <PCFKeypad_EASY.h>

void setup() {
    Serial.begin(9600);
    i2c_init(400000);

    pcfpad_init(0x20);   // PCF8574 地址

    if (!pcfpad_is_connected()) {
        Serial.println(F("PCF8574 not found!"));
        while (1);
    }
}

void loop() {
    uint8_t state = pcfpad_get_key();
    char key = pcfpad_get_current();

    if (state == PCFPAD_KEY_PRESSED) {
        Serial.print(F("Key: "));
        Serial.println(key);
    }
    else if (state == PCFPAD_KEY_HOLD) {
        Serial.print(F("Hold: "));
        Serial.println(key);
    }
}
```

## 📖 示例

| 📂 示例 | 📝 描述 | 📦 依赖项 |
|---|---|---|
| **basic** | 最小示例 — 将按键输出到串口 | `PCFKeypad_EASY` |
| **Custom** | 自定义引脚和布局 | `PCFKeypad_EASY` |
> **注意：** 示例 `Custom` 演示了如何通过 `pcfpad_init_custom()` 使用非标准键盘。

## 📚 API

### ⚙️ 初始化

| 🛠️ 函数 | 📝 描述 |
|---|---|
| `pcfpad_init(addr)` | 初始化（标准引脚和布局） |
| `pcfpad_init_custom(addr, rows, cols, keys)` | 自定义引脚和布局 |
| `pcfpad_is_connected()` | 检查连接 (1 = 已连接) |
| `pcfpad_get_error()` | 最后一次 I2C 错误代码 |
| `pcfpad_reset()` | 重置状态 |

### 🔍 轮询

| 🛠️ 函数 | 📝 描述 |
|---|---|
| `pcfpad_get_key()` | 状态 (`PCFPAD_KEY_*`) |
| `pcfpad_get_current()` | 当前字符 |
| `pcfpad_is_pressed()` | 是否有任何按键被按下 |

### 🎛️ 配置

| 🛠️ 函数 | 📝 描述 | 🔢 默认值 |
|---|---|---|
| `pcfpad_set_debounce(ms)` | 防抖时间 | 50 毫秒 |
| `pcfpad_set_hold_delay(ms)` | 连发延迟 | 800 毫秒 |
| `pcfpad_set_repeat_delay(ms)` | 连发间隔 | 150 毫秒 |
| `pcfpad_set_scan_interval(ms)` | 扫描间隔 | 1 毫秒 |
| `pcfpad_set_active_low(enable)` | 信号极性 | 1 (true) |

### 🔋 省电模式

| 🛠️ 函数 | 📝 描述 |
|---|---|
| `pcfpad_set_power_save(enable)` | 开启/关闭 |
| `pcfpad_set_power_save_timeout(ms)` | 休眠超时时间 |
| `pcfpad_is_power_save()` | 检查模式 |

### ⚡ 中断

| 🛠️ 函数 | 📝 描述 |
|---|---|
| `pcfpad_handle_interrupt()` | 在 ISR 中调用 |

## 📊 常量

```cpp
// 状态 (由 pcfpad_get_key 返回)
#define PCFPAD_KEY_RELEASED   0      // 按键已释放
#define PCFPAD_KEY_PRESSED    1      // 按键刚被按下
#define PCFPAD_KEY_HOLD       2      // 按键被按住 (自动连发)
#define PCFPAD_KEY_ERROR      0xFE   // I2C 错误
```

## 📊 错误代码

| 🔢 代码 | 🏷️ 常量 | 📝 描述 |
|---|---|---|
| 0 | `I2C_OK` | 成功 |
| 1 | `I2C_ERR_TIMEOUT` | 等待 TWINT 超时 |
| 2 | `I2C_ERR_START` | START 错误 |
| 3 | `I2C_ERR_ADDR_NACK` | 设备无响应 |
| 4 | `I2C_ERR_DATA_NACK` | 数据 NACK |
| 5 | `I2C_ERR_BUS` | 总线状态错误 |
| 6 | `I2C_ERR_ARB_LOST` | 仲裁丢失 |
| 7 | `I2C_ERR_BUS_LOCKED` | SDA 被拉低锁定 |

## 🔔 中断

### ⚡ 中断示例

```cpp
#include <I2C_EASY.h>
#include <PCFKeypad_EASY.h>

#define INT_PIN  2

void setup() {
    Serial.begin(9600);
    i2c_init(400000);
    pcfpad_init(0x20);

    pinMode(INT_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(INT_PIN), []() {
        pcfpad_handle_interrupt();
    }, FALLING);
}

void loop() {
    uint8_t state = pcfpad_get_key();
    if (state == PCFPAD_KEY_PRESSED) {
        Serial.println(pcfpad_get_current());
    }
}
```

> **注意：** `pcfpad_handle_interrupt()` 仅设置一个标志位。所有实际工作都在 `pcfpad_get_key()` 中完成。这确保了 ISR 尽可能短小。

## 💾 资源占用

| 📊 资源 | 📦 已用 | 🟢 剩余 |
|---|---|---|
| ⚡ **Flash** | ~2.5 КБ (8%) | ~29 КБ |
| 💾 **RAM** | ~50 字节 (2%) | ~2 КБ |

> **注意：** `PCFKeypad_EASY` 使用 **静态缓冲区**（不使用 `malloc`）。RAM 消耗极小且固定。

## 📂 目录结构

```text
PCFKeypad_EASY/
├── examples/
│   ├── Basic/
│   │   └── Basic.ino
│   └── Custom/
│       └── Custom.ino
├── src/
│   ├── PCFKeypad_EASY.h
│   └── PCFKeypad_EASY.cpp
├── library.properties
├── keywords.txt
├── README.md
├── README_CN.md
└── LICENSE
```

## 📝 版本历史

| 🏷️ 版本 | 📅 日期 | 📝 变更 |
|---|---|---|
| **v1.0.0** | 2026-10-19 | 首次发布。基础 API、2 个示例、诊断功能。 |

## 📄 许可证

MIT License. 参见 [LICENSE](LICENSE)。

## 📧 联系方式

- **Email:** gray_wolf19@mail.ru
- **GitHub:** [klenov1900-lang](https://github.com/klenov1900-lang)

---

⭐ 如果这个库对您有帮助，请在 GitHub 上点个星！⭐
