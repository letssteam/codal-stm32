// #include "STM32AdvertisingBLE.h"

// #include <vector>

// #include "EventModel.h"
// #include "Timer.h"
// #include "codal_target_hal.h"

// using namespace std;
// using namespace codal;

// constexpr uint16_t EVENT_STATE_UPDATE   = 0x1415;  // 0x1415 = 5141 in decimal...\o/
// constexpr uint16_t EVENT_CLEAN_MESSAGES = 0x1135;  // Fibonacci time !

// bool STM32AdvertisingBLE::isTimerSet = false;

// STM32AdvertisingBLE::STM32AdvertisingBLE(uint8_t channel)
//     : state(OFF),
//       hasData(false),
//       isRunning(false),
//       channel(channel),
//       durationScan(5000),
//       durationEmit(5000),
//       retainingTime(BLE_DEFAULT_RETENTION_TIME),
//       lastStateChange(0),
//       localName("Default Name")
// {
//     messages.clear();

//     if (EventModel::defaultEventBus) {
//         if (!isTimerSet) {
//             system_timer_event_every(250, DEVICE_ID_RADIO, EVENT_STATE_UPDATE);
//             system_timer_event_every(BLE_MIN_RETENTION_TIME, DEVICE_ID_RADIO, EVENT_CLEAN_MESSAGES);
//             isTimerSet = true;
//         }

//         EventModel::defaultEventBus->listen(DEVICE_ID_RADIO, EVENT_STATE_UPDATE, this,
//                                             &STM32AdvertisingBLE::stateUpdate);
//         EventModel::defaultEventBus->listen(DEVICE_ID_RADIO, EVENT_CLEAN_MESSAGES, this,
//                                             &STM32AdvertisingBLE::cleanMessages);
//     }
// }

// void STM32AdvertisingBLE::begin()
// {
//     lastStateChange = 0;
//     if (hasData) {
//         enableAdvertising();
//     }
//     else {
//         enableScan();
//     }

//     isRunning = true;
// }

// void STM32AdvertisingBLE::end()
// {
//     isRunning = false;
//     state     = OFF;
//     disableAdvertising();
//     disableScan();
// }

// void STM32AdvertisingBLE::setServiceData(uint16_t uuidService, uint8_t* data, size_t length)
// {
//     setData(uuidService, data, length, true);
// }

// void STM32AdvertisingBLE::clearData()
// {
//     advData.clear();
//     hasData = false;
// }

// bool STM32AdvertisingBLE::hasReceivedMessage()
// {
//     for (auto it = messages.begin(); it != messages.end(); it++) {
//         if (!it->isRead) return true;
//     }

//     return false;
// }

// bool STM32AdvertisingBLE::hasReceivedMessageFrom(std::string name)
// {
//     for (auto msg : messages) {
//         if (msg.name == name && !msg.isRead) return true;
//     }

//     return false;
// }

// bool STM32AdvertisingBLE::hasReceivedMessageFrom(std::string name, std::string uuid)
// {
//     for (auto msg : messages) {
//         if (msg.name == name && msg.uuid == uuid && !msg.isRead) return true;
//     }

//     return false;
// }

// std::deque<ReceivedMessage> STM32AdvertisingBLE::getAllReceivedMessage()
// {
//     std::deque<ReceivedMessage> result;

//     for (auto it = messages.begin(); it != messages.end(); it++) {
//         if (!it->isRead) {
//             result.push_back(ReceivedMessage(*it));
//             it->isRead = true;
//         }
//     }

//     return result;
// }

// std::deque<ReceivedMessage> STM32AdvertisingBLE::getAllReceivedMessageFrom(std::string name)
// {
//     std::deque<ReceivedMessage> result;

//     for (auto it = messages.begin(); it != messages.end(); it++) {
//         if (it->name == name && !it->isRead) {
//             result.push_back(ReceivedMessage(*it));
//             it->isRead = true;
//         }
//     }

//     return result;
// }

// ReceivedMessage STM32AdvertisingBLE::getReceivedMessageFrom(std::string name, std::string uuid)
// {
//     for (auto it = messages.begin(); it != messages.end(); it++) {
//         if (it->name == name && it->uuid == uuid && !it->isRead) {
//             return ReceivedMessage(*it);
//             it->isRead = true;
//         }
//     }

//     return ReceivedMessage();
// }

// void STM32AdvertisingBLE::stateUpdate(Event)
// {
//     if (!isRunning) {
//         return;
//     }

//     uint32_t delta = getCurrentMillis() - lastStateChange;

//     switch (state) {
//         case EMIT:
//             if (delta >= durationEmit && durationScan > 0) {
//                 enableScan();
//             }
//             break;

//         case SCAN:
//             if (delta >= durationScan && hasData && durationEmit > 0) {
//                 saveScanResult();
//                 enableAdvertising();
//             }
//             else {
//                 saveScanResult();
//             }
//             break;

//         case OFF:
//         default:
//             break;
//     }
// }

// void STM32AdvertisingBLE::cleanMessages(Event)
// {
//     uint32_t currentTime = getCurrentMillis();

//     for (auto it = messages.begin(); it != messages.end(); it++) {
//         if (it->expirationTime <= currentTime) {
//             it = messages.erase(it);
//         }
//     }
// }

// void STM32AdvertisingBLE::setData(uint16_t uuid, uint8_t* data, size_t length, bool isService)
// {
//     size_t dataLength = length > MAX_ADVERTISING_DATA_LENGTH ? MAX_ADVERTISING_DATA_LENGTH : length;

//     if (isRunning && state == EMIT) {
//         BLE.stopAdvertise();
//     }

//     advData.clear();
//     if (isService) {
//         advData.setManufacturerData(NULL, 0);
//         advData.setAdvertisedServiceData(uuid, data, dataLength);
//     }
//     else {
//         advData.setAdvertisedServiceData(uuid, NULL, 0);
//         advData.setManufacturerData(uuid, data, dataLength);
//     }

//     BLE.setAdvertisingData(advData);

//     if (isRunning && state == EMIT) {
//         BLE.advertise(channel);
//     }

//     hasData = true;
// }

// void STM32AdvertisingBLE::enableScan()
// {
//     disableAdvertising();
//     BLE.scan(true);

//     state           = SCAN;
//     lastStateChange = getCurrentMillis() + (target_random(101) - 50);  // Add random to avoid synchronous symptoms
// }

// void STM32AdvertisingBLE::disableScan()
// {
//     BLE.stopScan();
// }

// void STM32AdvertisingBLE::enableAdvertising()
// {
//     disableScan();

//     BLE.setLocalName(localName);
//     BLE.setConnectable(false);
//     BLE.setAdvertisingData(advData);
//     BLE.advertise(channel);

//     state           = EMIT;
//     lastStateChange = getCurrentMillis() + (target_random(101) - 50);  // Add random to avoid synchronous symptoms
// }

// void STM32AdvertisingBLE::disableAdvertising()
// {
//     BLE.stopAdvertise();
// }

// void STM32AdvertisingBLE::saveScanResult()
// {
//     bool addMessage = true;
//     std::string tmpUuid;
//     std::vector<uint8_t> tmpMsg;

//     for (BLEDevice device = BLE.available(); device; device = BLE.available()) {
//         for (auto i = 0; i < device.advertisingDataCount(); i++) {
//             addMessage = true;
//             tmpUuid    = device.getAdvertisingDataUuid(i);

//             for (auto it = messages.begin(); it != messages.end(); it++) {
//                 if (it->address == device.address() && it->uuid == tmpUuid) {
//                     tmpMsg = device.getAdvertisingData(i);

//                     if (it->message != tmpMsg) {
//                         it->name           = device.hasLocalName() ? device.localName() : "N/A";
//                         it->rssi           = device.rssi();
//                         it->message        = tmpMsg;
//                         it->isRead         = false;
//                         it->expirationTime = getCurrentMillis() + retainingTime;
//                     }

//                     addMessage = false;
//                     break;
//                 }
//             }

//             if (addMessage) {
//                 messages.push_back(ReceivedMessage(device.address(), device.hasLocalName() ? device.localName() :
//                 "N/A",
//                                                    tmpUuid.c_str(), device.getAdvertisingData(i), device.rssi(),
//                                                    retainingTime));
//             }
//         }
//     }
// }
