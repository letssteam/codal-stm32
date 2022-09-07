#include "LPS22HB_Temperature.h"

using namespace codal;

LPS22HB_Temperature::LPS22HB_Temperature(uint8_t id, STM32I2C* i2c, uint16_t address, uint16_t sensitivity,
                                         uint16_t samplePeriod)
    : LPS22HB(i2c, address), Sensor(id, sensitivity, samplePeriod), isInitialized(false)
{
    configure();
}

void LPS22HB_Temperature::configure()
{
    LPS22HB::init();

    isInitialized = true;
}
