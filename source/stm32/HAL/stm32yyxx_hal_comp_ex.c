/* HAL raised several warnings, ignore them */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

#ifdef STM32L0xx
    #include "stm32l0xx_hal_comp_ex.c"
#endif
#pragma GCC diagnostic pop
