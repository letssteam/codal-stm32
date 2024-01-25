#include "ism330dl.h"

using namespace codal;
using namespace std;

constexpr uint8_t REG_INT1_CTRL = 0x0D;
constexpr uint8_t REG_WHO_AM_I  = 0x0F;
constexpr uint8_t REG_CTRL1_XL  = 0x10;
constexpr uint8_t REG_CTRL2_G   = 0x11;
constexpr uint8_t REG_CTRL3_C   = 0x12;
constexpr uint8_t REG_CTRL6_C   = 0x15;
constexpr uint8_t REG_CTRL7_G   = 0x16;
constexpr uint8_t REG_OUTX_L_G  = 0x22;
constexpr uint8_t REG_OUTX_H_G  = 0x23;
constexpr uint8_t REG_OUTY_L_G  = 0x24;
constexpr uint8_t REG_OUTY_H_G  = 0x25;
constexpr uint8_t REG_OUTZ_L_G  = 0x26;
constexpr uint8_t REG_OUTZ_H_G  = 0x27;
constexpr uint8_t REG_OUTX_L_XL = 0x28;
constexpr uint8_t REG_OUTX_H_XL = 0x29;
constexpr uint8_t REG_OUTY_L_XL = 0x2A;
constexpr uint8_t REG_OUTY_H_XL = 0x2B;
constexpr uint8_t REG_OUTZ_L_XL = 0x2C;
constexpr uint8_t REG_OUTZ_H_XL = 0x2D;

ISM330DL::ISM330DL(STM32I2C* i2c, uint16_t address) : i2c(i2c), address(address), fsGyro(250), fsAccel(2) {}

void ISM330DL::init()
{
    uint8_t ctrl3 = readRegister(REG_CTRL3_C);

    setBitRegister(&ctrl3, 2);
    clearBitRegister(&ctrl3, 6);

    writeRegister(REG_CTRL3_C, ctrl3);
}

uint8_t ISM330DL::whoAmI()
{
    return readRegister(REG_WHO_AM_I);
}

void ISM330DL::setGyroscopeODR(ISM_ODR odr, bool highPerformanceMode)
{
    uint8_t ctrl2 = readRegister(REG_CTRL2_G);
    uint8_t ctrl7 = readRegister(REG_CTRL7_G);

    ctrl2 &= ~(0x0F << 4);
    ctrl2 |= (uint8_t)odr << 4;

    if (highPerformanceMode) {
        clearBitRegister(&ctrl7, 7);
    }
    else {
        setBitRegister(&ctrl7, 7);
    }

    writeRegister(REG_CTRL2_G, ctrl2);
    writeRegister(REG_CTRL7_G, ctrl7);
}

void ISM330DL::setAccelerometerODR(ISM_ODR odr, bool highPerformanceMode)
{
    uint8_t ctrl1 = readRegister(REG_CTRL1_XL);
    uint8_t ctrl6 = readRegister(REG_CTRL6_C);

    ctrl1 &= ~(0x0F << 4);
    ctrl1 |= (uint8_t)odr << 4;

    if (highPerformanceMode) {
        clearBitRegister(&ctrl6, 4);
    }
    else {
        setBitRegister(&ctrl6, 4);
    }

    writeRegister(REG_CTRL1_XL, ctrl1);
    writeRegister(REG_CTRL6_C, ctrl6);
}

void ISM330DL::setGyroscopeFullScale(ISM_G_FS fs)
{
    uint8_t ctrl2 = readRegister(REG_CTRL2_G);

    ctrl2 &= ~(0b11 << 2);
    ctrl2 |= (uint8_t)fs << 2;

    writeRegister(REG_CTRL2_G, ctrl2);

    switch (fs) {
        case ISM_G_FS::FS_250_DPS:
            fsGyro = 250.0;
            break;

        case ISM_G_FS::FS_500_DPS:
            fsGyro = 500.0;
            break;

        case ISM_G_FS::FS_1000_DPS:
            fsGyro = 1000.0;
            break;

        case ISM_G_FS::FS_2000_DPS:
            fsGyro = 2000.0;
            break;
    }
}

void ISM330DL::setAccelerometerFullScale(ISM_XL_FS fs)
{
    uint8_t ctrl1 = readRegister(REG_CTRL1_XL);

    ctrl1 &= ~(0b11 << 2);
    ctrl1 |= (uint8_t)fs << 2;

    writeRegister(REG_CTRL1_XL, ctrl1);

    switch (fs) {
        case ISM_XL_FS::FS_2_G:
            fsAccel = 2.0;
            break;

        case ISM_XL_FS::FS_4_G:
            fsAccel = 4.0;
            break;

        case ISM_XL_FS::FS_8_G:
            fsAccel = 8.0;
            break;

        case ISM_XL_FS::FS_16_G:
            fsAccel = 16.0;
            break;
    }
}

ISM_Data ISM330DL::readGyroscopeData()
{
    float sensitivity = fsGyro / 32768.0;
    vector<uint8_t> raw({0, 0, 0, 0, 0, 0});

    for (uint8_t i = 0; i < 6; ++i) {
        raw[i] = i2c->readRegister(address, REG_OUTX_L_G + i, 1)[0];
    }

    int16_t rawX = (int16_t(raw[1]) << 8) | int16_t(raw[0]);
    int16_t rawY = (int16_t(raw[3]) << 8) | int16_t(raw[2]);
    int16_t rawZ = (int16_t(raw[5]) << 8) | int16_t(raw[4]);

    return ISM_Data{x : float(rawX) * sensitivity, y : float(rawY) * sensitivity, z : float(rawZ) * sensitivity};
}

ISM_Data ISM330DL::readAccelerometerData()
{
    float sensitivity = fsAccel / 32768.0;
    vector<uint8_t> raw({0, 0, 0, 0, 0, 0});

    for (uint8_t i = 0; i < 6; ++i) {
        raw[i] = i2c->readRegister(address, REG_OUTX_L_XL + i, 1)[0];
    }

    int16_t rawX = (int16_t(raw[1]) << 8) | int16_t(raw[0]);
    int16_t rawY = (int16_t(raw[3]) << 8) | int16_t(raw[2]);
    int16_t rawZ = (int16_t(raw[5]) << 8) | int16_t(raw[4]);

    return ISM_Data{x : float(rawX) * sensitivity, y : float(rawY) * sensitivity, z : float(rawZ) * sensitivity};
}

void ISM330DL::writeRegister(uint8_t reg, uint8_t value)
{
    i2c->beginTransmission(address);
    i2c->writeRegister(reg, value);
    i2c->endTransmission();
}

uint8_t ISM330DL::readRegister(uint8_t reg)
{
    return i2c->readRegister(address, reg, 1)[0];
}