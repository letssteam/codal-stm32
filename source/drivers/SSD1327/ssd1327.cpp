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
    writeCommand(0xae);  //--turn off oled panel

    writeCommand(0x15);  //   set column address
    writeCommand(0x00);  //  start column   0
    writeCommand(0x7f);  //  end column   127

    writeCommand(0x75);  //   set row address
    writeCommand(0x00);  //  start row   0
    writeCommand(0x7f);  //  end row   127

    writeCommand(0x81);  // set contrast control
    writeCommand(0x80);

    writeCommand(0xa0);  // gment remap
    writeCommand(0x51);  // 51

    writeCommand(0xa1);  // start line
    writeCommand(0x00);

    writeCommand(0xa2);  // display offset
    writeCommand(0x00);

    writeCommand(0xa4);  // rmal display
    writeCommand(0xa8);  // set multiplex ratio
    writeCommand(0x7f);

    writeCommand(0xb1);  // set phase leghth
    writeCommand(0xf1);

    writeCommand(0xb3);  // set dclk
    writeCommand(0x00);  // 80Hz:0xc1 90Hz:0xe1   100Hz:0x00   110Hz:0x30 120Hz:0x50   130Hz:0x70     01

    writeCommand(0xab);  //
    writeCommand(0x01);  //

    writeCommand(0xb6);  // set phase leghth
    writeCommand(0x0f);

    writeCommand(0xbe);
    writeCommand(0x0f);

    writeCommand(0xbc);
    writeCommand(0x08);

    writeCommand(0xd5);
    writeCommand(0x62);

    writeCommand(0xfd);
    writeCommand(0x12);

    writeCommand(0xb9);

    writeCommand(0xAF);
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
    writeCommand(0x15);  // Set Column Address
    writeCommand(0x00);  // Start Column Address
    writeCommand(0x3F);  // End Column Address

    writeCommand(0x75);  // Set Row Address (segment)
    writeCommand(0x00);  // Satrt Row Address
    writeCommand(0x7F);  // End Row Address

    writeData(buffer, bufferSize);
}
