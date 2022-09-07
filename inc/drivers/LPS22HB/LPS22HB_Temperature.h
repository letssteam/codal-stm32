#pragma once

#include "LPS22HB.h"
#include "STM32I2C.h"
#include "Sensor.h"

namespace codal {

class LPS22HB_Temperature : public LPS22HB, public Sensor {
  public:
    LPS22HB_Temperature(uint8_t id, STM32I2C* i2c, uint16_t address, uint16_t sensitivity = SENSOR_DEFAULT_SENSITIVITY,
                        uint16_t samplePeriod = SENSOR_DEFAULT_SAMPLE_PERIOD);

    /**
     * @brief Configure the sensor
     *
     */
    void configure();

    /**
     * Read the value from underlying hardware.
     */
    virtual int readValue() override final
    {
        if (!isInitialized) configure();
        return (int)getTemperature();
    }

  private:
    bool isInitialized;
};

}  // namespace codal