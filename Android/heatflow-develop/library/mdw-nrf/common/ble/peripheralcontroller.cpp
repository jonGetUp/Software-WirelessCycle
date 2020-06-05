/************************************************************************************************************
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 ************************************************************************************************************/

/************************************************************************************************************/
/* Header files                                                                                             */
/************************************************************************************************************/
#include "peripheralcontroller.h"
#include <tools/logtrace.h>

#if defined(SOFTDEVICE_PRESENT)
#include <factory.h>
#include <nrf_sdh_ble_ext.h>


/************************************************************************************************************/
/* DEBUG COMPILATION OPTION & MACRO                                                                         */
/************************************************************************************************************/
#define DEBUG_PERIPHERAL_CONTROLLER_ENABLE  0
#if (DEBUG_PERIPHERAL_CONTROLLER_ENABLE != 0) && defined(DEBUG_NRF_USER)
    #define PCTRL_DEBUG                     LOG_TRACE_DEBUG
#else
    #define PCTRL_DEBUG(...)                {(void) (0);}
#endif

#define DEBUG_PERIPHERAL_CTRL_HR_ENABLE     0
#if (DEBUG_PERIPHERAL_CONTROLLER_HR_ENABLE != 0)
    #define PCTRLHR_DEBUG                   LOG_TRACE_DEBUG
#else
    #define PCTRLHR_DEBUG(...)              {(void) (0);}
#endif


/************************************************************************************************************/
/* Namespace's Declaration                                                                                  */
/************************************************************************************************************/
using ble::PeripheralController;

    
/************************************************************************************************************/
/* Declaration section                                                                                      */
/************************************************************************************************************/
APP_TIMER_DEF(_advertiseWithoutWhitelistTimerId);


/************************************************************************************************************/
/* --- Extern "C" Section ---                                                                               */
/*     Callback's section                                                                                   */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
void ble::advertiseWithoutWhitelistTimeoutHandler(void* pContext)
{
    if(pContext != NULL)
    {
        PCTRL_DEBUG("[PeripheralController] advertiseWithoutWhitelistTimeoutHandler() > Atop advertising without whitelist...\r\n");
        (static_cast<PeripheralController*>(pContext))->backFromSkippingWhitelist();
    }
}

//------------------------------------------------------------------------------------------------------------
#if defined(BLE_DB_DISCOVERY_ENABLED) && (BLE_DB_DISCOVERY_ENABLED != 0)
void ble::bleDbDiscoveryDispatch(ble_db_discovery_evt_t* pDbDiscoveryEvent)
{
    Factory::peripheralController.dbDiscoveryEventHandler(pDbDiscoveryEvent);
}
#endif  // #if defined(BLE_DB_DISCOVERY_ENABLED) && (BLE_DB_DISCOVERY_ENABLED != 0)

/************************************************************************************************************/
/* CONSTRUCTOR                                                                                              */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
PeripheralController::PeripheralController() : _connHandle(BLE_CONN_HANDLE_INVALID)
{
    #if (NUMBER_OF_HEI_PRIVATE_SERVICES + NUMBER_OF_SOPROD_PRIVATE_SERVICES > 0)
    for(uint8_t i = 0; i < _nbPrivateServices; i++)
    {
        _privateServices[i] = NULL;
    }
    #endif  // #if (NUMBER_OF_HEI_PRIVATE_SERVICES + NUMBER_OF_SOPROD_PRIVATE_SERVICES > 0)
    _isHighReactivityModeTimerEnabled = false;
}

//------------------------------------------------------------------------------------------------------------
PeripheralController::~PeripheralController()   {}

    
/************************************************************************************************************/
/* PUBLIC SECTION                                                                                           */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
void PeripheralController::initialize(ble_gap_conn_params_t* lowPowerConnParams, 
                                      ble_gap_conn_params_t* highReactivityConnParams)
{
    _bleController               = NULL;
    #if (USE_CALENDAR != 0)
    _calendar                    = NULL;
    #else
    this->startBehaviour();
    #endif  //#if (USE_CALENDAR != 0)
    _securityRequest.isRequested = false;

    uint32_t errCode = tools::SDKTools::appTimerCreate(&_advertiseWithoutWhitelistTimerId, APP_TIMER_MODE_SINGLE_SHOT, (void*) ble::advertiseWithoutWhitelistTimeoutHandler);
    if(errCode != NRF_SUCCESS)
    {
        PCTRL_DEBUG("[PeripheralController] initialize() > advertiseWithoutWhitelistTimer: SDKTools::appTimerCreate() FAILED (error: 0x%04x)\r\n", errCode);
        ASSERT(false);
    }
    
    // Setup the connection's parameters
    if(lowPowerConnParams != NULL)
    {
        memcpy(&_loPowerConnParams, lowPowerConnParams, sizeof(ble_gap_conn_params_t));
    }
    else
    {
        _loPowerConnParams.min_conn_interval = MIN_CONN_INTERVAL;
        _loPowerConnParams.max_conn_interval = MAX_CONN_INTERVAL;
        _loPowerConnParams.slave_latency     = SLAVE_LATENCY;
        _loPowerConnParams.conn_sup_timeout  = CONN_SUP_TIMEOUT;
    }
    
    if(highReactivityConnParams != NULL)
    {
        memcpy(&_hiReactConnParams, highReactivityConnParams, sizeof(ble_gap_conn_params_t));
    }
    else
    {
        _hiReactConnParams.min_conn_interval = REACT_MIN_CONN_INTERVAL;
        _hiReactConnParams.max_conn_interval = REACT_MAX_CONN_INTERVAL;
        _hiReactConnParams.slave_latency     = SLAVE_LATENCY;
        _hiReactConnParams.conn_sup_timeout  = CONN_SUP_TIMEOUT;
    }
    
    // GATT initialization
    _gatt.initialize();
    
    #if defined(BLE_DB_DISCOVERY_ENABLED) && (BLE_DB_DISCOVERY_ENABLED != 0)
    // DB Discovery initialization
    // Register DB Discovery event handler
    NRF_SDH_BLE_OBSERVER_CPP(_dbDiscoveryObs, BLE_DB_DISC_BLE_OBSERVER_PRIO, ble_db_discovery_on_ble_evt, &_dbDiscovery);
    PCTRL_DEBUG("[PeripheralController] initialize() > DB Discovery Observer Registration succeeded!\r\n");
    
    // Initialize DB Discovery module
    errCode = ble_db_discovery_init(ble::bleDbDiscoveryDispatch);
    if(errCode != NRF_SUCCESS)
    {
        PCTRL_DEBUG("[PeripheralController] initialize() > ble_db_discovery_init() FAILED! errCode: 0x%04x\r\n", errCode);
        ASSERT(false);
    }
    
    #endif  // #if defined(BLE_DB_DISCOVERY_ENABLED) && (BLE_DB_DISCOVERY_ENABLED != 0)
}

//------------------------------------------------------------------------------------------------------------
void PeripheralController::bind(BleController* bleCtrl
    #if (USE_CALENDAR != 0)
                              , cal::Calendar* calendar
    #endif
    #if (ANCS_SUPPORT != 0)
                              , notification::ANCSHandler* ancsHandler
    #endif
    #if (AMS_SUPPORT != 0)
                              , media::AMSHandler* amsHandler
    #endif
    )
{
    _bleController  = bleCtrl;
    
    // Old enable() method
    if(_bleController->readBtAddress(&_btaddress) == 0)
    {
        PCTRL_DEBUG("[PeripheralController] bind() > Impossible to read the Bluettoth address of the device\r\n");
    }

    #if (USE_CALENDAR != 0)
    if(calendar == NULL)
    {
        PCTRL_DEBUG("[PeripheralController] bind() > NULL pointer error on calendar !\r\n");
        ASSERT(false);
    }
    _calendar       = calendar;
    #endif
    #if (ANCS_SUPPORT != 0)
    if(ancsHandler == NULL)
    {
        PCTRL_DEBUG("[PeripheralController] bind() > NULL pointer error on ancsHandler !\r\n");
        ASSERT(false);
    }
    _ancsHandler    = ancsHandler;
    #endif
    #if (AMS_SUPPORT != 0)
    if(amsHandler == NULL)
    {
        PCTRL_DEBUG("[PeripheralController] bind() > NULL pointer error on amsHandler !\r\n");
        ASSERT(false);
    }
    _amsHandler = amsHandler;    
    #endif
    
    // Initialize Security
    _security.initialize(_bleController, this, CLEAR_BONDED_INFO, USE_FIXED_PASSKEY);

    // Register BLE services observers
    _bleServices.addObserver(this);
}

//------------------------------------------------------------------------------------------------------------
ble::PrivateService* PeripheralController::getService(uint16_t uuid)
{
    #if (NUMBER_OF_HEI_PRIVATE_SERVICES + NUMBER_OF_SOPROD_PRIVATE_SERVICES > 0)
    for(uint8_t i = 0; i < _nbPrivateServices; i++)
    {
        if((_privateServices[i]) != NULL && 
           (_privateServices[i]->uuid() == uuid))
        {
            return _privateServices[i];
        }    
    }
    #endif  // #if (NUMBER_OF_HEI_PRIVATE_SERVICES + NUMBER_OF_SOPROD_PRIVATE_SERVICES > 0)
    return NULL;
}

//------------------------------------------------------------------------------------------------------------
bool PeripheralController::registerService(ble::PrivateService* service)
{
    #if (NUMBER_OF_HEI_PRIVATE_SERVICES + NUMBER_OF_SOPROD_PRIVATE_SERVICES > 0)
    if(service == NULL)
    {
        PCTRL_DEBUG("[PeripheralController] registerService() > The given service is NULL ! [FAILED]\r\n");
        return false;
    }
    
    for(uint8_t i = 0; i < _nbPrivateServices; i++)
    {
        if(_privateServices[i] == service)
        {
            PCTRL_DEBUG("[PeripheralController] registerService() > The given service is already in !\r\n");
            return true;
        }
        if(_privateServices[i] == NULL)
        {
            _privateServices[i] = service;
            return true;
        }    
    }
    return false;
    #else
    return true;
    #endif  // #if (NUMBER_OF_HEI_PRIVATE_SERVICES + NUMBER_OF_SOPROD_PRIVATE_SERVICES > 0)
}

// ------------------------------------------------------------------------------------------------------------------
void PeripheralController::initSecurityAndGap()
{
    initSecurityAndGap(_persistor.useJustWorks(), _persistor.useMitm(), _persistor.useWhiteList());
}

//------------------------------------------------------------------------------------------------------------
void PeripheralController::initSecurityAndGap(bool useJustWorks, bool useMitm, bool useWhitelist, bool highReactivity)
{
    ble::Security::eSecurityLevelId securityLevelId = _security.securityLevelFromConfig(useJustWorks, useMitm);    
    _advertiser.initialize(_bleController, useWhitelist);
    if(!tools::SDKTools::setTxPower(BLE_GAP_TX_POWER_ROLE_ADV, _advertiser.advertisingHandle(), _advertiser.txPowerLevel()))
    {
        PCTRL_DEBUG("[PeripheralController] initSecurityAndGap() > Impossible to set the Radio's Transmit Power for the Advertiser...\r\n");
        ASSERT(false);
    }
    
    // Initializing the BLE services adopted by the Bluetooth SIG
    _bleServices.initializeAdoptedServices();

    // Initialize all private services
    #if (NUMBER_OF_HEI_PRIVATE_SERVICES + NUMBER_OF_SOPROD_PRIVATE_SERVICES > 0)
    for(uint8_t i = 0; i < _nbPrivateServices; i++)
    {
        PrivateService * serv = _privateServices[i];
        if(serv != NULL)
        {
            #if (NUMBER_OF_SOPROD_PRIVATE_SERVICES > 0)
            serv->initializeService(Factory::soprodVendor.uuidType(), securityLevelId);
            #endif

            #if (NUMBER_OF_HEI_PRIVATE_SERVICES > 0)
            serv->initializeService(Factory::heiVendor.uuidType(), securityLevelId);
            #endif
        }
    }
    #endif  // #if (NUMBER_OF_HEI_PRIVATE_SERVICES + NUMBER_OF_SOPROD_PRIVATE_SERVICES > 0)

    #if (BAS_SUPPORT != 0) && (ADVERTISE_BAS_UUID != 0)
    _advertiser.setAdvertiseUUID(BLE_UUID_BATTERY_SERVICE, BLE_UUID_TYPE_BLE, (ADVERTISE_BAS_UUID == 1 ? false:true));
    #endif
    #if (DIS_SUPPORT != 0) && (ADVERTISE_DIS_UUID != 0)
    _advertiser.setAdvertiseUUID(BLE_UUID_DEVICE_INFORMATION_SERVICE, BLE_UUID_TYPE_BLE, (ADVERTISE_DIS_UUID == 1 ? false:true));
    #endif
    #if (IAS_SUPPORT != 0) && (ADVERTISE_IAS_UUID != 0)
    _advertiser.setAdvertiseUUID(BLE_UUID_IMMEDIATE_ALERT_SERVICE, BLE_UUID_TYPE_BLE, (ADVERTISE_IAS_UUID == 1 ? false:true));
    #endif
    _setAdvertiseForPrivateServices();

    setSecuritySettings(useJustWorks, useMitm, useWhitelist, highReactivity);
    _gap.connParamsInit();
    _gap.addObserver(this);
    
    _security.localDatabaseHasChanged();
}

//------------------------------------------------------------------------------------------------------------
void PeripheralController::setSecuritySettings(bool useJustWorks, bool useMitm, bool useWhitelist, bool highReactivity)
{
    // Save first security configuration in the FLASH
    PCTRL_DEBUG("[PeripheralController] setSecuritySettings() > useJustWorks = %s, useMitm = %s, useWhitelist = %s, highReactivity = %s\r\n", 
                useJustWorks ? "YES":"NO", useMitm ? "YES":"NO", useWhitelist ? "YES":"NO", highReactivity ? "YES":"NO");
    _persistor.setSecuritySettings(useJustWorks, useMitm, useWhitelist);

    // if the device is connected disconnect the device first
    if(isConnected())
    {
        _securityRequest.isRequested    = true;
        _securityRequest.highReactivity  = highReactivity;
        disconnect();
    }
    // otherwise update immediately the security
    else
    {
        _security.setWhitelistUsage(useWhitelist);
        uint32_t errCode = _security.setupSecurityParameter(useJustWorks, useMitm, USE_FIXED_PASSKEY);
        if(errCode != NRF_SUCCESS)
        {
            PCTRL_DEBUG("[PeripheralController] setSecuritySettings() > setupSecurityParameter() FAILED: 0x%04x\r\n", errCode);        
            ASSERT(false);
        }
    
        updateSecurityForCharacteristics();
        if(highReactivity)
        {
            _gap.initialize(_bleController, this, &_hiReactConnParams, &_securityMode);
        }
        else
        {
            _gap.initialize(_bleController, this, &_loPowerConnParams, &_securityMode, &_hiReactConnParams);
        }
        _advertiser.setupParameters(useWhitelist);
    }
}

//------------------------------------------------------------------------------------------------------------
void PeripheralController::updateSecurityForCharacteristics()
{
    #if (NUMBER_OF_HEI_PRIVATE_SERVICES + NUMBER_OF_SOPROD_PRIVATE_SERVICES > 0)
    for(uint8_t i = 0; i < _nbPrivateServices; i++)
    {
        PrivateService * serv = _privateServices[i];
        if(serv != NULL)
        {
            serv->updateSecurityForCharacteristics(_security.getSecurityLevelId());
        }
    }
    #endif  // #if (NUMBER_OF_HEI_PRIVATE_SERVICES + NUMBER_OF_SOPROD_PRIVATE_SERVICES > 0)
}

//------------------------------------------------------------------------------------------------------------
bool PeripheralController::setTxPowerLevel(int8_t level, uint8_t role)
{
    int8_t localLevel = tools::SDKTools::getTxPower(role);
    switch(role)
    {
        case BLE_GAP_TX_POWER_ROLE_ADV:
        {
            if(level == _advertiser.txPowerLevel() && level == localLevel)
            {
                PCTRL_DEBUG("[PeripheralController] setTxPowerLevel() > Advertising radio's transmit power is already set to %3dBm\r\n", level);
                return true;
            }
            
            uint32_t errCode = _advertiser.stopAdvertising();
            if(errCode == NRF_SUCCESS || errCode == NRF_ERROR_INVALID_STATE)
            {
                if(_advertiser.setTxPowerLevel(level))
                {
                    if(tools::SDKTools::setTxPower(role, _advertiser.advertisingHandle(), level))
                    {
                        if(errCode == NRF_SUCCESS)
                        {
                            _advertiser.startAdvertising();
                        }
                        return true;
                    }
                }
                else
                {
                    PCTRL_DEBUG("[PeripheralController] setTxPowerLevel() > _advertiser.setTxPowerLevel(level) FAILED\r\n");
                }
            }
            else
            {
                PCTRL_DEBUG("[PeripheralController] setTxPowerLevel() > _advertiser.stopAdvertising() returns 0x%04x\r\n", errCode);
            }
            break;
        }
        case BLE_GAP_TX_POWER_ROLE_CONN:
        {
            if(level == localLevel)
            {
                PCTRL_DEBUG("[PeripheralController] setTxPowerLevel() > Connection radio's transmit power is already set to %3dBm\r\n", level);
                return true;
            }
            
            if(_connHandle != BLE_CONN_HANDLE_INVALID)
            {
                return tools::SDKTools::setTxPower(role, _connHandle, level);
            }
            else
            {
                PCTRL_DEBUG("[PeripheralController] setTxPowerLevel() > Peripheral not connected !\r\n");
            }
            break;
        }
        case BLE_GAP_TX_POWER_ROLE_SCAN_INIT:
        {
            if(level == localLevel)
            {
                PCTRL_DEBUG("[PeripheralController] setTxPowerLevel() > ScanInit radio's transmit power is already set to %3dBm\r\n", level);
                return true;
            }
            
            return tools::SDKTools::setTxPower(role, _connHandle, level);
        }
    }
    return false;
}


//------------------------------------------------------------------------------------------------------------
// BleAdvObserver interface
//------------------------------------------------------------------------------------------------------------
void PeripheralController::onAdvStateChanged(ble_adv_evt_t bleAdvEvent)
{
    #if (ADVERTISE_FOREVER != 0) && (START_ADVERTISE_AUTO != 0)
    if(bleAdvEvent == BLE_ADV_EVT_IDLE)
    {
        startAdvertising();
        return;
    }
    #endif  // #if (ADVERTISE_FOREVER != 0) && (START_ADVERTISE_AUTO != 0)
    _notifyAdvState(bleAdvEvent);
}

//------------------------------------------------------------------------------------------------------------
void PeripheralController::onAdvError(uint32_t nrfError)
{
    if(nrfError != NRF_ERROR_INVALID_STATE)
    {
        PCTRL_DEBUG("[PeripheralController] onAdvError() > nrfError: 0x%04x\r\n", nrfError);
        _notifyAdvError(nrfError);
    }
}

//------------------------------------------------------------------------------------------------------------
void PeripheralController::onAdvWhitelistReq()
{
    _addressCount    = BLE_GAP_WHITELIST_ADDR_MAX_COUNT;
    _irkCount        = BLE_GAP_WHITELIST_ADDR_MAX_COUNT;
    uint32_t errCode = _security.getWhitelist(_whitelistAddress, &_addressCount, _whitelistIrks, &_irkCount);
    if(errCode      != NRF_SUCCESS)
    {
        PCTRL_DEBUG("[PeripheralController] onAdvWhitelistReq() > _security.createWhitelist() ERROR: 0x%04x\r\n", errCode);
        ASSERT(false);
    }
    else
    {
        PCTRL_DEBUG("[PeripheralController] onAdvWhitelistReq() > _security.createWhitelist() SUCCEEDED. (addr_count: %d)\r\n", _addressCount);
    }

    // Apply the whitelist on the advertiser
    errCode        = _advertiser.whitelistReply(_whitelistAddress, _addressCount, _whitelistIrks, _irkCount);
    if(errCode    != NRF_SUCCESS)
    {
        PCTRL_DEBUG("[PeripheralController] onAdvWhitelistReq() > _advertiser.whitelistReply() ERROR: 0x%04x\r\n", errCode);
        ASSERT(false);
    }
}

////------------------------------------------------------------------------------------------------------------
//void PeripheralController::onAdvPeerAddrReq()
//{
//    PCTRL_DEBUG("[PeripheralController] onAdvPeerAddrReq()\r\n");
//    _security.peerAddressRequest();
//}


//------------------------------------------------------------------------------------------------------------
// BleGapObserver interface
//------------------------------------------------------------------------------------------------------------
void PeripheralController::onGapConnParamEventFailed()
{
    PCTRLHR_DEBUG("[PeripheralController] onGapConnParamEventFailed() -> device will disconnect with reason: BLE_HCI_CONN_INTERVAL_UNACCEPTABLE.\r\n");
    disconnect(true);
}

//------------------------------------------------------------------------------------------------------------
void PeripheralController::onGapConnParamUpdate(Gap* gap, uint16_t connHandle)
{
    PCTRLHR_DEBUG("[PeripheralController] onGapConnParamUpdate()\r\n");
    if(gap->isInHighReactivityMode())
    {
        _notifyGapState(PERIPHERAL_STATE_HIGH_REACTIVITY);
        _highReactivityCounter = HIGH_REACTIVITY_DELAY_COUNTER;
        _handleHighReactivityModeTimer(true);
    }
    else
    {
        _handleHighReactivityModeTimer(false);
        _notifyGapState(PERIPHERAL_STATE_LOW_ENERGY);
    }
}

//------------------------------------------------------------------------------------------------------------
void PeripheralController::onGapConnParamUpdateError(Gap* gap, uint32_t errCode)
{
    PCTRLHR_DEBUG("[PeripheralController] onGapConnParamUpdateError() ERROR: 0x%04x\r\n", errCode);
    if(errCode == NRF_ERROR_INVALID_DATA)
    {
        PCTRLHR_DEBUG("[PeripheralController] onGapConnParamUpdateError() -> device will disconnect with reason: BLE_HCI_CONN_INTERVAL_UNACCEPTABLE.\r\n");
        disconnect(true);
    }
}

//------------------------------------------------------------------------------------------------------------
void PeripheralController::onGapConnected(Gap* gap, uint16_t connHandle)
{
    PCTRL_DEBUG("[PeripheralController] onGapConnected()\r\n");
    _connHandle = connHandle;
    tools::SDKTools::appTimerStop(_advertiseWithoutWhitelistTimerId);
    
    // Set the radio's transmit power for connection's state.
    if(!setTxPowerLevel(static_cast<int8_t>(TX_POWER_LEVEL), BLE_GAP_TX_POWER_ROLE_CONN))
    {
        PCTRL_DEBUG("[PeripheralController] onGapConnected() > Connection radio's transmit power could not be set to %3d !\r\n", TX_POWER_LEVEL);
        ASSERT(false);
    }
    _notifyConnectionState(true);
}

//------------------------------------------------------------------------------------------------------------
void PeripheralController::onGapDisconnected(Gap* gap, uint16_t connHandle)
{
    PCTRL_DEBUG("[PeripheralController] onGapDisconnected()\r\n");
    _connHandle = BLE_CONN_HANDLE_INVALID;
    if(_securityRequest.isRequested)
    {
        _securityRequest.isRequested = false;
        setSecuritySettings(_persistor.useJustWorks(), _persistor.useMitm(), _persistor.useWhiteList(), 
                            _securityRequest.highReactivity);
    }
    _handleHighReactivityModeTimer(false);
    _useHighReactivityTimer = true;
    _notifyConnectionState(false);
    
    #if (AMS_SUPPORT != 0)
    _amsHandler->resetServiceDiscoverd();
    #endif
    #if (ANCS_SUPPORT != 0)
    _ancsHandler->resetServiceDiscoverd();
    #endif
    
    #if (ADVERTISE_FOREVER != 0) && (START_ADVERTISE_AUTO != 0)
    PCTRL_DEBUG("[PeripheralController] onGapDisconnected() > startAdvertising...\r\n");
    startAdvertising();
    #endif
}

//------------------------------------------------------------------------------------------------------------
void PeripheralController::onGapPasskeyDisplay(Gap* gap, uint16_t connHandle, char* passkey)
{
    PCTRL_DEBUG("[PeripheralController] onGapPasskeyDisplay() > passkey: %s\r\n", passkey);
    _notifyPasskeyDisplay(passkey);
}

//------------------------------------------------------------------------------------------------------------
void PeripheralController::onGapAuthenticationStatus(Gap* gap, uint16_t connHandle, ble_gap_evt_auth_status_t authStatus)
{
    PCTRL_DEBUG("[PeripheralController] onGapAuthenticationStatus() > status 0x%04x\r\n", authStatus.auth_status);
    _notifyAuthStatus(authStatus);
    if(authStatus.auth_status != BLE_GAP_SEC_STATUS_SUCCESS)
    {
        disconnect();
    }
}

//------------------------------------------------------------------------------------------------------------
// BleServicesObserver interface implementation
#if (IAS_SUPPORT != 0)
//------------------------------------------------------------------------------------------------------------
void PeripheralController::onImmediateAlertChanged(uint8_t alertLevel)
{
    PCTRL_DEBUG("[PeripheralController] onImmediateAlertChanged() > New Alert Level is %d.\r\n", alertLevel);
    _notifyOnImmediateAlertStateChanged(alertLevel
}
#endif


//------------------------------------------------------------------------------------------------------------
// Advertiser related
void PeripheralController::startAdvertising(bool withoutWhitelist, ble_adv_mode_t advertisingMode)
{
    uint32_t errCode;
    if(withoutWhitelist)
    {
        _skipWhitelist = true;
        if(!setTxPowerLevel(static_cast<int8_t>(TX_POWER_LEVEL_WITHOUT_WHITELIST), BLE_GAP_TX_POWER_ROLE_ADV))
        {
            PCTRL_DEBUG("[PeripheralController] startAdvertising() > setTxPowerLevel() FAILED for BLE_GAP_TX_POWER_ROLE_ADV !!!\r\n");
            ASSERT(false);
        }
        if((errCode = tools::SDKTools::appTimerStart(_advertiseWithoutWhitelistTimerId, ADV_WITHOUT_WHITELIST_DELAY, (void*) this)) != NRF_SUCCESS)
        {
            PCTRL_DEBUG("[PeripheralController] startAdvertising() > advertiseWithoutWhitelistTimer: SDKTools::appTimerStart() ERROR: 0x%04x\r\n", errCode);
            ASSERT(false);
        }
    }
    else if(!_skipWhitelist)
    {
        if(!setTxPowerLevel(static_cast<int8_t>(TX_POWER_LEVEL), BLE_GAP_TX_POWER_ROLE_ADV))
        {
            PCTRL_DEBUG("[PeripheralController] startAdvertising() > setTxPowerLevel() failed for BLE_GAP_TX_POWER_ROLE_ADV\r\n");
            ASSERT(false);
        }
    }

    #if (ADVERTISE_WHITELIST_EMPTY != 0)
    if(withoutWhitelist || !_security.isWhitelistEmpty())
    #endif
    {
        errCode = _advertiser.startAdvertising(withoutWhitelist, advertisingMode);
        if(errCode != NRF_SUCCESS)
        {
            PCTRL_DEBUG("[PeripheralController] startAdvertising() > ERROR: 0x%04x\r\n", errCode);
        }
    }
}

//------------------------------------------------------------------------------------------------------------
// Advertiser related
void PeripheralController::backFromSkippingWhitelist()
{
    _skipWhitelist = false;
    _advertiser.clearSkipWhitelistFlag();
    if(!isConnected())
    {
        startAdvertising();
    }
}

//------------------------------------------------------------------------------------------------------------
void PeripheralController::setHighReactivityMode(bool useHighReactivityTimer)
{
    if(!isConnected()) 
    {
        PCTRLHR_DEBUG("[PeripheralController] setHighReactivityMode() > Peripheral is NOT connected !\r\n");
    }
    else
    {
        PCTRLHR_DEBUG("[PeripheralController] setLowEnergyMode() > Peripheral should go in High Reactivity mode %susing a timer...\r\n", useHighReactivityTimer ? "":"WITHOUT ");
        _useHighReactivityTimer = useHighReactivityTimer;
        _gap.handleConnParamUpdate(true);
    }
}

//------------------------------------------------------------------------------------------------------------
void PeripheralController::setLowEnergyMode()
{
    if(!isConnected()) 
    {
        PCTRLHR_DEBUG("[PeripheralController] setLowEnergyMode() > Peripheral is NOT connected !\r\n");
    }
    else
    {
        PCTRLHR_DEBUG("[PeripheralController] setLowEnergyMode() > Peripheral should go in Low Energy Mode...\r\n");
        _gap.handleConnParamUpdate(false);
    }
}

#if (USE_CALENDAR == 0)
//------------------------------------------------------------------------------------------------------------
void PeripheralController::resetHighReactivityCounter()
{
    if(_isHighReactivityModeTimerEnabled)
    {
        _isHighReactivityModeTimerEnabled = false;
        getThread()->unscheduleTimeout(1, this);
    }

    _isHighReactivityModeTimerEnabled = true;
    getThread()->scheduleTimeout(1, HIGH_REACTIVITY_DELAY_COUNTER * 1000, this);
}
#endif // #if (USE_CALENDAR == 0)

//------------------------------------------------------------------------------------------------------------
void PeripheralController::disconnect(bool intervalUnacceptable)
{
    _gap.forceToDisconnect(_connHandle, intervalUnacceptable);
}

  
#if defined(BLE_DB_DISCOVERY_ENABLED) && (BLE_DB_DISCOVERY_ENABLED != 0)
// --------------------------------------------------------------------------------------------------------------
uint32_t PeripheralController::startServiceDiscovery(uint16_t connHandle)
{
    if(connHandle != _connHandle)
    {
        PCTRL_DEBUG("[PeripheralController] startServiceDiscovery() > The given connection's handle (0x%04x) isn't equal to the local one (0x%04x)\r\n", connHandle, _connHandle);
    }
        
    uint32_t errCode = ble_db_discovery_start(&_dbDiscovery, connHandle);
    switch(errCode)
    {
        case NRF_SUCCESS:
        {
            PCTRL_DEBUG("[PeripheralController] startServiceDiscovery() > ble_db_discovery_start(%d) succeeded !\r\n", connHandle);
            _newDiscoveryNeeded = false;
            break;
        }
        case NRF_ERROR_BUSY:
        {
            PCTRL_DEBUG("[PeripheralController] startServiceDiscovery() > ble_db_discovery_start(%d) failed with NRF_ERROR_BUSY ... A discovery is still active\r\n", connHandle);
            break;
        }
        default:
        {
            PCTRL_DEBUG("[PeripheralController] startServiceDiscovery() > ble_db_discovery_start(%d) failed ! (errCode: 0x%04x)\r\n", connHandle, errCode);
            ASSERT(false);
        }
    }
    return errCode;
}
#endif

#if (USE_CALENDAR != 0)
// --------------------------------------------------------------------------------------------------------------
void PeripheralController::onSecond(cal::Calendar* calendar)
{
    if(_highReactivityCounter > 0)
    {
        PCTRLHR_DEBUG("[PeripheralController] _updateHighReactivityCounter() > _highReactivityCounter: %d\r\n", _highReactivityCounter);
        if(--_highReactivityCounter == 0)
        {
            setLowEnergyMode();
        }
    }
}
#else
//------------------------------------------------------------------------------------------------------------
EventStatus PeripheralController::processEvent()
{
    switch(getCurrentEvent()->getEventType())
    {
        case IXFEvent::Initial:
            PCTRL_DEBUG("[PeripheralController] processEvent() > IXFEvent::Initial\r\n");
            break;

        case IXFEvent::Timeout:
            PCTRL_DEBUG("[PeripheralController] processEvent() > IXFEvent::Timeout:\r\n");
            if(_isHighReactivityModeTimerEnabled)
            {
                setLowEnergyMode();
            }
            break;

        default:
            break;
    }
    return EventStatus::Consumed;
}

#endif

#if (USE_FLASH != 0)
//------------------------------------------------------------------------------------------------------------
uint32_t* PeripheralController::save(uint16_t* size)
{
    *size = sizeof(PeripheralPersistor) / 4;
    PCTRL_DEBUG("[PeripheralController] save() > saving security parameters: useJustWorks = %d, useMitm = %d, useWhiteList = %d \r\n", _persistor.useJustWorks(), _persistor.useMitm(), _persistor.useWhiteList());
    return _persistor.data();
}

//------------------------------------------------------------------------------------------------------------
void PeripheralController::restore(uint32_t* data)
{
    memcpy(_persistor.data(), data, sizeof(PeripheralPersistor));
    PCTRL_DEBUG("[PeripheralController] restore() > restoring security parameters: useJustWorks = %d, useMitm = %d, useWhiteList = %d \r\n", _persistor.useJustWorks(), _persistor.useMitm(), _persistor.useWhiteList());
}
#endif


/************************************************************************************************************/
/* PROTECTED DECALRATION SECTION                                                                            */
/************************************************************************************************************/
#if defined(BLE_DB_DISCOVERY_ENABLED) && (BLE_DB_DISCOVERY_ENABLED != 0)
// -----------------------------------------------------------------------------------------------------------
void PeripheralController::dbDiscoveryEventHandler(ble_db_discovery_evt_t* pDbDiscoveryEvent)
{
    #if (AMS_SUPPORT != 0)
    ble_ams_c_on_db_disc_evt(_amsHandler->getAmsClient(), pDbDiscoveryEvent);
    #endif
    #if (ANCS_SUPPORT != 0)
    ble_ancs_c_on_db_disc_evt(_ancsHandler->getAncsClient(), pDbDiscoveryEvent);
    #endif
    
    if(_newDiscoveryNeeded)
    {
        startServiceDiscovery(_connHandle);
    }
}
#endif

/************************************************************************************************************/
/* PRIVATE SECTION                                                                                          */
/************************************************************************************************************/
// -----------------------------------------------------------------------------------------------------------
void PeripheralController::_handleHighReactivityModeTimer(bool enable)
{
    if(enable)
    {
        if(!_isHighReactivityModeTimerEnabled)
        {
            _isHighReactivityModeTimerEnabled = true;
            if(_useHighReactivityTimer)
            {
                #if (USE_CALENDAR != 0)
                if(_calendar != NULL)
                {
                    PCTRLHR_DEBUG("[PeripheralController] onGapConnParamUpdate() > _calendar->addObserver(this)\r\n");
                    _calendar->addObserver(this);
                }
                #else
                getThread()->scheduleTimeout(1, HIGH_REACTIVITY_DELAY_COUNTER * 1000, this);
                #endif
            }
            else
            {
                PCTRLHR_DEBUG("[PeripheralController] onGapConnParamUpdate() > High Reactivity timer not needed !\r\n");
            }
        }
    }
    else
    {
        if(_isHighReactivityModeTimerEnabled)
        {
            _isHighReactivityModeTimerEnabled = false;
            #if (USE_CALENDAR != 0)
            if(_calendar != NULL)
            {
                PCTRLHR_DEBUG("[PeripheralController] onGapConnParamUpdate() > _calendar->delObserver(this)\r\n");
                _calendar->delObserver(this);
            }
            #else
                getThread()->unscheduleTimeout(1, this);
            #endif
            _highReactivityCounter = 0;
        }
    }
}

//------------------------------------------------------------------------------------------------------------
void PeripheralController::_setAdvertiseForPrivateServices()
{
    #if (NUMBER_OF_HEI_PRIVATE_SERVICES + NUMBER_OF_SOPROD_PRIVATE_SERVICES > 0)
    for(uint8_t i = 0; i < _nbPrivateServices; i++)
    {
        PrivateService * serv = _privateServices[i];
        if((serv != NULL) && (serv->advertiseUUID() != 0))
        {
            _advertiser.setAdvertiseUUID(serv->uuid(), serv->uuidType(), (serv->advertiseUUID() == 1 ? false:true));
        }
    }
    #endif  // #if (NUMBER_OF_HEI_PRIVATE_SERVICES + NUMBER_OF_SOPROD_PRIVATE_SERVICES > 0)
}

//------------------------------------------------------------------------------------------------------------
void PeripheralController::_notifyAdvState(ble_adv_evt_t bleAdvEvent)
{
    for(uint_fast8_t i = 0; i < observersCount(); i++)
    {
        if(observer(i) != NULL)
        {
            if(bleAdvEvent == BLE_ADV_EVT_IDLE)
            {
                observer(i)->onAdvertiseStopped(this);
            }
            else
            {
                observer(i)->onAdvertiseStateChanged(this, bleAdvEvent);
            }
        }
    }
}

//------------------------------------------------------------------------------------------------------------
void PeripheralController::_notifyAdvError(uint32_t errCode)
{
    for(uint_fast8_t i = 0; i < observersCount(); i++)
    {
        if(observer(i) != NULL)
        {
            observer(i)->onAdvertiseError(this, errCode);
        }
    }
}

//------------------------------------------------------------------------------------------------------------
void PeripheralController::_notifyConnectionState(bool connected)
{
    for(uint_fast8_t i = 0; i < observersCount(); i++)
    {
        if(observer(i) != NULL)
        {
            if(connected)
            {
                observer(i)->onPeripheralConnected(this);
            }
            else
            {
                observer(i)->onPeripheralDisconnected(this);
            }
        }
    }
}

//------------------------------------------------------------------------------------------------------------
void PeripheralController::_notifyGapState(peripheralState_e state)
{
    for(uint_fast8_t i = 0; i < observersCount(); i++)
    {
        if(observer(i) != NULL)
        {
            observer(i)->onPeripheralStateChanged(this, (uint8_t) state);
        }
    }
}

//------------------------------------------------------------------------------------------------------------
void PeripheralController::_notifyPasskeyDisplay(char* passkey)
{
    for(uint_fast8_t i = 0; i < observersCount(); i++)
    {
        if(observer(i) != NULL)
        {
            observer(i)->onPasskeyDisplay(this, passkey);
        }
    }
}

//------------------------------------------------------------------------------------------------------------
void PeripheralController::_notifyAuthStatus(ble_gap_evt_auth_status_t status)
{
    for(uint_fast8_t i = 0; i < observersCount(); i++)
    {
        if(observer(i) != NULL)
        {
            observer(i)->onAuthStatusUpdated(this, status);
        }
    }  
}

//------------------------------------------------------------------------------------------------------------
#if (IAS_SUPPORT != 0)
void PeripheralController::_notifyOnImmediateAlertStateChanged(uint8_t alertLevel)
{
    for(uint_fast8_t i = 0; i < observersCount(); i++)
    {
        if(observer(i) != NULL)
        {
            observer(i)->onPeripheralImmediateAlertChanged(this, alertLevel)
        }
    }
}
#endif

#endif  // #if defined(SOFTDEVICE_PRESENT)
