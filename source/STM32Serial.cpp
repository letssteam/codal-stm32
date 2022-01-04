#include "STM32Serial.h"

// required for gcc-6 (but not 7!)
#undef putc
#undef getc

using namespace codal;
using namespace std;

STM32Serial* codal::default_serial_debug = nullptr;

// We need to be sure that this object should be constructed before any static STM32Serial object. I'm so sorry for
// that. Please don't judge me :)
std::map<serial_t*, STM32Serial*> STM32Serial::mapSerialInstance __attribute__((init_priority(102)));

STM32Serial::STM32Serial(STM32Pin& rxPin, STM32Pin& txPin)
    : Serial(txPin, rxPin), rxPin(rxPin), txPin(txPin), serial(), baudrate(), databits(), parity(), stopBit()
{
    mapSerialInstance.insert(pair<serial_t*, STM32Serial*>(&serial, this));
}

STM32Serial::~STM32Serial()
{
    mapSerialInstance.erase(&serial);
}

void STM32Serial::init(uint32_t baudrate, LengthSerial databits, ParitySerial parity, StopBitSerial stopBit)
{
    this->baudrate = baudrate;
    this->databits = databits;
    this->parity   = parity;
    this->stopBit  = stopBit;

    serial.pin_rx = (PinName)rxPin.name;
    serial.pin_tx = (PinName)txPin.name;

    stm32_uart_init(&serial, baudrate, (uint32_t)databits, (uint32_t)parity, (uint32_t)stopBit);
    uart_attach_rx_callback(&serial, RxIRQ);
}

int STM32Serial::enableInterrupt(SerialInterruptType t)
{
    if (t == RxInterrupt) {
        uart_attach_rx_callback(&serial, RxIRQ);
    }
    else if (t == TxInterrupt) {
        while (txBufferedSize() > 0) {
            dataTransmitted();
        }
    }

    return DEVICE_OK;
}

int STM32Serial::disableInterrupt(SerialInterruptType t)
{
    return DEVICE_OK;
}

int STM32Serial::setBaudrate(uint32_t baudrate)
{
    this->baudrate = baudrate;

    uart_deinit(&serial);
    stm32_uart_init(&serial, baudrate, (uint32_t)databits, (uint32_t)parity, (uint32_t)stopBit);

    return DEVICE_OK;
}

int STM32Serial::configurePins(Pin& tx, Pin& rx)
{
    txPin = (STM32Pin&)tx;
    rxPin = (STM32Pin&)rx;

    serial.pin_rx = (PinName)rx.name;
    serial.pin_tx = (PinName)tx.name;

    uart_deinit(&serial);
    stm32_uart_init(&serial, baudrate, (uint32_t)databits, (uint32_t)parity, (uint32_t)stopBit);

    return DEVICE_OK;
}

int STM32Serial::putc(char c)
{
    return uart_write(&serial, c, 1);
}

int STM32Serial::getc()
{
    return this->getChar(ASYNC);
}

void STM32Serial::RxIRQ(serial_t* obj)
{
    unsigned char c;

    if (uart_getc(obj, &c) == 0) {
        STM32Serial* ser = mapSerialInstance[obj];

        uint16_t i = (unsigned int)(ser->rxBuffHead + 1) % ser->rxBuffSize;

        if (i != ser->rxBuffTail) {
            ser->rxBuff[ser->rxBuffHead] = c;
            ser->rxBuffHead              = i;
        }
    }
}

#ifdef __GNUC__
/* With GCC/RAISONANCE, small msg_info (option LD Linker->Libraries->Small msg_info
   set to 'Yes') calls __io_putchar() */

#if !defined(PUTCHAR_PROTOTYPE)
#define PUTCHAR_PROTOTYPE WEAK int __io_putchar(int ch)
/**
 * @brief Retargets the C library msg_info function to the USART.
 * @param None
 * @retval None
 */
extern "C" PUTCHAR_PROTOTYPE
{
    /* Place your implementation of fputc here */
    /* e.g. write a character to the serial port and Loop until the end of transmission */
    if (default_serial_debug != nullptr && default_serial_debug->isWriteable()) {
        return default_serial_debug->sendChar(ch);
    }
    return 0;
}
#endif

#if !defined(GETCHAR_PROTOTYPE)
#define GETCHAR_PROTOTYPE WEAK int __io_getchar(void)
/**
 * @brief Retargets the C library scanf function to the USART.
 * @param None
 * @retval None
 */
extern "C" GETCHAR_PROTOTYPE
{
    /* Place your implementation of fgetc here */
    /* e.g. readwrite a character to the USART2 and Loop until the end of transmission */
    uint8_t ch = 0;
    if (default_serial_debug != nullptr && default_serial_debug->isReadable()) {
        ch = default_serial_debug->read();
    }
    return ch;
}
#endif
#endif /* __GNUC__ */
