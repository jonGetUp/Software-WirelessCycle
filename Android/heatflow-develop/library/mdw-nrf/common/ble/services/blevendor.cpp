/************************************************************************************************************
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 ************************************************************************************************************/

/************************************************************************************************************/
/* Header files                                                                                             */
/************************************************************************************************************/
#include "blevendor.h"
#include <assert-config.h>


/************************************************************************************************************/
/* DEBUG COMPILATION OPTION & MACRO                                                                         */
/************************************************************************************************************/
#define DEBUG_BLE_VENDOR_ENABLE             0
#if (DEBUG_BLE_VENDOR_ENABLE != 0) && defined(DEBUG_NRF_USER)
    #include <tools/logtrace.h>
    #define BLEV_DEBUG                      LOG_TRACE_DEBUG
#else
    #define BLEV_DEBUG(...)                 {(void) (0);}
#endif

/************************************************************************************************************/
/* Namespace's Declaration                                                                                  */
/************************************************************************************************************/
using ble::BleVendor;

/************************************************************************************************************/
/* CONSTRUCTOR                                                                                              */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
BleVendor::BleVendor() : _uuidType(0), _isVendorRegistered(false) {}

//------------------------------------------------------------------------------------------------------------
BleVendor::~BleVendor() {}

/************************************************************************************************************/
/* PUBLIC SECTION                                                                                           */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
void BleVendor::registerVendor(ble_uuid128_t* serviceUUIDs, uint16_t ownerMask)
{
    if(!_isVendorRegistered)
    {
        serviceUUIDs->uuid128[OWNER_MASK_LOWER_IDX]  = (unsigned char)( ownerMask       & 0x00ff);
        serviceUUIDs->uuid128[OWNER_MASK_HIGHER_IDX] = (unsigned char)((ownerMask >> 8) & 0x00ff);

        uint32_t errCode = sd_ble_uuid_vs_add(serviceUUIDs, &_uuidType);
        if(errCode != NRF_SUCCESS)
        {
            BLEV_DEBUG("[BleVendor] registerVendor() > sd_ble_uuid_vs_add() error: 0x%04x\r\n", errCode);
            ASSERT(false);
        }
        _isVendorRegistered = true;
    }
    else
    {
        BLEV_DEBUG("[BleVendor] registerVendor() > Vendor 0x%04x already registered !\r\n", ownerMask);
    }
}
