#pragma once

#include <array>
#include <cstdint>
#include <list>
#include <string>

#include "opcode.h"
#include "packets.h"

enum class AdvertisingType : uint8_t {
    ADV_IND             = 0x00,
    ADV_DIRECT_IND_HIGH = 0x01,
    ADV_SCAN_IND        = 0x02,
    ADV_NONCONN_IND     = 0x03,
    ADV_DIRECT_IND_LOW  = 0x04
};

enum class OwnAddressType : uint8_t {
    PUBLIC               = 0x00,
    RANDOM               = 0x01,
    CONTROLLER_OR_PUBLIC = 0x02,
    CONTROLLER_OR_RANDOM = 0x03
};

enum class PeerAddressType : uint8_t { PUBLIC = 0x00, RANDOM = 0x01 };

enum class AdvertisingFilterPolicy : uint8_t {
    ALL_DEVICE       = 0x00,
    SCAN_ACCESS_LIST = 0x01,
    CONN_ACCESS_LIST = 0x02,
    ONLY_ACCESS_LIST = 0x03
};

constexpr uint8_t ADVERTISING_CHANNEL_37 = 0x01;
constexpr uint8_t ADVERTISING_CHANNEL_38 = 0x02;
constexpr uint8_t ADVERTISING_CHANNEL_39 = 0x04;

class HCI {
  public:
    HCI() : isDebug(false) {}
    virtual ~HCI() {}

    void enableDebug() { isDebug = true; }
    void disableDebug() { isDebug = false; }

    virtual void init()          = 0;
    virtual void resetHardware() = 0;

    void poll();

    bool resetSoftware();
    bool setEventMask(uint64_t mask);
    std::array<uint32_t, 2> readBD_ADDR();

    bool leSetEventMask(uint64_t mask);
    std::array<uint32_t, 2> leRand();
    bool leSetAdvertisingParameters(
        uint16_t advertisingIntervalMin = 0x0800, uint16_t advertisingIntervalMax = 0x0800,
        AdvertisingType advertisingType = AdvertisingType::ADV_IND,
        OwnAddressType ownAddressType   = OwnAddressType::PUBLIC,
        PeerAddressType peerAddressType = PeerAddressType::PUBLIC, uint64_t peerAddress = 0x0000000000000000,
        uint8_t advertisingChannelMap = ADVERTISING_CHANNEL_37 | ADVERTISING_CHANNEL_38 | ADVERTISING_CHANNEL_39,
        AdvertisingFilterPolicy advertisingFilterPolicy = AdvertisingFilterPolicy::ALL_DEVICE);
    uint8_t leReadAdvertisingPhysicalChannelTxPower();
    bool leSetAdvertisingData(uint8_t length, uint8_t* data);
    bool leSetScanResponseData(uint8_t length, uint8_t* data);
    bool leSetAdvertisingEnable(bool enable);

  protected:
    bool isDebug;
    std::list<EventPacket> eventPackets;
    std::list<AsyncDataPacket> asyncDataPackets;

    virtual std::vector<uint8_t> sendCommand(OpCodeCommand command, uint8_t nbArgs, const uint8_t* args) = 0;
    virtual std::vector<uint8_t> sendCommand(OpCodeCommand command) { return sendCommand(command, 0, NULL); }
    virtual uint8_t available()                          = 0;
    virtual uint8_t readByte()                           = 0;
    virtual void readBytes(uint8_t* data, uint8_t size)  = 0;
    virtual void writeBytes(uint8_t* data, uint8_t size) = 0;

  private:
    void cleanPackets();
};