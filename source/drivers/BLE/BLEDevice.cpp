/*
  This file is part of the ArduinoBLE library.
  Copyright (c) 2018 Arduino SA. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include "BLEDevice.h"

#include <strings.h>

#include "remote/BLERemoteDevice.h"
#include "utility/ATT.h"
#include "utility/BLEUuid.h"
#include "utility/HCI.h"

BLEDevice::BLEDevice() : _advertisementTypeMask(0), _eirDataLength(0), _rssi(127)
{
    memset(_address, 0x00, sizeof(_address));
}

BLEDevice::BLEDevice(uint8_t addressType, uint8_t address[6])
    : _addressType(addressType), _advertisementTypeMask(0), _eirDataLength(0), _rssi(127)
{
    memcpy(_address, address, sizeof(_address));
}

BLEDevice::~BLEDevice() {}

void BLEDevice::poll()
{
    HCI.poll();
}

void BLEDevice::poll(unsigned long timeout)
{
    HCI.poll(timeout);
}

bool BLEDevice::connected() const
{
    HCI.poll();

    if (!(*this)) {
        return false;
    }

    return ATT.connected(_addressType, _address);
}

bool BLEDevice::disconnect()
{
    return ATT.disconnect(_addressType, _address);
}

std::string BLEDevice::address() const
{
    char result[18];
    sprintf(result, "%02x:%02x:%02x:%02x:%02x:%02x", _address[5], _address[4], _address[3], _address[2], _address[1],
            _address[0]);

    return result;
}

bool BLEDevice::hasLocalName() const
{
    return (localName().length() > 0);
}

bool BLEDevice::hasAdvertisedServiceUuid() const
{
    return hasAdvertisedServiceUuid(0);
}

bool BLEDevice::hasAdvertisedServiceUuid(int index) const
{
    return (advertisedServiceUuid(index).length() > 0);
}

int BLEDevice::advertisedServiceUuidCount() const
{
    int advertisedServiceCount = 0;

    for (unsigned char i = 0; i < _eirDataLength;) {
        int eirLength = _eirData[i++];
        int eirType   = _eirData[i++];

        if (eirType == 0x02 || eirType == 0x03 || eirType == 0x06 || eirType == 0x07) {
            int uuidLength;

            if (eirType == 0x02 || eirType == 0x03) {
                uuidLength = 2;
            }
            else /*if (eirType == 0x06 || eirType == 0x07)*/ {
                uuidLength = 16;
            }

            for (int j = 0; j < (eirLength - 1); j += uuidLength) {
                advertisedServiceCount++;
            }
        }

        i += (eirLength - 1);
    }

    return advertisedServiceCount;
}

std::string BLEDevice::localName() const
{
    std::string localName = "";

    for (int i = 0; i < _eirDataLength;) {
        int eirLength = _eirData[i++];
        int eirType   = _eirData[i++];

        if (eirType == 0x08 || eirType == 0x09) {
            localName.reserve(eirLength - 1);

            for (int j = 0; j < (eirLength - 1); j++) {
                localName += (char)_eirData[i + j];
            }
            break;
        }

        i += (eirLength - 1);
    }

    return localName;
}

std::string BLEDevice::advertisedServiceUuid() const
{
    return advertisedServiceUuid(0);
}

std::string BLEDevice::advertisedServiceUuid(int index) const
{
    std::string serviceUuid;
    int uuidIndex = 0;

    for (unsigned char i = 0; i < _eirDataLength;) {
        int eirLength = _eirData[i++];
        int eirType   = _eirData[i++];

        if (eirType == 0x02 || eirType == 0x03 || eirType == 0x06 || eirType == 0x07) {
            int uuidLength;

            if (eirType == 0x02 || eirType == 0x03) {
                uuidLength = 2;
            }
            else /*if (eirType == 0x06 || eirType == 0x07)*/ {
                uuidLength = 16;
            }

            for (int j = 0; j < (eirLength - 1); j += uuidLength) {
                if (uuidIndex == index) {
                    serviceUuid = BLEUuid::uuidToString(&_eirData[i + j * uuidLength], uuidLength);
                }

                uuidIndex++;
            }
        }

        i += (eirLength - 1);
    }

    return serviceUuid;
}

int BLEDevice::rssi()
{
    uint16_t handle = ATT.connectionHandle(_addressType, _address);

    if (handle != 0xffff) {
        return HCI.readRssi(handle);
    }

    return _rssi;
}

bool BLEDevice::connect()
{
    return ATT.connect(_addressType, _address);
}

bool BLEDevice::discoverAttributes()
{
    return ATT.discoverAttributes(_addressType, _address, NULL);
}

bool BLEDevice::discoverService(const char* serviceUuid)
{
    return ATT.discoverAttributes(_addressType, _address, serviceUuid);
}

BLEDevice::operator bool() const
{
    uint8_t zeros[6] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };

    return (memcmp(_address, zeros, sizeof(zeros)) != 0);
}

bool BLEDevice::operator==(const BLEDevice& rhs) const
{
    return ((_addressType == rhs._addressType) && memcmp(_address, rhs._address, sizeof(_address)) == 0);
}

bool BLEDevice::operator!=(const BLEDevice& rhs) const
{
    return ((_addressType != rhs._addressType) || memcmp(_address, rhs._address, sizeof(_address)) != 0);
}

std::string BLEDevice::deviceName()
{
    BLERemoteDevice* device = ATT.device(_addressType, _address);

    if (device) {
        BLEService genericAccessService = service("1800");

        if (genericAccessService) {
            BLECharacteristic deviceNameCharacteristic = genericAccessService.characteristic("2a00");

            if (deviceNameCharacteristic) {
                deviceNameCharacteristic.read();

                std::string result;
                int valueLength   = deviceNameCharacteristic.valueLength();
                const char* value = (const char*)deviceNameCharacteristic.value();

                result.reserve(valueLength);

                for (int i = 0; i < valueLength; i++) {
                    result += value[i];
                }

                return result;
            }
        }
    }

    return "";
}

int BLEDevice::appearance()
{
    BLERemoteDevice* device = ATT.device(_addressType, _address);

    if (device) {
        BLEService genericAccessService = service("1801");

        if (genericAccessService) {
            BLECharacteristic appearanceCharacteristic = genericAccessService.characteristic("2a01");

            if (appearanceCharacteristic) {
                appearanceCharacteristic.read();

                uint16_t result = 0;

                memcpy(&result, appearanceCharacteristic.value(),
                       std::min((int)sizeof(result), appearanceCharacteristic.valueLength()));

                return result;
            }
        }
    }

    return 0;
}

int BLEDevice::serviceCount() const
{
    BLERemoteDevice* device = ATT.device(_addressType, _address);

    if (device) {
        return device->serviceCount();
    }

    return 0;
}

bool BLEDevice::hasService(const char* uuid) const
{
    return hasService(uuid, 0);
}

bool BLEDevice::hasService(const char* uuid, int index) const
{
    BLERemoteDevice* device = ATT.device(_addressType, _address);

    if (device) {
        int count       = 0;
        int numServices = device->serviceCount();

        for (int i = 0; i < numServices; i++) {
            BLERemoteService* s = device->service(i);

            if (strcasecmp(uuid, s->uuid()) == 0) {
                if (count == index) {
                    return true;
                }

                count++;
            }
        }
    }

    return false;
}

BLEService BLEDevice::service(int index) const
{
    BLERemoteDevice* device = ATT.device(_addressType, _address);

    if (device) {
        if (index < (int)device->serviceCount()) {
            return BLEService(device->service(index));
        }
    }

    return BLEService();
}

BLEService BLEDevice::service(const char* uuid) const
{
    return service(uuid, 0);
}

BLEService BLEDevice::service(const char* uuid, int index) const
{
    BLERemoteDevice* device = ATT.device(_addressType, _address);

    if (device) {
        int count       = 0;
        int numServices = device->serviceCount();

        for (int i = 0; i < numServices; i++) {
            BLERemoteService* s = device->service(i);

            if (strcasecmp(uuid, s->uuid()) == 0) {
                if (count == index) {
                    return BLEService(s);
                }

                count++;
            }
        }
    }

    return BLEService();
}

int BLEDevice::characteristicCount() const
{
    BLERemoteDevice* device = ATT.device(_addressType, _address);

    if (device) {
        int result      = 0;
        int numServices = device->serviceCount();

        for (int i = 0; i < numServices; i++) {
            result += device->service(i)->characteristicCount();
        }

        return result;
    }

    return 0;
}

bool BLEDevice::hasCharacteristic(const char* uuid) const
{
    return hasCharacteristic(uuid, 0);
}

bool BLEDevice::hasCharacteristic(const char* uuid, int index) const
{
    BLERemoteDevice* device = ATT.device(_addressType, _address);

    if (device) {
        int count       = 0;
        int numServices = device->serviceCount();

        for (int i = 0; i < numServices; i++) {
            BLERemoteService* s = device->service(i);

            int numCharacteristics = s->characteristicCount();

            for (int j = 0; j < numCharacteristics; j++) {
                BLERemoteCharacteristic* c = s->characteristic(j);

                if (strcasecmp(c->uuid(), uuid) == 0) {
                    if (count == index) {
                        return true;
                    }
                }

                count++;
            }
        }
    }

    return false;
}

BLECharacteristic BLEDevice::characteristic(int index) const
{
    BLERemoteDevice* device = ATT.device(_addressType, _address);

    if (device) {
        int count       = 0;
        int numServices = device->serviceCount();

        for (int i = 0; i < numServices; i++) {
            BLERemoteService* s = device->service(i);

            int numCharacteristics = s->characteristicCount();

            for (int j = 0; j < numCharacteristics; j++) {
                if (count == index) {
                    BLERemoteCharacteristic* c = s->characteristic(j);

                    return BLECharacteristic(c);
                }

                count++;
            }
        }
    }

    return BLECharacteristic();
}

BLECharacteristic BLEDevice::characteristic(const char* uuid) const
{
    return characteristic(uuid, 0);
}

BLECharacteristic BLEDevice::characteristic(const char* uuid, int index) const
{
    BLERemoteDevice* device = ATT.device(_addressType, _address);

    if (device) {
        int count       = 0;
        int numServices = device->serviceCount();

        for (int i = 0; i < numServices; i++) {
            BLERemoteService* s = device->service(i);

            int numCharacteristics = s->characteristicCount();

            for (int j = 0; j < numCharacteristics; j++) {
                BLERemoteCharacteristic* c = s->characteristic(j);

                if (strcasecmp(c->uuid(), uuid) == 0) {
                    if (count == index) {
                        return BLECharacteristic(c);
                    }

                    count++;
                }
            }
        }
    }

    return BLECharacteristic();
}

uint8_t BLEDevice::manufacturerDataCount() const
{
    uint8_t result = 0;
    uint8_t length = 0;
    uint8_t type   = 0;

    for (uint8_t i = 0; i < _eirDataLength;) {
        length = _eirData[i];
        ++i;
        type = _eirData[i];
        i += length;

        if (type == 0xFF) {
            result++;
        }
    }

    return result;
}

ManufacturerData BLEDevice::getManufacturerData(uint8_t index) const
{
    uint8_t currentIndex = 0;
    uint8_t pos          = 0;
    uint8_t length       = 0;
    uint8_t type         = 0;

    while (pos < _eirDataLength) {
        length = _eirData[pos];
        pos += 1;
        type = _eirData[pos];

        if (type == 0xFF) {
            if (currentIndex == index) {
                pos += 1;
                break;
            }

            currentIndex++;
            pos += length;
        }
    }

    if (pos == _eirDataLength) {
        return {0x0000, std::vector<uint8_t>()};
    }

    uint16_t id = _eirData[pos] + (_eirData[pos + 1] << 8);
    pos += 2;

    return {id, std::vector<uint8_t>(_eirData + pos, _eirData + pos + length - 3)};
}

uint8_t BLEDevice::advertisingDataCount() const
{
    uint8_t result = 0;
    uint8_t length = 0;
    uint8_t type   = 0;

    for (uint8_t i = 0; i < _eirDataLength;) {
        length = _eirData[i];
        ++i;
        type = _eirData[i];
        i += length;

        if (type == 0x16) {
            result++;
        }
    }

    return result;
}

const std::vector<uint8_t> BLEDevice::getAdvertisingData(uint8_t index) const
{
    uint8_t currentIndex = 0;
    uint8_t pos          = 0;
    uint8_t length       = 0;
    uint8_t type         = 0;

    while (pos < _eirDataLength) {
        length = _eirData[pos];
        pos += 1;
        type = _eirData[pos];

        if (type == 0x16) {
            if (currentIndex == index) {
                pos += 1;
                break;
            }

            currentIndex++;
            pos += length;
        }
    }

    if (pos == _eirDataLength) {
        return std::vector<uint8_t>();
    }

    return std::vector<uint8_t>(_eirData + pos, _eirData + pos + length);
}

bool BLEDevice::hasAddress(uint8_t addressType, uint8_t address[6])
{
    return (_addressType == addressType) && (memcmp(_address, address, sizeof(_address)) == 0);
}

void BLEDevice::setAdvertisementData(uint8_t type, uint8_t eirDataLength, uint8_t eirData[], int8_t rssi)
{
    _advertisementTypeMask = (1 << type);
    _eirDataLength         = eirDataLength;
    memcpy(_eirData, eirData, eirDataLength);
    _rssi = rssi;
}

void BLEDevice::setScanResponseData(uint8_t eirDataLength, uint8_t eirData[], int8_t rssi)
{
    _advertisementTypeMask |= (1 << 0x04);
    memcpy(&_eirData[_eirDataLength], eirData, eirDataLength);
    _eirDataLength += eirDataLength;
    _rssi = rssi;
}

bool BLEDevice::discovered()
{
    // expect, 0x03 or 0x04 flag to be set
    return (_advertisementTypeMask & 0x18) != 0;
}