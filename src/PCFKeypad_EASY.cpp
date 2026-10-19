// ============================================================
// PCFKeypad_EASY.cpp
// Матричная клавиатура 4×4 через PCF8574.
// TWI через I2C_EASY. Прямая работа с регистрами.
// ============================================================
#include <Arduino.h>
#include "PCFKeypad_EASY.h"
#include <util/delay.h>
#include <string.h>

// ============================================================
// ВНУТРЕННИЕ ПЕРЕМЕННЫЕ
// ============================================================
static uint8_t  pcfpad_addr        = PCFPAD_ADDR_DEFAULT;
static uint8_t  pcfpad_activeLow   = 1;
static uint8_t  pcfpad_error       = I2C_OK;

// Распиновка и раскладка
static uint8_t  pcfpad_rows[4];
static uint8_t  pcfpad_cols[4];
static char     pcfpad_keys[4][4];

// Маски для PCF8574
static uint8_t  pcfpad_rowMasks[4];
static uint8_t  pcfpad_colMasks[4];
static uint8_t  pcfpad_idleMask;

// Состояние клавиатуры
static volatile uint8_t pcfpad_dataChanged = 0;
static volatile uint8_t pcfpad_keyPressed  = 0;
static volatile char    pcfpad_currentKey  = 0;

// Тайминги
static uint16_t pcfpad_debounceDelay  = 50;
static uint16_t pcfpad_holdDelay      = 800;
static uint16_t pcfpad_repeatDelay    = 150;
static uint16_t pcfpad_scanInterval   = 1;
static uint16_t pcfpad_scanIntervalIdle = 5;

// Метки времени
static uint32_t pcfpad_lastPressTime     = 0;
static uint32_t pcfpad_lastRepeatTime    = 0;
static uint32_t pcfpad_holdStartTime     = 0;
static uint32_t pcfpad_lastScanTime      = 0;
static uint32_t pcfpad_lastAnyKeyScanTime = 0;
static uint32_t pcfpad_lastActivityTime  = 0;
static uint32_t pcfpad_lastIdleTime      = 0;

// Энергосбережение
static uint8_t  pcfpad_powerSaveMode    = 0;
static uint8_t  pcfpad_powerSaveEnabled = 0;
static uint32_t pcfpad_powerSaveTimeout = 5000;

// ============================================================
// АТОМАРНЫЕ ОПЕРАЦИИ
// ============================================================
static void pcfpad_atomicSetChanged(uint8_t value) {
    uint8_t sreg = SREG;
    cli();
    pcfpad_dataChanged = value ? 1 : 0;
    SREG = sreg;
}

static uint8_t pcfpad_atomicGetChanged(void) {
    uint8_t sreg = SREG;
    cli();
    uint8_t result = pcfpad_dataChanged;
    SREG = sreg;
    return result;
}

// ============================================================
// РАСЧЁТ МАСОК
// ============================================================
static void pcfpad_calculateMasks(void) {
    if (pcfpad_activeLow) {
        pcfpad_idleMask = 0xFF;
        for (uint8_t i = 0; i < 4; i++) {
            pcfpad_rowMasks[i] = 0xFF & ~(1 << pcfpad_rows[i]);
            pcfpad_colMasks[i] = 1 << pcfpad_cols[i];
        }
    } else {
        pcfpad_idleMask = 0x00;
        for (uint8_t i = 0; i < 4; i++) {
            pcfpad_rowMasks[i] = 1 << pcfpad_rows[i];
            pcfpad_colMasks[i] = 0xFF & ~(1 << pcfpad_cols[i]);
        }
    }
}

// ============================================================
// НИЗКОУРОВНЕВЫЕ I2C ОПЕРАЦИИ (через I2C_EASY)
// ============================================================
static void pcfpad_writePCF(uint8_t data) {
    i2c_write(pcfpad_addr, &data, 1);
    // ★ ПРАВКА 2: сохраняем реальный код ошибки
    pcfpad_error = i2c_last_error();
}

static uint8_t pcfpad_readPCF(void) {
    uint8_t data = pcfpad_idleMask;
    if (i2c_read(pcfpad_addr, &data, 1) != I2C_OK) {
        // ★ ПРАВКА 2: сохраняем реальный код ошибки
        pcfpad_error = i2c_last_error();
        return pcfpad_idleMask;
    }
    pcfpad_error = I2C_OK;
    return data;
}

// ============================================================
// ИНИЦИАЛИЗАЦИЯ
// ============================================================
void pcfpad_init(uint8_t addr) {
    static const uint8_t defaultRows[4] = {4, 5, 6, 7};
    static const uint8_t defaultCols[4] = {0, 1, 2, 3};
    static const char defaultKeys[4][4] = {
        {'D', '#', '0', '*'},
        {'C', '9', '8', '7'},
        {'B', '6', '5', '4'},
        {'A', '3', '2', '1'}
    };
    pcfpad_init_custom(addr, defaultRows, defaultCols, defaultKeys);
}

void pcfpad_init_custom(uint8_t addr,
                        const uint8_t rows[4],
                        const uint8_t cols[4],
                        const char keys[4][4]) {
    pcfpad_addr      = addr;
    pcfpad_activeLow = 1;
    pcfpad_error     = I2C_OK;

    memcpy(pcfpad_rows, rows, sizeof(pcfpad_rows));
    memcpy(pcfpad_cols, cols, sizeof(pcfpad_cols));
    memcpy(pcfpad_keys, keys, sizeof(pcfpad_keys));

    pcfpad_calculateMasks();

    // Настройка клавиатуры (все строки — 1, столбцы — входы)
    pcfpad_writePCF(pcfpad_idleMask);

    // Сброс состояния
    pcfpad_dataChanged   = 0;
    pcfpad_keyPressed    = 0;
    pcfpad_currentKey    = 0;
    pcfpad_lastPressTime = 0;
    pcfpad_lastRepeatTime = 0;
    pcfpad_holdStartTime = 0;
    pcfpad_lastScanTime  = 0;
    pcfpad_lastAnyKeyScanTime = 0;
    pcfpad_powerSaveMode = 0;
    pcfpad_lastActivityTime = millis();
    pcfpad_lastIdleTime  = 0;
}

// ============================================================
// RESET
// ============================================================
void pcfpad_reset(void) {
    uint8_t sreg = SREG;
    cli();
    pcfpad_dataChanged = 0;
    pcfpad_keyPressed  = 0;
    pcfpad_currentKey  = 0;
    SREG = sreg;

    pcfpad_lastPressTime  = 0;
    pcfpad_lastRepeatTime = 0;
    pcfpad_holdStartTime  = 0;
    pcfpad_lastScanTime   = 0;
    pcfpad_lastAnyKeyScanTime = 0;
    pcfpad_lastActivityTime = millis();
    pcfpad_lastIdleTime   = 0;
    pcfpad_powerSaveMode  = 0;
    pcfpad_writePCF(pcfpad_idleMask);
}

// ============================================================
// ОБРАБОТКА ПРЕРЫВАНИЯ
// ============================================================
void pcfpad_handle_interrupt(void) {
    // Одна запись volatile — атомарна на AVR.
    // ISR должна быть максимально короткой.
    pcfpad_dataChanged = 1;
}

// ============================================================
// ПРОВЕРКА ПОДКЛЮЧЕНИЯ
// ============================================================
uint8_t pcfpad_is_connected(void) {
    return i2c_ping(pcfpad_addr) ? 1 : 0;
}

uint8_t pcfpad_get_error(void) {
    return pcfpad_error;
}

// ============================================================
// ЭНЕРГОСБЕРЕЖЕНИЕ
// ============================================================
static void pcfpad_enterPowerSave(void) {
    if (pcfpad_powerSaveMode || !pcfpad_powerSaveEnabled) return;
    pcfpad_writePCF(pcfpad_idleMask);
    pcfpad_powerSaveMode = 1;
    pcfpad_lastIdleTime  = millis();
}

static void pcfpad_exitPowerSave(void) {
    if (!pcfpad_powerSaveMode) return;
    pcfpad_writePCF(pcfpad_idleMask);
    pcfpad_powerSaveMode = 0;
    pcfpad_lastActivityTime = millis();
}

void pcfpad_set_power_save(uint8_t enable) {
    pcfpad_powerSaveEnabled = enable ? 1 : 0;
    if (!enable && pcfpad_powerSaveMode) {
        pcfpad_exitPowerSave();
    }
}

void pcfpad_set_power_save_timeout(uint32_t timeout) {
    pcfpad_powerSaveTimeout = timeout;
}

uint8_t pcfpad_is_power_save(void) {
    return pcfpad_powerSaveMode;
}

static uint8_t pcfpad_checkActivity(void) {
    if (!pcfpad_powerSaveEnabled) return 1;

    if (pcfpad_powerSaveMode) {
        if (pcfpad_atomicGetChanged()) {
            pcfpad_exitPowerSave();
            return 1;
        }
        return 0;
    }

    uint32_t now = millis();
    if (pcfpad_keyPressed) {
        pcfpad_lastActivityTime = now;
        return 1;
    }

    if (now - pcfpad_lastActivityTime > pcfpad_powerSaveTimeout) {
        pcfpad_enterPowerSave();
        return 0;
    }

    return 1;
}

// ============================================================
// ЗАЩИТА ОТ GHOSTING
// ============================================================
static uint8_t pcfpad_checkGhosting(uint8_t row, uint8_t col) {
    for (uint8_t r = 0; r < 4; r++) {
        if (r == row) continue;

        pcfpad_writePCF(pcfpad_rowMasks[r]);
        // ★ ПРАВКА 1: уменьшено с 200 до 100 мкс (при 400 кГц достаточно)
        _delay_us(100);
        uint8_t state = pcfpad_readPCF();
        pcfpad_writePCF(pcfpad_idleMask);   // сброс сразу после чтения

        if (pcfpad_error != I2C_OK) {
            return 1;   // при ошибке считаем ghosting
        }

        uint8_t pressed = pcfpad_activeLow ?
            ((state & pcfpad_colMasks[col]) == 0) :
            ((state & pcfpad_colMasks[col]) != 0);

        if (pressed) return 1;
    }
    return 0;
}

// ============================================================
// ПРОВЕРКА НАЖАТИЯ ЛЮБОЙ КЛАВИШИ
// ============================================================
uint8_t pcfpad_is_pressed(void) {
    if (!pcfpad_checkActivity()) return 0;

    uint32_t now = millis();
    uint32_t interval = pcfpad_powerSaveMode ? pcfpad_scanIntervalIdle : pcfpad_scanInterval;

    if (pcfpad_lastAnyKeyScanTime != 0 &&
        (now - pcfpad_lastAnyKeyScanTime) < interval) {
        return pcfpad_keyPressed;
    }
    pcfpad_lastAnyKeyScanTime = now;

    for (uint8_t r = 0; r < 4; r++) {
        pcfpad_writePCF(pcfpad_rowMasks[r]);
        // ★ ПРАВКА 1: уменьшено с 200 до 100 мкс
        _delay_us(100);
        uint8_t state = pcfpad_readPCF();
        pcfpad_writePCF(pcfpad_idleMask);

        if (pcfpad_error != I2C_OK) {
            return pcfpad_keyPressed;
        }

        for (uint8_t c = 0; c < 4; c++) {
            uint8_t pressed = pcfpad_activeLow ?
                ((state & pcfpad_colMasks[c]) == 0) :
                ((state & pcfpad_colMasks[c]) != 0);

            if (pressed) {
                if (!pcfpad_powerSaveMode) {
                    pcfpad_lastActivityTime = millis();
                }
                return 1;
            }
        }
    }
    return 0;
}

// ============================================================
// СКАНИРОВАНИЕ (С ЗАЩИТОЙ ОТ МНОЖЕСТВЕННОГО НАЖАТИЯ)
// ============================================================
static char pcfpad_scanKey(void) {
    if (pcfpad_powerSaveMode) return 0;

    uint32_t now = millis();
    if (pcfpad_lastScanTime != 0 &&
        (now - pcfpad_lastScanTime) < pcfpad_scanInterval) {
        return pcfpad_currentKey;
    }
    pcfpad_lastScanTime = now;

    char foundKey = 0;
    uint8_t pressedCount = 0;
    uint8_t foundRow = 0, foundCol = 0;

    for (uint8_t r = 0; r < 4; r++) {
        pcfpad_writePCF(pcfpad_rowMasks[r]);
        // ★ ПРАВКА 1: уменьшено с 200 до 100 мкс
        _delay_us(100);
        uint8_t state = pcfpad_readPCF();
        pcfpad_writePCF(pcfpad_idleMask);

        if (pcfpad_error != I2C_OK) {
            return PCFPAD_KEY_ERROR;
        }

        for (uint8_t c = 0; c < 4; c++) {
            uint8_t pressed = pcfpad_activeLow ?
                ((state & pcfpad_colMasks[c]) == 0) :
                ((state & pcfpad_colMasks[c]) != 0);

            if (pressed) {
                foundKey = pcfpad_keys[r][c];
                foundRow = r;
                foundCol = c;
                pressedCount++;
            }
        }
    }

    // Игнорируем множественное нажатие
    if (pressedCount > 1) return 0;

    // Проверка ghosting для одиночного нажатия
    if (pressedCount == 1) {
        if (pcfpad_checkGhosting(foundRow, foundCol)) return 0;
        pcfpad_lastActivityTime = millis();
        return foundKey;
    }

    return 0;
}

// ============================================================
// ОСНОВНОЙ МЕТОД ПОЛУЧЕНИЯ КЛАВИШИ
// ============================================================
uint8_t pcfpad_get_key(void) {
    if (!pcfpad_is_connected()) {
        pcfpad_error = I2C_ERR_ADDR_NACK;
        return PCFPAD_KEY_ERROR;
    }

    if (pcfpad_powerSaveMode && pcfpad_atomicGetChanged()) {
        pcfpad_exitPowerSave();
    }

    uint32_t currentTime = millis();

    if (!pcfpad_is_pressed()) {
        if (pcfpad_keyPressed) {
            uint8_t sreg = SREG;
            cli();
            pcfpad_keyPressed = 0;
            pcfpad_currentKey = 0;
            SREG = sreg;

            pcfpad_holdStartTime = 0;
            pcfpad_lastRepeatTime = 0;
            pcfpad_atomicSetChanged(0);
            pcfpad_lastActivityTime = currentTime;
            return PCFPAD_KEY_RELEASED;
        }
        pcfpad_atomicSetChanged(0);
        return PCFPAD_KEY_RELEASED;
    }

    char scannedKey = pcfpad_scanKey();

    if (scannedKey == PCFPAD_KEY_ERROR) {
        return PCFPAD_KEY_ERROR;
    }

    if (scannedKey != 0) {
        if (!pcfpad_keyPressed) {
            if (currentTime - pcfpad_lastPressTime > pcfpad_debounceDelay) {
                uint8_t sreg = SREG;
                cli();
                pcfpad_keyPressed = 1;
                pcfpad_currentKey = scannedKey;
                SREG = sreg;

                pcfpad_holdStartTime = currentTime;
                pcfpad_lastRepeatTime = currentTime;
                pcfpad_lastActivityTime = currentTime;
                pcfpad_atomicSetChanged(0);
                return PCFPAD_KEY_PRESSED;
            }
        } else if (scannedKey == pcfpad_currentKey) {
            if (currentTime - pcfpad_holdStartTime > pcfpad_holdDelay) {
                if (currentTime - pcfpad_lastRepeatTime > pcfpad_repeatDelay) {
                    pcfpad_lastRepeatTime = currentTime;
                    pcfpad_lastActivityTime = currentTime;
                    pcfpad_atomicSetChanged(0);
                    return PCFPAD_KEY_HOLD;
                }
            }
        } else {
            uint8_t sreg = SREG;
            cli();
            pcfpad_keyPressed = 0;
            pcfpad_currentKey = scannedKey;
            SREG = sreg;

            pcfpad_holdStartTime = currentTime;
            pcfpad_lastRepeatTime = currentTime;
            pcfpad_lastPressTime = currentTime;
            pcfpad_lastActivityTime = currentTime;
            pcfpad_atomicSetChanged(0);
            return PCFPAD_KEY_PRESSED;
        }
    } else {
        if (pcfpad_keyPressed && pcfpad_error == I2C_OK) {
            uint8_t sreg = SREG;
            cli();
            pcfpad_keyPressed = 0;
            pcfpad_currentKey = 0;
            SREG = sreg;

            pcfpad_holdStartTime = 0;
            pcfpad_lastRepeatTime = 0;
            pcfpad_atomicSetChanged(0);
            return PCFPAD_KEY_RELEASED;
        }
    }

    return PCFPAD_KEY_RELEASED;
}

// ============================================================
// ТЕКУЩАЯ КЛАВИША
// ============================================================
char pcfpad_get_current(void) {
    uint8_t sreg = SREG;
    cli();
    char k = pcfpad_currentKey;
    SREG = sreg;
    return k;
}

// ============================================================
// НАСТРОЙКА
// ============================================================
void pcfpad_set_debounce(uint16_t ms) {
    pcfpad_debounceDelay = ms;
}

void pcfpad_set_hold_delay(uint16_t ms) {
    pcfpad_holdDelay = ms;
}

void pcfpad_set_repeat_delay(uint16_t ms) {
    pcfpad_repeatDelay = ms;
}

void pcfpad_set_scan_interval(uint16_t ms) {
    if (ms < 1) ms = 1;
    pcfpad_scanInterval = ms;
    pcfpad_scanIntervalIdle = ms * 5;
}

void pcfpad_set_active_low(uint8_t enable) {
    pcfpad_activeLow = enable ? 1 : 0;
    pcfpad_calculateMasks();
}