#include "ssd1306.h"

using namespace codal;

constexpr uint8_t SET_CONTRAST        = 0x81;
constexpr uint8_t SET_ENTIRE_ON       = 0xa4;
constexpr uint8_t SET_NORM_INV        = 0xa6;
constexpr uint8_t SET_DISP            = 0xae;
constexpr uint8_t SET_MEM_ADDR        = 0x20;
constexpr uint8_t SET_COL_ADDR        = 0x21;
constexpr uint8_t SET_PAGE_ADDR       = 0x22;
constexpr uint8_t SET_DISP_START_LINE = 0x40;
constexpr uint8_t SET_SEG_REMAP       = 0xa0;
constexpr uint8_t SET_MUX_RATIO       = 0xa8;
constexpr uint8_t SET_COM_OUT_DIR     = 0xc0;
constexpr uint8_t SET_DISP_OFFSET     = 0xd3;
constexpr uint8_t SET_COM_PIN_CFG     = 0xda;
constexpr uint8_t SET_DISP_CLK_DIV    = 0xd5;
constexpr uint8_t SET_PRECHARGE       = 0xd9;
constexpr uint8_t SET_VCOM_DESEL      = 0xdb;
constexpr uint8_t SET_CHARGE_PUMP     = 0x8d;

SSD1306::SSD1306(unsigned width, unsigned height, bool externalVCC)
    : FrameBuffer(width > 128 ? 128 : width, height > 64 ? 64 : height, FrameBuffer::Format::MONO_VLSB),
      externalVCC(externalVCC)
{
}

SSD1306::~SSD1306() {}

void SSD1306::init()
{
    writeCommand(SET_DISP | 0x00);  // set display OFF

    // address setting
    writeCommand(SET_MEM_ADDR);
    writeCommand(0x00);  // horizontal

    // Resolution and layout
    writeCommand(SET_DISP_START_LINE | 0x00);  // Start the line on RAM address : 0x00
    writeCommand(SET_SEG_REMAP | 0x01);        // column addr 127 mapped to SEG0
    writeCommand(SET_MUX_RATIO);
    writeCommand(height - 1);
    writeCommand(SET_COM_OUT_DIR | 0x08);  // scan form COM[N] to COM0
    writeCommand(SET_DISP_OFFSET);
    writeCommand(0x00);  // No Display offset
    writeCommand(SET_COM_PIN_CFG);
    writeCommand(height == 32 ? 0x02 : 0x12);  // Select the right COM pin regarding the screen's height

    // Timing and driving scheme
    writeCommand(SET_DISP_CLK_DIV);
    writeCommand(0x80);  // Use defaut clock divider and oscillator frequency
    writeCommand(SET_PRECHARGE);
    writeCommand(externalVCC ? 0x22 : 0xF1);  // Set pre-charge period regarding the usage of external VCC,
                                              // defaut (0x22) if true
                                              // higher period for phase 2 than period 1 (0xF1) otherwise.
    writeCommand(SET_VCOM_DESEL);
    writeCommand(0x30);  // Set the regulator ouput ( 0x320 = 0.83 * Vcc), for deselection level.

    // Display
    writeCommand(SET_CONTRAST);
    writeCommand(0xFF);           // Maximum contrast
    writeCommand(SET_ENTIRE_ON);  // output follows RAM contents
    writeCommand(SET_NORM_INV);   // not inverted

    // Charged pump
    writeCommand(SET_CHARGE_PUMP);
    writeCommand(externalVCC ? 0x10 : 0x14);  // enable Charge Pump only when using external VCC.
    writeCommand(SET_DISP | 0x01);            // set display ON
}

void SSD1306::powerOff()
{
    writeCommand(SET_DISP | 0x00);
}

void SSD1306::powerOn()
{
    writeCommand(SET_DISP | 0x01);
}

void SSD1306::contrast(uint8_t contrast)
{
    writeCommand(SET_CONTRAST);
    writeCommand(contrast);
}

void SSD1306::invert(bool invert)
{
    writeCommand(SET_NORM_INV | (invert & 1));
}

void SSD1306::show()
{
    uint8_t x0 = 0;
    uint8_t x1 = width - 1;

    if (width == 64) {
        // Display with width of 64 pixels are shifted by 32
        x0 += 32;
        x1 += 32;
    }

    writeCommand(SET_COL_ADDR);
    writeCommand(x0);
    writeCommand(x1);

    writeCommand(SET_PAGE_ADDR);
    writeCommand(0);
    writeCommand(pages - 1);

    writeData(buffer, bufferSize);
}
