#include "STM32SingleWireSerial.h"

using namespace std;
using namespace codal;

// Like STM32Serial, we need to be sure that this object should be constructed before any static STM32Serial object. I'm
// so sorry for that. One again, please don't judge me :)
// std::map<serial_t*, STM32SingleWireSerial*> STM32SingleWireSerial::mapSingleWireSerialInstance
//     __attribute__((init_priority(102)));

STM32SingleWireSerial::STM32SingleWireSerial(STM32Pin& pin, uint16_t bufferSize, uint16_t id)
    : SingleWireSerial{pin, id},
      pin{pin},
      nc_pin{0, PinName::NC, PinCapability::PIN_CAPABILITY_ALL},
      serial{pin, nc_pin}
//   bufferSize{uint16_t(bufferSize + 1)},
//   buffer{new uint8_t[bufferSize]},
//   bufferHead{0},
//   bufferTail{0}
{
    // mapSingleWireSerialInstance.insert(pair<serial_t*, STM32SingleWireSerial*>(&serial, this));
}

void STM32SingleWireSerial::init(uint32_t baudrate, LengthSerial databits, ParitySerial parity, StopBitSerial stopBit)
{
    serial.init(baudrate, databits, parity, stopBit);
    // serial.pin_tx = (PinName)pin.name;
    // serial.pin_rx = PinName::NC;

    // uart_init(&serial, baudrate, uint32_t(databits), uint32_t(parity), uint32_t(stopBit));
    // uart_attach_rx_callback(&serial, SingleWireRxIRQ);
}

int STM32SingleWireSerial::putc(char c)
{
    return serial.putc(c);
}

int STM32SingleWireSerial::getc()
{
    return serial.getc();
}

int STM32SingleWireSerial::send(uint8_t* buf, int len)
{
    return serial.send(buf, len);
}

int STM32SingleWireSerial::receive(uint8_t* buf, int len)
{
    return serial.read(buf, len);
}

int STM32SingleWireSerial::setBaud(uint32_t baud)
{
    return serial.setBaud(baud);
}

uint32_t STM32SingleWireSerial::getBaud()
{
    return 0;
}

int STM32SingleWireSerial::getBytesReceived()
{
    return 0;
}

int STM32SingleWireSerial::getBytesTransmitted()
{
    return 0;
}

int STM32SingleWireSerial::sendBreak()
{
    return 0;
}

// void STM32SingleWireSerial::SingleWireRxIRQ(serial_t* obj)
// {
//     unsigned char c;

//     if (uart_getc(obj, &c) == 0) {
//         STM32SingleWireSerial* ser = mapSingleWireSerialInstance[obj];

//         uint16_t i = (unsigned int)(ser->bufferHead + 1) % ser->bufferSize;

//         if (i != ser->bufferTail) {
//             ser->buffer[ser->bufferHead] = c;
//             ser->bufferHead              = i;
//         }
//     }
// }

void STM32SingleWireSerial::configureRxInterrupt(int enable)
{
    return;
}

int STM32SingleWireSerial::configureTx(int)
{
    return 0;
}

int STM32SingleWireSerial::configureRx(int)
{
    return 0;
}
