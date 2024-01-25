#pragma once

#include <string>
#include "lcd_utils.h"
#include "STM32I2C.h"

#define LCD_ADDRESS_DEFAULT (0x4E)
#define LCD_COLS_DEFAULT    16
#define LCD_ROWS_DEFAULT    2

namespace codal {
    
class I2C_LCD {
  public:
    I2C_LCD(STM32I2C& i2c, uint16_t address = LCD_ADDRESS_DEFAULT, uint8_t cols = LCD_COLS_DEFAULT,
            uint8_t rows = LCD_ROWS_DEFAULT);

    void init();

    void displayOn();
    void displayOff();

    void cursorOn();
    void cursorOff();

    void blinkOn();
    void blinkOff();

    void backlightOn();
    void backlightOff();

    void clear();
    void home();
    void setCursor(uint8_t col, uint8_t row);

    void print(char c);
    void print(const char* str);
    void print(std::string str) { print(str.c_str()); }
    void print(unsigned char value, PrintRadix radix = PrintRadix::DEC) { printNumber(value, radix, 8); };
    void print(int value, PrintRadix radix = PrintRadix::DEC) { printNumber(value, radix, 16); };
    void print(unsigned int value, PrintRadix radix = PrintRadix::DEC) { printNumber(value, radix, 16); };
    void print(long value, PrintRadix radix = PrintRadix::DEC) { printNumber(value, radix, 32); };
    void print(unsigned long value, PrintRadix radix = PrintRadix::DEC) { printNumber(value, radix, 32); };
    void print(float value, uint8_t decimal = 2);

  private:
    codal::STM32I2C& i2c;
    uint16_t address;
    uint8_t cols;
    uint8_t rows;

    uint8_t entryMode;
    uint8_t displayControl;
    uint8_t functionSet;
    bool backlight;

    void sendCommand(uint8_t value) { send(value, true); }
    void send(uint8_t value, bool isCommand);
    void write4bits(uint8_t data, bool isCommand);
    void writeExpender(uint8_t data);
    void toBinString(char* str, uint32_t value, uint8_t size);

    template <typename T>
    void printNumber(T value, PrintRadix radix, uint8_t size)
    {
        char tmp[33] = {0};

        switch (radix) {
            case PrintRadix::HEX:
                sprintf(tmp, "%#x", value);
                break;

            case PrintRadix::OCT:
                sprintf(tmp, "%#o", value);
                break;

            case PrintRadix::BIN:
                toBinString(tmp, value, size);
                break;

            case PrintRadix::DEC:
            default:
                sprintf(tmp, "%d", value);
                break;
        }

        print(tmp);
    }
};  // namespace lcd
}  // namespace codal