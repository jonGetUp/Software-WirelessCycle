/* Copyright (c) 2018, HES-SO Valais
 * All rights reserved.
 */
#ifndef BLE_DFU_CONFIG_H
#define BLE_DFU_CONFIG_H

// -----------------------------------------------------------------------------------------------------------
// Include or not the service_changed characteristic. if not enabled, the server's database cannot be changed 
// for the lifetime of the device
#if !defined(IS_SRVC_CHANGED_CHARACT_PRESENT)
    #define IS_SRVC_CHANGED_CHARACT_PRESENT     1
#else
    #undef  IS_SRVC_CHANGED_CHARACT_PRESENT
    #define IS_SRVC_CHANGED_CHARACT_PRESENT     1                                                                           ///< \brief Include or not the service_changed characteristic. if not enabled, the server's database cannot be changed for the lifetime of the device
#endif

// -----------------------------------------------------------------------------------------------------------
#if (DFU_SUPPORT != 0)
#   define DFU_REV_MAJOR                    0x00                                                                        ///< \brief DFU Major revision number to be exposed.
#   define DFU_REV_MINOR                    0x01                                                                        ///< \brief DFU Minor revision number to be exposed.
#   define DFU_REVISION                     ((DFU_REV_MAJOR << 8) | DFU_REV_MINOR)                                      ///< \brief DFU Revision number to be exposed. Combined of major and minor versions.
#   define APP_SERVICE_HANDLE_START         0x000C                                                                      ///< \brief< Handle of first application specific service when when service changed characteristic is present.
#   define BLE_HANDLE_MAX                   0xFFFF                                                                      ///< \brief< Max handle value in BLE.

    STATIC_ASSERT(IS_SRVC_CHANGED_CHARACT_PRESENT);                                                                     ///< \brief When having DFU Service support in application the Service Changed Characteristic should always be present.
#endif // BLE_DFU_APP_SUPPORT

#endif // BLE_DFU_CONFIG_H
