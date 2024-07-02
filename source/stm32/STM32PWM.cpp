#include "STM32PWM.h"

#include <cmath>

#include "PeripheralPins.h"

using namespace codal;

STM32PWM::STM32PWM(PinName pin, uint32_t frequency)
    : isStart{false}, pinName{pin}, timFunction{pinmap_find_function(pinName, PinMap_TIM)}, frequency{frequency}
{
    init();
}

void STM32PWM::init()
{
    GPIO_InitTypeDef GPIO_InitStruct;
    uint32_t fct = pinmap_find_function(pinName, PinMap_TIM);

    memset(&htim, 0, sizeof(htim));
    memset(&sConfigOC, 0, sizeof(sConfigOC));
    memset(&sMasterConfig, 0, sizeof(sMasterConfig));
    memset(&GPIO_InitStruct, 0, sizeof(GPIO_InitStruct));

    htim.Instance               = (TIM_TypeDef*)pinmap_peripheral(pinName, PinMap_TIM);
    htim.Init.Prescaler         = 0;
    htim.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim.Init.Period            = 65535;
    htim.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    if (HAL_TIM_PWM_Init(&htim) != HAL_OK) {
        Error_Handler();
    }

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim, &sMasterConfig) != HAL_OK) {
        Error_Handler();
    }

    sConfigOC.OCMode     = TIM_OCMODE_PWM1;
    sConfigOC.Pulse      = 30000;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    if (HAL_TIM_PWM_ConfigChannel(&htim, &sConfigOC, getTimChannel(STM_PIN_CHANNEL(timFunction))) != HAL_OK) {
        Error_Handler();
    }

    // ===== HAL_TIM_MspPostInit(pin, &htim); =====

    enableRccClock(get_GPIO_Port(STM_PORT(pinName)));

    GPIO_InitStruct.Pin       = STM_GPIO_PIN(pinName);
    GPIO_InitStruct.Mode      = STM_PIN_FUNCTION(fct);
    GPIO_InitStruct.Pull      = STM_PIN_PUPD(fct);
    GPIO_InitStruct.Speed     = STM_PIN_SPEED(fct);
    GPIO_InitStruct.Alternate = STM_PIN_AFNUM(fct);
    HAL_GPIO_Init(get_GPIO_Port(STM_PORT(pinName)), &GPIO_InitStruct);

    setFrequency(frequency);
}

void STM32PWM::setFrequency(uint32_t freq)
{
    if (freq == 0) freq = 1;

    frequency          = freq;
    uint32_t prescaler = (HAL_RCC_GetHCLKFreq() / ((uint32_t)65536 * frequency)) + 1;
    uint32_t period    = (HAL_RCC_GetHCLKFreq() / ((prescaler + 1) * frequency)) - 1;

    if (period < 255)
        period = 255;
    else if (period > 65535)
        period = 65535;

    htim.Init.Prescaler = prescaler;
    htim.Init.Period    = period;

    if (isStart) restart();
}

void STM32PWM::setDutyCyle(uint8_t percent)
{
    if (percent > 100) percent = 100;

    sConfigOC.Pulse = floor((float)htim.Init.Period * ((float)percent / 100.0f));

    if (isStart) restart();
}

void STM32PWM::setDutyCycleFromResolution(uint32_t value, uint8_t resolution)
{
    sConfigOC.Pulse = map(value, 0, (uint32_t)1 << resolution, 0, htim.Init.Period);

    if (isStart) restart();
}

void STM32PWM::setDutyCycleFromUs(uint32_t us)
{
    uint32_t usMax  = (uint32_t)1000000 / frequency;
    sConfigOC.Pulse = map(us, 0, usMax, 0, htim.Init.Period);

    if (isStart) restart();
}

void STM32PWM::start()
{
    if (isStart) return;

    if (HAL_TIM_PWM_Init(&htim) != HAL_OK) {
        Error_Handler();
    }

    if (HAL_TIM_PWM_ConfigChannel(&htim, &sConfigOC, getTimChannel(STM_PIN_CHANNEL(timFunction))) != HAL_OK) {
        Error_Handler();
    }

    HAL_TIM_PWM_Start(&htim, getTimChannel(STM_PIN_CHANNEL(timFunction)));

    isStart = true;
}

void STM32PWM::stop()
{
    if (!isStart) return;

    HAL_TIM_PWM_Stop(&htim, getTimChannel(STM_PIN_CHANNEL(timFunction)));

    isStart = false;
}

uint32_t STM32PWM::getTimChannel(uint32_t channel)
{
    switch (channel) {
#if defined(TIM_CHANNEL_1)
        case 1:
            return TIM_CHANNEL_1;
#endif

#if defined(TIM_CHANNEL_2)
        case 2:
            return TIM_CHANNEL_2;
#endif

#if defined(TIM_CHANNEL_3)
        case 3:
            return TIM_CHANNEL_3;
#endif

#if defined(TIM_CHANNEL_4)
        case 4:
            return TIM_CHANNEL_4;
#endif

#if defined(TIM_CHANNEL_5)
        case 5:
            return TIM_CHANNEL_5;
#endif

#if defined(TIM_CHANNEL_6)
        case 6:
            return TIM_CHANNEL_6;
#endif

        default:
            return TIM_CHANNEL_ALL;
    }
}

void STM32PWM::enableRccClock(GPIO_TypeDef* port)
{
#if defined GPIOA_BASE
    if (port == GPIOPort[PortA]) {
        __HAL_RCC_GPIOA_CLK_ENABLE();
    }
#endif

#if defined GPIOB_BASE
    if (port == GPIOPort[PortB]) {
        __HAL_RCC_GPIOB_CLK_ENABLE();
    }
#endif

#if defined GPIOC_BASE
    if (port == GPIOPort[PortC]) {
        __HAL_RCC_GPIOC_CLK_ENABLE();
    }
#endif

#if defined GPIOD_BASE
    if (port == GPIOPort[PortD]) {
        __HAL_RCC_GPIOD_CLK_ENABLE();
    }
#endif

#if defined GPIOE_BASE
    if (port == GPIOPort[PortE]) {
        __HAL_RCC_GPIOE_CLK_ENABLE();
    }
#endif

#if defined GPIOF_BASE
    if (port == GPIOPort[PortF]) {
        __HAL_RCC_GPIOF_CLK_ENABLE();
    }
#endif

#if defined GPIOG_BASE
    if (port == GPIOPort[PortG]) {
        __HAL_RCC_GPIOG_CLK_ENABLE();
    }
#endif

#if defined GPIOH_BASE
    if (port == GPIOPort[PortH]) {
        __HAL_RCC_GPIOH_CLK_ENABLE();
    }
#endif

#if defined GPIOI_BASE
    if (port == GPIOPort[PortI]) {
        __HAL_RCC_GPIOI_CLK_ENABLE();
    }
#endif

#if defined GPIOJ_BASE
    if (port == GPIOPort[PortJ]) {
        __HAL_RCC_GPIOJ_CLK_ENABLE();
    }
#endif

#if defined GPIOK_BASE
    if (port == GPIOPort[PortK]) {
        __HAL_RCC_GPIOK_CLK_ENABLE();
    }
#endif

#if defined GPIOZ_BASE
    if (port == GPIOPort[PortZ]) {
        __HAL_RCC_GPIOZ_CLK_ENABLE();
    }
#endif
}

void STM32PWM::restart()
{
    isStart = false;
    start();
}

uint32_t STM32PWM::map(uint32_t x, uint32_t in_min, uint32_t in_max, uint32_t out_min, uint32_t out_max)
{
    if (x > in_max) return out_max;
    if (x < in_min) return out_min;

    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
