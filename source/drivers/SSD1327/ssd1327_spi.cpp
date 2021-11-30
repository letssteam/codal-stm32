#include <algorithm>

#include "ssd1327.h"

using namespace codal;
using namespace std;

SSD1327_SPI::SSD1327_SPI(STM32SPI* spi, STM32Pin* chipSelect, STM32Pin* dataCommand, unsigned width, unsigned height)
    : SSD1327(width, height), spi(spi), cs(chipSelect), dc(dataCommand)
{
    cs->setDigitalValue(1);
    dc->setDigitalValue(0);
}

SSD1327_SPI::~SSD1327_SPI() {}

void SSD1327_SPI::writeCommand(uint8_t cmd)
{
    dc->setDigitalValue(0);
    cs->setDigitalValue(0);

    spi->write(cmd);

    cs->setDigitalValue(1);
}

void SSD1327_SPI::writeData(uint8_t* buf, unsigned len)
{
    cs->setDigitalValue(0);

    dc->setDigitalValue(0);
    spi->beginTransaction();

    spi->writeTransaction(0x15);  // Set Column Address
    spi->writeTransaction(0x00);  // Start Column Address
    spi->writeTransaction(0x3F);  // End Column Address
    spi->writeTransaction(0x75);  // Set Row Address (segment)
    spi->writeTransaction(0x00);  // Satrt Row Address
    spi->writeTransaction(0x7F);  // End Row Address

    spi->endTransaction();

    dc->setDigitalValue(1);
    spi->beginTransaction();

    for (unsigned i = 0; i < len; ++i) {
        spi->writeTransaction(buf[i]);
    }

    spi->endTransaction();

    cs->setDigitalValue(1);
}
