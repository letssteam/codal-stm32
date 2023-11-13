#pragma once

#include "DMASingleWireSerial.h"
#include "STM32Pin.h"

namespace codal {

class STM32DmaSingleWireSerial : DMASingleWireSerial {
  public:
    STM32DmaSingleWireSerial(STM32Pin& pin);
};

}  // namespace codal