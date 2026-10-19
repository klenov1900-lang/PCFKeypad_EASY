# ⌨️ PCFKeypad_EASY

**Языки:** Русский · [中文](README_CN.md)

[![Version](https://img.shields.io/badge/version-1.0.0-blue)](https://github.com/klenov1900-lang/PCFKeypad_EASY/releases)
[![License](https://img.shields.io/badge/license-MIT-green)](LICENSE)
[![Arduino](https://img.shields.io/badge/Arduino-Nano%20%7C%20Uno%20%7C%20Pro%20Mini-teal)](https://www.arduino.cc/)
[![Platform](https://img.shields.io/badge/platform-AVR-orange)](https://www.microchip.com/)
[![Status](https://img.shields.io/badge/status-stable-brightgreen)](https://github.com/klenov1900-lang/PCFKeypad_EASY)
[![Library Manager](https://img.shields.io/badge/Arduino%20Library%20Manager-approved-blue)](https://www.arduino.cc/reference/en/libraries/)

Матричная клавиатура **4×4** через I2C-расширитель **PCF8574** для ATmega328P. Использует `I2C_EASY` как низкоуровневый слой TWI. **Без `Wire.h`.** Часть семейства `*_EASY`.

## ✨ Особенности

- ✅ **Использует `I2C_EASY`** — без `Wire.h`
- ✅ **Сканирование матрицы 4×4** — все 16 клавиш
- ✅ **Антидребезг** — настраиваемый (по умолчанию 50 мс)
- ✅ **Автоповтор** — hold + repeat
- ✅ **Энергосбережение** — переход в сон через заданное время
- ✅ **Защита от ghosting** — игнорирование фантомных нажатий
- ✅ **Игнорирование множественного нажатия** — возвращается только «чистое» одиночное нажатие
- ✅ **Обработка ошибок I2C** — коды ошибок
- ✅ **Своя распиновка и раскладка** — поддержка нестандартных клавиатур
- ✅ **Поддержка прерываний** — опционально
- ✅ **C-совместимость** — `extern "C"`

## ⚙️ Требования и совместимость

- **Микроконтроллер:** ATmega328P (8-битный AVR).
- **Платы:** Arduino Uno, Nano, Pro Mini.
- **Расширитель:** PCF8574 (адрес 0x20 по умолчанию).
- **Зависимости:** [`I2C_EASY`](https://github.com/klenov1900-lang/I2C_EASY).
- **Другие платформы:** ESP8266, ESP32, STM32 — **не поддерживаются** (нет прямого доступа к регистрам AVR).

## 📦 Установка

### Способ 1: Через менеджер библиотек Arduino IDE

1. Откройте Arduino IDE
2. Перейдите в **Скетч → Подключить библиотеку → Управлять библиотеками…**
3. В поиске введите `PCFKeypad_EASY`
4. Нажмите **Установить**

### Способ 2: Вручную (через ZIP)

1. Скачайте ZIP-архив с [последней версией](https://github.com/klenov1900-lang/PCFKeypad_EASY/archive/refs/heads/main.zip)
2. Arduino IDE: **Скетч → Подключить библиотеку → Добавить .ZIP библиотеку…**
3. Выберите скачанный архив

### Способ 3: Через Git

```bash
cd ~/Arduino/libraries/
git clone https://github.com/klenov1900-lang/I2C_EASY.git
git clone https://github.com/klenov1900-lang/PCFKeypad_EASY.git
```

## 🔌 Подключение

### 🧩 Схема подключения

| 🔌 Arduino Nano | ↔️ | 🧠 PCF8574 | ↔️ | ⌨️ Клавиатура 4x4 |
| :--- | :---: | :--- | :---: | :--- |
| 📍 **A4 (SDA)** | → | **SDA** | | |
| 📍 **A5 (SCL)** | → | **SCL** | | |
| ⚡ **5V** | → | **VCC** | | |
| ⏚ **GND** | → | **GND** | | |
| | | **P4** | → | 🔴 **R1** (строка 1) |
| | | **P5** | → | 🔴 **R2** (строка 2) |
| | | **P6** | → | 🔴 **R3** (строка 3) |
| | | **P7** | → | 🔴 **R4** (строка 4) |
| | | **P0** | → | 🔵 **C1** (столбец 1) |
| | | **P1** | → | 🔵 **C2** (столбец 2) |
| | | **P2** | → | 🔵 **C3** (столбец 3) |
| | | **P3** | → | 🔵 **C4** (столбец 4) |

### 📏 Распиновка шлейфа клавиатуры

| 🔢 Номер провода | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
| :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: |
| **📍 Пин** | 🟥 R1 | 🟥 R2 | 🟥 R3 | 🟥 R4 | 🟦 C1 | 🟦 C2 | 🟦 C3 | 🟦 C4 |

### 🏷️ Адрес PCF8574

- По умолчанию: **0x20**
- Если перемычки A0-A2 замкнуты: адрес меняется
- Проверить адрес можно I2C-сканером (`i2c_scan()` из [`I2C_EASY`](https://github.com/klenov1900-lang/I2C_EASY))

## 🚀 Быстрый старт

```cpp
#include <I2C_EASY.h>
#include <PCFKeypad_EASY.h>

void setup() {
    Serial.begin(9600);
    i2c_init(400000);

    pcfpad_init(0x20);   // адрес PCF8574

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

## 📖 Примеры

| 📂 Пример | 📝 Описание | 📦 Зависимости |
|---|---|---|
| **basic** | Минимальный пример — вывод нажатий в Serial | `PCFKeypad_EASY` |
| **Custom** | Своя распиновка и раскладка | `PCFKeypad_EASY` |
> **Примечание:** Пример `Custom` показывает, как использовать `pcfpad_init_custom()` для нестандартных клавиатур.

## 📚 API

### ⚙️ Инициализация

| 🛠️ Функция | 📝 Описание |
|---|---|
| `pcfpad_init(addr)` | Инициализация (стандартная распиновка и раскладка) |
| `pcfpad_init_custom(addr, rows, cols, keys)` | Своя распиновка и раскладка |
| `pcfpad_is_connected()` | Проверка связи (1 = есть) |
| `pcfpad_get_error()` | Код последней ошибки I2C |
| `pcfpad_reset()` | Сброс состояния |

### 🔍 Опрос

| 🛠️ Функция | 📝 Описание |
|---|---|
| `pcfpad_get_key()` | Состояние (`PCFPAD_KEY_*`) |
| `pcfpad_get_current()` | Текущий символ |
| `pcfpad_is_pressed()` | Нажата ли любая клавиша |

### 🎛️ Настройка

| 🛠️ Функция | 📝 Описание | 🔢 По умолчанию |
|---|---|---|
| `pcfpad_set_debounce(ms)` | Время антидребезга | 50 мс |
| `pcfpad_set_hold_delay(ms)` | Задержка до автоповтора | 800 мс |
| `pcfpad_set_repeat_delay(ms)` | Интервал автоповтора | 150 мс |
| `pcfpad_set_scan_interval(ms)` | Интервал сканирования | 1 мс |
| `pcfpad_set_active_low(enable)` | Полярность сигнала | 1 (true) |

### 🔋 Энергосбережение

| 🛠️ Функция | 📝 Описание |
|---|---|
| `pcfpad_set_power_save(enable)` | Включить/выключить |
| `pcfpad_set_power_save_timeout(ms)` | Таймаут до сна |
| `pcfpad_is_power_save()` | Проверка режима |

### ⚡ Прерывания

| 🛠️ Функция | 📝 Описание |
|---|---|
| `pcfpad_handle_interrupt()` | Вызывать из ISR |

## 📊 Константы

```cpp
// Состояния (возврат из pcfpad_get_key)
#define PCFPAD_KEY_RELEASED   0      // Клавиша не нажата
#define PCFPAD_KEY_PRESSED    1      // Клавиша только что нажата
#define PCFPAD_KEY_HOLD       2      // Клавиша удерживается (автоповтор)
#define PCFPAD_KEY_ERROR      0xFE   // Ошибка I2C
```

## 📊 Коды ошибок

| 🔢 Код | 🏷️ Константа | 📝 Описание |
|---|---|---|
| 0 | `I2C_OK` | Успех |
| 1 | `I2C_ERR_TIMEOUT` | Таймаут ожидания TWINT |
| 2 | `I2C_ERR_START` | Ошибка START |
| 3 | `I2C_ERR_ADDR_NACK` | Устройство не ответило |
| 4 | `I2C_ERR_DATA_NACK` | NACK на данные |
| 5 | `I2C_ERR_BUS` | Ошибка состояния шины |
| 6 | `I2C_ERR_ARB_LOST` | Потеря арбитража |
| 7 | `I2C_ERR_BUS_LOCKED` | SDA удерживается низко |

## 🔔 Прерывания

### ⚡ Пример с прерыванием

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

> **Примечание:** `pcfpad_handle_interrupt()` только устанавливает флаг. Вся работа — в `pcfpad_get_key()`. Это позволяет ISR быть максимально короткой.

## 💾 Потребление ресурсов

| 📊 Ресурс | 📦 Занято | 🟢 Свободно |
|---|---|---|
| ⚡ **Flash** | ~2.5 КБ (8%) | ~29 КБ |
| 💾 **ОЗУ** | ~50 байт (2%) | ~2 КБ |

> **Примечание:** `PCFKeypad_EASY` использует **статические буферы** (никаких `malloc`). Потребление RAM минимально и детерминировано.

## 📂 Структура

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

## 📝 История версий

| 🏷️ Версия | 📅 Дата | 📝 Изменения |
|---|---|---|
| **v1.0.0** | 2026-10-19 | Первый релиз. Базовый API, 2 примера, диагностика. |

## 📄 Лицензия

MIT License. См. [LICENSE](LICENSE).

## 📧 Контакты

- **Email:** gray_wolf19@mail.ru
- **GitHub:** [klenov1900-lang](https://github.com/klenov1900-lang)

---

⭐ Если библиотека вам полезна, поставьте звезду на GitHub! ⭐
