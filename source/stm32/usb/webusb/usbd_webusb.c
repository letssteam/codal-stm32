/**
 ******************************************************************************
 * @file    usbd_cdc.c
 * @author  MCD Application Team
 * @brief   This file provides the high layer firmware functions to manage the
 *          following functionalities of the USB CDC Class:
 *           - Initialization and Configuration of high and low layer
 *           - Enumeration as CDC Device (and enumeration for each implemented memory interface)
 *           - OUT/IN data transfer
 *           - Command IN transfer (class requests management)
 *           - Error management
 *
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2015 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 *  @verbatim
 *
 *          ===================================================================
 *                                CDC Class Driver Description
 *          ===================================================================
 *           This driver manages the "Universal Serial Bus Class Definitions for Communications Devices
 *           Revision 1.2 November 16, 2007" and the sub-protocol specification of "Universal Serial Bus
 *           Communications Class Subclass Specification for PSTN Devices Revision 1.2 February 9, 2007"
 *           This driver implements the following aspects of the specification:
 *             - Device descriptor management
 *             - Configuration descriptor management
 *             - Enumeration as CDC device with 2 data endpoints (IN and OUT) and 1 command endpoint (IN)
 *             - Requests management (as described in section 6.2 in specification)
 *             - Abstract Control Model compliant
 *             - Union Functional collection (using 1 IN endpoint for control)
 *             - Data interface class
 *
 *           These aspects may be enriched or modified for a specific user application.
 *
 *            This driver doesn't implement the following aspects of the specification
 *            (but it is possible to manage these features with some modifications on this driver):
 *             - Any class-specific aspect relative to communication classes should be managed by user application.
 *             - All communication classes other than PSTN are not managed
 *
 *  @endverbatim
 *
 ******************************************************************************
 */

#ifdef USBCON
    #ifdef USBD_USE_CDC

        /* Includes ------------------------------------------------------------------*/
        #include "usbd_webusb.h"

        #include "usbd_ctlreq.h"

/** @addtogroup STM32_USB_DEVICE_LIBRARY
 * @{
 */

/** @defgroup USBD_WebUSB
 * @brief usbd core module
 * @{
 */

/** @defgroup USBD_WebUSB_Private_TypesDefinitions
 * @{
 */
/**
 * @}
 */

/** @defgroup USBD_WebUSB_Private_Defines
 * @{
 */
/**
 * @}
 */

/** @defgroup USBD_WebUSB_Private_Macros
 * @{
 */

/**
 * @}
 */

/** @defgroup USBD_WebUSB_Private_FunctionPrototypes
 * @{
 */

static uint8_t USBD_WebUSB_Init(USBD_HandleTypeDef* pdev, uint8_t cfgidx);
static uint8_t USBD_WebUSB_DeInit(USBD_HandleTypeDef* pdev, uint8_t cfgidx);
static uint8_t USBD_WebUSB_Setup(USBD_HandleTypeDef* pdev, USBD_SetupReqTypedef* req);
static uint8_t USBD_WebUSB_DataIn(USBD_HandleTypeDef* pdev, uint8_t epnum);
static uint8_t USBD_WebUSB_DataOut(USBD_HandleTypeDef* pdev, uint8_t epnum);
static uint8_t USBD_WebUSB_EP0_RxReady(USBD_HandleTypeDef* pdev);
        #ifndef USE_USBD_COMPOSITE
static uint8_t* USBD_WebUSB_GetFSCfgDesc(uint16_t* length);
static uint8_t* USBD_WebUSB_GetHSCfgDesc(uint16_t* length);
static uint8_t* USBD_WebUSB_GetOtherSpeedCfgDesc(uint16_t* length);
uint8_t* USBD_WebUSB_GetDeviceQualifierDescriptor(uint16_t* length);
        #endif /* USE_USBD_COMPOSITE  */

        #ifndef USE_USBD_COMPOSITE
/* USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t USBD_WebUSB_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END = {
    USB_LEN_DEV_QUALIFIER_DESC, USB_DESC_TYPE_DEVICE_QUALIFIER, 0x00, 0x02, 0x00, 0x00, 0x00, 0x40, 0x01, 0x00,
};
        #endif /* USE_USBD_COMPOSITE  */
/**
 * @}
 */

/** @defgroup USBD_WebUSB_Private_Variables
 * @{
 */

/* Prevent dynamic allocation */
USBD_WebUSB_HandleTypeDef _hcdc;

/* CDC interface class callbacks structure */
USBD_ClassTypeDef USBD_WebUSB = {
    USBD_WebUSB_Init,
    USBD_WebUSB_DeInit,
    USBD_WebUSB_Setup,
    NULL, /* EP0_TxSent */
    USBD_WebUSB_EP0_RxReady,
    USBD_WebUSB_DataIn,
    USBD_WebUSB_DataOut,
    NULL,
    NULL,
    NULL,
        #ifdef USE_USBD_COMPOSITE
    NULL,
    NULL,
    NULL,
    NULL,
        #else
    USBD_WebUSB_GetHSCfgDesc,
    USBD_WebUSB_GetFSCfgDesc,
    USBD_WebUSB_GetOtherSpeedCfgDesc,
    USBD_WebUSB_GetDeviceQualifierDescriptor,
        #endif /* USE_USBD_COMPOSITE  */
};

        #ifndef USE_USBD_COMPOSITE
/* USB CDC device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_WebUSB_CfgHSDesc[USB_WebUSB_CONFIG_DESC_SIZ] __ALIGN_END = {
    /* Configuration Descriptor */
    0x09,                        /* bLength: Configuration Descriptor size */
    USB_DESC_TYPE_CONFIGURATION, /* bDescriptorType: Configuration */
    USB_WebUSB_CONFIG_DESC_SIZ,  /* wTotalLength:no of returned bytes */
    0x00, 0x02,                  /* bNumInterfaces: 2 interfaces */
    0x01,                        /* bConfigurationValue: Configuration value */
    0x00,                        /* iConfiguration: Index of string descriptor describing the configuration */
            #if (USBD_SELF_POWERED == 1U)
    0xC0, /* bmAttributes: Bus Powered according to user configuration */
            #else
    0x80, /* bmAttributes: Bus Powered according to user configuration */
            #endif
    USBD_MAX_POWER, /* MaxPower (mA) */

    /*---------------------------------------------------------------------------*/

    /* Interface Descriptor */
    0x09,                    /* bLength: Interface Descriptor size */
    USB_DESC_TYPE_INTERFACE, /* bDescriptorType: Interface */
    0x00,                    /* bInterfaceNumber: Number of Interface */
    0x00,                    /* bAlternateSetting: Alternate setting */
    0x01,                    /* bNumEndpoints: One endpoints used */
    0x02,                    /* bInterfaceClass: Communication Interface Class */
    0x02,                    /* bInterfaceSubClass: Abstract Control Model */
    0x00,                    /* bInterfaceProtocol: No specific protocol */
    0x00,                    /* iInterface */

    /* Header Functional Descriptor */
    0x05, /* bLength: Endpoint Descriptor size */
    0x24, /* bDescriptorType: CS_INTERFACE */
    0x00, /* bDescriptorSubtype: Header Func Desc */
    0x10, /* bcdCDC: spec release number */
    0x01,

    /* Call Management Functional Descriptor */
    0x05, /* bFunctionLength */
    0x24, /* bDescriptorType: CS_INTERFACE */
    0x01, /* bDescriptorSubtype: Call Management Func Desc */
    0x00, /* bmCapabilities: D0+D1 */
    0x01, /* bDataInterface: 1 */

    /* ACM Functional Descriptor */
    0x04, /* bFunctionLength */
    0x24, /* bDescriptorType: CS_INTERFACE */
    0x02, /* bDescriptorSubtype: Abstract Control Management desc */
    0x02, /* bmCapabilities */

    /* Union Functional Descriptor */
    0x05, /* bFunctionLength */
    0x24, /* bDescriptorType: CS_INTERFACE */
    0x06, /* bDescriptorSubtype: Union func desc */
    0x00, /* bMasterInterface: Communication class interface */
    0x01, /* bSlaveInterface0: Data Class Interface */

    /* Endpoint 2 Descriptor */
    0x07,                                             /* bLength: Endpoint Descriptor size */
    USB_DESC_TYPE_ENDPOINT,                           /* bDescriptorType: Endpoint */
    CDC_CMD_EP,                                       /* bEndpointAddress */
    0x03,                                             /* bmAttributes: Interrupt */
    LOBYTE(CDC_CMD_PACKET_SIZE),                      /* wMaxPacketSize */
    HIBYTE(CDC_CMD_PACKET_SIZE), WebUSB_HS_BINTERVAL, /* bInterval */
    /*---------------------------------------------------------------------------*/

    /* Data class interface descriptor */
    0x09,                    /* bLength: Endpoint Descriptor size */
    USB_DESC_TYPE_INTERFACE, /* bDescriptorType: */
    0x01,                    /* bInterfaceNumber: Number of Interface */
    0x00,                    /* bAlternateSetting: Alternate setting */
    0x02,                    /* bNumEndpoints: Two endpoints used */
    0x0A,                    /* bInterfaceClass: CDC */
    0x00,                    /* bInterfaceSubClass */
    0x00,                    /* bInterfaceProtocol */
    0x00,                    /* iInterface */

    /* Endpoint OUT Descriptor */
    0x07,                                      /* bLength: Endpoint Descriptor size */
    USB_DESC_TYPE_ENDPOINT,                    /* bDescriptorType: Endpoint */
    CDC_OUT_EP,                                /* bEndpointAddress */
    0x02,                                      /* bmAttributes: Bulk */
    LOBYTE(CDC_DATA_HS_MAX_PACKET_SIZE),       /* wMaxPacketSize */
    HIBYTE(CDC_DATA_HS_MAX_PACKET_SIZE), 0x00, /* bInterval: ignore for Bulk transfer */

    /* Endpoint IN Descriptor */
    0x07,                                      /* bLength: Endpoint Descriptor size */
    USB_DESC_TYPE_ENDPOINT,                    /* bDescriptorType: Endpoint */
    CDC_IN_EP,                                 /* bEndpointAddress */
    0x02,                                      /* bmAttributes: Bulk */
    LOBYTE(CDC_DATA_HS_MAX_PACKET_SIZE),       /* wMaxPacketSize */
    HIBYTE(CDC_DATA_HS_MAX_PACKET_SIZE), 0x00, /* bInterval: ignore for Bulk transfer */

    /* --- WEB USB --- */
    /* Interface Descriptor */
    0x09,                     // bLength
    USB_DESC_TYPE_INTERFACE,  // bDescriptorType : Interface
    0x03,                     // bInterfaceNumber
    0x00,                     // bAlternateSetting
    0x00,                     // bNumEndpoints
    0xFF,                     // bInterfaceClass
    0x03,                     // bInterfaceSubClass
    0x00,                     // bInterfaceProtocol
    0x0A,                     // bInterface

};

/* USB CDC device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_WebUSB_CfgFSDesc[USB_WebUSB_CONFIG_DESC_SIZ] __ALIGN_END = {
    /* Configuration Descriptor */
    0x09,                        /* bLength: Configuration Descriptor size */
    USB_DESC_TYPE_CONFIGURATION, /* bDescriptorType: Configuration */
    USB_WebUSB_CONFIG_DESC_SIZ,  /* wTotalLength: nb of returned bytes */
    0x00, 0x02,                  /* bNumInterfaces: 2 interfaces */
    0x01,                        /* bConfigurationValue: Configuration value */
    0x00,                        /* iConfiguration: Index of string descriptor
                                    describing the configuration */
            #if (USBD_SELF_POWERED == 1U)
    0xC0, /* bmAttributes: Bus Powered according to user configuration */
            #else
    0x80, /* bmAttributes: Bus Powered according to user configuration */
            #endif  /* USBD_SELF_POWERED */
    USBD_MAX_POWER, /* MaxPower (mA) */

    /*---------------------------------------------------------------------------*/

    /* Interface Descriptor */
    0x09,                    /* bLength: Interface Descriptor size */
    USB_DESC_TYPE_INTERFACE, /* bDescriptorType: Interface */
    /* Interface descriptor type */
    0x00, /* bInterfaceNumber: Number of Interface */
    0x00, /* bAlternateSetting: Alternate setting */
    0x01, /* bNumEndpoints: One endpoint used */
    0x02, /* bInterfaceClass: Communication Interface Class */
    0x02, /* bInterfaceSubClass: Abstract Control Model */
    0x00, /* bInterfaceProtocol: No specific protocol */
    0x00, /* iInterface */

    /* Header Functional Descriptor */
    0x05, /* bLength: Endpoint Descriptor size */
    0x24, /* bDescriptorType: CS_INTERFACE */
    0x00, /* bDescriptorSubtype: Header Func Desc */
    0x10, /* bcdCDC: spec release number */
    0x01,

    /* Call Management Functional Descriptor */
    0x05, /* bFunctionLength */
    0x24, /* bDescriptorType: CS_INTERFACE */
    0x01, /* bDescriptorSubtype: Call Management Func Desc */
    0x00, /* bmCapabilities: D0+D1 */
    0x01, /* bDataInterface */

    /* ACM Functional Descriptor */
    0x04, /* bFunctionLength */
    0x24, /* bDescriptorType: CS_INTERFACE */
    0x02, /* bDescriptorSubtype: Abstract Control Management desc */
    0x02, /* bmCapabilities */

    /* Union Functional Descriptor */
    0x05, /* bFunctionLength */
    0x24, /* bDescriptorType: CS_INTERFACE */
    0x06, /* bDescriptorSubtype: Union func desc */
    0x00, /* bMasterInterface: Communication class interface */
    0x01, /* bSlaveInterface0: Data Class Interface */

    /* Endpoint 2 Descriptor */
    0x07,                                             /* bLength: Endpoint Descriptor size */
    USB_DESC_TYPE_ENDPOINT,                           /* bDescriptorType: Endpoint */
    CDC_CMD_EP,                                       /* bEndpointAddress */
    0x03,                                             /* bmAttributes: Interrupt */
    LOBYTE(CDC_CMD_PACKET_SIZE),                      /* wMaxPacketSize */
    HIBYTE(CDC_CMD_PACKET_SIZE), WebUSB_FS_BINTERVAL, /* bInterval */
    /*---------------------------------------------------------------------------*/

    /* Data class interface descriptor */
    0x09,                    /* bLength: Endpoint Descriptor size */
    USB_DESC_TYPE_INTERFACE, /* bDescriptorType: */
    0x01,                    /* bInterfaceNumber: Number of Interface */
    0x00,                    /* bAlternateSetting: Alternate setting */
    0x02,                    /* bNumEndpoints: Two endpoints used */
    0x0A,                    /* bInterfaceClass: CDC */
    0x00,                    /* bInterfaceSubClass */
    0x00,                    /* bInterfaceProtocol */
    0x00,                    /* iInterface */

    /* Endpoint OUT Descriptor */
    0x07,                                      /* bLength: Endpoint Descriptor size */
    USB_DESC_TYPE_ENDPOINT,                    /* bDescriptorType: Endpoint */
    CDC_OUT_EP,                                /* bEndpointAddress */
    0x02,                                      /* bmAttributes: Bulk */
    LOBYTE(CDC_DATA_FS_MAX_PACKET_SIZE),       /* wMaxPacketSize */
    HIBYTE(CDC_DATA_FS_MAX_PACKET_SIZE), 0x00, /* bInterval: ignore for Bulk transfer */
    /* Endpoint IN Descriptor */
    0x07,                                      /* bLength: Endpoint Descriptor size */
    USB_DESC_TYPE_ENDPOINT,                    /* bDescriptorType: Endpoint */
    CDC_IN_EP,                                 /* bEndpointAddress */
    0x02,                                      /* bmAttributes: Bulk */
    LOBYTE(CDC_DATA_FS_MAX_PACKET_SIZE),       /* wMaxPacketSize */
    HIBYTE(CDC_DATA_FS_MAX_PACKET_SIZE), 0x00, /* bInterval: ignore for Bulk transfer */

    /* --- WEB USB --- */
    /* Interface Descriptor */
    0x09,                     // bLength
    USB_DESC_TYPE_INTERFACE,  // bDescriptorType : Interface
    0x03,                     // bInterfaceNumber
    0x00,                     // bAlternateSetting
    0x00,                     // bNumEndpoints
    0xFF,                     // bInterfaceClass
    0x03,                     // bInterfaceSubClass
    0x00,                     // bInterfaceProtocol
    0x0A,                     // bInterface
};

__ALIGN_BEGIN static uint8_t USBD_WebUSB_OtherSpeedCfgDesc[USB_WebUSB_CONFIG_DESC_SIZ] __ALIGN_END = {
    0x09, /* bLength: Configuration Descriptor size */
    USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION, USB_WebUSB_CONFIG_DESC_SIZ, 0x00, 0x02, /* bNumInterfaces: 2 interfaces */
    0x01,                                                                            /* bConfigurationValue */
    0x04,                                                                            /* iConfiguration */
            #if (USBD_SELF_POWERED == 1U)
    0xC0, /* bmAttributes: Bus Powered according to user configuration */
            #else
    0x80, /* bmAttributes: Bus Powered according to user configuration */
            #endif
    USBD_MAX_POWER, /* MaxPower (mA) */

    /* Interface Descriptor */
    0x09,                    /* bLength: Interface Descriptor size */
    USB_DESC_TYPE_INTERFACE, /* bDescriptorType: Interface */
    /* Interface descriptor type */
    0x00, /* bInterfaceNumber: Number of Interface */
    0x00, /* bAlternateSetting: Alternate setting */
    0x01, /* bNumEndpoints: One endpoints used */
    0x02, /* bInterfaceClass: Communication Interface Class */
    0x02, /* bInterfaceSubClass: Abstract Control Model */
    0x00, /* bInterfaceProtocol: No specific protocol */
    0x00, /* iInterface */

    /* Header Functional Descriptor */
    0x05, /* bLength: Endpoint Descriptor size */
    0x24, /* bDescriptorType: CS_INTERFACE */
    0x00, /* bDescriptorSubtype: Header Func Desc */
    0x10, /* bcdCDC: spec release number */
    0x01,

    /* Call Management Functional Descriptor */
    0x05, /* bFunctionLength */
    0x24, /* bDescriptorType: CS_INTERFACE */
    0x01, /* bDescriptorSubtype: Call Management Func Desc */
    0x00, /* bmCapabilities */
    0x01, /* bDataInterface */

    /* ACM Functional Descriptor */
    0x04, /* bFunctionLength */
    0x24, /* bDescriptorType: CS_INTERFACE */
    0x02, /* bDescriptorSubtype: Abstract Control Management desc */
    0x02, /* bmCapabilities */

    /* Union Functional Descriptor */
    0x05, /* bFunctionLength */
    0x24, /* bDescriptorType: CS_INTERFACE */
    0x06, /* bDescriptorSubtype: Union func desc */
    0x00, /* bMasterInterface: Communication class interface */
    0x01, /* bSlaveInterface0: Data Class Interface */

    /* Endpoint 2 Descriptor */
    0x07,                                             /* bLength: Endpoint Descriptor size */
    USB_DESC_TYPE_ENDPOINT,                           /* bDescriptorType: Endpoint */
    CDC_CMD_EP,                                       /* bEndpointAddress */
    0x03,                                             /* bmAttributes: Interrupt */
    LOBYTE(CDC_CMD_PACKET_SIZE),                      /* wMaxPacketSize */
    HIBYTE(CDC_CMD_PACKET_SIZE), WebUSB_FS_BINTERVAL, /* bInterval */

    /*---------------------------------------------------------------------------*/

    /*Data class interface descriptor*/
    0x09,                    /* bLength: Endpoint Descriptor size */
    USB_DESC_TYPE_INTERFACE, /* bDescriptorType: */
    0x01,                    /* bInterfaceNumber: Number of Interface */
    0x00,                    /* bAlternateSetting: Alternate setting */
    0x02,                    /* bNumEndpoints: Two endpoints used */
    0x0A,                    /* bInterfaceClass: CDC */
    0x00,                    /* bInterfaceSubClass */
    0x00,                    /* bInterfaceProtocol */
    0x00,                    /* iInterface */

    /*Endpoint OUT Descriptor*/
    0x07,                   /* bLength: Endpoint Descriptor size */
    USB_DESC_TYPE_ENDPOINT, /* bDescriptorType: Endpoint */
    CDC_OUT_EP,             /* bEndpointAddress */
    0x02,                   /* bmAttributes: Bulk */
    0x40,                   /* wMaxPacketSize */
    0x00, 0x00,             /* bInterval: ignore for Bulk transfer */

    /*Endpoint IN Descriptor*/
    0x07,                   /* bLength: Endpoint Descriptor size */
    USB_DESC_TYPE_ENDPOINT, /* bDescriptorType: Endpoint */
    CDC_IN_EP,              /* bEndpointAddress */
    0x02,                   /* bmAttributes: Bulk */
    0x40,                   /* wMaxPacketSize */
    0x00, 0x00,             /* bInterval */

    /* --- WEB USB --- */
    /* Interface Descriptor */
    0x09,                     // bLength
    USB_DESC_TYPE_INTERFACE,  // bDescriptorType : Interface
    0x03,                     // bInterfaceNumber
    0x00,                     // bAlternateSetting
    0x00,                     // bNumEndpoints
    0xFF,                     // bInterfaceClass
    0x03,                     // bInterfaceSubClass
    0x00,                     // bInterfaceProtocol
    0x0A,                     // bInterface

};
        #endif /* USE_USBD_COMPOSITE  */

static uint8_t WebUSBInEpAdd  = CDC_IN_EP;
static uint8_t WebUSBOutEpAdd = CDC_OUT_EP;
static uint8_t WebUSBCmdEpAdd = CDC_CMD_EP;

/**
 * @}
 */

/** @defgroup USBD_WebUSB_Private_Functions
 * @{
 */

/**
 * @brief  USBD_WebUSB_Init
 *         Initialize the WebUSB interface
 * @param  pdev: device instance
 * @param  cfgidx: Configuration index
 * @retval status
 */
static uint8_t USBD_WebUSB_Init(USBD_HandleTypeDef* pdev, uint8_t cfgidx)
{
    UNUSED(cfgidx);
    USBD_WebUSB_HandleTypeDef* hcdc;

    // hcdc = (USBD_WebUSB_HandleTypeDef *)USBD_malloc(sizeof(USBD_WebUSB_HandleTypeDef));
    hcdc = &_hcdc;

    if (hcdc == NULL) {
        pdev->pClassDataCmsit[pdev->classId] = NULL;
        return (uint8_t)USBD_EMEM;
    }

    (void)USBD_memset(hcdc, 0, sizeof(USBD_WebUSB_HandleTypeDef));

    pdev->pClassDataCmsit[pdev->classId] = (void*)hcdc;
    pdev->pClassData                     = pdev->pClassDataCmsit[pdev->classId];

        #ifdef USE_USBD_COMPOSITE
    /* Get the Endpoints addresses allocated for this class instance */
    WebUSBInEpAdd  = USBD_CoreGetEPAdd(pdev, USBD_EP_IN, USBD_EP_TYPE_BULK, (uint8_t)pdev->classId);
    WebUSBOutEpAdd = USBD_CoreGetEPAdd(pdev, USBD_EP_OUT, USBD_EP_TYPE_BULK, (uint8_t)pdev->classId);
    WebUSBCmdEpAdd = USBD_CoreGetEPAdd(pdev, USBD_EP_IN, USBD_EP_TYPE_INTR, (uint8_t)pdev->classId);
        #endif /* USE_USBD_COMPOSITE */

    if (pdev->dev_speed == USBD_SPEED_HIGH) {
        /* Open EP IN */
        (void)USBD_LL_OpenEP(pdev, WebUSBInEpAdd, USBD_EP_TYPE_BULK, WebUSB_DATA_HS_IN_PACKET_SIZE);

        pdev->ep_in[WebUSBInEpAdd & 0xFU].is_used = 1U;

        /* Open EP OUT */
        (void)USBD_LL_OpenEP(pdev, WebUSBOutEpAdd, USBD_EP_TYPE_BULK, WebUSB_DATA_HS_OUT_PACKET_SIZE);

        pdev->ep_out[WebUSBOutEpAdd & 0xFU].is_used = 1U;

        /* Set bInterval for WebUSB CMD Endpoint */
        pdev->ep_in[WebUSBCmdEpAdd & 0xFU].bInterval = WebUSB_HS_BINTERVAL;
    }
    else {
        /* Open EP IN */
        (void)USBD_LL_OpenEP(pdev, WebUSBInEpAdd, USBD_EP_TYPE_BULK, WebUSB_DATA_FS_IN_PACKET_SIZE);

        pdev->ep_in[WebUSBInEpAdd & 0xFU].is_used = 1U;

        /* Open EP OUT */
        (void)USBD_LL_OpenEP(pdev, WebUSBOutEpAdd, USBD_EP_TYPE_BULK, WebUSB_DATA_FS_OUT_PACKET_SIZE);

        pdev->ep_out[WebUSBOutEpAdd & 0xFU].is_used = 1U;

        /* Set bInterval for CMD Endpoint */
        pdev->ep_in[WebUSBCmdEpAdd & 0xFU].bInterval = WebUSB_FS_BINTERVAL;
    }

    /* Open Command IN EP */
    (void)USBD_LL_OpenEP(pdev, WebUSBCmdEpAdd, USBD_EP_TYPE_INTR, CDC_CMD_PACKET_SIZE);
    pdev->ep_in[WebUSBCmdEpAdd & 0xFU].is_used = 1U;

    hcdc->RxBuffer                             = NULL;

    /* Init  physical Interface components */
    ((USBD_WebUSB_ItfTypeDef*)pdev->pUserData[pdev->classId])->Init();

    /* Init Xfer states */
    hcdc->TxState = 0U;
    hcdc->RxState = 0U;

    if (hcdc->RxBuffer == NULL) {
        return (uint8_t)USBD_EMEM;
    }

    if (pdev->dev_speed == USBD_SPEED_HIGH) {
        /* Prepare Out endpoint to receive next packet */
        (void)USBD_LL_PrepareReceive(pdev, WebUSBOutEpAdd, hcdc->RxBuffer, WebUSB_DATA_HS_OUT_PACKET_SIZE);
    }
    else {
        /* Prepare Out endpoint to receive next packet */
        (void)USBD_LL_PrepareReceive(pdev, WebUSBOutEpAdd, hcdc->RxBuffer, WebUSB_DATA_FS_OUT_PACKET_SIZE);
    }

    return (uint8_t)USBD_OK;
}

/**
 * @brief  USBD_WebUSB_DeInit
 *         DeInitialize the WebUSB layer
 * @param  pdev: device instance
 * @param  cfgidx: Configuration index
 * @retval status
 */
static uint8_t USBD_WebUSB_DeInit(USBD_HandleTypeDef* pdev, uint8_t cfgidx)
{
    UNUSED(cfgidx);

        #ifdef USE_USBD_COMPOSITE
    /* Get the Endpoints addresses allocated for this WebUSB class instance */
    WebUSBInEpAdd  = USBD_CoreGetEPAdd(pdev, USBD_EP_IN, USBD_EP_TYPE_BULK, (uint8_t)pdev->classId);
    WebUSBOutEpAdd = USBD_CoreGetEPAdd(pdev, USBD_EP_OUT, USBD_EP_TYPE_BULK, (uint8_t)pdev->classId);
    WebUSBCmdEpAdd = USBD_CoreGetEPAdd(pdev, USBD_EP_IN, USBD_EP_TYPE_INTR, (uint8_t)pdev->classId);
        #endif /* USE_USBD_COMPOSITE */

    /* Close EP IN */
    (void)USBD_LL_CloseEP(pdev, WebUSBInEpAdd);
    pdev->ep_in[WebUSBInEpAdd & 0xFU].is_used = 0U;

    /* Close EP OUT */
    (void)USBD_LL_CloseEP(pdev, WebUSBOutEpAdd);
    pdev->ep_out[WebUSBOutEpAdd & 0xFU].is_used = 0U;

    /* Close Command IN EP */
    (void)USBD_LL_CloseEP(pdev, WebUSBCmdEpAdd);
    pdev->ep_in[WebUSBCmdEpAdd & 0xFU].is_used   = 0U;
    pdev->ep_in[WebUSBCmdEpAdd & 0xFU].bInterval = 0U;

    /* DeInit  physical Interface components */
    if (pdev->pClassDataCmsit[pdev->classId] != NULL) {
        ((USBD_WebUSB_ItfTypeDef*)pdev->pUserData[pdev->classId])->DeInit();
        /* No need to free as hhid is no more dynamically allocated */
        // (void)USBD_free(pdev->pClassDataCmsit[pdev->classId]);
        pdev->pClassDataCmsit[pdev->classId] = NULL;
        pdev->pClassData                     = NULL;
    }

    return (uint8_t)USBD_OK;
}

/**
 * @brief  USBD_WebUSB_Setup
 *         Handle the WebUSB specific requests
 * @param  pdev: instance
 * @param  req: usb requests
 * @retval status
 */
static uint8_t USBD_WebUSB_Setup(USBD_HandleTypeDef* pdev, USBD_SetupReqTypedef* req)
{
    USBD_WebUSB_HandleTypeDef* hcdc = (USBD_WebUSB_HandleTypeDef*)pdev->pClassDataCmsit[pdev->classId];
    uint16_t len;
    uint8_t ifalt          = 0U;
    uint16_t status_info   = 0U;
    USBD_StatusTypeDef ret = USBD_OK;

    if (hcdc == NULL) {
        return (uint8_t)USBD_FAIL;
    }

    switch (req->bmRequest & USB_REQ_TYPE_MASK) {
        case USB_REQ_TYPE_CLASS:
            if (req->wLength != 0U) {
                if ((req->bmRequest & 0x80U) != 0U) {
                    ((USBD_WebUSB_ItfTypeDef*)pdev->pUserData[pdev->classId])
                        ->Control(req->bRequest, (uint8_t*)hcdc->data, req->wLength);

                    len = MIN(WebUSB_REQ_MAX_DATA_SIZE, req->wLength);
                    (void)USBD_CtlSendData(pdev, (uint8_t*)hcdc->data, len);
                }
                else {
                    hcdc->CmdOpCode = req->bRequest;
                    hcdc->CmdLength = (uint8_t)MIN(req->wLength, USB_MAX_EP0_SIZE);

                    (void)USBD_CtlPrepareRx(pdev, (uint8_t*)hcdc->data, hcdc->CmdLength);
                }
            }
            else {
                ((USBD_WebUSB_ItfTypeDef*)pdev->pUserData[pdev->classId])->Control(req->bRequest, (uint8_t*)req, 0U);
            }
            break;

        case USB_REQ_TYPE_STANDARD:
            switch (req->bRequest) {
                case USB_REQ_GET_STATUS:
                    if (pdev->dev_state == USBD_STATE_CONFIGURED) {
                        (void)USBD_CtlSendData(pdev, (uint8_t*)&status_info, 2U);
                    }
                    else {
                        USBD_CtlError(pdev, req);
                        ret = USBD_FAIL;
                    }
                    break;

                case USB_REQ_GET_INTERFACE:
                    if (pdev->dev_state == USBD_STATE_CONFIGURED) {
                        (void)USBD_CtlSendData(pdev, &ifalt, 1U);
                    }
                    else {
                        USBD_CtlError(pdev, req);
                        ret = USBD_FAIL;
                    }
                    break;

                case USB_REQ_SET_INTERFACE:
                    if (pdev->dev_state != USBD_STATE_CONFIGURED) {
                        USBD_CtlError(pdev, req);
                        ret = USBD_FAIL;
                    }
                    break;

                case USB_REQ_CLEAR_FEATURE:
                    break;

                default:
                    USBD_CtlError(pdev, req);
                    ret = USBD_FAIL;
                    break;
            }
            break;

        default:
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
            break;
    }

    return (uint8_t)ret;
}

/**
 * @brief  USBD_WebUSB_DataIn
 *         Data sent on non-control IN endpoint
 * @param  pdev: device instance
 * @param  epnum: endpoint number
 * @retval status
 */
static uint8_t USBD_WebUSB_DataIn(USBD_HandleTypeDef* pdev, uint8_t epnum)
{
    USBD_WebUSB_HandleTypeDef* hcdc;
    PCD_HandleTypeDef* hpcd = (PCD_HandleTypeDef*)pdev->pData;

    if (pdev->pClassDataCmsit[pdev->classId] == NULL) {
        return (uint8_t)USBD_FAIL;
    }

    hcdc = (USBD_WebUSB_HandleTypeDef*)pdev->pClassDataCmsit[pdev->classId];

    if ((pdev->ep_in[epnum & 0xFU].total_length > 0U) &&
        ((pdev->ep_in[epnum & 0xFU].total_length % hpcd->IN_ep[epnum & 0xFU].maxpacket) == 0U)) {
        /* Update the packet total length */
        pdev->ep_in[epnum & 0xFU].total_length = 0U;

        /* Send ZLP */
        (void)USBD_LL_Transmit(pdev, epnum, NULL, 0U);
    }
    else {
        hcdc->TxState = 0U;

        if (((USBD_WebUSB_ItfTypeDef*)pdev->pUserData[pdev->classId])->TransmitCplt != NULL) {
            ((USBD_WebUSB_ItfTypeDef*)pdev->pUserData[pdev->classId])
                ->TransmitCplt(hcdc->TxBuffer, &hcdc->TxLength, epnum);
        }
    }

    return (uint8_t)USBD_OK;
}

/**
 * @brief  USBD_WebUSB_DataOut
 *         Data received on non-control Out endpoint
 * @param  pdev: device instance
 * @param  epnum: endpoint number
 * @retval status
 */
static uint8_t USBD_WebUSB_DataOut(USBD_HandleTypeDef* pdev, uint8_t epnum)
{
    USBD_WebUSB_HandleTypeDef* hcdc = (USBD_WebUSB_HandleTypeDef*)pdev->pClassDataCmsit[pdev->classId];

    if (pdev->pClassDataCmsit[pdev->classId] == NULL) {
        return (uint8_t)USBD_FAIL;
    }

    /* Get the received data length */
    hcdc->RxLength = USBD_LL_GetRxDataSize(pdev, epnum);

    /* USB data will be immediately processed, this allow next USB traffic being
    NAKed till the end of the application Xfer */

    ((USBD_WebUSB_ItfTypeDef*)pdev->pUserData[pdev->classId])->Receive(hcdc->RxBuffer, &hcdc->RxLength);

    return (uint8_t)USBD_OK;
}

/**
 * @brief  USBD_WebUSB_EP0_RxReady
 *         Handle EP0 Rx Ready event
 * @param  pdev: device instance
 * @retval status
 */
static uint8_t USBD_WebUSB_EP0_RxReady(USBD_HandleTypeDef* pdev)
{
    USBD_WebUSB_HandleTypeDef* hcdc = (USBD_WebUSB_HandleTypeDef*)pdev->pClassDataCmsit[pdev->classId];

    if (hcdc == NULL) {
        return (uint8_t)USBD_FAIL;
    }

    if ((pdev->pUserData[pdev->classId] != NULL) && (hcdc->CmdOpCode != 0xFFU)) {
        ((USBD_WebUSB_ItfTypeDef*)pdev->pUserData[pdev->classId])
            ->Control(hcdc->CmdOpCode, (uint8_t*)hcdc->data, (uint16_t)hcdc->CmdLength);
        hcdc->CmdOpCode = 0xFFU;
    }

    return (uint8_t)USBD_OK;
}
        #ifndef USE_USBD_COMPOSITE
/**
 * @brief  USBD_WebUSB_GetFSCfgDesc
 *         Return configuration descriptor
 * @param  length : pointer data length
 * @retval pointer to descriptor buffer
 */
static uint8_t* USBD_WebUSB_GetFSCfgDesc(uint16_t* length)
{
    USBD_EpDescTypeDef* pEpCmdDesc = USBD_GetEpDesc(USBD_WebUSB_CfgFSDesc, CDC_CMD_EP);
    USBD_EpDescTypeDef* pEpOutDesc = USBD_GetEpDesc(USBD_WebUSB_CfgFSDesc, CDC_OUT_EP);
    USBD_EpDescTypeDef* pEpInDesc  = USBD_GetEpDesc(USBD_WebUSB_CfgFSDesc, CDC_IN_EP);

    if (pEpCmdDesc != NULL) {
        pEpCmdDesc->bInterval = WebUSB_FS_BINTERVAL;
    }

    if (pEpOutDesc != NULL) {
        pEpOutDesc->wMaxPacketSize = CDC_DATA_FS_MAX_PACKET_SIZE;
    }

    if (pEpInDesc != NULL) {
        pEpInDesc->wMaxPacketSize = CDC_DATA_FS_MAX_PACKET_SIZE;
    }

    *length = (uint16_t)sizeof(USBD_WebUSB_CfgFSDesc);

    return USBD_WebUSB_CfgFSDesc;
}

/**
 * @brief  USBD_WebUSB_GetHSCfgDesc
 *         Return configuration descriptor
 * @param  length : pointer data length
 * @retval pointer to descriptor buffer
 */
static uint8_t* USBD_WebUSB_GetHSCfgDesc(uint16_t* length)
{
    USBD_EpDescTypeDef* pEpCmdDesc = USBD_GetEpDesc(USBD_WebUSB_CfgHSDesc, CDC_CMD_EP);
    USBD_EpDescTypeDef* pEpOutDesc = USBD_GetEpDesc(USBD_WebUSB_CfgHSDesc, CDC_OUT_EP);
    USBD_EpDescTypeDef* pEpInDesc  = USBD_GetEpDesc(USBD_WebUSB_CfgHSDesc, CDC_IN_EP);

    if (pEpCmdDesc != NULL) {
        pEpCmdDesc->bInterval = WebUSB_HS_BINTERVAL;
    }

    if (pEpOutDesc != NULL) {
        pEpOutDesc->wMaxPacketSize = CDC_DATA_HS_MAX_PACKET_SIZE;
    }

    if (pEpInDesc != NULL) {
        pEpInDesc->wMaxPacketSize = CDC_DATA_HS_MAX_PACKET_SIZE;
    }

    *length = (uint16_t)sizeof(USBD_WebUSB_CfgHSDesc);

    return USBD_WebUSB_CfgHSDesc;
}

/**
 * @brief  USBD_WebUSB_GetOtherSpeedCfgDesc
 *         Return configuration descriptor
 * @param  length : pointer data length
 * @retval pointer to descriptor buffer
 */
static uint8_t* USBD_WebUSB_GetOtherSpeedCfgDesc(uint16_t* length)
{
    USBD_EpDescTypeDef* pEpCmdDesc = USBD_GetEpDesc(USBD_WebUSB_OtherSpeedCfgDesc, CDC_CMD_EP);
    USBD_EpDescTypeDef* pEpOutDesc = USBD_GetEpDesc(USBD_WebUSB_OtherSpeedCfgDesc, CDC_OUT_EP);
    USBD_EpDescTypeDef* pEpInDesc  = USBD_GetEpDesc(USBD_WebUSB_OtherSpeedCfgDesc, CDC_IN_EP);

    if (pEpCmdDesc != NULL) {
        pEpCmdDesc->bInterval = WebUSB_FS_BINTERVAL;
    }

    if (pEpOutDesc != NULL) {
        pEpOutDesc->wMaxPacketSize = CDC_DATA_FS_MAX_PACKET_SIZE;
    }

    if (pEpInDesc != NULL) {
        pEpInDesc->wMaxPacketSize = CDC_DATA_FS_MAX_PACKET_SIZE;
    }

    *length = (uint16_t)sizeof(USBD_WebUSB_OtherSpeedCfgDesc);

    return USBD_WebUSB_OtherSpeedCfgDesc;
}

/**
 * @brief  USBD_WebUSB_GetDeviceQualifierDescriptor
 *         return Device Qualifier descriptor
 * @param  length : pointer data length
 * @retval pointer to descriptor buffer
 */
uint8_t* USBD_WebUSB_GetDeviceQualifierDescriptor(uint16_t* length)
{
    *length = (uint16_t)sizeof(USBD_WebUSB_DeviceQualifierDesc);

    return USBD_WebUSB_DeviceQualifierDesc;
}
        #endif /* USE_USBD_COMPOSITE  */
/**
 * @brief  USBD_WebUSB_RegisterInterface
 * @param  pdev: device instance
 * @param  fops: CD  Interface callback
 * @retval status
 */
uint8_t USBD_WebUSB_RegisterInterface(USBD_HandleTypeDef* pdev, USBD_WebUSB_ItfTypeDef* fops)
{
    if (fops == NULL) {
        return (uint8_t)USBD_FAIL;
    }

    pdev->pUserData[pdev->classId] = fops;

    return (uint8_t)USBD_OK;
}

        /**
         * @brief  USBD_WebUSB_SetTxBuffer
         * @param  pdev: device instance
         * @param  pbuff: Tx Buffer
         * @param  length: length of data to be sent
         * @param  ClassId: The Class ID
         * @retval status
         */
        #ifdef USE_USBD_COMPOSITE
uint8_t USBD_WebUSB_SetTxBuffer(USBD_HandleTypeDef* pdev, uint8_t* pbuff, uint32_t length, uint8_t ClassId)
{
    USBD_WebUSB_HandleTypeDef* hcdc = (USBD_WebUSB_HandleTypeDef*)pdev->pClassDataCmsit[ClassId];
        #else
uint8_t USBD_WebUSB_SetTxBuffer(USBD_HandleTypeDef* pdev, uint8_t* pbuff, uint32_t length)
{
    USBD_WebUSB_HandleTypeDef* hcdc = (USBD_WebUSB_HandleTypeDef*)pdev->pClassDataCmsit[pdev->classId];
        #endif /* USE_USBD_COMPOSITE */

    if (hcdc == NULL) {
        return (uint8_t)USBD_FAIL;
    }

    hcdc->TxBuffer = pbuff;
    hcdc->TxLength = length;

    return (uint8_t)USBD_OK;
}

/**
 * @brief  USBD_WebUSB_SetRxBuffer
 * @param  pdev: device instance
 * @param  pbuff: Rx Buffer
 * @retval status
 */
uint8_t USBD_WebUSB_SetRxBuffer(USBD_HandleTypeDef* pdev, uint8_t* pbuff)
{
    USBD_WebUSB_HandleTypeDef* hcdc = (USBD_WebUSB_HandleTypeDef*)pdev->pClassDataCmsit[pdev->classId];

    if (hcdc == NULL) {
        return (uint8_t)USBD_FAIL;
    }

    hcdc->RxBuffer = pbuff;

    return (uint8_t)USBD_OK;
}

        /**
         * @brief  USBD_WebUSB_TransmitPacket
         *         Transmit packet on IN endpoint
         * @param  pdev: device instance
         * @param  ClassId: The Class ID
         * @retval status
         */
        #ifdef USE_USBD_COMPOSITE
uint8_t USBD_WebUSB_TransmitPacket(USBD_HandleTypeDef* pdev, uint8_t ClassId)
{
    USBD_WebUSB_HandleTypeDef* hcdc = (USBD_WebUSB_HandleTypeDef*)pdev->pClassDataCmsit[ClassId];
        #else
uint8_t USBD_WebUSB_TransmitPacket(USBD_HandleTypeDef* pdev)
{
    USBD_WebUSB_HandleTypeDef* hcdc = (USBD_WebUSB_HandleTypeDef*)pdev->pClassDataCmsit[pdev->classId];
        #endif /* USE_USBD_COMPOSITE */

    USBD_StatusTypeDef ret = USBD_BUSY;

        #ifdef USE_USBD_COMPOSITE
    /* Get the Endpoints addresses allocated for this class instance */
    WebUSBInEpAdd = USBD_CoreGetEPAdd(pdev, USBD_EP_IN, USBD_EP_TYPE_BULK, ClassId);
        #endif /* USE_USBD_COMPOSITE */

    if (hcdc == NULL) {
        return (uint8_t)USBD_FAIL;
    }

    if (hcdc->TxState == 0U) {
        /* Tx Transfer in progress */
        hcdc->TxState = 1U;

        /* Update the packet total length */
        pdev->ep_in[WebUSBInEpAdd & 0xFU].total_length = hcdc->TxLength;

        /* Transmit next packet */
        (void)USBD_LL_Transmit(pdev, WebUSBInEpAdd, hcdc->TxBuffer, hcdc->TxLength);

        ret = USBD_OK;
    }

    return (uint8_t)ret;
}

/**
 * @brief  USBD_WebUSB_ReceivePacket
 *         prepare OUT Endpoint for reception
 * @param  pdev: device instance
 * @retval status
 */
uint8_t USBD_WebUSB_ReceivePacket(USBD_HandleTypeDef* pdev)
{
    USBD_WebUSB_HandleTypeDef* hcdc = (USBD_WebUSB_HandleTypeDef*)pdev->pClassDataCmsit[pdev->classId];

        #ifdef USE_USBD_COMPOSITE
    /* Get the Endpoints addresses allocated for this class instance */
    WebUSBOutEpAdd = USBD_CoreGetEPAdd(pdev, USBD_EP_OUT, USBD_EP_TYPE_BULK, (uint8_t)pdev->classId);
        #endif /* USE_USBD_COMPOSITE */

    if (pdev->pClassDataCmsit[pdev->classId] == NULL) {
        return (uint8_t)USBD_FAIL;
    }

    if (pdev->dev_speed == USBD_SPEED_HIGH) {
        /* Prepare Out endpoint to receive next packet */
        (void)USBD_LL_PrepareReceive(pdev, WebUSBOutEpAdd, hcdc->RxBuffer, WebUSB_DATA_HS_OUT_PACKET_SIZE);
    }
    else {
        /* Prepare Out endpoint to receive next packet */
        (void)USBD_LL_PrepareReceive(pdev, WebUSBOutEpAdd, hcdc->RxBuffer, WebUSB_DATA_FS_OUT_PACKET_SIZE);
    }

    return (uint8_t)USBD_OK;
}
        #ifdef USE_USBD_COMPOSITE
uint8_t USBD_WebUSB_ClearBuffer(USBD_HandleTypeDef* pdev, uint8_t ClassId)
{
    /* Suspend or Resume USB Out process */
    if (pdev->pClassDataCmsit[classId] != NULL) {
        #else
uint8_t USBD_WebUSB_ClearBuffer(USBD_HandleTypeDef* pdev)
{
    /* Suspend or Resume USB Out process */
    if (pdev->pClassDataCmsit[pdev->classId] != NULL) {
        #endif /* USE_USBD_COMPOSITE */
        /* Prepare Out endpoint to receive next packet */
        USBD_LL_PrepareReceive(pdev, CDC_OUT_EP, 0, 0);
        return (uint8_t)USBD_OK;
    }
    else {
        return (uint8_t)USBD_FAIL;
    }
}

    #endif /* USBD_USE_CDC */
#endif     /* USBCON */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
