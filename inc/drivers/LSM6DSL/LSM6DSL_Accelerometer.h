#ifndef __LSM6DSL_ACCELEROMETER_H__
#define __LSM6DSL_ACCELEROMETER_H__

#include "STM32I2C.h"
#include "LSM6DSL.h"
#include "Accelerometer.h"

namespace codal{

class LSM6DSL_Accelerometer : public LSM6DSL, public Accelerometer {

    public:
        LSM6DSL_Accelerometer( STM32I2C& i2c, uint8_t address, CoordinateSpace& coordinateSpace, uint16_t id = DEVICE_ID_ACCELEROMETER);


        /**
         * Configures the accelerometer for G range and sample rate defined
         * in this object. The nearest values are chosen to those defined
         * that are supported by the hardware. The instance variables are then
         * updated to reflect reality.
         *
         * @return DEVICE_OK on success, DEVICE_I2C_ERROR if the accelerometer could not be configured.
         *
         * @note This method should be overidden by the hardware driver to implement the requested
         * changes in hardware.
         */
        virtual int configure() override final;

        /**
         * Poll to see if new data is available from the hardware. If so, update it.
         * n.b. it is not necessary to explicitly call this function to update data
         * (it normally happens in the background when the scheduler is idle), but a check is performed
         * if the user explicitly requests up to date data.
         *
         * @return DEVICE_OK on success, DEVICE_I2C_ERROR if the update fails.
         *
         * @note This method should be overidden by the hardware driver to implement the requested
         * changes in hardware.
         */
        virtual int requestUpdate() override final;

    private:
        bool isConfigured;

        LSM6DSL_ACC_GYRO_FS_XL_t getBestFullScale(float value);
        LSM6DSL_ACC_GYRO_ODR_XL_t getBestODR(float ms);

};

}

#endif