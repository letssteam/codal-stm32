#pragma once

#include "STM32Pin.h"
#include "STM32Serial.h"

namespace codal {

/**
 * @brief the class representing the OpenLog micro-sd card reader
 */
class OpenLog {
  public:
    /**
     * @brief Constructor of the OpenLog class
     *
     * @param tx the pin used to transmit data via serial to the OpenLog
     * @param rx the pin used to receive data via serial to the OpenLog
     * @param reset the pin used to reset the OpenLog (via OpenLog's GRN pin)
     *
     * @note As serial is used to communicate with the OpenLog, serial-compatible pins must be provided
     */
    OpenLog(STM32Pin& tx, STM32Pin& rx, STM32Pin& reset);
    ~OpenLog();

    /**
     * @brief Writes data to the micro-sd card, in a file which name's defined in the .cpp.
     *
     * @param data the text to write to the micro-sd card
     * @param dataSize the size of the text to write
     *
     * @return Wether the write operation was successful or not
     *
     * @note As the OpenLog is implemented in order to allow offline sensor data logging,
     * being able to set a file name has not been implemented (as not needed at the moment)
     */
    bool write(const char* data, unsigned dataSize);

  private:
    STM32Serial* serial;
    STM32Pin& resetPin;
    uint8_t* buffer;

    /**
     * @brief an utility function to send the data via serial in fixed sized buffers
     *
     * @param data the data to send
     * @param dataSize the size of the data to send
     *
     * @return an int representing the number of transmited bytes (should be equal to dataSize), or an error code
     * (DEVICE_INVALID_PARAMETER or DEVICE_SERIAL_IN_USE)
     */
    int sendData(const uint8_t* data, unsigned dataSize);

    /**
     * @brief Sets the command mode on the OpenLog, enabling file operations on the micro-sd card
     * @note Can also be used to end some of the openlog commands (e.g the append command)
     */
    void goToCommandMode();

    /**
     * @brief hard resets the OpenLog via `resetPin`
     *
     * @note this function is used to restart the OpenLog after this class' serial is initialized, in order to ensure
     * later that the OpenLog is responding well to the send commands / text
     */
    void reset();

    /**
     * @brief listens to the OpenLog's serial output until the character `waitchar`
     *
     * @param waitChar the char to wait for (must be `>` or `<`)
     * @note this function is used to wait until the Openlog is ready to accept commands / text
     * @see
     * https://github.com/sparkfun/OpenLog/blob/master/firmware/Arduino_Examples/Example3_ReadFile/Example3_ReadFile.ino
     */
    void waitUntilReady(const char waitChar);
};
}  // namespace codal