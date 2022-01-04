#include "STM32I2C.h"

#include <algorithm>

#include "PeripheralPins.h"
// #include "codal_target_hal.h"

using namespace std;
using namespace codal;

STM32I2C::STM32I2C(STM32Pin& sda, STM32Pin& scl) : I2C(sda, scl), currentAddress(0), isOnTransmission(false)
{
    i2c.sda         = (PinName)sda.name;
    i2c.scl         = (PinName)scl.name;
    i2c.isMaster    = 1;
    i2c.generalCall = 0;
}

int STM32I2C::setFrequency(uint32_t frequency)
{
    stm32_i2c_setTiming(&i2c, frequency);
    return DEVICE_OK;
}

void STM32I2C::beginTransmission(uint16_t address)
{
    if (isOnTransmission) return;

    // target_disable_irq();

    isOnTransmission = true;
    currentAddress   = address;
    dataToSent.clear();
}

void STM32I2C::endTransmission(bool sendStop)
{
    if (!isOnTransmission) return;

    stm32_i2c_init(&i2c);

    setXferOptions(sendStop);

    unsigned packets = dataToSent.size() / getBufferSize();

    if (getBufferSize() * packets < dataToSent.size()) {
        packets++;
    }

    for (unsigned i = 0; i < packets; ++i) {
        auto offset = i * getBufferSize();
        auto length = min<unsigned>(dataToSent.size() - offset, getBufferSize());

        stm32_i2c_master_write(&i2c, currentAddress, dataToSent.data() + offset, length);
    }

    stm32_i2c_deinit(&i2c);
    isOnTransmission = false;
    currentAddress   = 0;

    // target_enable_irq();
}

int STM32I2C::write(uint8_t data)
{
    if (!isOnTransmission) {
        return 0;
    }

    dataToSent.push_back(data);

    return DEVICE_OK;
}

int STM32I2C::write(uint8_t* data, size_t len)
{
    if (!isOnTransmission) {
        return 0;
    }

    dataToSent.insert(dataToSent.end(), data, data + len);

    return DEVICE_OK;
}

void STM32I2C::writeRegister(uint8_t reg, uint8_t value)
{
    write(reg);
    write(value);
}

vector<uint8_t> STM32I2C::read(uint8_t address, size_t len, bool sendStop)
{
    // target_disable_irq();

    vector<uint8_t> data(len);

    stm32_i2c_init(&i2c);

    setXferOptions(sendStop);
    stm32_i2c_master_read(&i2c, address, data.data(), len);

    stm32_i2c_deinit(&i2c);

    // target_enable_irq();

    return data;
}

vector<uint8_t> STM32I2C::readRegister(uint8_t address, uint8_t reg, size_t len, bool sendStop)
{
    beginTransmission(address);
    write(reg);
    endTransmission(false);

    return read(address, len, sendStop);
}

int STM32I2C::write(uint16_t address, uint8_t* data, int len, bool repeated)
{
    beginTransmission(address);
    write(data, len);
    endTransmission(!repeated);

    return DEVICE_OK;
}

int STM32I2C::read(uint16_t address, uint8_t* data, int len, bool repeated)
{
    auto result = read(address, len, !repeated);
    memcpy(data, result.data(), len);
    return DEVICE_OK;
}

void STM32I2C::setXferOptions(bool sendStop)
{
#if defined(I2C_OTHER_FRAME)
    if (sendStop) {
        i2c.handle.XferOptions = I2C_OTHER_AND_LAST_FRAME;
    }
    else {
        i2c.handle.XferOptions = I2C_OTHER_FRAME;
    }
#endif
}
