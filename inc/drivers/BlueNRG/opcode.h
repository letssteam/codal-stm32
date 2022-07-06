#pragma once

#include <cstdint>

/*  OGF VALUE  */
enum class OGFValue : uint8_t {
    OGF_LINK_CONTROL        = 0x01,
    OGF_LINK_POLICY         = 0x02,
    OGF_CONTROLLER_BASEBAND = 0x03,
    OGF_INFORMATIONAL       = 0x04,
    OGF_STATUS              = 0x05,
    OGF_TESTING             = 0x06,
    OGF_LE_ONLY             = 0x08,
    OGF_VENDOR_SPEC         = 0x3F,
};

constexpr uint16_t makeOpCode(OGFValue ogf, uint16_t ocf)
{
    return ((uint16_t)ogf << 10) | ocf;
}

enum class OpCodeCommand : uint16_t {

    // BT OpCode
    READ_BD_ADDR = makeOpCode(OGFValue::OGF_INFORMATIONAL, 0x0009),

    SET_EVENT_MASK   = makeOpCode(OGFValue::OGF_CONTROLLER_BASEBAND, 0x0001),
    RESET            = makeOpCode(OGFValue::OGF_CONTROLLER_BASEBAND, 0x0003),
    WRITE_LOCAL_NAME = makeOpCode(OGFValue::OGF_CONTROLLER_BASEBAND, 0x0013),

    // LE OpCode
    LE_SET_EVENT_MASK     = makeOpCode(OGFValue::OGF_LE_ONLY, 0x0001),
    LE_SET_ADV_PARAM      = makeOpCode(OGFValue::OGF_LE_ONLY, 0x0006),
    LE_READ_TX_POWER      = makeOpCode(OGFValue::OGF_LE_ONLY, 0x0007),
    LE_SET_ADV_DATA       = makeOpCode(OGFValue::OGF_LE_ONLY, 0x0008),
    LE_SET_SCAN_RESP_DATA = makeOpCode(OGFValue::OGF_LE_ONLY, 0x0009),
    LE_SET_ADV_ENABLE     = makeOpCode(OGFValue::OGF_LE_ONLY, 0x000A),
    LE_RAND               = makeOpCode(OGFValue::OGF_LE_ONLY, 0x0018),
};