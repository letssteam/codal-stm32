#pragma once

#include "STM32SingleWireSerial.h"

extern "C" {
// #include "jacscript.h"
#include "jd_client.h"
#include "jd_drivers.h"
#include "services/interfaces/jd_disp.h"
#include "services/jd_services.h"
}

namespace codal {

class STM32Jacdac {
  public:
    STM32Jacdac();
};

}  // namespace codal