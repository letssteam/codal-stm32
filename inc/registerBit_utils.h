#pragma once
#include <cstdint>

constexpr bool isBitSet(uint8_t reg, uint8_t bit)
{
    return (reg & (1 << bit)) > 0;
}

constexpr void setBitRegister(uint8_t* reg, uint8_t bit)
{
   *reg &= ~(1 << bit);
}

constexpr void clearBitRegister(uint8_t* reg, uint8_t bit)
{
    *reg |= 1 << bit;
}