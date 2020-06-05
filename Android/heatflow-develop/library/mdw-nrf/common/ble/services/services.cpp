/************************************************************************************************************
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 ************************************************************************************************************/

/************************************************************************************************************/
/* Header files                                                                                             */
/************************************************************************************************************/
#include "services.h"
#include <platform-config.h>

#if defined(SOFTDEVICE_PRESENT)

#include <ble_advdata.h>
#include <nrf_sdh_ble_ext.h>
#include <sdk_config.h>
#include "uicr-config.h"

#include "ble/interface/bleservicesobserver.h"
#include <factory.h>


#if !defined(BAS_SUPPORT) || !defined(DIS_SUPPORT) || !defined(IAS_SUPPORT)
    #error "BAS_SUPPORT or DIS_SUPPORT or IAS_SUPPORT not yet defined !"
#endif

/************************************************************************************************************/
/* DEBUG COMPILATION OPTION & MACRO                                                                         */
/************************************************************************************************************/
#define DEBUG_SERVICES_ENABLE               0
#if (DEBUG_SERVICES_ENABLE != 0) && defined(DEBUG_NRF_USER)
    #include <tools/logtrace.h>
    #define SRV_DEBUG                       LOG_TRACE_DEBUG
#else
    #define SRV_DEBUG(...)                  {(void) (0);}
#endif


/************************************************************************************************************/
/* Namespace's Declaration                                                                                  */
/************************************************************************************************************/
using ble::Services;


/************************************************************************************************************/
/* --- Extern "C" Section ---                                                                               */
/*     Callback's section                                                                                   */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
#if (BAS_SUPPORT != 0)
void ble::onBasEvent(ble_bas_t* pBas, ble_bas_evt_t* pEvt)
{
    switch(pEvt->evt_type)
    {
        case BLE_BAS_EVT_NOTIFICATION_ENABLED:
            SRV_DEBUG("[Services] onBasEvent() > Event Type: BLE_BAS_EVT_NOTIFICATION_ENABLED\r\n");
            break;

        case BLE_BAS_EVT_NOTIFICATION_DISABLED:
            SRV_DEBUG("[Services] onBasEvent() > Event Type: BLE_BAS_EVT_NOTIFICATION_DISABLED\r\n");
            break;

        default:
            break;
    }
}
#endif

//------------------------------------------------------------------------------------------------------------
#if (IAS_SUPPORT != 0)
void ble::onIasEvent(ble_ias_t* pIas, ble_ias_evt_t* pEvt)
{
    SRV_DEBUG("[Services] onIasEvent() > Alert level: %d\r\n", pEvt->params.alert_level);

    switch(pEvt->evt_type)
    {
        case BLE_IAS_EVT_ALERT_LEVEL_UPDATED:
            SRV_DEBUG("[Services] onIasEvent() > Event Type: BLE_IAS_EVT_ALERT_LEVEL_UPDATED\r\n");
            Factory::peripheralController.bleAdoptedServices()->notifyAlertLevel(pEvt->params.alert_level);
            break;

        default:
            break;
    }
}
#endif

/************************************************************************************************************/
/* CONSTRUCTOR                                                                                              */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
Services::Services()    {}

//------------------------------------------------------------------------------------------------------------
Services::~Services()   {}


/************************************************************************************************************/
/* PUBLIC SECTION                                                                                           */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
void Services::initializeAdoptedServices()
{
#if (BAS_SUPPORT != 0)
    // Register the observer of BLE event for the Battery Service
    SRV_DEBUG("[Services] initializeAdoptedServices() > Registering observer of BLE event for the Battery Service...\r\n");
    NRF_SDH_BLE_OBSERVER_CPP(_basBleObs, BLE_BAS_BLE_OBSERVER_PRIO, ble_bas_on_ble_evt, &_bas);

    // Initialize Battery Service.
    SRV_DEBUG("[Services] initializeAdoptedServices() > Battery Service: _basInit() ...\r\n");
    _basInit();
#endif

#if (DIS_SUPPORT != 0)
    // Initialize Device Information Service.
    SRV_DEBUG("[Services] initializeAdoptedServices() > Device Information Service: _disInit() ...\r\n");
    _disInit();
#endif

#if (IAS_SUPPORT != 0)
    // Register the observer of BLE event for the Battery Service
    SRV_DEBUG("[Services] initializeAdoptedServices() > Registering observer of BLE event for the Immediate Alert Service...\r\n");
    NRF_SDH_BLE_OBSERVER_CPP(_iasBleObs, BLE_IAS_BLE_OBSERVER_PRIO, ble_ias_on_ble_evt, &_ias);

    // Initialize Device Information Service.
    SRV_DEBUG("[Services] initializeAdoptedServices() > Immediate Alert Service: _iasInit() ...\r\n");
    _iasInit();
#endif    // USE_IMMEDIATE_ALERT_SERVICE
}

//------------------------------------------------------------------------------------------------------------
#if (IAS_SUPPORT != 0)
void Services::notifyAlertLevel(uint8_t alertLevel)
{
    for(uint_fast8_t i = 0; i < observersCount(); i++)
    {
        if(observer(i) != NULL)
        {
            observer(i)->onImmediateAlertChanged(alertLevel);
        }
    }
}
#endif


/************************************************************************************************************/
/* PRIVATE SECTION                                                                                          */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
#if (BAS_SUPPORT != 0)
void Services::_basInit()
{
    ble_bas_init_t initObj;
    memset(&initObj, 0, sizeof(ble_bas_init_t));

    initObj.evt_handler             = ble::onBasEvent;
    initObj.support_notification    = true;
    initObj.p_report_ref            = NULL;
    initObj.initial_batt_level      = 100;

    // Here the sec level for the Battery Service can be changed/increased.
    initObj.bl_rd_sec               = SEC_OPEN;
    initObj.bl_cccd_wr_sec          = SEC_OPEN;
    initObj.bl_report_rd_sec        = SEC_OPEN;

    uint32_t errCode                = ble_bas_init(&_bas, &initObj);
    SRV_DEBUG("[Services] _basInit() > ble_bas_init() ... %s (err_code: 0x%04x)\r\n", (errCode == NRF_SUCCESS ? "[Ok]" : "[Failed]"), errCode);
    ASSERT(errCode == NRF_SUCCESS);
}
#endif

//------------------------------------------------------------------------------------------------------------
#if (DIS_SUPPORT != 0)
void Services::_disInit()
{
    // Initialize Device Information Service
    memset(&_dis, 0, sizeof(ble_dis_init_t));

    ble_srv_ascii_to_utf8(&_dis.manufact_name_str, (char*) MANUFACTURER_NAME);
    SRV_DEBUG("[Services] _disInit() > Manufaturer Name: %s\r\n", (char*) MANUFACTURER_NAME);
    ble_srv_ascii_to_utf8(&_dis.model_num_str, (char*) MODEL_NUMBER);
    SRV_DEBUG("[Services] _disInit() > Model Number: %s\r\n", (char*) MODEL_NUMBER);
#if defined(HARDWARE_REVISION)
    ble_srv_ascii_to_utf8(&_dis.hw_rev_str, (char*) HARDWARE_REVISION);
    SRV_DEBUG("[Services] _disInit() > Hardware Revision: %s\r\n", (char*) HARDWARE_REVISION);
#endif
#if defined(FIRMWARE_REVISION)
    ble_srv_ascii_to_utf8(&_dis.fw_rev_str, (char*) FIRMWARE_REVISION);
    SRV_DEBUG("[Services] _disInit() > Firmware Revision: %s\r\n", (char*) FIRMWARE_REVISION);
#endif

    // Adding the serial number to the DIS
    char strSerial[DIS_SERIAL_NUMBER_SIZE] = { 0 };
#if defined(SERIAL_NUMBER) && (SERIAL_NUMBER != 0)
    uint32_t serialNumber = SERIAL_NUMBER;
#else    
    uint32_t serialNumber = UICR_GET_SERIAL_NUMBER;
#endif
    SRV_DEBUG("[Services] _disInit() > Serial Number 0x%u\r\n", serialNumber);
    if(serialNumber != 0xffffffff)
    {
        snprintf(strSerial, DIS_SERIAL_NUMBER_SIZE, "%lu", serialNumber);
        ble_srv_ascii_to_utf8(&_dis.serial_num_str, strSerial);
    }

    _dis.dis_char_rd_sec = SEC_OPEN;

    uint32_t errCode = ble_dis_init(&_dis);
    SRV_DEBUG("[Services] _disInit() > _disInit() ... %s (err_code: 0x%04x)\r\n", (errCode == NRF_SUCCESS ? "[Ok]" : "[Failed]"), errCode);
    ASSERT(errCode == NRF_SUCCESS);
}
#endif

//------------------------------------------------------------------------------------------------------------
#if (IAS_SUPPORT != 0)
void Services::_iasInit()
{
    ble_ias_init_t    initObj;

    memset(&initObj, 0, sizeof(ble_ias_init_t));
    initObj.evt_handler = ble::onIasEvent;

    uint32_t errCode = ble_ias_init(&_ias, &initObj);
    SRV_DEBUG("[Services] _disInit() > _iasInit() ... %s (err_code: 0x%04x)\r\n", (errCode == NRF_SUCCESS ? "[Ok]" : "[Failed]"), errCode);
    ASSERT(errCode == NRF_SUCCESS);
}
#endif


#endif    // #if defined(SOFTDEVICE_PRESENT)
