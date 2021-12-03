#pragma once

// #define MYNEWT_VAL_BLE_MESH_MODEL_KEY_COUNT   0
// #define MYNEWT_VAL_BLE_MESH_MODEL_GROUP_COUNT 0

#include "NimBLEDevice.h"

namespace codal {

class STM32BLEAdvertising {
  public:
    STM32BLEAdvertising(codal::NimBLEDevice& device) : device(device) { device.init(); }
    ~STM32BLEAdvertising(){};

  private:
    codal::NimBLEDevice& device;
};

}  // namespace codal