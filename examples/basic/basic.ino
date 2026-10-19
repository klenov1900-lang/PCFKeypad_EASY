// ============================================================
// Basic.ino
// PCFKeypad_EASY — минимальный пример.
// Вывод нажатий в Serial (для отладки).
// ============================================================
#include <I2C_EASY.h>
#include <PCFKeypad_EASY.h>

void setup() {
    Serial.begin(9600);
    delay(500);
    Serial.println(F("=== PCFKeypad_EASY Basic ==="));

    // 1. Инициализация I2C на 400 кГц
    i2c_init(400000);

    // 2. Инициализация клавиатуры (адрес 0x20 по умолчанию)
    pcfpad_init(0x20);
    
    // (Опционально) настройка таймингов:
    // pcfpad_set_debounce(50);
    // pcfpad_set_hold_delay(800);
    // pcfpad_set_repeat_delay(150);

    if (!pcfpad_is_connected()) {
        Serial.println(F("FAIL: PCF8574 not found!"));
        while (1);
    }
    Serial.println(F("PCF8574 found. Press any key..."));
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
    else if (state == PCFPAD_KEY_ERROR) {
        Serial.print(F("I2C Error: "));
        Serial.println(pcfpad_get_error());
    }
}