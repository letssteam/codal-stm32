#pragma once

#include "HCI.h"
#include "STM32Pin.h"
#include "STM32SPI.h"
#include "clock.h"
#include "codal_target_hal.h"

class HCI_SPI : public HCI {
  public:
    HCI_SPI(codal::STM32SPI& spi, codal::STM32Pin& cs, codal::STM32Pin& irq, codal::STM32Pin& rst,
            uint32_t freq = 8000000);

    virtual ~HCI_SPI() {}

    virtual void init() override final;
    virtual void resetHardware() override final;
    void poll();

  protected:
    virtual std::vector<uint8_t> sendCommand(OpCodeCommand command, uint8_t nbARgs, const uint8_t* args) override final;
    virtual uint8_t available() override final;
    virtual uint8_t readByte() override final;
    virtual void readBytes(uint8_t* data, uint8_t size) override final;
    virtual void writeBytes(uint8_t* data, uint8_t size) override final;

  private:
    codal::STM32SPI& spi;
    codal::STM32Pin& cs;
    codal::STM32Pin& irq;
    codal::STM32Pin& rst;

    void enableCS()
    {
        cs.setDigitalValue(0);
        target_wait_us(500);
    }
    void disableCS() { cs.setDigitalValue(1); }
    void waitForInit();

    bool isReadyToWrite(uint8_t nbByteToWrite);
};