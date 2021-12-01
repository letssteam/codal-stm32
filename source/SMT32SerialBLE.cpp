// #include <algorithm>
// #include <functional>

// #include "STM32SerialBLE.h"

// using namespace std;
// using namespace codal;

// #define DEVICE_INITIALIZED 0x01

// #define BLE_TX_BUFFER_SIZE 512
// #define BLE_RX_BUFFER_SIZE 512

// STM32SerialBLE::STM32SerialBLE(const char* serviceUUID, const char* rxUUID, const char* txUUID)
//     : CodalComponent(STM32_ID_BLE_SERIAL, 0),
//       serialService(serviceUUID),
//       rxSerialCharac(rxUUID, BLEWrite | BLEWriteWithoutResponse, BLE_RX_BUFFER_SIZE),
//       txSerialCharac(txUUID, BLEIndicate, BLE_TX_BUFFER_SIZE),
//       charOnDelimiter(0)
// {
// }

// void STM32SerialBLE::periodicCallback()
// {
//     if ((status & DEVICE_INITIALIZED) != 0) {
//         BLE.poll();
//     }
// }

// int STM32SerialBLE::begin()
// {
//     if ((status & DEVICE_INITIALIZED) != 0) {
//         return DEVICE_NOT_SUPPORTED;
//     }

//     status |= DEVICE_INITIALIZED;

//     serialService.addCharacteristic(rxSerialCharac);
//     serialService.addCharacteristic(txSerialCharac);
//     BLE.addService(serialService);

//     rxSerialCharac.setEventHandler(BLEWritten, [&](BLEDevice d, BLECharacteristic c) { rxReceivedData(c); });
//     return DEVICE_OK;
// }

// string STM32SerialBLE::readUntil(char delimiter)
// {
//     string result;

//     while (result.back() != delimiter && !rxBuffer.empty()) {
//         result += (char)rxBuffer.front();
//         rxBuffer.pop();
//     }

//     return result;
// }

// vector<uint8_t> STM32SerialBLE::readBuffer(size_t nb)
// {
//     vector<uint8_t> result;
//     size_t byteToRead;

//     if (nb == 0) {
//         byteToRead = rxBuffer.size();
//     }
//     else {
//         byteToRead = std::min(nb, rxBuffer.size());
//     }

//     for (size_t i = 0; i < byteToRead; ++i) {
//         result.push_back(rxBuffer.front());
//         rxBuffer.pop();
//     }

//     return result;
// }

// void STM32SerialBLE::send(string str)
// {
//     if (str.size() > BLE_TX_BUFFER_SIZE) {
//         txSerialCharac.writeValue(str.substr(0, BLE_TX_BUFFER_SIZE));
//         send(str.substr(BLE_TX_BUFFER_SIZE + 1));
//     }
//     else {
//         txSerialCharac.writeValue(str);
//     }
// }

// void STM32SerialBLE::sendBuffer(uint8_t* buf, size_t len)
// {
//     string str((char*)buf, len);
//     send(str);
// }

// void STM32SerialBLE::rxReceivedData(BLECharacteristic& characteristic)
// {
//     auto value = characteristic.value();
//     for (int i = 0; i < characteristic.valueLength(); ++i) {
//         rxBuffer.push(value[i]);
//     }

//     if (count(value, value + characteristic.valueLength(), (uint8_t)charOnDelimiter) > 0) {
//         codal::Event(STM32_ID_BLE_SERIAL, STM32_BLE_EVT_DELIMITER);
//     }
// }
