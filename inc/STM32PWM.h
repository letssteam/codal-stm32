#pragma once

#include "PinNames.h"
#include "pinmap.h"

namespace codal {
class STM32PWM {
  public:
    STM32PWM(PinName pin, uint32_t frequency);
    ~STM32PWM() {}

    void init();
    void setFrequency(uint32_t freq);
    void setDutyCyle(uint8_t percent);
    void setDutyCycleFromResolution(uint32_t value, uint8_t resolution = 10);
    void setDutyCycleFromUs(uint32_t us);
    void start();
    void stop();

  private:
    bool isStart;
    PinName pinName;
    uint32_t timFunction;
    uint32_t frequency;

    TIM_OC_InitTypeDef sConfigOC;
    TIM_MasterConfigTypeDef sMasterConfig;
    TIM_HandleTypeDef htim;

    uint32_t getTimChannel(uint32_t channel);
    void enableRccClock(GPIO_TypeDef* port);
    void restart();
    uint32_t map(uint32_t x, uint32_t in_min, uint32_t in_max, uint32_t out_min, uint32_t out_max);
};
}  // namespace codal