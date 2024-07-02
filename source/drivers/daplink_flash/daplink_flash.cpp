#include "daplink_flash.h"

using namespace std;
using namespace codal;

constexpr uint8_t DAPFLASH_WHO_AM_I     = 0x01;
constexpr uint8_t DAPFLASH_SET_FILENAME = 0x03;
constexpr uint8_t DAPFLASH_GET_FILENAME = 0x04;
constexpr uint8_t DAPFLASH_CLEAR_FLASH  = 0x10;
constexpr uint8_t DAPFLASH_WRITE_DATA   = 0x11;
constexpr uint8_t DAPFLASH_STATUS_REG   = 0x80;
constexpr uint8_t DAPFLASH_ERROR_REG    = 0x81;

constexpr uint16_t I2C_WRITE_DATA_LEN = 30;

DaplinkFlash::DaplinkFlash(STM32I2C& i2c, uint16_t address) : i2c{i2c}, address{address} {}

uint8_t DaplinkFlash::whoAmI()
{
    return i2c.readRegister(address, DAPFLASH_WHO_AM_I, 1)[0];
}

bool DaplinkFlash::setFilename(const char* filename, const char* extension)
{
    uint16_t len_file = strlen(filename);
    uint16_t len_ext  = strlen(extension);

    if (len_file == 0) {
        return false;
    }

    uint8_t data[11];

    memset(data, 0x20, 11);

    memcpy(data, filename, len_file <= 8 ? len_file : 8);
    memcpy(data + 8, extension, len_ext <= 3 ? len_ext : 3);

    if (!wait_busy(1000)) {
        return false;
    }

    i2c.beginTransmission(address);
    i2c.write(DAPFLASH_SET_FILENAME);
    i2c.write(data, 11);
    i2c.endTransmission();

    return true;
}

string DaplinkFlash::getFilename()
{
    if (!wait_busy(1000)) {
        return "";
    }

    auto arr = i2c.readRegister(address, DAPFLASH_GET_FILENAME, 11);
    return string((char*)arr.data(), 11);
}

void DaplinkFlash::clearFlash()
{
    if (!wait_busy(1000)) {
        return;
    }

    i2c.beginTransmission(address);
    i2c.write(DAPFLASH_CLEAR_FLASH);
    i2c.endTransmission();
}

uint16_t DaplinkFlash::writeData(uint8_t* data, uint16_t length)
{
    uint16_t sent_data = 0;
    uint16_t available;
    uint16_t bytes_to_send;

    while (sent_data < length) {
        if (!wait_busy(1000)) {
            break;
        }

        available     = length - sent_data;
        bytes_to_send = available > I2C_WRITE_DATA_LEN ? I2C_WRITE_DATA_LEN : available;

        i2c.beginTransmission(address);
        i2c.write(DAPFLASH_WRITE_DATA);
        i2c.write(bytes_to_send);

        for (uint16_t i = sent_data; i < (sent_data + bytes_to_send); ++i) {
            i2c.write(data[i]);
        }

        for (uint16_t pad = bytes_to_send; pad < I2C_WRITE_DATA_LEN; ++pad) {
            i2c.write(0x00);
        }

        i2c.endTransmission();

        sent_data += bytes_to_send;
    }

    return sent_data;
}

uint16_t DaplinkFlash::writeString(const char* str)
{
    return writeData((uint8_t*)str, strlen(str));
}

uint16_t DaplinkFlash::writeNumber(uint32_t val)
{
    return writeString(to_string(val).c_str());
}

uint16_t DaplinkFlash::writeFloat(float val, uint8_t precision)
{
    int ent = (int)val;
    int dec = (int)((val - ent) * pow(10, precision));

    return writeString((to_string(ent) + "," + to_string(dec)).c_str());
}

DapLinkFlash_Status_Register DaplinkFlash::getStatusRegister()
{
    DapLinkFlash_Status_Register status;

    status.status_register = i2c.readRegister(address, DAPFLASH_STATUS_REG, 1)[0];

    return status;
}

DapLinkFlash_Error_Register DaplinkFlash::getErrorRegister()
{
    DapLinkFlash_Error_Register error;

    error.error_register = i2c.readRegister(address, DAPFLASH_ERROR_REG, 1)[0];

    return error;
}

bool DaplinkFlash::wait_busy(uint32_t timeout_ms)
{
    uint32_t start = getCurrentMillis();

    while (getStatusRegister().busy == 1) {
        if (getCurrentMillis() - start >= timeout_ms) {
            printf("Wait timeout !");
            return false;
        }
        printf("Wait...");
    }

    return true;
}