// #ifndef __STM32_SERIAL_BLUETOOTH__
// #define __STM32_SERIAL_BLUETOOTH__
// #include <queue>
// #include <string>
// #include <vector>

// #include "CodalComponent.h"
// #include "Event.h"
// #include "ExternalEvents.h"
// #include "STM32duinoBLE.h"

// namespace codal {

// class STM32SerialBLE : CodalComponent {
//   public:
//     /**
//      * @brief Construct a new STM32SerialBLE object
//      *
//      * @param serviceUUID UUID of service (string format "XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX")
//      * @param rxUUID UUID of RX Characteristic (string format "XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX")
//      * @param txUUID UUID of RX Characteristic (string format "XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX")
//      */
//     STM32SerialBLE(const char* serviceUUID, const char* rxUUID, const char* txUUID);

//     /**
//      * @brief Read a specified amount of byte in RX Buffer
//      *
//      * @param nb The number of characters to read (by default, '0' mean all)
//      * @return std::string
//      */
//     std::string read(size_t nb = 0) { return std::string((char*)(readBuffer(nb).data())); }

//     /**
//      * @brief Read string from buffer until the delimiter (included)
//      *
//      * @param delimiter the character to delimit the end of reading
//      * @return std::string
//      */
//     std::string readUntil(char delimiter);

//     /**
//      * @brief Same as `read()` function, but return a raw value instead a string
//      *
//      * @param nb The number of bytes to read (by default, '0' mean all)
//      * @return std::vector<uint8_t>
//      */
//     std::vector<uint8_t> readBuffer(size_t nb = 0);

//     /**
//      * @brief Send string to serial communication
//      *
//      * @param str the string to send
//      */
//     void send(std::string str);

//     /**
//      * @brief send byte to serial communication
//      *
//      * @param buf pointer to buffer
//      * @param len length of data in buffer
//      */
//     void sendBuffer(uint8_t* buf, size_t len);

//     /**
//      * @brief Get the number of byte avaible for reading
//      *
//      * @return size_t the number of bytes
//      */
//     size_t available() { return rxBuffer.size(); }

//     /**
//      * @brief Get the BLEService object
//      *
//      * @return BLEService the service where buffer characteristics are registered
//      */
//     const BLEService getService() { return serialService; }

//     /**
//      * @brief Set the delimiter char when delimiter event should be fired
//      *
//      * @param delim the delimiter
//      */
//     void onDelimiterReceived(char delim) { charOnDelimiter = delim; }

//     virtual void periodicCallback() override final;
//     virtual int begin() final;

//   private:
//     std::queue<uint8_t> rxBuffer;
//     BLEService serialService;
//     BLEStringCharacteristic rxSerialCharac;
//     BLEStringCharacteristic txSerialCharac;

//     char charOnDelimiter;

//     void rxReceivedData(BLECharacteristic& characteristic);
// };

// }  // namespace codal

// #endif
