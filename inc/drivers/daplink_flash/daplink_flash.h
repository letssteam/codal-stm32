#pragma once

#include <STM32I2C.h>

#include <cinttypes>
#include <string>

namespace codal {

struct DapLinkFlash_Status_Register {
    union {
        struct {
            /**
             * @brief Reserved
             */
            uint8_t RSV : 7;

            /**
             * @brief The flash is busy
             */
            uint8_t busy : 1;
        };
        uint8_t status_register;
    };
};

struct DapLinkFlash_Error_Register {
    union {
        struct {
            /**
             * @brief Parameters sent after command are invalid (For exemple: Too short or too long filename)
             */
            uint8_t badParameter : 1;

            /**
             * @brief Reserved
             */
            uint8_t RSV : 4;

            /**
             * @brief The file has reached its maximum size, and no more data can be added.
             */
            uint8_t fillIsFull : 1;

            /**
             * @brief The filename contains invalid characters
             */
            uint8_t badFilename : 1;

            /**
             * @brief The last command failed
             */
            uint8_t lastCommandFailed : 1;
        };
        uint8_t error_register;
    };
};

class DaplinkFlash {
  public:
    DaplinkFlash(STM32I2C& i2c, uint16_t address = 0x76);

    /**
     * @brief Get the device ID (0x4C)
     *
     * @return uint8_t
     */
    uint8_t whoAmI();

    /**
     * @brief Defines the file name, using the 8.3 naming standard
     *
     * @param filename the filename string, must including the terminating null character
     * @param extension the extension string, must including the terminating null character
     * @return true if success, false otherwise
     */
    bool setFilename(const char* filename, const char* extension);

    /**
     * @brief Read the filename save in flash
     *
     * @return std::string the filename (using the 8.3 naming standard)
     */
    std::string getFilename();

    /**
     * @brief Erase entire flash
     *
     */
    void clearFlash();

    /**
     * @brief Save raw data in flash
     *
     * @param data data array
     * @param length data array length
     * @return uint16_t the number of bytes sent to the flash
     */
    uint16_t writeData(uint8_t* data, uint16_t length);

    /**
     * @brief Save string in flash
     *
     * @param str The string to save. The string must contain a terminating null character.
     */
    uint16_t writeString(const char* str);

    uint16_t writeNumber(uint32_t val);

    uint16_t writeFloat(float val, uint8_t precision);

    /**
     * @brief Get the Status Register
     *
     * @return DapLinkFlash_Status_Register
     */
    DapLinkFlash_Status_Register getStatusRegister();

    /**
     * @brief Get the Error Register
     *
     * @return DapLinkFlash_Error_Register
     */
    DapLinkFlash_Error_Register getErrorRegister();

  private:
    STM32I2C& i2c;
    uint16_t address;

    bool wait_busy(uint32_t timeout_ms);
};

}  // namespace codal
