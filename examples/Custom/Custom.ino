// ============================================================
// Custom.ino
// PCFKeypad_EASY — пример со своей распиновкой и раскладкой.
// ============================================================
#include <I2C_EASY.h>
#include <PCFKeypad_EASY.h>

// 1. Своя распиновка (какой пин PCF8574 к какой строке/столбцу)
// Например, если шлейф подключен в обратном порядке:
const uint8_t rows[4] = {7, 6, 5, 4};  // P7, P6, P5, P4
const uint8_t cols[4] = {3, 2, 1, 0};  // P3, P2, P1, P0

// 2. Своя раскладка (классический калькулятор)
const char keys[4][4] = {
    {'7', '8', '9', '/'},
    {'4', '5', '6', '*'},
    {'1', '2', '3', '-'},
    {'C', '0', '=', '+'}
};

void setup() {
    Serial.begin(9600);
    delay(500);
    Serial.println(F("=== PCFKeypad_EASY Custom ==="));

    i2c_init(400000);

    // Передаем кастомные массивы в функцию инициализации
    pcfpad_init_custom(0x20, rows, cols, keys);

    if (!pcfpad_is_connected()) {
        Serial.println(F("FAIL: PCF8574 not found!"));
        while (1);
    }
    Serial.println(F("Custom keypad ready. Press any key..."));
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
