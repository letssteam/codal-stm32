#pragma once

#include <array>
#include <cstdint>
#include <deque>
#include <list>

#include "BLEAdvertisingReport.h"
#include "EventMaskBuilder.h"
#include "LeEventMaskBuilder.h"
#include "data_format.h"
#include "le_advertising_parameters.h"
#include "opcode.h"
#include "result_types.h"

constexpr uint8_t ADVERTISING_CHANNEL_37 = 0x01;
constexpr uint8_t ADVERTISING_CHANNEL_38 = 0x02;
constexpr uint8_t ADVERTISING_CHANNEL_39 = 0x04;

class HCI {
  public:
    HCI() : isDebug(false), isPollLocked(false) {}
    virtual ~HCI() {}

    void enableDebug() { isDebug = true; }
    void disableDebug() { isDebug = false; }

    virtual bool init()          = 0;
    virtual void resetHardware() = 0;

    void poll();

    /**
     * @brief The HCI_Reset command will reset the Controller and the Link Manager on the BR/EDR Controller or the Link
      Layer on an LE Controller. If the Controller supports both BR/EDR and LE then the HCI_Reset command shall reset
     the Link Manager, Baseband and Link Layer. The HCI_Reset command shall not affect the used HCI transport layer
     since the HCI transport layers may have reset mechanisms of their own. After the reset is completed, the current
     operational state will be lost, the Controller will enter standby mode and the Controller will automatically revert
     to the default values for the parameters for which default values are defined in the specification.

      Note: The HCI_Reset command will not necessarily perform a hardware reset. This is implementation defined. The
     Host shall not send additional HCI commands before the

      HCI_Command_Complete event related to the HCI_Reset command has been received.
     *
     * @return true if succes, false otherwise
     */
    bool resetSoftware();

    /**
     * @brief On a BR/EDR Controller, this command reads the Bluetooth Controller address (BD_ADDR). (See [Vol 2] Part
     * B, Section 1.2 and [Vol 3] Part C, Section 3.2.1). On an LE Controller, this command shall read the Public Device
     * Address as defined in [Vol 6] Part B, Section 1.3. If this Controller does not have a Public Device Address, the
     * value 0x000000000000 shall be returned.
     *
     * @return std::array<uint32_t, 2>
     */
    std::array<uint32_t, 2> readBD_ADDR();

    /**
     * @brief The HCI_Set_Event_Mask command is used to control which events are generated by the HCI for the Host.
     * If the bit in the Event_Mask is set to a one, then the event associated with that bit will be enabled. For an
     * LE Controller, the “LE Meta event” bit in the event_Mask shall enable or disable all LE events in the LE Meta
     * event (see Section 7.7.65). The event mask allows the Host to control how much it is interrupted.
     *
     * @param msbMask the first 32 bits (MSB) from the 64-bits mask
     * @param lsbMask the last 32 bits (LSB) from the 64-bits mask
     * @return true if succes, false otherwise
     */
    bool setEventMask(uint32_t msbMask, uint32_t lsbMask);

    /**
     * @brief The HCI_LE_Set_Event_Mask command is used to control which LE events are generated by the HCI for the
     * Host. If the bit in the LE_Event_Mask is set to a one, then the event associated with that bit will be enabled.
     * The event mask allows the Host to control which events will interrupt it.
     *
     * @param msbMask the first 32 bits (MSB) from the 64-bits mask
     * @param lsbMask the last 32 bits (LSB) from the 64-bits mask
     * @return true if succes, false otherwise
     */
    bool leSetEventMask(uint32_t msbMask, uint32_t lsbMask);

    /**
     * @brief This command is used to read the maximum size of the data portion of ACL data packets and isochronous data
     * packets sent from the Host to the Control- ler. The Host shall segment the data transmitted to the Controller
     * according to these values so that the HCI ACL Data packets and HCI ISO Data packets will contain data up to this
     * size. The HCI_LE_Read_Buffer_Size command also returns the total number of HCI LE ACL Data packets and
     * isochronous data packets that can be stored in the data buffers of the Controller. The HCI_LE_- Read_Buffer_Size
     * command shall be issued by the Host before it sends any data to an LE Controller (see Section 4.1.1). If the
     * Controller supports HCI ISO Data packets, it shall return non-zero values for the ISO_Data_Packet_Length and
     * Total_Num_ISO_Data_Packets parameters.
     *
     * @return LeReadBufferSizeResult
     */
    LeReadBufferSizeResult leReadBufferSize();

    /**
     * @brief The HCI_LE_Set_Advertising_Parameters command is used by the Host to set the advertising parameters.
     *
     * @param advertisingIntervalMin Minimum advertising interval for undirected and low duty cycle directed
     * advertising. (Time Range: 20 ms to 10240 ms (10.24 s) )
     * @param advertisingIntervalMax Maximum advertising interval for undirected and low duty cycle directed
     * advertising. (Time Range: 20 ms to 10240 ms (10.24 s) )
     * @param advertisingType is used to determine the packet type that is used for advertising when advertising is
     * enabled.
     * @param ownAddressType parameter indicates the type of address being used in the advertising packets.
     * @param peerAddressType contains the Peer’s Identity Type
     * @param peerAddress Public Device Address, Random Device Address, Public Identity Address, or Random (static)
     * Identity Address of the device to be connected
     * @param advertisingChannelMap is a bit field that indicates the advertising channel indices that shall be used
     * when transmitting advertising packets. At least one channel bit shall be set
     * @param advertisingFilterPolicy parameter shall be ignored when directed advertising is enabled
     * @return true if succes, false otherwise
     */
    bool leSetAdvertisingParameters(
        float advertisingIntervalMin = 1280, float advertisingIntervalMax = 1280,
        AdvertisingType advertisingType            = AdvertisingType::ADV_IND,
        OwnAddressType ownAddressType              = OwnAddressType::PUBLIC,
        AdvertisingPeerAddressType peerAddressType = AdvertisingPeerAddressType::PUBLIC,
        uint64_t peerAddress                       = 0x0000000000000000,
        uint8_t advertisingChannelMap = ADVERTISING_CHANNEL_37 | ADVERTISING_CHANNEL_38 | ADVERTISING_CHANNEL_39,
        AdvertisingFilterPolicy advertisingFilterPolicy = AdvertisingFilterPolicy::ALL_DEVICE);

    /**
     * @brief The HCI_LE_Read_Advertising_Physical_Channel_Tx_Power command is used by the Host to read the transmit
     * power level used for LE advertising physical channel packets
     *
     * @return int8_t Range: -127 to 20 (dBm). Accuracy: ±4 dB
     */
    int8_t leReadAdvertisingPhysicalChannelTxPower();

    /**
     * @brief The HCI_LE_Set_Advertising_Data command is used to set the data used in advertising packets that have a
     * data field.
     *
     * @param significantSize The number of significant octets in the Advertising_Data.
     * @param data 31 octets of Scan_Response_Data formatted as defined in [Vol 3] PartC, Section 11. All octets zero
     * (default)
     * @return true if succes, false otherwise
     */
    bool leSetAdvertisingData(uint8_t significantSize, uint8_t* data);

    /**
     * @brief This command is used to provide data used in Scanning Packets that have a data field.
     *
     * @param significantSize The number of significant octets in the Scan_Response_Data.
     * @param data 31 octets of Scan_Response_Data formatted as defined in [Vol 3] PartC, Section 11. All octets zero
     * (default)
     * @return true if succes, false otherwise
     */
    bool leSetScanResponseData(uint8_t significantSize, uint8_t* data);

    /**
     * @brief The HCI_LE_Set_Advertising_Enable command is used to request the Controller to start or stop advertising.
     *
     * @param enable true to enable advertising, false otherwise.
     * @return true if succes, false otherwise
     */
    bool leSetAdvertisingEnable(bool enable);

    /**
     * @brief The HCI_LE_Set_Scan_Parameters command is used to set the scan parameters.
     *
     * @param scanType controls the type of scan to perform (active or passive)
     * @param scanInterval This is defined as the time interval from when the Controller started its last LE scan until
     * it begins the subsequent LE scan. (Time Range: 2.5 ms to 10240 ms (10.24 s) )
     * @param scanWindowThe duration of the LE scan. LE_Scan_Window shall be less than or equal to LE_Scan_Interval
     * (Time Range: 2.5 ms to 10240 ms (10.24 s) )
     * @param addressType indicates the type of address being used in the scan request packets.
     * @param filterPolicy
     * @return true if succes, false otherwise
     */
    bool leSetScanParameters(ScanType scanType = ScanType::PASSIVE, float scanInterval = 10.0f,
                             float scanWindow = 10.0f, OwnAddressType addressType = OwnAddressType::PUBLIC,
                             ScanningFilterPolicy filterPolicy = ScanningFilterPolicy::BASIC_UNFILTERED);

    /**
     * @brief The HCI_LE_Set_Scan_Enable command is used to start and stop scanning for legacy PDUs (but not extended
     * PDUs, even if the device supports extended advertising). Scanning is used to discover advertising devices nearby.
     *
     * @param enable true to enable scanning, false otherwise.
     * @param filterDuplicate controls whether the Link Layer should filter out duplicate advertising reports (set to
     * true) to the Host, or if the Link Layer should generate advertising reports for each packet received (set to
     * false)
     * @return true if succes, false otherwise
     */
    bool leSetScanEnable(bool enable, bool filterDuplicate = false);

    /**
     * @brief The HCI_LE_Set_Random_Address command is used by the Host to set the LE Random Device Address in the
     * Controller (see [Vol 6] Part B, Section 1.3).
     *
     * @param address the address array of 6 uint8_t
     * @return true if succes, false otherwise
     */
    bool leSetRandomAddress(uint8_t* address);

    /**
     * @brief The HCI_LE_Rand command is used to request the Controller to generate 8 octets of random data to be sent
     * to the Host.
     *
     * @return std::array<uint32_t, 2>
     */
    std::array<uint32_t, 2> leRand();

    /**
     * @brief Get the number of available advertising report
     *
     * @return uint8_t
     */
    uint8_t availableAdvReport() { return advertisingReports.size(); }

    /**
     * @brief Return all the report, and remove then form the HCI
     *
     * @return std::deque<BLEAdvertisingReport>
     */
    std::deque<BLEAdvertisingReport> getAdvReports()
    {
        auto reports = advertisingReports;
        advertisingReports.clear();
        return reports;
    }

    bool isDebugEnable() { return isDebug; }

  protected:
    bool isDebug;
    std::list<EventPacket> eventPackets;
    std::list<AsyncDataPacket> asyncDataPackets;
    std::deque<BLEAdvertisingReport> advertisingReports;

    virtual std::vector<uint8_t> sendCommand(OpCodeCommand command, uint8_t nbArgs, const uint8_t* args) = 0;
    virtual std::vector<uint8_t> sendCommand(OpCodeCommand command) { return sendCommand(command, 0, NULL); }
    virtual void readBytes(uint8_t* data, uint8_t size) = 0;
    virtual uint8_t available()                         = 0;

    void handleLeEvent(EventPacket pkt);

  private:
    bool isPollLocked;

    void cleanPackets();
};