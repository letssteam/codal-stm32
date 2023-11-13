#pragma once

#include <map>

#include "STM32Pin.h"
#include "STM32Serial.h"
#include "STM32SerialType.h"
#include "SingleWireSerial.h"
#include "uart.h"

namespace codal {
class STM32SingleWireSerial : public SingleWireSerial {
  public:
    STM32SingleWireSerial(STM32Pin& pin, uint16_t bufferSize = 32, uint16_t id = DEVICE_ID_SINGLE_WIRE_SERIAL);

    void init(uint32_t baudrate = 115200, LengthSerial databits = LengthSerial::SERIAL_8_BITS,
              ParitySerial parity   = ParitySerial::SERIAL_NONE_PARITY,
              StopBitSerial stopBit = StopBitSerial::SERIAL_STOP_1);

    int putc(char c) final override;
    int getc() final override;

    int send(uint8_t* buf, int len) final override;
    int receive(uint8_t* buf, int len) final override;

    int setBaud(uint32_t baud) final override;
    uint32_t getBaud() final override;

    int getBytesReceived() final override;
    int getBytesTransmitted() final override;

    int sendBreak() final override;

  protected:
    // static void SingleWireRxIRQ(serial_t* obj);

    void configureRxInterrupt(int enable) final override;
    int configureTx(int) final override;
    int configureRx(int) final override;

  private:
    // static std::map<serial_t*, STM32SingleWireSerial*> mapSingleWireSerialInstance;

    // serial_t serial;
    STM32Pin& pin;
    STM32Pin nc_pin;
    STM32Serial serial;
    // uint16_t bufferSize;
    // uint8_t* buffer;
    // uint16_t bufferHead;
    // uint16_t bufferTail;
};
}  // namespace codal