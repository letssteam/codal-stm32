#include "HCI.h"

#include "clock.h"

constexpr uint32_t EXPIRATION_PACKETS_MS = 10000;
constexpr uint32_t TIMEOUT_COMMAND_MS    = 1000;

/*  BLUENRG SPI OPERATION */
constexpr uint8_t BLUENRG_WRITE_OP = 0x0A;
constexpr uint8_t BLUENRG_READ_OP  = 0x0B;

/*  TYPE OF PACKET  */
constexpr uint8_t HCI_COMMAND_PKT   = 0x01;
constexpr uint8_t HCI_ASYNCDATA_PKT = 0x02;
constexpr uint8_t HCI_EVENT_PKT     = 0x04;

void HCI::poll()
{
    for (uint8_t nbBytesToRead = available(); nbBytesToRead > 0; nbBytesToRead = available()) {
        uint8_t typeByte = readByte();

        if (typeByte == HCI_EVENT_PKT) {
            EventPacket pkt;
            pkt.eventCode = readByte();
            pkt.length    = readByte();

            pkt.params.resize(pkt.length);
            readBytes(pkt.params.data(), pkt.length);

            pkt.time = getCurrentMillis();

            eventPackets.push_front(pkt);

            if (isDebug) {
                printf("[poll] received event : [0x%0.2x] - %d bytes\r\n", pkt.eventCode, pkt.params.size());
                printf("\t");
                for (auto v : pkt.params) printf("0x%0.2x ", v);
                printf("\r\n");
            }
        }
        else if (typeByte == HCI_ASYNCDATA_PKT) {
            if (isDebug) printf("[poll] received data : not implemented\r\n");
            // if (isDebug) printf("[poll] received data : [%d] - %d bytes", pkt.eventCode, pkt.length);
        }
        else {
            // Delete message ?
            if (isDebug) printf("[poll] received unknown message type : 0x%0.2X\r\n", typeByte);
        }
    }

    cleanPackets();
}

bool HCI::resetSoftware()
{
    auto result = sendCommand(OpCodeCommand::RESET);
    return result.size() > 0 && result.front() > 0;
}

bool HCI::setEventMask(uint64_t mask)
{
    return false;
}

std::array<uint32_t, 2> HCI::readBD_ADDR()
{
    auto result                    = sendCommand(OpCodeCommand::READ_BD_ADDR);
    std::array<uint32_t, 2> values = {0x00, 0x00};

    if (result.empty() || result.size() != 7 || result[0] != 0x00) {
        return values;
    }

    for (uint8_t i = 0; i < 2; ++i) {
        values[0] |= (uint32_t)result[5 + i] << (8 * i);
    }

    for (uint8_t i = 0; i < 4; ++i) {
        values[1] |= (uint32_t)result[1 + i] << (8 * i);
    }

    return values;
}

bool HCI::leSetEventMask(uint64_t mask)
{
    return false;
}

std::array<uint32_t, 2> HCI::leRand()
{
    auto result                    = sendCommand(OpCodeCommand::LE_RAND);
    std::array<uint32_t, 2> values = {0x00, 0x00};

    if (result.empty() || result.size() != 9 || result[0] != 0x00) {
        return values;
    }

    for (uint8_t i = 0; i < 4; ++i) {
        values[0] |= (uint32_t)result[5 + i] << (8 * i);
        values[1] |= (uint32_t)result[1 + i] << (8 * i);
    }

    return values;
}

bool HCI::leSetAdvertisingParameters(uint16_t advertisingIntervalMin, uint16_t advertisingIntervalMax,
                                     AdvertisingType advertisingType, OwnAddressType ownAddressType,
                                     PeerAddressType peerAddressType, uint64_t peerAddress,
                                     uint8_t advertisingChannelMap, AdvertisingFilterPolicy advertisingFilterPolicy)
{
    return false;
}

uint8_t HCI::leReadAdvertisingPhysicalChannelTxPower()
{
    return 0;
}

bool HCI::leSetAdvertisingData(uint8_t length, uint8_t* data)
{
    return false;
}

bool HCI::leSetScanResponseData(uint8_t length, uint8_t* data)
{
    return false;
}

bool HCI::leSetAdvertisingEnable(bool enable)
{
    uint8_t data[1] = {enable ? (uint8_t)0x01 : (uint8_t)0x00};
    auto result     = sendCommand(OpCodeCommand::LE_SET_ADV_ENABLE, 1, data);
    return !result.empty() && result.front() > 0;
}

void HCI::cleanPackets()
{
    eventPackets.remove_if([](EventPacket e) -> bool { return getCurrentMillis() - e.time >= EXPIRATION_PACKETS_MS; });

    asyncDataPackets.remove_if(
        [](AsyncDataPacket e) -> bool { return getCurrentMillis() - e.time >= EXPIRATION_PACKETS_MS; });
}