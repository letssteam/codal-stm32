#include "STM32SPI.h"

#include "CodalConfig.h"
#include "CodalDmesg.h"
#include "ErrorNo.h"

using namespace codal;
using namespace std;

STM32SPI::STM32SPI(STM32Pin& miso, STM32Pin& mosi, STM32Pin& sclk, uint32_t freq, int mode, bool msbFirst)
    : SPI(),
      frequency(freq),
      mode(modeToSpiMode(mode)),
      isMsbFirst(msbFirst),
      isOnTransaction(false),
      isMISODisabled(false)
{
    pin_miso = (miso.name == PinNumber::NC) ? PinName::NC : (PinName)miso.name;
    pin_mosi = (mosi.name == PinNumber::NC) ? PinName::NC : (PinName)mosi.name;
    pin_sclk = (sclk.name == PinNumber::NC) ? PinName::NC : (PinName)sclk.name;
}

STM32SPI::~STM32SPI() {}

void STM32SPI::beginTransaction()
{
    if (isOnTransaction) return;

    hspi.Instance               = (SPI_TypeDef*)pinmap_peripheral(pin_sclk, PinMap_SPI_SCLK);
    hspi.Init.Mode              = SPI_MODE_MASTER;
    hspi.Init.Direction         = SPI_DIRECTION_2LINES;
    hspi.Init.DataSize          = SPI_DATASIZE_8BIT;
    hspi.Init.CLKPolarity       = getClockPolarity();
    hspi.Init.CLKPhase          = getClockPhase();
    hspi.Init.NSS               = SPI_NSS_SOFT;
    hspi.Init.BaudRatePrescaler = getPrescaler();
    hspi.Init.FirstBit          = isMsbFirst ? SPI_FIRSTBIT_MSB : SPI_FIRSTBIT_LSB;
    hspi.Init.TIMode            = SPI_TIMODE_DISABLE;
    hspi.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
    hspi.Init.CRCPolynomial     = 7;
    hspi.Init.CRCLength         = SPI_CRC_LENGTH_DATASIZE;
    hspi.Init.NSSPMode          = SPI_NSS_PULSE_DISABLE;

    enableSPIClock();

    if (HAL_SPI_Init(&hspi) != HAL_OK) {
        Error_Handler();
    }

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_TypeDef* port;

    if (pin_sclk != NC) {
        port                      = set_GPIO_Port_Clock(STM_PORT(pin_sclk));
        GPIO_InitStruct.Pin       = STM_LL_GPIO_PIN(pin_sclk);
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_PULLUP;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = STM_PIN_AFNUM(pinmap_function(pin_sclk, PinMap_SPI_SCLK));
        HAL_GPIO_Init(port, &GPIO_InitStruct);
    }

    if (pin_mosi != NC) {
        port                      = set_GPIO_Port_Clock(STM_PORT(pin_mosi));
        GPIO_InitStruct.Pin       = STM_LL_GPIO_PIN(pin_mosi);
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_PULLUP;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = STM_PIN_AFNUM(pinmap_function(pin_mosi, PinMap_SPI_MOSI));
        HAL_GPIO_Init(port, &GPIO_InitStruct);
    }

    if (pin_miso != NC && !isMISODisabled) {
        port                      = set_GPIO_Port_Clock(STM_PORT(pin_miso));
        GPIO_InitStruct.Pin       = STM_LL_GPIO_PIN(pin_miso);
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_PULLUP;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = STM_PIN_AFNUM(pinmap_function(pin_miso, PinMap_SPI_MISO));
        HAL_GPIO_Init(port, &GPIO_InitStruct);
    }

    buffer.clear();
    isOnTransaction = true;
}

vector<uint8_t> STM32SPI::endTransaction(uint32_t timeout, bool skipReceive)
{
    if (!isOnTransaction) return vector<uint8_t>();

    auto result = vector<uint8_t>(buffer.size());

    // spi_init(&spi, frequency, mode, isMsbFirst);

    // spi_transfer(&spi, buffer.data(), result.data(), buffer.size(), timeout, skipReceive);

    // spi_deinit(&spi);

    __HAL_SPI_ENABLE(&hspi);

    for (size_t i = 0; i < buffer.size(); ++i) {
        SPI1->CR2 |= (1) << 12;

        while ((SPI1->SR & (1 << 1)) == 0)
            ;

        *((uint8_t*)(&SPI1->DR)) = buffer[i];

        while (SPI1->SR & (1 << 7))
            ;  // Wait for not busy

        while ((SPI1->SR & (1 << 0)) == 0)
            ;  // Wait for the receiving area to be empty

        result.push_back(*((uint8_t*)(&SPI1->DR)));
    }

    __HAL_SPI_DISABLE(&hspi);
    disableSPIClock();
    isOnTransaction = false;

    return result;
}

void STM32SPI::writeTransaction(uint8_t data)
{
    if (!isOnTransaction) return;
    buffer.push_back(data);
}

uint8_t* STM32SPI::write(uint8_t* tx_buffer, uint8_t* rx_buffer, uint16_t len)
{
    // spi_init(&spi, frequency, mode, isMsbFirst);
    // spi_transfer(&spi, tx_buffer, rx_buffer, 1, 1000, false);
    // spi_deinit(&spi);
    // return rx_buffer;

    return 0;
}

int STM32SPI::write16(uint16_t data)
{
    // uint16_t tmp;
    // uint16_t result = 0;

    // if (isMsbFirst) {
    //     tmp  = ((data & 0xff00) >> 8) | ((data & 0xff) << 8);
    //     data = tmp;
    // }

    // spi_init(&spi, frequency, mode, isMsbFirst);
    // spi_transfer(&spi, (uint8_t*)&data, (uint8_t*)&result, sizeof(uint16_t), 1000, false);
    // spi_deinit(&spi);

    // if (isMsbFirst) {
    //     tmp    = ((result & 0xff00) >> 8) | ((result & 0xff) << 8);
    //     result = tmp;
    // }

    // return (int)result;
    return 0;
}

int STM32SPI::write(int data)
{
    // uint8_t sendData = data;
    // uint8_t result   = 0;

    // // miso_value = miso_pin->getDigitalValue();
    // spi_init(&spi, frequency, mode, isMsbFirst);
    // // miso_pin->setDigitalValue(miso_value, true);
    // spi_transfer(&spi, &sendData, &result, 1, 1000, true);
    // spi_deinit(&spi);

    // return (int)result;

    beginTransaction();
    writeTransaction(data);
    return (int)endTransaction()[0];
}

spi_mode_e STM32SPI::modeToSpiMode(int mode)
{
    switch (mode) {
        case 0:
            return SPI_MODE_0;

        case 1:
            return SPI_MODE_1;

        case 2:
            return SPI_MODE_2;

        case 3:
            return SPI_MODE_3;

        default:
            return SPI_MODE_0;
    }
}