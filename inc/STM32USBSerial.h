#ifndef __STM32USBSERIAL_H__
#define __STM32USBSERIAL_H__

#include <cstdint>
#include "Serial.h"
#include "STM32Pin.h"

namespace codal {
    class STM32USBSerial : public Serial {

        public:
            STM32USBSerial();
            ~STM32USBSerial() {}

            int init() final override;
            int putc(char c) final override;
            int getc() final override { return this->getChar(ASYNC); }

            bool isConnected();

        protected:
            int enableInterrupt(SerialInterruptType t);
            int disableInterrupt(SerialInterruptType t) final override { return DEVICE_OK; }
            int setBaudrate(uint32_t baudrate) final override { return DEVICE_OK; }
            int configurePins(Pin& tx, Pin& rx) final override { return DEVICE_OK; }

        private:
            STM32Pin nonePin;
            static STM32USBSerial* instance;
            
            static void receivedCallback();
    };
}

#endif