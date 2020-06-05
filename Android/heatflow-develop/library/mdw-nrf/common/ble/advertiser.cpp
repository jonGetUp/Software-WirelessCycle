/************************************************************************************************************
 * Copyright (C) Soprod SA & Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2018
 * Created by Thierry Hischier, Patrice Rudaz
 * All rights reserved.
 ************************************************************************************************************/

/************************************************************************************************************/
/* Header files                                                                                             */
/************************************************************************************************************/
#include "advertiser.h"
#include "blecontroller.h"

#if defined(SOFTDEVICE_PRESENT)

#include <factory.h>

#include <ble_advdata.h>
#include <nordic_common.h>
#include <nrf_sdh_ble_ext.h>
#include <nrf_sdh_soc_ext.h>

/************************************************************************************************************/
/* DEBUG COMPILATION OPTION & MACRO                                                                         */
/************************************************************************************************************/
#define DEBUG_ADVERTISER_ENABLE             0
#if (DEBUG_ADVERTISER_ENABLE != 0) && defined(DEBUG_NRF_USER)
    #include <tools/logtrace.h>
    #define ADV_DEBUG                       LOG_TRACE_DEBUG
#else
    #define ADV_DEBUG(...)                  {(void) (0);}
#endif


/************************************************************************************************************/
/* Namespace's Declaration                                                                                  */
/************************************************************************************************************/
using ble::Advertiser;


/************************************************************************************************************/
/* --- Extern "C" Section ---                                                                               */
/*     Callback's section                                                                                   */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
void ble::advEventHandler(ble_adv_evt_t bleAdvEvent)
{
#if (DEBUG_ADVERTISER_ENABLE != 0)
    switch(bleAdvEvent)
    {
        case BLE_ADV_EVT_DIRECTED_HIGH_DUTY:    ADV_DEBUG("[Advertiser] advEventHandler() > BLE_ADV_EVT_DIRECTED_HIGH_DUTY received ...\r\n");  break;
        case BLE_ADV_EVT_DIRECTED:              ADV_DEBUG("[Advertiser] advEventHandler() > BLE_ADV_EVT_DIRECTED received ...\r\n");            break;
        case BLE_ADV_EVT_FAST:                  ADV_DEBUG("[Advertiser] advEventHandler() > BLE_ADV_EVT_FAST received ...\r\n");                break;
        case BLE_ADV_EVT_FAST_WHITELIST:        ADV_DEBUG("[Advertiser] advEventHandler() > BLE_ADV_EVT_FAST_WHITELIST received ...\r\n");      break;
        case BLE_ADV_EVT_IDLE:                  ADV_DEBUG("[Advertiser] advEventHandler() > BLE_ADV_EVT_IDLE received ...\r\n");                break;
        case BLE_ADV_EVT_SLOW:                  ADV_DEBUG("[Advertiser] advEventHandler() > BLE_ADV_EVT_SLOW received ...\r\n");                break;
        case BLE_ADV_EVT_SLOW_WHITELIST:        ADV_DEBUG("[Advertiser] advEventHandler() > BLE_ADV_EVT_SLOW_WHITELIST received ...\r\n");      break;
        default:                                                                                                                                break;
    }
#endif

#if (APP_ADV_WHITELIST_ENABLE == true) || (APP_ADV_DIRECTED_MODE_ENABLE == true)
    switch(bleAdvEvent)
    {
        case BLE_ADV_EVT_PEER_ADDR_REQUEST:
            ADV_DEBUG("[Advertiser] advEventHandler() > BLE_ADV_EVT_PEER_ADDR_REQUEST received ...\r\n");
            Factory::peripheralController.onAdvPeerAddrReq();
            break;

        case BLE_ADV_EVT_WHITELIST_REQUEST:
            ADV_DEBUG("[Advertiser] advEventHandler() > BLE_ADV_EVT_WHITELIST_REQUEST received...\r\n");
            Factory::peripheralController.onAdvWhitelistReq();
            break;

        default:
            Factory::peripheralController.onAdvStateChanged(bleAdvEvent);
            break;
    }
#else
    Factory::peripheralController.onAdvStateChanged(bleAdvEvent);
#endif
}

//------------------------------------------------------------------------------------------------------------
void ble::advErrorHandler(uint32_t nrfError)
{
    Factory::peripheralController.onAdvError(nrfError);
}

/************************************************************************************************************/
/* CONSTRUCTOR                                                                                              */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
Advertiser::Advertiser() : _txPowerLevel(static_cast<int8_t>(TX_POWER_LEVEL)), _timeout(0), 
                           _advertiseUUIDsCount(0), _scanResponseUUIDsCount(0)
{
    // Register a handler for SoftDevice BLE events.
    ADV_DEBUG("[Advertiser] Advertiser() > Register SoftDevice BLE observer...\r\n");
    NRF_SDH_BLE_OBSERVER_CPP(_advParamsBleObs, BLE_ADV_BLE_OBSERVER_PRIO, ble_advertising_on_ble_evt, &_advertisingInstance);

    // Register a handler for SoftDevice SoC events.
    ADV_DEBUG("[Advertiser] Advertiser() > Register SoftDevice SoC observer...\r\n");
    NRF_SDH_SOC_OBSERVER_CPP(_advParamsSocObs, BLE_ADV_SOC_OBSERVER_PRIO, ble_advertising_on_sys_evt, &_advertisingInstance);
}

//------------------------------------------------------------------------------------------------------------
Advertiser::~Advertiser()
{
}


/************************************************************************************************************/
/* PUBLIC SECTION                                                                                           */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
void Advertiser::initialize(bool    whitelistModeEnabled,
                            bool    directedSlowModeEnabled,
                            bool    directedFastModeEnabled,
                            bool    fastModeEnabled,
                            bool    slowModeEnabled,
                            bool    extendedModeEnabled,
                            uint8_t connectionConfigTag,
                            bool    alternateTimings)
{    
    // Basic initialisation
    ADV_DEBUG("[Advertiser] initialize() > Setting up some basic initialisation data...\r\n");
    _advertisingInstance.adv_mode_current                           = BLE_ADV_MODE_IDLE;
    _advertisingInstance.conn_cfg_tag                               = connectionConfigTag;
    _advertisingInstance.evt_handler                                = ble::advEventHandler;
    _advertisingInstance.error_handler                              = ble::advErrorHandler;
    _advertisingInstance.current_slave_link_conn_handle             = BLE_CONN_HANDLE_INVALID;
    _advertisingInstance.p_adv_data                                 = &_advertisingInstance.adv_data;
    _advertisingInstance.adv_handle                                 = BLE_GAP_ADV_SET_HANDLE_NOT_SET;
    _advertisingInstance.adv_data.adv_data.p_data                   = _advertisingInstance.enc_advdata;
    _advertisingInstance.adv_data.adv_data.len                      = BLE_GAP_ADV_SET_DATA_SIZE_MAX;
   
    // Clear peer address
    memset(&_advertisingInstance.peer_address, 0, sizeof(_advertisingInstance.peer_address));

    // Set up the interval timings for all advertising modes.
    ADV_DEBUG("[Advertiser] initialize() > Setting up the interval timings for all advertising modes...\r\n");
    memset(&_advertisingInstance.adv_modes_config, 0, sizeof(ble_adv_modes_config_t));
    if(alternateTimings)
    {
        _advertisingInstance.adv_modes_config.ble_adv_directed_timeout  = APP_ALT_ADV_DIRECTED_TIMEOUT;
        _advertisingInstance.adv_modes_config.ble_adv_directed_interval = APP_ALT_ADV_DIRECTED_INTERVAL;
        _advertisingInstance.adv_modes_config.ble_adv_fast_timeout      = APP_ALT_ADV_FAST_TIMEOUT;
        _advertisingInstance.adv_modes_config.ble_adv_fast_interval     = APP_ALT_ADV_FAST_INTERVAL;
        _advertisingInstance.adv_modes_config.ble_adv_slow_timeout      = APP_ALT_ADV_SLOW_TIMEOUT;
        _advertisingInstance.adv_modes_config.ble_adv_slow_interval     = APP_ALT_ADV_SLOW_INTERVAL;
    }
    else
    {
        _advertisingInstance.adv_modes_config.ble_adv_directed_timeout  = APP_ADV_DIRECTED_TIMEOUT;
        _advertisingInstance.adv_modes_config.ble_adv_directed_interval = APP_ADV_DIRECTED_INTERVAL;
        _advertisingInstance.adv_modes_config.ble_adv_fast_timeout      = APP_ADV_FAST_TIMEOUT;
        _advertisingInstance.adv_modes_config.ble_adv_fast_interval     = APP_ADV_FAST_INTERVAL;
        _advertisingInstance.adv_modes_config.ble_adv_slow_timeout      = APP_ADV_SLOW_TIMEOUT;
        _advertisingInstance.adv_modes_config.ble_adv_slow_interval     = APP_ADV_SLOW_INTERVAL;
    }
    setupParameters(whitelistModeEnabled, directedSlowModeEnabled, directedFastModeEnabled, fastModeEnabled, slowModeEnabled, extendedModeEnabled, false);

    // Setup Advertise and Scan Response data
    ADV_DEBUG("[Advertiser] initialize() > Setting up advertise data...\r\n");
    _advertiseData.name_type                                        = BLE_ADVDATA_FULL_NAME;
    _advertiseData.include_appearance                               = true;
    _advertiseData.p_tx_power_level                                 = &_txPowerLevel;
    
    // Manufacturer DATA...
#if defined(COMPANY_IDENTIFIER) && (COMPANY_IDENTIFIER != 0)
    // Update the manufactuer data according to the project
    ADV_DEBUG("[Advertiser] initialize() > Setting up the manufacturer data...\r\n");
    #if defined(HEI)
    _manufacturerSpecificData.company_identifier                    = ManufacturerData::HEI_COMPANY_ID;
    #elif defined(SOPROD)
    _manufacturerSpecificData.company_identifier                    = ManufacturerData::SOPROD_COMPANY_ID;
    #else
    _manufacturerSpecificData.company_identifier                    = ManufacturerData::NORDIC_COMPANY_ID;
    #endif
    _manufacturerSpecificData.data.p_data                           = _manufacturerData.data();
    _manufacturerSpecificData.data.size                             = sizeof(_manufacturerData);
    _advertiseData.p_manuf_specific_data                            = &_manufacturerSpecificData;
#else
    _advertiseData.p_manuf_specific_data                            = NULL;
#endif
    
    // Copy advertising and scan response data.
    _advertisingInstance.adv_data.adv_data.p_data                   = _advertisingInstance.enc_advdata;
    
    // Configure a initial advertising configuration. The advertising data and and advertising parameters will be 
    // changed later when we call @ref ble_advertising_start, but must be set to legal values here to define an 
    // advertising handle.
    _advertisingInstance.adv_params.primary_phy                     = BLE_GAP_PHY_1MBPS;
    _advertisingInstance.adv_params.duration                        = _advertisingInstance.adv_modes_config.ble_adv_fast_timeout;
    _advertisingInstance.adv_params.properties.type                 = BLE_GAP_ADV_TYPE_CONNECTABLE_SCANNABLE_UNDIRECTED;
    _advertisingInstance.adv_params.p_peer_addr                     = NULL;
    _advertisingInstance.adv_params.filter_policy                   = BLE_GAP_ADV_FP_ANY;
    _advertisingInstance.adv_params.interval                        = _advertisingInstance.adv_modes_config.ble_adv_fast_interval;
    
    _advertisingInstance.initialized                                = _updateAdvertisingSet();
    ADV_DEBUG("[Advertiser] initialize() > Advertise initialisation %s\r\n", _advertisingInstance.initialized ? "done ...":"failed !!!");
}

//------------------------------------------------------------------------------------------------------------
void Advertiser::setupParameters(bool whitelistModeEnabled, 
                                 bool directedSlowModeEnabled,
                                 bool directedFastModeEnabled,
                                 bool fastModeEnabled, 
                                 bool slowModeEnabled, 
                                 bool extendedEnabled,
                                 bool updateAdvertisingSet)
{
    // Disabling the advertising restart coded in ble_advertising.c
    ADV_DEBUG("[Advertiser] setupParameters() > Disable advertising on disconnect...\r\n");
    _advertisingInstance.adv_modes_config.ble_adv_on_disconnect_disabled = true;

    if(isAdvertising())
    {
        stopAdvertising();
    }
    
    // Setup Advertise and Scan Response data
    ADV_DEBUG("[Advertiser] setupParameters() > Update advertise and scan response data...\r\n");
    _advertiseData.flags = (whitelistModeEnabled ? BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE : BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
    
    ADV_DEBUG("[Advertiser] setupParameters() > %s WHITELIST Mode...\r\n", (whitelistModeEnabled ? "Enable" : "Disable"));
    _advertisingInstance.adv_modes_config.ble_adv_whitelist_enabled = whitelistModeEnabled;

    ADV_DEBUG("[Advertiser] setupParameters() > %s DIRECTED HIGH DUTY Mode...\r\n", (directedFastModeEnabled ? "Enable" : "Disable"));
    _advertisingInstance.adv_modes_config.ble_adv_directed_high_duty_enabled = directedFastModeEnabled;

    ADV_DEBUG("[Advertiser] setupParameters() > %s DIRECTED Mode...\r\n", (directedSlowModeEnabled ? "Enable" : "Disable"));
    _advertisingInstance.adv_modes_config.ble_adv_directed_enabled = directedSlowModeEnabled;

    ADV_DEBUG("[Advertiser] setupParameters() > %s FAST Mode...\r\n", (fastModeEnabled ? "Enable" : "Disable"));
    _advertisingInstance.adv_modes_config.ble_adv_fast_enabled = fastModeEnabled;

    ADV_DEBUG("[Advertiser] setupParameters() > %s SLOW Mode...\r\n", (slowModeEnabled ? "Enable" : "Disable"));
    _advertisingInstance.adv_modes_config.ble_adv_slow_enabled = slowModeEnabled;
    
    ADV_DEBUG("[Advertiser] setupParameters() > %s EXTENDED Mode...\r\n", (extendedEnabled ? "Enable" : "Disable"));
    _advertisingInstance.adv_modes_config.ble_adv_extended_enabled = extendedEnabled;
    if(extendedEnabled)
    {
        _advertisingInstance.adv_params.properties.include_tx_power = 1;
    }
    
    if(updateAdvertisingSet)
    {
        ADV_DEBUG("[Advertiser] setupParameters() > Updating new advertiser's parameters in SoftDevice...\r\n");
        _updateAdvertisingSet();
    }
}

//------------------------------------------------------------------------------------------------------------
bool Advertiser::setAdvertiseUUID(uint16_t serviceUUID, uint8_t uuidType, bool inScanResponse)
{
    ble_uuid_t uuids = {serviceUUID, uuidType};

    if(inScanResponse)
    {
        if(_scanResponseUUIDsCount < APP_ADV_MAX_RSP_VENDOR_UUIDS_COUNT)
        {
            _scanResponseUUIDs[_scanResponseUUIDsCount] = uuids;
            _scanResponseUUIDsCount++;
            ADV_DEBUG("[Advertiser] setAdvertiseUUID() > 0x%04x UUID added to the scan response record as type #%d.\r\n", serviceUUID, uuidType);
        }
        else
        {
            ADV_DEBUG("[Advertiser] setAdvertiseUUID() > 0x%04x UUID could not be added to the scan response record.\r\n", serviceUUID);
            return false;
        }
    }
    else
    {
        if(_advertiseUUIDsCount < APP_ADV_MAX_ADV_UUIDS_COUNT)
        {
            _advertiseUUIDs[_advertiseUUIDsCount] = uuids;
            _advertiseUUIDsCount++;
            ADV_DEBUG("[Advertiser] setAdvertiseUUID() > 0x%04x UUID added to the advertisement data as type #%d.\r\n", serviceUUID, uuidType);
        }
        else
        {
            ADV_DEBUG("[Advertiser] setAdvertiseUUID() > 0x%04x UUID could not be added to the advertisement data.\r\n", serviceUUID);
            return false;
        }
    }

    _updateAdvertisingSet();
    return true;
}

//------------------------------------------------------------------------------------------------------------
uint32_t Advertiser::startAdvertising(bool withoutWhitelist, ble_adv_mode_t advertisingMode)
{
    if(withoutWhitelist)
    {
        ADV_DEBUG("[Advertiser] startAdvertising() > Restart advertising without wihtelist...\r\n");
        // As we change teh TX power, we have to re-initialize the advData and so the internal
        // advertising mode is set back to IDLE. That's why the module won't start advertising
        // without the whitelist.
        // The trick here is to start manually to advertive.
        uint32_t errCode = ble_advertising_restart_without_whitelist(&_advertisingInstance);
        if(errCode != NRF_SUCCESS)
        {
            ADV_DEBUG("[Advertiser] startAdvertising() > ble_advertising_restart_without_whitelist() returns an error: 0x%04x\r\n", errCode);
        }
        return errCode;
    }
    else
    {
        if(isAdvertising())
        {
            stopAdvertising();
        }
        ADV_DEBUG("[Advertiser] startAdvertising() > Start advertising...\r\n");
        return ble_advertising_start(&_advertisingInstance, advertisingMode);
    }
}

//------------------------------------------------------------------------------------------------------------
uint32_t Advertiser::peerAddressReply(ble_gap_addr_t* pPeerAddr)
{
    return ble_advertising_peer_addr_reply(&_advertisingInstance, pPeerAddr);
}

//------------------------------------------------------------------------------------------------------------
bool Advertiser::setTxPowerLevel(int8_t txPowerLevel)
{
    if(_txPowerLevel != txPowerLevel)
    {
        if(isAdvertising())
        {
            stopAdvertising();
        }
        _txPowerLevel = txPowerLevel;
        ADV_DEBUG("[Advertiser] setTxPowerLevel() > _txPowerLevel = %d [dBm]\r\n", _txPowerLevel);
        // Setup Advertise and Scan Response data
        ADV_DEBUG("[Advertiser] setTxPowerLevel() > Update advertise and scan response data...\r\n");
        _advertiseData.p_tx_power_level = &_txPowerLevel;
        return _updateAdvertisingSet();
    }
    return true;
}

//------------------------------------------------------------------------------------------------------------
bool Advertiser::updateIntervals(uint32_t fastInterval, uint32_t slowInterval)
{
    if(isAdvertising())
    {
        stopAdvertising();
    }
    
    bool anyChange = false;
    if(_advertisingInstance.adv_modes_config.ble_adv_fast_enabled)
    {
        _advertisingInstance.adv_modes_config.ble_adv_fast_interval = fastInterval;
        anyChange = true;
    }

    if(_advertisingInstance.adv_modes_config.ble_adv_slow_enabled)
    {
        _advertisingInstance.adv_modes_config.ble_adv_slow_interval = slowInterval;
        anyChange = true;
    }

    if(anyChange)
    {
        if(!_updateAdvertisingSet())
        {
            ADV_DEBUG("[Advertiser] updateIntervals() > Could not changed the interval parameters!\r\n");
            return false;
        }
    }
    return anyChange;
}

//------------------------------------------------------------------------------------------------------------
bool Advertiser::updateTimeouts(uint8_t advertiseTimeout, uint32_t slowTimeout, uint32_t fastTimeout)
{
    if(isAdvertising())
    {
        stopAdvertising();
    }
    
    if(_timeout != advertiseTimeout)
    {
        _timeout = advertiseTimeout;
    }

    bool anyChange = false;
    if(_advertisingInstance.adv_modes_config.ble_adv_fast_enabled)
    {
        _advertisingInstance.adv_modes_config.ble_adv_fast_timeout = fastTimeout;
        anyChange = true;
    }

    if(_advertisingInstance.adv_modes_config.ble_adv_slow_enabled)
    {
        _advertisingInstance.adv_modes_config.ble_adv_slow_timeout = slowTimeout;
        anyChange = true;
    }

    if(anyChange)
    {
        if(!_updateAdvertisingSet())
        {
            ADV_DEBUG("[Advertiser] updateTimeouts() > Could not changed the timeouts!\r\n");
            return false;
        }
    }
    return anyChange;
}

//------------------------------------------------------------------------------------------------------------
void Advertiser::setWhitelistModeEnabled(bool enabled)
{
    if(isAdvertising())
    {
        stopAdvertising();
    }
    
    _advertiseData.flags = (enabled ? BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE : BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
    _advertisingInstance.adv_modes_config.ble_adv_whitelist_enabled = enabled;
    _updateAdvertisingSet();
}

//------------------------------------------------------------------------------------------------------------
void Advertiser::setDirectedModeEnabled(bool enabled)
{
    if(isAdvertising())
    {
        stopAdvertising();
    }
    
    if(enabled != _advertisingInstance.adv_modes_config.ble_adv_directed_enabled)
    {
        _advertisingInstance.adv_modes_config.ble_adv_directed_enabled = enabled;
        _updateAdvertisingSet();
    }
}

//------------------------------------------------------------------------------------------------------------
void Advertiser::setFastModeEnabled(bool enabled)
{
    if(isAdvertising())
    {
        stopAdvertising();
    }
    
    if(enabled != _advertisingInstance.adv_modes_config.ble_adv_fast_enabled)
    {
        _advertisingInstance.adv_modes_config.ble_adv_fast_enabled = enabled;
        _updateAdvertisingSet();
    }
}

//------------------------------------------------------------------------------------------------------------
void Advertiser::setSlowModeEnabled(bool enabled)
{
    if(isAdvertising())
    {
        stopAdvertising();
    }
    
    if(enabled != _advertisingInstance.adv_modes_config.ble_adv_slow_enabled)
    {
        _advertisingInstance.adv_modes_config.ble_adv_slow_enabled = enabled;
        _updateAdvertisingSet();
    }
}


/************************************************************************************************************/
/* PRIVATE SECTION                                                                                          */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
bool Advertiser::_updateAdvertisingSet()
{
    // Set up the service's UUIDs to be advertised and Advertising data
    _advertiseData.uuids_complete.uuid_cnt                  = _advertiseUUIDsCount;
    _advertiseData.uuids_complete.p_uuids                   = (_advertiseUUIDsCount == 0 ? NULL : _advertiseUUIDs);
    _advertiseData.include_ble_device_addr                  = ADVERTISE_INCLUDE_DEVICE_ADDR;

    // Set up the service's UUIDs to be advertised in Scan Response
    if(_scanResponseUUIDsCount != 0)
    {
        ADV_DEBUG("[Advertiser] initialize() > Setting up the scan response data ...\r\n");
        memset(&_scanResponseData, 0, sizeof(ble_advdata_t));
        _scanResponseData.uuids_complete.uuid_cnt           = _scanResponseUUIDsCount;
        _scanResponseData.uuids_complete.p_uuids            = _scanResponseUUIDs;
        _advertisingInstance.adv_data.scan_rsp_data.p_data  = _advertisingInstance.enc_scan_rsp_data;
    }
    
    // Populate Advertising Instance...
    _advertisingInstance.adv_data.adv_data.len              = BLE_GAP_ADV_SET_DATA_SIZE_MAX;
    uint32_t errorCode = ble_advdata_encode(&_advertiseData, _advertisingInstance.enc_advdata, &_advertisingInstance.adv_data.adv_data.len);
    if(errorCode != NRF_SUCCESS)
    {
        ADV_DEBUG("[Advertiser] _updateAdvertisingSet() > ble_advdata_encode(_advertiseData) returns an error: 0x%04x\r\n", errorCode);
    }

    if(_scanResponseUUIDsCount != 0)
    {
        _advertisingInstance.adv_data.scan_rsp_data.p_data  = _advertisingInstance.enc_scan_rsp_data;
        _advertisingInstance.adv_data.scan_rsp_data.len     = BLE_GAP_ADV_SET_DATA_SIZE_MAX;
        errorCode = ble_advdata_encode(&_scanResponseData, _advertisingInstance.enc_scan_rsp_data, &_advertisingInstance.adv_data.scan_rsp_data.len);
        if(errorCode != NRF_SUCCESS)
        {
            ADV_DEBUG("[Advertiser] _updateAdvertisingSet() > ble_advdata_encode(_scanResponseData) returns an error: 0x%04x\r\n", errorCode);
        }
    }
    else
    {
        _advertisingInstance.adv_data.scan_rsp_data.p_data  = NULL;
        _advertisingInstance.adv_data.scan_rsp_data.len     = 0;
    }
    
    errorCode = sd_ble_gap_adv_set_configure(&_advertisingInstance.adv_handle, NULL, &_advertisingInstance.adv_params);
    if(errorCode != NRF_SUCCESS)
    {
        ADV_DEBUG("[Advertiser] _updateAdvertisingSet() > sd_ble_gap_adv_set_configure() returns an error: 0x%04x\r\n", errorCode);
    }
    return true;
}


#endif  // #if defined(SOFTDEVICE_PRESENT)
