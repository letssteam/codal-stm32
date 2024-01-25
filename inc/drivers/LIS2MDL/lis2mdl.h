#pragma once

#include "STM32I2C.h"
#include "../registerBit_utils.h"

enum class LIS2_ODR : uint8_t { F_10_HZ = 0x00, F_20_HZ = 0x01, F_50_HZ = 0x02, F_100_HZ = 0x03 };

struct Lis2Data {
    float x;
    float y;
    float z;
};

namespace codal {
class LIS2MDL {
  public:
    LIS2MDL(STM32I2C* i2c, uint16_t address = 0x3C);

    /**
     * @brief Initialize the magnetometer
     *
     */
    void init();

    /**
     * @brief Get the device ID of sensor (should be equal to 0x40)
     *
     * @return uint8_t
     */
    uint8_t whoAmI();

    /**
     * @brief Set the Output Data Rate of the magnetometer
     *
     * @param odr the output data rate
     */
    void setODR(LIS2_ODR odr);

    /**
     * @brief Set the Low Power Mode
     *
     * @param enable `true` to enable low power mode, `false` otherwise
     */
    void setLowPowerMode(bool enable);

    /**
     * @brief Set the Low Pass Filter
     *
     * @param enable `true` to enable low pass filter, `false` otherwise
     */
    void setLowPassFilter(bool enable);

    /**
     * @brief Read the last data from the magnetometer
     *
     * @return Lis2Data
     */
    Lis2Data readData();

  private:
    STM32I2C* i2c;
    uint16_t address;

    void sendRegister(uint8_t reg, uint8_t value);
    uint8_t readRegister(uint8_t reg);
};
}  // namespace codal