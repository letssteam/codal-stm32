#ifndef __SSD1327_H__
#define __SSD1327_H__

#include <cstdint>

#include "STM32SPI.h"
#include "frameBuffer.h"

namespace codal {

class SSD1327 : public FrameBuffer {
  public:
    SSD1327(unsigned width, unsigned height);
    ~SSD1327();

    void init();
    void powerOff();
    void powerOn();
    void contrast(uint8_t contrast);
    void invert(bool invert);
    void show();

    virtual void writeCommand(uint8_t cmd)              = 0;
    virtual void writeData(uint8_t* buf, unsigned size) = 0;
};

class SSD1327_SPI : public SSD1327 {
  public:
    SSD1327_SPI(STM32SPI* spi, STM32Pin* chipSelect, STM32Pin* dataCommand, unsigned width, unsigned height);
    ~SSD1327_SPI();

    void writeCommand(uint8_t cmd);
    void writeData(uint8_t* buf, unsigned len);

  private:
    STM32SPI* spi;
    STM32Pin* cs;
    STM32Pin* dc;
};

}  // namespace codal

#endif