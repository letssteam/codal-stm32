#pragma once

#include <STM32I2C.h>
#include "../registerBit_utils.h"

#include <cstdint>
#include <functional>

enum class ISM_ODR : uint8_t {
    POWER_DOWN = 0x00,
    // 1.66 Hz is only for the Accelerometer (will be 12.5 Hz in high performance mode)
    F_1_66_HZ  = 0x0B,
    F_12_5_HZ  = 0x01,
    F_26_HZ    = 0x02,
    F_52_HZ    = 0x03,
    F_104_HZ   = 0x04,
    F_208_HZ   = 0x05,
    F_416_HZ   = 0x06,
    F_833_HZ   = 0x07,
    F_1_66_KHZ = 0x08,
    F_3_33_KHZ = 0x09,
    F_6_66_KHZ = 0x0A,
};

struct ISM_Data {
    float x;
    float y;
    float z;
};

enum class ISM_XL_FS : uint8_t { FS_2_G = 0x00, FS_16_G = 0x01, FS_4_G = 0x02, FS_8_G = 0x03 };
enum class ISM_G_FS : uint8_t { FS_250_DPS = 0x00, FS_500_DPS = 0x01, FS_1000_DPS = 0x02, FS_2000_DPS = 0x03 };

typedef std::function<void(float, float, float)> ISM_CALLBACK;

namespace codal {
class ISM330DL {
  public:
    ISM330DL(codal::STM32I2C* i2c, uint16_t address = 0xD6);

    /**
     * @brief Initialize the ISM330DL
     *
     */
    void init();

    /**
     * @brief Get the device ID (should be equal to 0x6A)
     *
     * @return uint8_t
     */
    uint8_t whoAmI();

    /**
     * @brief Set the Output Data Rate of Gyroscope.
     * If `highPerformanceMode` is disabled (false), then you can enable "Low-power" and "normal" modes. Modes will be
     * activated according to the selected ODR :
     * *  from 12.5Hz to 52Hz : Low-Power mode
     * *  from 104Hz to 208Hz : Normal mode
     * *  from 416Hz to 6.6kHz : high performance mode (even with `highPerformanceMode` to `false`)
     *
     * @param odr the ODR value
     * @param highPerformanceMode enable or disable the high performance mode
     */
    void setGyroscopeODR(ISM_ODR odr, bool highPerformanceMode = true);

    /**
     * @brief Set the Output Data Rate of Accelerometer.
     * If `highPerformanceMode` is disabled (false), then you can enable "Low-power" and "normal" modes. Modes will be
     * activated according to the selected ODR :
     * *  from 1.6Hz to 52Hz : Low-Power mode
     * *  from 104Hz to 208Hz : Normal mode
     * *  from 416Hz to 6.6kHz : high performance mode (even with `highPerformanceMode` to `false`)
     *
     * @param odr the ODR value
     * @param highPerformanceMode enable or disable the high performance mode
     */
    void setAccelerometerODR(ISM_ODR odr, bool highPerformanceMode = true);

    /**
     * @brief Set the Gyroscope Full Scale value
     *
     * @param fs fullscale value
     */
    void setGyroscopeFullScale(ISM_G_FS fs);

    /**
     * @brief Set the Accelerometer Full Scale value
     *
     * @param fs fullscale value
     */
    void setAccelerometerFullScale(ISM_XL_FS fs);

    /**
     * @brief Read last gyroscope data from the sensor
     *
     * @return ISM_Data
     */
    ISM_Data readGyroscopeData();

    /**
     * @brief Read last accelerometer data from the sensor
     *
     * @return ISM_Data
     */
    ISM_Data readAccelerometerData();

  private:
    codal::STM32I2C* i2c;
    uint16_t address;
    float fsGyro;
    float fsAccel;

    void writeRegister(uint8_t reg, uint8_t value);
    uint8_t readRegister(uint8_t reg);
};
}  // namespace codal
