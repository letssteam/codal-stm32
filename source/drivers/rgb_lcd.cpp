
#include "rgb_lcd.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "codal_target_hal.h"

using namespace codal;

#define delayMicroseconds target_wait_us

rgb_lcd::rgb_lcd(STM32I2C& i2c, uint8_t cols, uint8_t lines, uint8_t dotsize)
    : i2c(i2c), cols(cols), lines(lines), dotsize(dotsize)
{
}

void write_data(STM32I2C* i2c, uint8_t address, uint16_t reg, uint8_t* buffer, uint16_t length)
{
    unsigned byteToSend = 1;

    i2c->beginTransmission(address);
    i2c->write(reg);

    for (unsigned i = 0; i < length; i++) {
        if (byteToSend >= i2c->getBufferSize()) {
            i2c->endTransmission();
            i2c->beginTransmission(address);
            i2c->write(reg);

            byteToSend = 1;
        }

        i2c->write(buffer[i]);
        ++byteToSend;
    }

    i2c->endTransmission();
}
void write_register(STM32I2C* i2c, uint8_t address, uint16_t reg, uint8_t value)
{
    i2c->beginTransmission(address);
    i2c->write(reg);
    i2c->write(value);
    i2c->endTransmission();
}

void rgb_lcd::init()
{
    _displayfunction = 0;

    if (lines > 1) {
        _displayfunction |= LCD_2LINE;
    }

    _numlines = lines;
    _currline = 0;

    // for some 1 line displays you can select a 10 pixel high font
    if ((dotsize != 0) && (lines == 1)) {
        _displayfunction |= LCD_5x10DOTS;
    }

    // SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
    // according to datasheet, we need at least 40ms after power rises above 2.7V
    // before sending commands. Arduino can turn on way befer 4.5V so we'll wait 50
    target_wait(100);

    // this is according to the hitachi HD44780 datasheet
    // page 45 figure 23

    // Send function set command sequence
    command(LCD_FUNCTIONSET | _displayfunction);
    target_wait(5);

    // second try
    command(LCD_FUNCTIONSET | _displayfunction);
    target_wait(5);

    // third go
    command(LCD_FUNCTIONSET | _displayfunction);

    // finally, set # lines, font size, etc.
    command(LCD_FUNCTIONSET | _displayfunction);

    // turn the display on with no cursor or blinking default
    _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
    display();

    // clear it off
    clear();

    // Initialize to default text direction (for romance languages)
    _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
    // set the entry mode
    command(LCD_ENTRYMODESET | _displaymode);

    // backlight init
    writeRGBRegister(REG_MODE1, 0x00);
    // set LEDs controllable by both PWM and GRPPWM registers
    writeRGBRegister(REG_OUTPUT, 0xAA);
    // set MODE2 values
    // 0010 0000 -> 0x20  (DMBLNK to 1, ie blinky mode)
    writeRGBRegister(REG_MODE2, 0x00);

    setColorWhite();
}

/********** high level commands, for the user! */
void rgb_lcd::clear()
{
    command(LCD_CLEARDISPLAY);  // clear display, set cursor position to zero
    delayMicroseconds(2000);    // this command takes a long time!
}

void rgb_lcd::home()
{
    command(LCD_RETURNHOME);  // set cursor position to zero
    delayMicroseconds(2000);  // this command takes a long time!
}

void rgb_lcd::setCursor(uint8_t col, uint8_t row)
{
    col = (row == 0 ? col | 0x80 : col | 0xc0);
    write_register(&i2c, LCD_ADDRESS, 0x80, col);
}

// Turn the display on/off (quickly)
void rgb_lcd::noDisplay()
{
    _displaycontrol &= ~LCD_DISPLAYON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void rgb_lcd::display()
{
    _displaycontrol |= LCD_DISPLAYON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns the underline cursor on/off
void rgb_lcd::noCursor()
{
    _displaycontrol &= ~LCD_CURSORON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void rgb_lcd::cursor()
{
    _displaycontrol |= LCD_CURSORON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turn on and off the blinking cursor
void rgb_lcd::noBlink()
{
    _displaycontrol &= ~LCD_BLINKON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void rgb_lcd::blink()
{
    _displaycontrol |= LCD_BLINKON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// These commands scroll the display without changing the RAM
void rgb_lcd::scrollDisplayLeft(void)
{
    command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void rgb_lcd::scrollDisplayRight(void)
{
    command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void rgb_lcd::leftToRight(void)
{
    _displaymode |= LCD_ENTRYLEFT;
    command(LCD_ENTRYMODESET | _displaymode);
}

// This is for text that flows Right to Left
void rgb_lcd::rightToLeft(void)
{
    _displaymode &= ~LCD_ENTRYLEFT;
    command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'right justify' text from the cursor
void rgb_lcd::autoscroll(void)
{
    _displaymode |= LCD_ENTRYSHIFTINCREMENT;
    command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'left justify' text from the cursor
void rgb_lcd::noAutoscroll(void)
{
    _displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
    command(LCD_ENTRYMODESET | _displaymode);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void rgb_lcd::createChar(uint8_t location, uint8_t charmap[])
{
    location &= 0x7;  // we only have 8 locations 0-7
    command(LCD_SETCGRAMADDR | (location << 3));

    unsigned char dta[8];

    for (int i = 0; i < 8; i++) {
        dta[i] = charmap[i];
    }

    write_data(&i2c, LCD_ADDRESS, 0x40, dta, 8);
}

// Control the backlight LED blinking
void rgb_lcd::blinkLED(void)
{
    // blink period in seconds = (<reg 7> + 1) / 24
    // on/off ratio = <reg 6> / 256
    writeRGBRegister(0x07, 0x17);  // blink every second
    writeRGBRegister(0x06, 0x7f);  // half on, half off
}

void rgb_lcd::noBlinkLED(void)
{
    writeRGBRegister(0x07, 0x00);
    writeRGBRegister(0x06, 0xff);
}

/*********** mid level commands, for sending data/cmds */

// send command
inline void rgb_lcd::command(uint8_t value)
{
    write_register(&i2c, LCD_ADDRESS, 0x80, value);
}

// send data
inline size_t rgb_lcd::write(uint8_t value)
{
    write_register(&i2c, LCD_ADDRESS, 0x40, value);
    return 1;  // assume sucess
}

void rgb_lcd::writeRGBRegister(unsigned char addr, unsigned char dta)
{
    write_register(&i2c, RGB_ADDRESS, addr, dta);
}

void rgb_lcd::setRGB(unsigned char r, unsigned char g, unsigned char b)
{
    writeRGBRegister(REG_RED, r);
    writeRGBRegister(REG_GREEN, g);
    writeRGBRegister(REG_BLUE, b);
}

const unsigned char color_define[][3] = {
    {255, 255, 255},  // white
    {255, 0, 0},      // red
    {0, 255, 0},      // green
    {0, 0, 255},      // blue
    {0, 0, 0},        // black
};

void rgb_lcd::setColor(unsigned char color)
{
    if (color > 4) return;
    setRGB(color_define[color][0], color_define[color][1], color_define[color][2]);
}

size_t rgb_lcd::print(const char str[])
{
    return write(str);
}

size_t rgb_lcd::print(char c)
{
    return write(c);
}

size_t rgb_lcd::print(unsigned char b, PrintRadix base)
{
    return print((unsigned long)b, base);
}

size_t rgb_lcd::print(int n, PrintRadix base)
{
    return print((long)n, base);
}

size_t rgb_lcd::print(unsigned int n, PrintRadix base)
{
    return print((unsigned long)n, base);
}

size_t rgb_lcd::print(long n,PrintRadix base)
{
    if (base == PrintRadix::DEC) {
        return write(n);
    }
    else if (base == PrintRadix::HEX) {
        if (n < 0) {
            int t = print('-');
            n     = -n;
            return printNumber(n, PrintRadix::HEX) + t;
        }
        return printNumber(n, PrintRadix::HEX);
    }
    else {
        return printNumber(n, base);
    }
}

size_t rgb_lcd::print(unsigned long n, PrintRadix base)
{
    if (base == PrintRadix::DEC)
        return write(n);
    else
        return printNumber(n, base);
}

size_t rgb_lcd::print(double n, int digits)
{
    return printFloat(n, digits);
}

// Private Methods /////////////////////////////////////////////////////////////

size_t rgb_lcd::printNumber(unsigned long n, PrintRadix base)
{
    char buf[8 * sizeof(long) + 1];  // Assumes 8-bit chars plus zero byte.
    char* str = &buf[sizeof(buf) - 1];

    *str = '\0';

    // prevent crash if called with base == 1
    if (base != PrintRadix::DEC) {
        base = PrintRadix::DEC;  // Assuming DEC as default if base is not provided or is invalid
    }

    do {
        unsigned long m = n;
        n /= static_cast<uint8_t>(base);  // Cast base to uint8_t
        char c = m - static_cast<uint8_t>(base) * n;  // Cast base to uint8_t
        *--str = c < 10 ? c + '0' : c + 'A' - 10;
    } while (n);

    return write(str);
}

size_t rgb_lcd::printFloat(double number, uint8_t digits)
{
    size_t n = 0;

    if (isnan(number)) return print("nan");
    if (isinf(number)) return print("inf");
    if (number > 4294967040.0) return print("ovf");   // constant determined empirically
    if (number < -4294967040.0) return print("ovf");  // constant determined empirically

    // Handle negative numbers
    if (number < 0.0) {
        n += print('-');
        number = -number;
    }

    // Round correctly so that print(1.999, 2) prints as "2.00"
    double rounding = 0.5;
    for (uint8_t i = 0; i < digits; ++i) rounding /= 10.0;

    number += rounding;

    // Extract the integer part of the number and print it
    unsigned long int_part = (unsigned long)number;
    double remainder       = number - (double)int_part;
    n += print(int_part);

    // Print the decimal point, but only if there are digits beyond
    if (digits > 0) {
        n += print(".");
    }

    // Extract digits from the remainder one at a time
    while (digits-- > 0) {
        remainder *= 10.0;
        int toPrint = int(remainder);
        n += print(toPrint);
        remainder -= toPrint;
    }

    return n;
}