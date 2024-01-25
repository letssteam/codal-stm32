#include "lis2mdl.h"

using namespace std;
using namespace codal;

constexpr uint8_t WHO_AM_I   = 0x4F;
constexpr uint8_t CFG_REG_A  = 0x60;
constexpr uint8_t CFG_REG_B  = 0x61;
constexpr uint8_t CFG_REG_C  = 0x62;
constexpr uint8_t OUTX_L_REG = 0x68;
constexpr uint8_t OUTX_H_REG = 0x69;
constexpr uint8_t OUTY_L_REG = 0x6A;
constexpr uint8_t OUTY_H_REG = 0x6B;
constexpr uint8_t OUTZ_L_REG = 0x6C;
constexpr uint8_t OUTZ_H_REG = 0x6D;

constexpr float SENSITIVITY = 50.0 / 32768.0;

LIS2MDL::LIS2MDL(STM32I2C* i2c, uint16_t address) : i2c(i2c), address(address) {}

void LIS2MDL::init()
{
    uint8_t cfgA = readRegister(CFG_REG_A);
    uint8_t cfgC = readRegister(CFG_REG_C);

    cfgA &= ~(0b11);  // Mode 0 - Continuous mode
    cfgC |= 1 << 4;   // Enable BDU

    sendRegister(CFG_REG_A, cfgA);
    sendRegister(CFG_REG_C, cfgC);
}

uint8_t LIS2MDL::whoAmI()
{
    return readRegister(WHO_AM_I);
}

void LIS2MDL::setODR(LIS2_ODR odr)
{
    uint8_t cfgA = readRegister(CFG_REG_A);

    cfgA &= ~(0b11 << 2);
    cfgA |= uint8_t(odr) << 2;

    sendRegister(CFG_REG_A, cfgA);
}

void LIS2MDL::setLowPowerMode(bool enable)
{
    uint8_t cfgA = readRegister(CFG_REG_A);

    if (enable) {
        setBitRegister(&cfgA, 4);
    }
    else {
        clearBitRegister(&cfgA, 4);
    }

    sendRegister(CFG_REG_A, cfgA);
}

void LIS2MDL::setLowPassFilter(bool enable)
{
    uint8_t cfgB = readRegister(CFG_REG_B);

    if (enable) {
        setBitRegister(&cfgB, 0);
    }
    else {
        clearBitRegister(&cfgB, 0);
    }

    sendRegister(CFG_REG_B, cfgB);
}

Lis2Data LIS2MDL::readData()
{
    uint8_t raw[6] = {0x00};

    for (uint8_t i = 0; i < 6; i++) {
        raw[i] = readRegister(OUTX_L_REG + i);
    }

    int16_t rawX = (int16_t(raw[1]) << 8) | raw[0];
    int16_t rawY = (int16_t(raw[3]) << 8) | raw[2];
    int16_t rawZ = (int16_t(raw[5]) << 8) | raw[4];

    return Lis2Data{x : float(rawX) * SENSITIVITY, y : float(rawY) * SENSITIVITY, z : float(rawZ) * SENSITIVITY};
}

void LIS2MDL::sendRegister(uint8_t reg, uint8_t value)
{
    i2c->beginTransmission(address);
    i2c->writeRegister(reg, value);
    i2c->endTransmission();
}
uint8_t LIS2MDL::readRegister(uint8_t reg)
{
    return i2c->readRegister(address, reg, 1)[0];
}