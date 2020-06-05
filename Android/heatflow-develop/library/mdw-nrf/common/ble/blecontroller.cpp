/*************************************************************************************************************
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 ************************************************************************************************************/

/************************************************************************************************************/
/* Header files                                                                                             */
/************************************************************************************************************/
#include <app_timer.h>
#include <nordic_common.h>
#include <nrf_sdh.h>
#include <nrf_sdh_ble_ext.h>
#include <nrf_sdh_soc_ext.h>
#include <string.h>

#include <factory.h>

#include "ble/blecontroller.h"
#include "ble/advertiser.h"

#if (USE_SD_APP_SCHEDULER != 0)
    #include "ble/scheduler.h"
#endif

/************************************************************************************************************/
/* DEBUG COMPILATION OPTION & MACRO                                                                         */
/************************************************************************************************************/
#define DEBUG_BLECONTROLLER_ENABLE          0
#if (DEBUG_BLECONTROLLER_ENABLE != 0) && defined(DEBUG_NRF_USER)
    #include <tools/logtrace.h>
    #define BLECTRL_DEBUG                   LOG_TRACE_DEBUG
#else
    #define BLECTRL_DEBUG(...)              {(void) (0);}
#endif


/************************************************************************************************************/
/* Namespace's Declaration                                                                                  */
/************************************************************************************************************/
using ble::BleController;
    

/************************************************************************************************************/
/* --- Extern "C" Section ---                                                                               */
/*     Callback's section                                                                                   */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
void ble::bleEventHandler(ble_evt_t const * pBleEvt, void* pContext)
{
    if(pContext != NULL)
    {
        (static_cast<BleController*>(pContext))->notifyObserversOnBleEvent(pBleEvt);
    }
}

//------------------------------------------------------------------------------------------------------------
void ble::sysEventHandler(uint32_t sysEvt, void* pContext)
{
    //fs_sys_event_handler(sysEvt);
    if(pContext != NULL)
    {
        (static_cast<BleController*>(pContext))->notifyObserversOnSystemEvent(sysEvt);
    }
}


/* **********************************************************************************************************/
/* Global declaration                                                                                       */
/* **********************************************************************************************************/
//------------------------------------------------------------------------------------------------------------


/************************************************************************************************************/
/* CONSTRUCTOR                                                                                              */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
BleController::BleController()
{
}

//------------------------------------------------------------------------------------------------------------
BleController::~BleController()
{
}

/************************************************************************************************************/
/* PUBLIC SECTION                                                                                           */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
void BleController::initialize()
{
    uint32_t errCode = NRF_SUCCESS;

    errCode = _bleStackInit();
    if(errCode != NRF_SUCCESS)
    {
        BLECTRL_DEBUG("[BleController] initialize() > _bleStatckInit() FAILED! errCode: 0x%04x\r\n", errCode);
        ASSERT(false);
    }
}

//------------------------------------------------------------------------------------------------------------
void BleController::notifyObserversOnBleEvent(ble_evt_t const * pBleEvt)
{
    for(uint8_t i = 0; i < observersCount(); i++)
    {
        if(observer(i) != NULL)
        {
            observer(i)->onBleEvent(pBleEvt, NULL);
        }
    }
}

//------------------------------------------------------------------------------------------------------------
void BleController::notifyObserversOnSystemEvent(uint32_t sysEvt)
{
    for(uint8_t i = 0; i < observersCount(); i++)
    {
        if(observer(i) != NULL)
        {
            observer(i)->onSystemEvent(sysEvt);
        }
    }
}

//------------------------------------------------------------------------------------------------------------
uint8_t BleController::readBtAddress(ble_gap_addr_t* address)
{
    // Get BLE address.
    uint32_t errCode = NRF_SUCCESS;
    errCode = sd_ble_gap_addr_get(address);
    if(errCode != NRF_SUCCESS)
    {
        BLECTRL_DEBUG("[BleController] readBtAddress() > sd_ble_gap_addr_get() FAILED! (error: 0x%04x)\r\n", errCode);
        return 0;
    }
#if (DEBUG_BLECONTROLLER_ENABLE != 0)
    switch(address->addr_type)
    {
        case BLE_GAP_ADDR_TYPE_PUBLIC:
            BLECTRL_DEBUG("[BleController] readBtAddress() > BLE_GAP_ADDR_TYPE_PUBLIC 0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x\r\n",
                          address->addr[0], address->addr[1], address->addr[2], address->addr[3], address->addr[4], address->addr[5]);
            break;

        case BLE_GAP_ADDR_TYPE_RANDOM_STATIC:
            BLECTRL_DEBUG("[BleController] readBtAddress() > BLE_GAP_ADDR_TYPE_RANDOM_STATIC 0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x\r\n",
                          address->addr[0], address->addr[1], address->addr[2], address->addr[3], address->addr[4], address->addr[5]);
            break;

        case BLE_GAP_ADDR_TYPE_RANDOM_PRIVATE_RESOLVABLE:
            BLECTRL_DEBUG("[BleController] readBtAddress() > BLE_GAP_ADDR_TYPE_RANDOM_PRIVATE_RESOLVABLE 0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x\r\n",
                          address->addr[0], address->addr[1], address->addr[2], address->addr[3], address->addr[4], address->addr[5]);
            break;

        case BLE_GAP_ADDR_TYPE_RANDOM_PRIVATE_NON_RESOLVABLE:
            BLECTRL_DEBUG("[BleController] readBtAddress() > BLE_GAP_ADDR_TYPE_RANDOM_PRIVATE_NON_RESOLVABLE 0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x\r\n",
                          address->addr[0], address->addr[1], address->addr[2], address->addr[3], address->addr[4], address->addr[5]);
            break;
    }
#endif
    return sizeof(address->addr);
}

//------------------------------------------------------------------------------------------------------------
///< Function for initializing the BLE stack.
uint32_t BleController::_bleStackInit()
{
    ret_code_t errCode;

    errCode = nrf_sdh_enable_request();
    if(errCode != NRF_SUCCESS)
    {
        BLECTRL_DEBUG("[BleController] _bleStackInit() > nrf_sdh_enable_request() FAILED! errCode: 0x%04x\r\n", errCode);
        ASSERT(false);
    }

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ramStart = 0;
    errCode = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ramStart);
    if(errCode != NRF_SUCCESS)
    {
        BLECTRL_DEBUG("[BleController] _bleStackInit() > nrf_sdh_ble_default_cfg_set() FAILED! ramStart: 0x%08x, errCode: 0x%04x\r\n", ramStart, errCode);
        ASSERT(false);
    }
    
    // Enable BLE stack.
    errCode = nrf_sdh_ble_enable(&ramStart);
    if(errCode != NRF_SUCCESS)
    {
        BLECTRL_DEBUG("[BleController] _bleStackInit() > nrf_sdh_ble_enable() FAILED! ramStart: 0x%08x, errCode: 0x%04x\r\n", ramStart, errCode);
        ASSERT(false);
    }
    else
    {
        BLECTRL_DEBUG("[BleController] _bleStackInit() > nrf_sdh_ble_enable() [OK], ramStart: 0x%08x\r\n", ramStart);
    }

    // Register a handler for BLE events.
    BLECTRL_DEBUG("[BleController] _bleStackInit() > Register SoftDevice BLE observer...\r\n");
    NRF_SDH_BLE_OBSERVER_CPP(_bleControllerBleObs, APP_BLE_OBSERVER_PRIO, ble::bleEventHandler, (void*) this);
    
    // Register a handler for SoftDevice SoC events.
    BLECTRL_DEBUG("[BleController] _bleStackInit() > Register SoftDevice SoC observer...\r\n");
    NRF_SDH_SOC_OBSERVER_CPP(_bleControllerSocObs, APP_SOC_OBSERVER_PRIO, ble::sysEventHandler, (void*) this);
    
    // Enable or disable the DC/DC converter
    errCode = sd_power_dcdc_mode_set(DCDC_CONVERTER_MODE);
    if(errCode != NRF_SUCCESS)
    {
        BLECTRL_DEBUG("[BleController] _bleStackInit() > sd_power_dcdc_mode_set() FAILED! errCode: 0x%04x\r\n", errCode);
        ASSERT(false);
    }

    // "Todo: Add here the usage of the QWR module (nrf_ble_qwr_init(...))"
    return errCode;
}
