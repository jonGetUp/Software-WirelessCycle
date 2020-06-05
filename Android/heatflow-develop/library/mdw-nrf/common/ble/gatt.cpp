/************************************************************************************************************
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 ************************************************************************************************************/

/************************************************************************************************************/
/* Header files                                                                                             */
/************************************************************************************************************/
#include "gatt.h"

#if defined(SOFTDEVICE_PRESENT)
#include <nrf_sdh_ble_ext.h>
#include <sdk_config.h>

#include <factory.h>
#include "core/isubject.hpp"


/************************************************************************************************************/
/* DEBUG COMPILATION OPTION & MACRO                                                                         */
/************************************************************************************************************/
#define DEBUG_GATT_ENABLE                   0
#if (DEBUG_GATT_ENABLE != 0) && defined(DEBUG_NRF_USER)
    #include <tools/logtrace.h>
    #define GATT_DEBUG                      LOG_TRACE_DEBUG
#else
    #define GATT_DEBUG(...)                 {(void) (0);}
#endif


/************************************************************************************************************/
/* Namespace & Objects                                                                                      */
/************************************************************************************************************/
using ble::Gatt;


/************************************************************************************************************/
/* --- Extern "C" Section ---                                                                               */
/*     Callback's section                                                                                   */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
void ble::onGattEvent(nrf_ble_gatt_t* pGatt, nrf_ble_gatt_evt_t const* pEvent)
{
    Factory::peripheralController.gattEventHandler(pGatt, pEvent);
}


/************************************************************************************************************/
/* CONSTRUCTOR                                                                                              */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
Gatt::Gatt()
{
}

//------------------------------------------------------------------------------------------------------------
Gatt::~Gatt() 
{
}


/************************************************************************************************************/
/* PUBLIC SECTION                                                                                           */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
void Gatt::initialize()
{
    uint32_t errCode = nrf_ble_gatt_init(&_gatt, ble::onGattEvent);
    if(errCode != NRF_SUCCESS)
    {
        GATT_DEBUG("[Gatt] initialize() > nrf_ble_gatt_init() FAILED! errCode: 0x%04x\r\n", errCode);
        ASSERT(false);
    }

    errCode = nrf_ble_gatt_att_mtu_periph_set(&_gatt, NRF_SDH_BLE_GATT_MAX_MTU_SIZE);
    if(errCode != NRF_SUCCESS)
    {
        GATT_DEBUG("[Gatt] initialize() > nrf_ble_gatt_att_mtu_periph_set() FAILED! errCode: 0x%04x\r\n", errCode);
        ASSERT(false);
    }

    GATT_DEBUG("[Gatt] initialize() > Register SoftDevice BLE observer for GATT in the right section...\r\n");
    NRF_SDH_BLE_OBSERVER_CPP(_gattBleObs, NRF_BLE_GATT_BLE_OBSERVER_PRIO, nrf_ble_gatt_on_ble_evt, &_gatt);
}

//------------------------------------------------------------------------------------------------------------
void Gatt::gattEventHandler(nrf_ble_gatt_t* pGatt, nrf_ble_gatt_evt_t const* pEvent)
{
    switch(pEvent->evt_id)
    {
        case NRF_BLE_GATT_EVT_ATT_MTU_UPDATED:
            GATT_DEBUG("[Gatt] onGattEvent() > GATT MTU UPDATED for connection's handle: 0x%02x, MTU: %3d bytes. (central: %3d [B], peripheral: %3d [B])\r\n", 
                        pEvent->conn_handle, pEvent->params.att_mtu_effective, pGatt->att_mtu_desired_central, pGatt->att_mtu_desired_periph);
            _notifyObserverOnMtuUpdated(pEvent->conn_handle, pEvent->params.att_mtu_effective);
            break;
        
    #if !defined (S112)
        case NRF_BLE_GATT_EVT_DATA_LENGTH_UPDATED:
            GATT_DEBUG("[Gatt] onGattEvent() > Data length on connection 0x%x changed to %d.\r\n", 
                        pEvent->conn_handle, pEvent->params.data_length);
            _notifyObserverOnDataLengthUpdated(pEvent->conn_handle, pEvent->params.data_length);
            break;
    #endif
        
        default:
            break;
    }
}

/************************************************************************************************************/
/* PRIVATE SECTION                                                                                          */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
void Gatt::_notifyObserverOnMtuUpdated(uint16_t connHandle, uint16_t mtuEffective)
{
    for(uint8_t i = 0; i < observersCount(); i++)
    {
        if(observer(i) != NULL)
        {
            observer(i)->onGattMtuUpdated(this, connHandle, mtuEffective);
        }
    }
}

//------------------------------------------------------------------------------------------------------------
void Gatt::_notifyObserverOnDataLengthUpdated(uint16_t connHandle, uint8_t dataLength)
{
    for(uint8_t i = 0; i < observersCount(); i++)
    {
        if(observer(i) != NULL)
        {
            observer(i)->onGattDataLengthUpdated(this, connHandle, dataLength);
        }
    }
}

#endif  // #if defined(SOFTDEVICE_PRESENT)
