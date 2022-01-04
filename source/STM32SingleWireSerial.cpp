#include "STM32SingleWireSerial.h"

#include "PeripheralPins.h"
#include "codal_target_hal.h"
#include "pinmap.h"
#include "uart.h"

using namespace codal;

#define TX_CONFIGURED 0x02
#define RX_CONFIGURED 0x04

uint8_t buffer[1024] = {0};
uint16_t buffer_head = 0;
uint16_t buffer_tail = 0;
uint8_t uart_status  = 0;

static STM32SingleWireSerial* instances[4];

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

#define UART_ON (uart.Instance->CR1 & USART_CR1_UE)

#define LOG DMESG

#define ZERO(f) memset(&f, 0, sizeof(f))

static int enable_clock(uint32_t instance)
{
    switch (instance) {
        case USART1_BASE:
            __HAL_RCC_USART1_CLK_ENABLE();
            NVIC_SetPriority(USART1_IRQn, 2);
            NVIC_EnableIRQ(USART1_IRQn);
            return HAL_RCC_GetPCLK2Freq();
        case USART2_BASE:
            __HAL_RCC_USART2_CLK_ENABLE();
            NVIC_SetPriority(USART2_IRQn, 2);
            NVIC_EnableIRQ(USART2_IRQn);
            return HAL_RCC_GetPCLK1Freq();
#ifdef USART6_BASE
        case USART6_BASE:
            __HAL_RCC_USART6_CLK_ENABLE();
            NVIC_SetPriority(USART6_IRQn, 2);
            NVIC_EnableIRQ(USART6_IRQn);
            return HAL_RCC_GetPCLK2Freq();
#endif
        default:
            CODAL_ASSERT(0, DEVICE_HARDWARE_CONFIGURATION_ERROR);
            return 0;
    }
    return 0;
}

void STM32SingleWireSerial::_complete(uint32_t instance, uint32_t mode)
{
    uint8_t err = 0;
    for (unsigned i = 0; i < ARRAY_SIZE(instances); ++i) {
        if (instances[i] && (uint32_t)instances[i]->serial.handle.Instance == instance) {
            switch (mode) {
                case SWS_EVT_DATA_RECEIVED:
                case SWS_EVT_DATA_SENT:
                    if (instances[i]->cb) instances[i]->cb(mode);
                    break;

                case SWS_EVT_ERROR:
                    err = HAL_UART_GetError(&instances[i]->serial.handle);

                    // DMESG("HALE %d", err);

                    if (instances[i]->cb)
                        instances[i]->cb(SWS_EVT_DATA_RECEIVED);
                    else if (err == HAL_UART_ERROR_FE)
                        // a uart error disable any previously configured DMA transfers, we will always get a framing
                        // error... quietly restart...
                        HAL_UART_Receive_DMA(&instances[i]->serial.handle, instances[i]->buf, instances[i]->bufLen);
                    else {
                        if (instances[i]->cb)
                            instances[i]->cb(mode);
                        else
                            HAL_UART_Abort(&instances[i]->serial.handle);
                    }
                    break;

                default:
                    HAL_UART_IRQHandler(&instances[i]->serial.handle);
                    break;
            }
        }
    }
}

// void HAL_UART_TxCpltCallback(UART_HandleTypeDef* hspi)
int STM32SingleWireSerial::txCpltCallback(serial_t* hspi)
{
    STM32SingleWireSerial::_complete((uint32_t)hspi->handle.Instance, SWS_EVT_DATA_SENT);
    return 0;
}

// void HAL_UART_RxCpltCallback(UART_HandleTypeDef* hspi)
void STM32SingleWireSerial::rxCpltCallback(serial_t* hspi)
{
    STM32SingleWireSerial::_complete((uint32_t)hspi->handle.Instance, SWS_EVT_DATA_RECEIVED);
}

// void HAL_UART_ErrorCallback(UART_HandleTypeDef* hspi)
void STM32SingleWireSerial::errorCallback(serial_t* hspi)
{
    STM32SingleWireSerial::_complete((uint32_t)hspi->handle.Instance, SWS_EVT_ERROR);
}

/*
#define DEFIRQ(nm, id) \
    extern "C" void nm() { STM32SingleWireSerial::_complete(id, 0); }

DEFIRQ(USART1_IRQHandler, USART1_BASE)
DEFIRQ(USART2_IRQHandler, USART2_BASE)
#ifdef USART6_BASE
DEFIRQ(USART6_IRQHandler, USART6_BASE)
#endif
*/

void STM32SingleWireSerial::configureRxInterrupt(int enable) {}

STM32SingleWireSerial::STM32SingleWireSerial(Pin& p) : SingleWireSerial(p)
{
    // ZERO(serial);

    // UART_HandleTypeDef* uart = &(serial.handle);
    // // only the TX pin is operable in half-duplex mode
    // uart->Instance = (USART_TypeDef*)pinmap_peripheral((PinName)p.name, PinMap_UART_TX);

    // enable_clock((uint32_t)uart->Instance);

    // // set some reasonable defaults
    // uart->Init.BaudRate     = 115200;
    // uart->Init.WordLength   = UART_WORDLENGTH_8B;
    // uart->Init.StopBits     = UART_STOPBITS_1;
    // uart->Init.Parity       = UART_PARITY_NONE;
    // uart->Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    // uart->Init.Mode         = UART_MODE_RX;
    // uart->Init.OverSampling = UART_OVERSAMPLING_16;

    serial.pin_tx = (PinName)p.name;

    stm32_uart_init_with_mode(&serial, 115200, UART_WORDLENGTH_8B, UART_PARITY_NONE, UART_STOPBITS_1, UART_MODE_RX);

    serial.rx_callback  = rxCpltCallback;
    serial.tx_callback  = txCpltCallback;
    serial.err_callback = errorCallback;

    for (unsigned i = 0; i < ARRAY_SIZE(instances); ++i) {
        if (instances[i] == NULL) {
            instances[i] = this;
            break;
        }
    }

    status = 0;
}

int STM32SingleWireSerial::setBaud(uint32_t baud)
{
    serial.handle.Init.BaudRate = baud;
    return DEVICE_OK;
}

uint32_t STM32SingleWireSerial::getBaud()
{
    return serial.handle.Init.BaudRate;
}

int STM32SingleWireSerial::putc(char c)
{
    return send((uint8_t*)&c, 1);
}

int STM32SingleWireSerial::getc()
{
    char c  = 0;
    int res = receive((uint8_t*)&c, 1);

    if (res == DEVICE_OK) return c;

    return res;
}

int STM32SingleWireSerial::configureTx(int enable)
{
    if (enable && !(status & TX_CONFIGURED)) {
        uint8_t pin = (uint8_t)p.name;
        STM_PIN_DATA(STM_MODE_OUTPUT_PP, GPIO_NOPULL, 0);
        pin_function((PinName)pin, pinmap_function((PinName)pin, PinMap_UART_TX));
        serial.handle.Init.Mode = UART_MODE_TX;
        HAL_HalfDuplex_Init(&serial.handle);
        status |= TX_CONFIGURED;
    }
    else if (status & TX_CONFIGURED) {
        HAL_UART_DeInit(&serial.handle);
        status &= ~TX_CONFIGURED;
    }

    return DEVICE_OK;
}

int STM32SingleWireSerial::configureRx(int enable)
{
    if (enable && !(status & RX_CONFIGURED)) {
        uint8_t pin = (uint8_t)p.name;
        pin_function((PinName)pin, pinmap_function((PinName)pin, PinMap_UART_TX));
        STM_PIN_DATA(STM_MODE_OUTPUT_PP, GPIO_NOPULL, 0);
        // 5 us
        serial.handle.Init.Mode = UART_MODE_RX;

        HAL_HalfDuplex_Init(&serial.handle);
        // additional 9 us
        status |= RX_CONFIGURED;
    }
    else if (status & RX_CONFIGURED) {
        HAL_UART_DeInit(&serial.handle);
        status &= ~RX_CONFIGURED;
    }

    return DEVICE_OK;
}

int STM32SingleWireSerial::setMode(SingleWireMode sw)
{
    if (sw == SingleWireRx) {
        configureTx(0);
        configureRx(1);
    }
    else if (sw == SingleWireTx) {
        configureRx(0);
        configureTx(1);
    }
    else {
        configureTx(0);
        configureRx(0);
    }

    return DEVICE_OK;
}

int STM32SingleWireSerial::send(uint8_t* data, int len)
{
    if (!(status & TX_CONFIGURED)) setMode(SingleWireTx);

    int res = HAL_UART_Transmit(&serial.handle, data, len, 3);

    if (res == HAL_OK) return DEVICE_OK;

    return DEVICE_CANCELLED;
}

int STM32SingleWireSerial::receive(uint8_t* data, int len)
{
    if (!(status & RX_CONFIGURED)) setMode(SingleWireRx);

    int res = HAL_UART_Receive(&serial.handle, data, len, 3);

    if (res == HAL_OK) return DEVICE_OK;

    return DEVICE_CANCELLED;
}

int STM32SingleWireSerial::getBytesReceived()
{
    if (!(status & RX_CONFIGURED)) return DEVICE_INVALID_PARAMETER;

    return serial.handle.Instance->RDR;
}

int STM32SingleWireSerial::getBytesTransmitted()
{
    if (!(status & TX_CONFIGURED)) return DEVICE_INVALID_PARAMETER;

    return serial.handle.Instance->TDR;
}

int STM32SingleWireSerial::sendBreak()
{
    if (!(status & TX_CONFIGURED)) return DEVICE_INVALID_PARAMETER;

    HAL_LIN_SendBreak(&serial.handle);
    return DEVICE_OK;
}