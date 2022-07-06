#include "HCI_SPI.h"

using namespace codal;

constexpr uint32_t EXPIRATION_PACKETS_MS = 10000;
constexpr uint32_t TIMEOUT_COMMAND_MS    = 1000;

/*  BLUENRG SPI OPERATION */
constexpr uint8_t BLUENRG_WRITE_OP = 0x0A;
constexpr uint8_t BLUENRG_READ_OP  = 0x0B;

/*  TYPE OF PACKET  */
constexpr uint8_t HCI_COMMAND_PKT   = 0x01;
constexpr uint8_t HCI_ASYNCDATA_PKT = 0x02;
constexpr uint8_t HCI_EVENT_PKT     = 0x04;

HCI_SPI::HCI_SPI(STM32SPI& spi, STM32Pin& cs, STM32Pin& irq, STM32Pin& rst, uint32_t freq)
    : HCI(), spi(spi), cs(cs), irq(irq), rst(rst)
{
    spi.setFrequency(freq);
    spi.setMode(0);
}

void HCI_SPI::init()
{
    disableCS();

    irq.setPull(PullMode::Down);
    irq.getDigitalValue();

    resetHardware();
    waitForInit();
}

void HCI_SPI::resetHardware()
{
    rst.setDigitalValue(0);
    target_wait(5);
    rst.setDigitalValue(1);
    target_wait(5);
}

void HCI_SPI::poll()
{
    if (irq.getDigitalValue() == 0) return;

    enableCS();
    HCI::poll();
    disableCS();
}

void HCI_SPI::waitForInit()
{
    bool wait = true;

    if (isDebug) printf("Start initialization...\r\n");

    while (irq.getDigitalValue() == 0)
        ;

    while (wait) {
        enableCS();

        uint8_t bytesToRead = available();

        if (bytesToRead > 0) {
            if (bytesToRead == 6) {
                uint8_t buffer[6] = {0x00};
                readBytes(buffer, 6);

                if (buffer[0] == 0x04 && buffer[1] == 0xFF && buffer[2] == 0x03 && buffer[3] == 0x01 &&
                    buffer[4] == 0x00 && buffer[5] == 0x01) {
                    wait = false;
                }
            }
            else {
                readBytes(NULL, bytesToRead);
            }
        }

        disableCS();
    }

    if (isDebug) printf("Initialization done...\r\n");
}

std::vector<uint8_t> HCI_SPI::sendCommand(OpCodeCommand command, uint8_t nbArgs, const uint8_t* args)
{
    uint8_t opcodeHigh           = ((uint16_t)command & 0xFF00) >> 8;
    uint8_t opcodeLow            = ((uint16_t)command & 0x00FF);
    uint8_t payloadSize          = 4 + nbArgs;
    uint8_t payload[payloadSize] = {HCI_COMMAND_PKT, opcodeLow, opcodeHigh, nbArgs};
    auto result                  = std::vector<uint8_t>();

    memcpy(&payload[4], args, nbArgs);

    enableCS();

    // Wait device ready to send data
    for (uint32_t start = getCurrentMillis(); !isReadyToWrite(payloadSize);) {
        disableCS();
        if (getCurrentMillis() - start > TIMEOUT_COMMAND_MS) {
            return result;
        }

        target_wait(2);
        enableCS();
    }

    // Send data
    writeBytes(payload, payloadSize);
    disableCS();

    // Test command received
    uint32_t start = getCurrentMillis();

    while ((getCurrentMillis() - start) < TIMEOUT_COMMAND_MS) {
        poll();
        for (auto it = eventPackets.begin(); it != eventPackets.end(); ++it) {
            if (it->eventCode == EVT_COMMAND_COMPLETE && it->isFromCommand(command)) {
                if (it->params.size() > 3 && it->params[3] == 0x00) {
                    // Skip the 3 first bytes
                    for (uint8_t i = 3; i < it->length; i++) {
                        result.push_back(it->params[i]);
                    }
                }

                it = eventPackets.erase(it);
                return result;
            }
        }

        target_wait(5);
    }

    return result;
}

uint8_t HCI_SPI::available()
{
    spi.beginTransaction();
    spi.writeTransaction(BLUENRG_READ_OP);
    spi.writeTransaction(0x00);
    spi.writeTransaction(0x00);
    spi.writeTransaction(0x00);
    spi.writeTransaction(0x00);
    auto result = spi.endTransaction();

    if (isDebug) {
        printf("[available] ");
        for (auto e : result) {
            printf("0x%02x ", e);
        }

        printf("\r\n");
    }

    if (result[0] != 0x02) {
        return 0;
    }

    return result[3];
}

uint8_t HCI_SPI::readByte()
{
    uint8_t b = 0xff;
    spi.transfer(&b, 1, &b, 1);

    return b;
}

void HCI_SPI::readBytes(uint8_t* data, uint8_t size)
{
    uint8_t t[size] = {0xff};
    spi.transfer(t, size, data, size);
}

void HCI_SPI::writeBytes(uint8_t* data, uint8_t size)
{
    spi.transfer(data, size, NULL, 0);
}

bool HCI_SPI::isReadyToWrite(uint8_t nbByteToWrite)
{
    /*
        The SPI is not officialy supported for BLE HCI, to understand the SPI protocol of BlueNRG
        read the ST document UM1865, page 111 "SPI Interface"
    */
    spi.beginTransaction();
    spi.writeTransaction(BLUENRG_WRITE_OP);
    spi.writeTransaction(0x00);
    spi.writeTransaction(0x00);
    spi.writeTransaction(0x00);
    spi.writeTransaction(0x00);
    auto result = spi.endTransaction();

    if (isDebug) {
        printf("[isReadyToWrite] ");
        for (auto e : result) {
            printf("0x%02x ", e);
        }

        printf("\r\n");
    }

    if (result[0] != 0x02 || result[1] < nbByteToWrite) {
        return false;
    }

    return true;
}