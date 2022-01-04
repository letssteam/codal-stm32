#pragma once

#include "Pin.h"
#include "SingleWireSerial.h"
#include "pinmap.h"
#include "uart.h"

namespace codal {

class STM32SingleWireSerial : public SingleWireSerial {
    // UART_HandleTypeDef uart;
    serial_t serial;

    uint8_t* buf;
    uint16_t bufLen;

  protected:
    virtual void configureRxInterrupt(int enable);

    virtual int configureTx(int);

    virtual int configureRx(int);

  public:
    static void _complete(uint32_t instance, uint32_t mode);
    static int txCpltCallback(serial_t* hspi);
    static void rxCpltCallback(serial_t* hspi);
    static void errorCallback(serial_t* hspi);

    uint8_t* currentBuffer;
    uint32_t currentBufferIndex;

    // only works with a TX uart pin on STM.
    STM32SingleWireSerial(Pin& p);

    virtual int putc(char c);
    virtual int getc();

    virtual int send(uint8_t* data, int len);
    virtual int receive(uint8_t* data, int len);

    virtual int getBytesReceived() override;
    virtual int getBytesTransmitted() override;

    virtual int setBaud(uint32_t baud);
    virtual uint32_t getBaud();

    virtual int setMode(SingleWireMode sw);

    virtual int sendBreak();
};
}  // namespace codal