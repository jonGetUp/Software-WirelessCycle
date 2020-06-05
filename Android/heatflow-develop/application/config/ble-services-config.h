/* Copyright (c) 2018, HES-SO Valais
 * All rights reserved.
 */
#ifndef BLE_SERVICES_CONFIG_H
#define BLE_SERVICES_CONFIG_H

#include <platform-config.h>

/********************************************************************************************************************
 * Bluetooth SIG Base UUID
 ********************************************************************************************************************/
// -----------------------------------------------------------------------------------------------------------
#ifndef BLE_UUID_BLUETOOTH_SIG_BASE
    #define BLE_UUID_BLUETOOTH_SIG_BASE     {0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80, \
                                             0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
#endif

// -----------------------------------------------------------------------------------------------------------
// BAS - Adopted Services
#if !defined(BLE_BAS_ENABLED)
    #error "BLE_BAS_ENABLED not yet defined !"
#else                                           
    #define BAS_SUPPORT                     BLE_BAS_ENABLED
    #define ADVERTISE_BAS_UUID              0
#endif
// -----------------------------------------------------------------------------------------------------------
// DIS - Adopted Services
#if !defined(BLE_DIS_ENABLED)
    #error "BLE_DIS_ENABLED not yet defined !"
#else
    #define DIS_SUPPORT                     BLE_DIS_ENABLED
    #define ADVERTISE_DIS_UUID              0
#endif

#if (DIS_SUPPORT != 0)
    #define DIS_SERIAL_NUMBER_SIZE          11
#endif  // #if (DIS_SUPPORT != 0)

// -----------------------------------------------------------------------------------------------------------
// DFU related
#if !defined(BLE_DFU_ENABLED)
    #error "BLE_DFU_ENABLED not yet defined !"
#else
    #define DFU_SUPPORT                     BLE_DFU_ENABLED
    #define ADVERTISE_DFU_UUID              0
#endif

#if (DFU_SUPPORT != 0)
    #define SD_GATTS_SRV_CHANGED            1
#else
    #define SD_GATTS_SRV_CHANGED            0
#endif

// -----------------------------------------------------------------------------------------------------------
// AMS (Apple Media Service) related
#if !defined(BLE_AMS_C_ENABLED)
    #error "BLE_AMS_C_ENABLED not yet defined !"
#else
    #define AMS_SUPPORT                     BLE_AMS_C_ENABLED
#endif
                                         
// -----------------------------------------------------------------------------------------------------------
// ANCS related
#if !defined(BLE_ANCS_C_ENABLED)
    #error "BLE_ANCS_C_ENABLED not yet defined !"
#else
    #define ANCS_SUPPORT                    BLE_ANCS_C_ENABLED
#endif

// -----------------------------------------------------------------------------------------------------------
// IAS - Adopted Services
#if !defined(BLE_IAS_ENABLED)
    #error "BLE_IAS_ENABLED not yet defined !"
#else
    #define IAS_SUPPORT                     BLE_IAS_ENABLED
    #define ADVERTISE_IAS_UUID              0
#endif

#if (IAS_SUPPORT != 0)
    #define IAS_TIMEOUT_DELAY               10
#endif

// -----------------------------------------------------------------------------------------------------------
// Number of BLE adopted services activated
#define NUMBER_OF_ADOPTED_SERVICES          (BAS_SUPPORT+DIS_SUPPORT+DFU_SUPPORT+ANCS_SUPPORT+IAS_SUPPORT)

// -----------------------------------------------------------------------------------------------------------
// handling the includes
#if (BAS_SUPPORT != 0)
    #include <ble_bas/ble_bas.h>
#endif    // Battery Service

#if (DIS_SUPPORT != 0)
    #include <ble_dis/ble_dis.h>
#endif    // Device Information Service

#if (DFU_SUPPORT != 0)
    #include <ble_dfu/ble_dfu.h>
#endif

#if (ANCS_SUPPORT != 0)
    #include <ble_ancs_c/nrf_ble_ancs_c.h>
#endif    // Apple Notification Center Service

#if (IAS_SUPPORT != 0)
    #include <ble_ias/ble_ias.h>
#endif    // Immediate Alert Service
    
#endif // BLE_ADVERTISER_CONFIG_H
