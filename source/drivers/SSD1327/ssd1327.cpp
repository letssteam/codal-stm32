#include "ssd1327.h"

using namespace codal;

// constexpr uint8_t SET_CONTRAST        = 0x81;
// constexpr uint8_t SET_ENTIRE_ON       = 0xa4;
// constexpr uint8_t SET_NORM_INV        = 0xa6;
// constexpr uint8_t SET_DISP            = 0xae;
// constexpr uint8_t SET_MEM_ADDR        = 0x20;
// constexpr uint8_t SET_COL_ADDR        = 0x21;
// constexpr uint8_t SET_PAGE_ADDR       = 0x22;
// constexpr uint8_t SET_DISP_START_LINE = 0x40;
// constexpr uint8_t SET_SEG_REMAP       = 0xa0;
// constexpr uint8_t SET_MUX_RATIO       = 0xa8;
// constexpr uint8_t SET_COM_OUT_DIR     = 0xc0;
// constexpr uint8_t SET_DISP_OFFSET     = 0xd3;
// constexpr uint8_t SET_COM_PIN_CFG     = 0xda;
// constexpr uint8_t SET_DISP_CLK_DIV    = 0xd5;
// constexpr uint8_t SET_PRECHARGE       = 0xd9;
// constexpr uint8_t SET_VCOM_DESEL      = 0xdb;
// constexpr uint8_t SET_CHARGE_PUMP     = 0x8d;

constexpr uint8_t SET_DISPLAY_ON = 0x8d;

SSD1327::SSD1327(unsigned width, unsigned height)
    : FrameBuffer(width > 128 ? 128 : width, height > 128 ? 128 : height, FrameBuffer::Format::GS4_HMSB)
{
}

SSD1327::~SSD1327() {}

void SSD1327::init()
{
    writeCommand(0xAE);  // Set Display OFF

    writeCommand(0x15);  // Set Column Address
    writeCommand(0x00);  // Start Column Address
    writeCommand(0x3F);  // End Column Address

    writeCommand(0x75);  // Set Row Address (segment)
    writeCommand(0x00);  // Satrt Row Address
    writeCommand(0x7F);  // End Row Address

    writeCommand(0x81);  // Contrast Level
    writeCommand(0xFF);

    writeCommand(0xA0);  // Set Re-map
    writeCommand(0x42);  // Default Setting

    writeCommand(0xA1);  // Set Display Start Line
    writeCommand(0x00);

    writeCommand(0xA2);  // Set Display Offset
    writeCommand(0x00);

    writeCommand(0xA4);  // Set Display Mode
    // writeCommand(0xA5);	//Set Display Mode
    // writeCommand(0xA6);	//Set Display Mode
    // writeCommand(0xA7);	//Set Display Mode

    writeCommand(0xA8);  // Select Multiplex Ratio
    writeCommand(0x7F);  // Default => 0x3F (1/64 Duty)	0x1F(1/32 Duty)

    writeCommand(0xAB);  // Set Function SelectionA
    writeCommand(0x01);

    writeCommand(0xB1);  // Set Phase Length
    writeCommand(0xf1);

    writeCommand(0xB3);  // Set Display Clock Divide Ratio/Oscillator Frequency
    writeCommand(0x00);

    writeCommand(0xBC);  // Set Prechange Voltage
    writeCommand(0x07);

    writeCommand(0xBE);  // Set VCOMH Voltage
    writeCommand(0x07);

    writeCommand(0xB6);  // Set Second Pre-charge period
    writeCommand(0x2F);

    writeCommand(0xD5);  // Set Function selection B
    writeCommand(0x62);

    writeCommand(0xAF);  // Set Display On
}

void SSD1327::powerOff()
{
    writeCommand(0xAE);  // Set Display OFF
}

void SSD1327::powerOn()
{
    writeCommand(0xAF);  // Set Display On
}

void SSD1327::contrast(uint8_t contrast)
{
    // writeCommand(SET_CONTRAST);
    // writeCommand(contrast);
}

void SSD1327::invert(bool invert)
{
    // writeCommand(SET_NORM_INV | (invert & 1));
}

void SSD1327::show()
{
    // uint8_t x0 = 0;
    // uint8_t x1 = width - 1;

    // if (width == 64) {
    //     // Display with width of 64 pixels are shifted by 32
    //     x0 += 32;
    //     x1 += 32;
    // }

    // writeCommand(SET_COL_ADDR);
    // writeCommand(x0);
    // writeCommand(x1);

    // writeCommand(SET_PAGE_ADDR);
    // writeCommand(0);
    // writeCommand(pages - 1);

    // writeData(buffer, bufferSize);

    writeCommand(0x15);  // Set Column Address
    writeCommand(0x00);  // Start Column Address
    writeCommand(0x3F);  // End Column Address

    writeCommand(0x75);  // Set Row Address (segment)
    writeCommand(0x00);  // Satrt Row Address
    writeCommand(0x7F);  // End Row Address

    writeData(buffer, bufferSize);
}
