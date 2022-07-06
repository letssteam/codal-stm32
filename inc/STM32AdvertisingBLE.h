// #pragma once

// #include <cstddef>
// #include <cstdint>
// #include <deque>
// #include <map>

// #include "CodalComponent.h"
// #include "Event.h"
// #include "STM32duinoBLE.h"
// #include "clock.h"

// namespace codal {

// constexpr uint32_t BLE_MIN_RETENTION_TIME     = 30000;
// constexpr uint32_t BLE_DEFAULT_RETENTION_TIME = 300000;
// enum AdvertisingState { OFF, SCAN, EMIT };

// struct ReceivedMessage {
//     std::string address;
//     std::string name;
//     std::string uuid;
//     std::vector<uint8_t> message;
//     int rssi;
//     bool isRead;
//     uint32_t expirationTime;

//     ReceivedMessage(std::string address, std::string name, const char* uuid, std::vector<uint8_t> message, int rssi,
//                     uint32_t retentionTime = BLE_DEFAULT_RETENTION_TIME)
//         : address(address),
//           name(name),
//           uuid(uuid),
//           message(message),
//           rssi(rssi),
//           isRead(false),
//           expirationTime(getCurrentMillis() + retentionTime)
//     {
//     }

//     ReceivedMessage() : address(""), name(""), uuid(""), rssi(0), isRead(false), expirationTime(getCurrentMillis())
//     {}
// };

// class STM32AdvertisingBLE {
//   public:
//     /**
//      * @brief Construct a new STM32AdvertisingBLE object
//      *
//      * @param id
//      * @param channel the channel use to send advertising data (Note: advertising channel are: 37[default], 38 or 39)
//      */
//     STM32AdvertisingBLE(uint8_t channel = 37);

//     /**
//      * @brief Destroy the STM32AdvertisingBLE object
//      *
//      */
//     ~STM32AdvertisingBLE() {}

//     /**
//      * @brief Set the Service Data to send over advertising. Due to the limit of the advertising frame, the message
//      will
//      * be truncated if has a length greater than MAX_ADVERTISING_DATA_LENGTH bytes.
//      *
//      * @param uuidService The service UUID to use (see: https://www.bluetooth.com/specifications/assigned-numbers/)
//      * @param data The data to be sent
//      * @param length The length of data to send
//      */
//     void setServiceData(uint16_t uuidService, uint8_t* data, size_t length);

//     /**
//      * @brief Clear all data, and stop emitting
//      *
//      */
//     void clearData();

//     /**
//      * @brief Set the Duration scanning
//      *
//      * @param ms the duration in milliseconds (default: 5000 ms). 0 to disable scanning and only emitting.
//      */
//     void setDurationScanning(uint16_t ms) { durationScan = ms; }

//     /**
//      * @brief Set the Duration emitting
//      *
//      * @param ms the duration in milliseconds (default: 5000 ms). 0 to disable emitting and only scanning.
//      */
//     void setDurationEmitting(uint16_t ms) { durationEmit = ms; }

//     /**
//      * @brief Retaining time result, after this delay the result will be delete
//      *
//      * @param t The retaining time in milliseconds (default: BLE_DEFAULT_RETENTION_TIME). Min :
//      BLE_MIN_RETENTION_TIME
//      */
//     void setRetainingTime(uint32_t t) { retainingTime = (t < BLE_MIN_RETENTION_TIME) ? BLE_MIN_RETENTION_TIME : t; }

//     /**
//      * @brief Start Advertising and Scanning
//      *
//      */
//     void begin();

//     /**
//      * @brief Stop Advertising and Scanning
//      *
//      */
//     void end();

//     /**
//      * @brief Set the Local Name
//      *
//      * @param localName The device name sent in advertising packets
//      */
//     void setLocalName(const char* localName) { this->localName = localName; }

//     /**
//      * @brief Know if there is new messages
//      *
//      * @return true is there are new messages, false otherwise
//      */
//     bool hasReceivedMessage();

//     /**
//      * @brief Know if there is new messages from name device
//      *
//      * @param name The name of the device
//      * @return true is there are new messages, false otherwise
//      */
//     bool hasReceivedMessageFrom(std::string name);

//     /**
//      * @brief Know if there is new message from name device on the spécific UUID
//      *
//      * @param name The name of the device
//      * @return true is there are new messages, false otherwise
//      */
//     bool hasReceivedMessageFrom(std::string name, std::string uuid);

//     /**
//      * @brief Get all new messages. The reading is destructive, a message read cannot be read a second time.
//      *
//      * @return std::list<ReceivedMessage> The new messages
//      */
//     std::deque<ReceivedMessage> getAllReceivedMessage();

//     /**
//      * @brief Get all new messages. The reading is destructive, a message read cannot be read a second time.
//      *
//      * @param name The name of the device
//      * @return std::list<ReceivedMessage> The new messages
//      */
//     std::deque<ReceivedMessage> getAllReceivedMessageFrom(std::string name);

//     /**
//      * @brief Know if there is new message from name device on the spécific UUID
//      *
//      * @param name The name of the device
//      * @return The new message (all field have default value if no new message)
//      */
//     ReceivedMessage getReceivedMessageFrom(std::string name, std::string uuid);

//     /**
//      * @brief Get if the current status is EMITTING
//      *
//      * @return true is the device currently emitting, false otherwise
//      */
//     bool isEmitting() { return state == EMIT; }

//     /**
//      * @brief Get if the current status is SCANNING
//      *
//      * @return true is the device currently scanning, false otherwise
//      */
//     bool isScanning() { return state == SCAN; }

//     /**
//      * @brief Maximum advertising data length
//      *
//      */
//     static constexpr size_t MAX_ADVERTISING_DATA_LENGTH = 27;

//   private:
//     void stateUpdate(Event);
//     void cleanMessages(Event);
//     void setData(uint16_t uuid, uint8_t* data, size_t length, bool isService);
//     void enableScan();
//     void disableScan();
//     void enableAdvertising();
//     void disableAdvertising();
//     void saveScanResult();

//     AdvertisingState state;
//     BLEAdvertisingData advData;
//     bool hasData;
//     bool isRunning;
//     const uint8_t channel;
//     uint16_t durationScan;
//     uint16_t durationEmit;
//     uint32_t retainingTime;
//     uint32_t lastStateChange;
//     const char* localName;
//     std::deque<ReceivedMessage> messages;

//     static bool isTimerSet;
// };
// }  // namespace codal