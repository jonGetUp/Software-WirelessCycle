/************************************************************************************************************
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 ************************************************************************************************************/

/************************************************************************************************************/
/* Header files                                                                                             */
/************************************************************************************************************/
#include "gap.h"

#if defined(SOFTDEVICE_PRESENT)
#include <factory.h>
#include <ble_hci.h>
#include "tools/sdktools.h"

#if (USE_INFORMATION_FROM_UICR != 0)
    #include "uicr-config.h"
    #include <crc16.h>
#endif  // (USE_INFORMATION_FROM_UICR != 0)


/************************************************************************************************************/
/* DEBUG COMPILATION OPTION & MACRO                                                                         */
/************************************************************************************************************/
#define DEBUG_GAP_ENABLE                   0
#if (DEBUG_GAP_ENABLE != 0) && defined(DEBUG_NRF_USER)
    #include <tools/logtrace.h>
    #define GAP_DEBUG                       LOG_TRACE_DEBUG
#else
    #define GAP_DEBUG(...)                  {(void) (0);}
#endif

#define DEBUG_GAP_CPU_ENABLE                0
#if (DEBUG_GAP_CPU_ENABLE != 0) && defined(DEBUG_NRF_USER)
    #include <tools/logtrace.h>
    #define GAP_CPU_DEBUG                   LOG_TRACE_DEBUG
#else
    #define GAP_CPU_DEBUG(...)              {(void) (0);}
#endif

#define DEBUG_DEVICE_NAME                   1
#if (DEBUG_DEVICE_NAME != 0) && defined(DEBUG_NRF_USER)
    #include <tools/logtrace.h>
    #define DEVICE_NAME_DEBUG               LOG_TRACE_DEBUG
#else
    #define DEVICE_NAME_DEBUG(...)          {(void) (0);}
#endif


/************************************************************************************************************/
/* Namespace's Declaration                                                                                  */
/************************************************************************************************************/
using ble::Gap;

/************************************************************************************************************/
/* Declaration section                                                                                      */
/************************************************************************************************************/
#if (GAP_USE_XF_TIMER == 0)
    APP_TIMER_DEF(_bleBusyTimerId);
#endif // #if (GAP_USE_XF_TIMER == 0)


/************************************************************************************************************/
/* --- Extern "C" Section ---                                                                               */
/*     Callback's section                                                                                   */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
void ble::onConnParamsEvent(ble_conn_params_evt_t* pEvent)
{
    if(pEvent->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        GAP_CPU_DEBUG("[Gap] onConnParamsEvent() > BLE_CONN_PARAMS_EVT_FAILED\r\n");
        Factory::peripheralController.onGapConnParamEventFailed();
    }
    #if (DEBUG_GAP_CPU_ENABLE != 0) && defined(DEBUG_NRF_USER)
    else if(pEvent->evt_type == BLE_CONN_PARAMS_EVT_SUCCEEDED)
    {
        GAP_CPU_DEBUG("[Gap] onConnParamsEvent() > BLE_CONN_PARAMS_EVT_SUCCEEDED\r\n");
    }
    #endif  // #if (DEBUG_GAP_ENABLE != 0) && defined(DEBUG_NRF_USER)
}

//------------------------------------------------------------------------------------------------------------
void ble::onConnParamsErrorHandler(uint32_t nrfError)
{
    GAP_CPU_DEBUG("[Gap] connParamsErrorHandler() > Connection param update request failed with Error Code: %d.\r\n", nrfError);
    Factory::peripheralController.onGapConnParamUpdateError(NULL, nrfError);
}

#if (GAP_USE_XF_TIMER == 0)
//------------------------------------------------------------------------------------------------------------
void ble::busyTimeoutHandler(void* pContext)
{
    if(pContext != NULL)
    {
        (static_cast<Gap*>(pContext))->timeoutConnChangeParam();
    }
}
#endif // #if (GAP_USE_XF_TIMER == 0)

/************************************************************************************************************/
/* CONSTRUCTOR                                                                                              */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
Gap::Gap() : _peripheralController(NULL), _isTimerInitialized(false) {}

//------------------------------------------------------------------------------------------------------------
Gap::~Gap() {}

/************************************************************************************************************/
/* PUBLIC SECTION                                                                                           */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
void Gap::initialize(ble::BleController*        bleController,
                     ble::PeripheralController* peripheralController,
                     ble_gap_conn_params_t*     gapConnParams,
                     ble_gap_conn_sec_mode_t*   securityMode,
                     ble_gap_conn_params_t*     specificConnParams)
{
    _peripheralController = peripheralController;

    if((bleController != NULL) && !bleController->addObserver(this))
    {
        GAP_DEBUG("[Gap] initialize() > Impossible to add Gap as Observer of bleController !\r\n");
        ASSERT(false);
    }

    uint32_t errCode = _gapInit(gapConnParams, securityMode, specificConnParams);
    if(errCode != NRF_SUCCESS)
    {
        GAP_DEBUG("[Gap] initialize() > _gapInit() FAILED! errCode: 0x%04x\r\n", errCode);
    }
}

//------------------------------------------------------------------------------------------------------------
void Gap::connParamsInit()
{
    ble_conn_params_init_t connParamInit;

    GAP_DEBUG("[Gap] connParamsInit() ...\r\n");
    memset(&connParamInit, 0, sizeof(connParamInit));

    connParamInit.p_conn_params                   = NULL;
    connParamInit.first_conn_params_update_delay  = FIRST_CONN_PARAMS_UPDATE_DELAY;
    connParamInit.next_conn_params_update_delay   = NEXT_CONN_PARAMS_UPDATE_DELAY;
    connParamInit.max_conn_params_update_count    = MAX_CONN_PARAMS_UPDATE_COUNT;
    connParamInit.start_on_notify_cccd_handle     = BLE_GATT_HANDLE_INVALID;
    connParamInit.disconnect_on_fail              = false;
    connParamInit.evt_handler                     = ble::onConnParamsEvent;
    connParamInit.error_handler                   = ble::onConnParamsErrorHandler;

    uint32_t errCode = ble_conn_params_init(&connParamInit);
    if(errCode != NRF_SUCCESS)
    {
        GAP_DEBUG("[Gap] connParamsInit() > FAILED (error: 0x%04x)\r\n", errCode);
        ASSERT(false);
    }
}

//------------------------------------------------------------------------------------------------------------
void Gap::timeoutConnChangeParam()
{
    _isTimeoutConnParamUpdateScheduled = false;
    _sendConnParamUpdateRequest();
}

//------------------------------------------------------------------------------------------------------------
void Gap::handleConnParamUpdate(bool highReactiv)
{
    GAP_CPU_DEBUG("[Gap] handleConnParamUpdate() > highReactiv: %s\r\n", highReactiv ? "true" : "false");
    _updateForHigherReactivity      = highReactiv;
    if(_connParamUpdateInProgress)
    {
        GAP_CPU_DEBUG("[Gap] handleConnParamUpdate() > A Request is allready pending\r\n");
        _startGapBusyTimer();
    }
    else if(_checkConnectionParameters(highReactiv) == NRF_ERROR_INVALID_PARAM)
    {
        GAP_CPU_DEBUG("[Gap] handleConnParamUpdate() > Sending the update request...\r\n");
        _connParamUpdateInProgress  = true;
        _sendConnParamUpdateRequest();
    }
    else
    {
        GAP_CPU_DEBUG("[Gap] handleConnParamUpdate() > Connection's parameters are allready set as desired... Nothing to do !\r\n");
    }
}

//------------------------------------------------------------------------------------------------------------
void Gap::forceToDisconnect(uint16_t connHandle, bool invalidParameter)
{
    uint8_t disconnectReason = BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION;
    if(invalidParameter)
    {
        disconnectReason     = BLE_HCI_CONN_INTERVAL_UNACCEPTABLE;
    }
    
    uint32_t errCode         = sd_ble_gap_disconnect(connHandle, disconnectReason);
    if(errCode != NRF_SUCCESS)
    {
        GAP_DEBUG("[Gap] forceToDisconnect() > Failed ! Error Code = 0x%04x\r\n",  errCode);
    }
    else
    {
        GAP_DEBUG("[Gap] forceToDisconnect() > Succeeded !!!\r\n");
    }
}

//------------------------------------------------------------------------------------------------------------
void Gap::onBleEvent(ble_evt_t const* pBleEvent, void* pContext)
{
    switch(pBleEvent->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
        {
            GAP_DEBUG("[Gap] onBleEvent() > BLE_GAP_EVT_CONNECTED event.\r\n");
            _isGapReadyToUpdateConnParam = false;
            _notifyGapConnected(pBleEvent->evt.gap_evt.conn_handle);

            //Todo: If QWR module is used, add here nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);"
            break;
        }
        case BLE_GAP_EVT_DISCONNECTED:
        {
            GAP_DEBUG("[Gap] onBleEvent() > BLE_GAP_EVT_DISCONNECTED event.\r\n");
            _notifyGapDisconnected(pBleEvent->evt.gap_evt.conn_handle);
            _disableGapBusyTimer();
            break;
        }
        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
        {
            GAP_DEBUG("[Gap] onBleEvent() > BLE_GAP_EVT_PHY_UPDATE_REQUEST event.\r\n");
            GAP_DEBUG("[Gap] onBleEvent() > \\-> Peer RX preferred PHYS: %d, TX preferred PHYS: %d.\r\n", 
                      pBleEvent->evt.gap_evt.params.phy_update_request.peer_preferred_phys.rx_phys,
                      pBleEvent->evt.gap_evt.params.phy_update_request.peer_preferred_phys.tx_phys);

            ble_gap_phys_t const phys = { BLE_GAP_PHY_AUTO, BLE_GAP_PHY_AUTO };
            uint32_t errCode = sd_ble_gap_phy_update(pBleEvent->evt.gap_evt.conn_handle, &phys);
            if(errCode != NRF_SUCCESS)
            {
                GAP_DEBUG("[Gap] onBleEvent() > BLE_GAP_EVT_PHY_UPDATE_REQUEST sd_ble_gap_phy_update() [FAILED] (Error: 0x%04x)\r\n", errCode);
                ASSERT(false);
            }
            break;
        }    
        case BLE_GAP_EVT_PHY_UPDATE:
        {
            GAP_DEBUG("[Gap] onBleEvent() > BLE_GAP_EVT_PHY_UPDATE event ...\r\n");
            if(pBleEvent->evt.gap_evt.params.phy_update.status == NRF_SUCCESS) 
            {
                GAP_DEBUG("[Gap] onBleEvent() > \\-> RX PHY: %d, TX PHY: %d.\r\n", 
                          pBleEvent->evt.gap_evt.params.phy_update.rx_phy, 
                          pBleEvent->evt.gap_evt.params.phy_update.tx_phy);
            }
            else
            {
                GAP_DEBUG("[Gap] onBleEvent() > PHY update failed !!! (status: %d)\r\n", pBleEvent->evt.gap_evt.params.phy_update.status);
                ASSERT(false);
            }
            break;
        }
        case BLE_GAP_EVT_TIMEOUT:
        {
            #if (DEBUG_GAP_ENABLE != 0) && defined(DEBUG_NRF_USER)
            GAP_DEBUG("[Gap] onBleEvent() > BLE_GAP_EVT_TIMEOUT event. Timeout source: ");
            switch(pBleEvent->evt.gap_evt.params.timeout.src)
            {
                case BLE_GAP_TIMEOUT_SRC_AUTH_PAYLOAD:  GAP_DEBUG((char*) "BLE_GAP_TIMEOUT_SRC_AUTH_PAYLOAD.\r\n"); break;
                case BLE_GAP_TIMEOUT_SRC_SCAN:          GAP_DEBUG((char*) "BLE_GAP_TIMEOUT_SRC_SCAN.\r\n"); break;
                case BLE_GAP_TIMEOUT_SRC_CONN:          GAP_DEBUG((char*) "BLE_GAP_TIMEOUT_SRC_CONN.\r\n"); break;
            }
            #endif // DEBUG_GAP_ENABLE
            break;
        }
        case BLE_GAP_EVT_CONN_PARAM_UPDATE:
        {
            // Saving the current connection's parameters
            uint32_t errCode = sd_ble_gap_ppcp_set(&pBleEvent->evt.gap_evt.params.conn_param_update.conn_params);
            #if (DEBUG_GAP_CPU_ENABLE != 0) && defined(DEBUG_NRF_USER)
            {
                char minInterval[8] = {0};
                char maxInterval[8] = {0};
                sprintf(minInterval, "%3.2f", pBleEvent->evt.gap_evt.params.conn_param_update.conn_params.min_conn_interval * 1.25);
                sprintf(maxInterval, "%3.2f", pBleEvent->evt.gap_evt.params.conn_param_update.conn_params.max_conn_interval * 1.25);
                GAP_CPU_DEBUG("[Gap] onBleEvent() > BLE_GAP_EVT_CONN_PARAM_UPDATE event. `min_interval`: %s [ms] (0x%04x) and `max_interval`: %s [ms] (0x%04x).\r\n",
                              minInterval, pBleEvent->evt.gap_evt.params.conn_param_update.conn_params.min_conn_interval,
                              maxInterval, pBleEvent->evt.gap_evt.params.conn_param_update.conn_params.max_conn_interval);
            }
            #endif
            if(errCode == NRF_SUCCESS)
            {
                GAP_CPU_DEBUG("[Gap] onBleEvent() > BLE_GAP_EVT_CONN_PARAM_UPDATE event. sd_ble_gap_ppcp_set() succeeded !\r\n");
            }
            else
            {
                GAP_CPU_DEBUG("[Gap] onBleEvent() > BLE_GAP_EVT_CONN_PARAM_UPDATE event. sd_ble_gap_ppcp_set() FAILED ! (errCode: 0x%04x)\r\n", errCode);
            }

            GAP_CPU_DEBUG("[Gap] onBleEvent() > BLE_GAP_EVT_CONN_PARAM_UPDATE event => Notify observers about connection's parameters update...\r\n");
            _notifyConnParamUpdate(pBleEvent->evt.gap_evt.conn_handle);
            _connParamUpdateInProgress = false;

            GAP_CPU_DEBUG("[Gap] onBleEvent() > BLE_GAP_EVT_CONN_PARAM_UPDATE event => GAP ready again for a next update request !!!\r\n");
            _isGapReadyToUpdateConnParam = true;
            break;
        }
        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
        {
            GAP_DEBUG("[Gap] onBleEvent() > BLE_GAP_SEC_PARAMS_REQUEST event.\r\n");
            break;
        }
        case BLE_GAP_EVT_PASSKEY_DISPLAY:
        {
            char passkey[PASS_KEY_SIZE + 1] = { 0 };
            memcpy(passkey, pBleEvent->evt.gap_evt.params.passkey_display.passkey, PASS_KEY_SIZE);
            GAP_DEBUG("[Gap] onBleEvent() > BLE_GAP_EVT_PASSKEY_DISPLAY event. Observer notified. Passkey: %s\r\n", passkey);
            _notifyGapPasskeyDisplay(pBleEvent->evt.gap_evt.conn_handle, passkey);
            break;
        }
        case BLE_GAP_EVT_AUTH_STATUS:
        {
            GAP_DEBUG("[Gap] onBleEvent() > BLE_GAP_EVT_AUTH_STATUS event. Status: 0x%04x.\r\n", pBleEvent->evt.gap_evt.params.auth_status.auth_status);
            _notifyGapAuthenticationStatus(pBleEvent->evt.gap_evt.conn_handle, pBleEvent->evt.gap_evt.params.auth_status);
            break;
        }
        case BLE_EVT_USER_MEM_REQUEST:
        {
            uint32_t errCode = sd_ble_user_mem_reply(pBleEvent->evt.gattc_evt.conn_handle, NULL);
            if(errCode != NRF_SUCCESS)
            {
                GAP_DEBUG("[Gap] onBleEvent() > BLE_EVT_USER_MEM_REQUEST sd_ble_user_mem_reply() [FAILED] (Error: 0x%04x)\r\n", errCode);
                ASSERT(false);
            }
            break;//BLE_EVT_USER_MEM_REQUEST
        }
        #if !defined (S112)
        case BLE_GAP_EVT_DATA_LENGTH_UPDATE_REQUEST:
        {
            GAP_DEBUG("[Gap] onBleEvent() > BLE_GAP_EVT_DATA_LENGTH_UPDATE_REQUEST event...\r\n");
            GAP_DEBUG("[Gap] onBleEvent() > +-> peer max RX bytes: %3d, max RX time: %d [us]\r\n", 
                      pBleEvent->evt.gap_evt.params.data_length_update_request.peer_params.max_rx_octets, 
                      pBleEvent->evt.gap_evt.params.data_length_update_request.peer_params.max_rx_time_us);
            GAP_DEBUG("[Gap] onBleEvent() > \\-> peer max TX bytes: %3d, max TX time: %d [us].\r\n", 
                      pBleEvent->evt.gap_evt.params.data_length_update_request.peer_params.max_tx_octets, 
                      pBleEvent->evt.gap_evt.params.data_length_update_request.peer_params.max_tx_time_us);
            
            // Reply the the stat length update request: dlParams set to NULL to use automatic values for all members.
            uint32_t errCode = sd_ble_gap_data_length_update(pBleEvent->evt.gap_evt.conn_handle, NULL, NULL);
            if(errCode != NRF_SUCCESS)
            {
                GAP_DEBUG("[Gap] onBleEvent() > BLE_GAP_EVT_DATA_LENGTH_UPDATE_REQUEST sd_ble_gap_data_length_update() [FAILED] (Error: 0x%04x)\r\n", errCode);
                ASSERT(false);
            }
            break;//BLE_GATTS_EVT_SYS_ATTR_MISSING
        }
        case BLE_GAP_EVT_DATA_LENGTH_UPDATE:
        {
            GAP_DEBUG("[Gap] onBleEvent() > BLE_GAP_EVT_DATA_LENGTH_UPDATE event...\r\n");
            GAP_DEBUG("[Gap] onBleEvent() > +-> effective max RX bytes: %d, max RX time: %d [us]\r\n", 
                      pBleEvent->evt.gap_evt.params.data_length_update.effective_params.max_rx_octets, 
                      pBleEvent->evt.gap_evt.params.data_length_update.effective_params.max_rx_time_us);
            GAP_DEBUG("[Gap] onBleEvent() > \\-> effective max TX bytes: %d, max TX time: %d [us].\r\n", 
                      pBleEvent->evt.gap_evt.params.data_length_update.effective_params.max_tx_octets, 
                      pBleEvent->evt.gap_evt.params.data_length_update.effective_params.max_tx_time_us);
            break;
        }
        #endif //!defined (S112)
        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
        {
            GAP_DEBUG("[Gap] onBleEvent() > BLE_GATTS_EVT_SYS_ATTR_MISSING event.\r\n");
            // No system attributes have been stored.
            uint32_t errCode = sd_ble_gatts_sys_attr_set(pBleEvent->evt.gap_evt.conn_handle, NULL, 0, 0);
            if(errCode != NRF_SUCCESS)
            {
                GAP_DEBUG("[Gap] onBleEvent() > BLE_GATTS_EVT_SYS_ATTR_MISSING sd_ble_gatts_sys_attr_set() [FAILED] (Error: 0x%04x)\r\n", errCode);
                ASSERT(false);
            }
            break;
        }
        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
        {
            GAP_DEBUG("[Gap] onBleEvent() > BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST event.\r\n");
            ble_gatts_rw_authorize_reply_params_t authReply;
            if(pBleEvent->evt.gatts_evt.params.authorize_request.type != BLE_GATTS_AUTHORIZE_TYPE_INVALID)
            {
                if((pBleEvent->evt.gatts_evt.params.authorize_request.request.write.op == BLE_GATTS_OP_PREP_WRITE_REQ) ||
                   (pBleEvent->evt.gatts_evt.params.authorize_request.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_NOW) ||
                   (pBleEvent->evt.gatts_evt.params.authorize_request.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_CANCEL))
                {
                    if(pBleEvent->evt.gatts_evt.params.authorize_request.type == BLE_GATTS_AUTHORIZE_TYPE_WRITE)
                    {
                        authReply.type = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
                    }
                    else
                    {
                        authReply.type = BLE_GATTS_AUTHORIZE_TYPE_READ;
                    }
                    authReply.params.write.gatt_status = APP_FEATURE_NOT_SUPPORTED;
                    uint32_t errCode = sd_ble_gatts_rw_authorize_reply(pBleEvent->evt.gatts_evt.conn_handle, &authReply);
                    if(errCode != NRF_SUCCESS)
                    {
                        GAP_DEBUG("[Gap] onBleEvent() > BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST sd_ble_gatts_rw_authorize_reply() [FAILED] (Error: 0x%04x)\r\n", errCode);
                        ASSERT(false);
                    }
                }
            }
            break;  //BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST
        }
        case BLE_GATTC_EVT_TIMEOUT:
        {
            GAP_DEBUG("[Gap] onBleEvent() > BLE_GATTC_EVT_TIMEOUT event.\r\n");
            break;
        }
        case BLE_GATTS_EVT_TIMEOUT:
        {
            GAP_DEBUG("[Gap] onBleEvent() > BLE_GATTS_EVT_TIMEOUT event.\r\n");
            break;
        }
        case BLE_GAP_EVT_QOS_CHANNEL_SURVEY_REPORT:
        {
            GAP_DEBUG("[Gap] onBleEvent() > BLE_GAP_EVT_QOS_CHANNEL_SURVEY_REPORT event.\r\n");
            break;
        }
        case BLE_GAP_EVT_ADV_SET_TERMINATED:
        {
            GAP_DEBUG("[Gap] onBleEvent() > BLE_GAP_EVT_ADV_SET_TERMINATED event.\r\n");
            break;
        }
        default:
            break;
    }
}

//------------------------------------------------------------------------------------------------------------
void Gap::onSystemEvent(uint32_t systemEvent)
{
}


#if (GAP_USE_XF_TIMER != 0)
/************************************************************************************************************/
/* PROTECTED SECTION                                                                                        */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
EventStatus Gap::processEvent()
{
    switch(getCurrentEvent()->getEventType())
    {
        case IXFEvent::Initial:
            GAP_DEBUG("[Gap] processEvent() > IXFEvent::Initial\r\n");
            break;

        case IXFEvent::Timeout:
            GAP_DEBUG("[Gap] processEvent() > IXFEvent::Timeout:\r\n");
            timeoutConnChangeParam();
            break;

        default:
            break;
    }
    return EventStatus::Consumed;
}
#endif  // #if (GAP_USE_XF_TIMER != 0)

/************************************************************************************************************/
/* PRIVATE SECTION                                                                                          */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
uint32_t Gap::_gapInit(ble_gap_conn_params_t*   stdConnParams, 
                       ble_gap_conn_sec_mode_t* securityMode, 
                       ble_gap_conn_params_t*   specificConnParams)
{
    uint32_t errCode = NRF_SUCCESS;
    
    // Set security mode: no protection, open link
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(securityMode);

    // Set up the device's name
    uint8_t localDevicenameSize = strlen(DEVICE_NAME);
    #if (ADD_BT_ADDR_TO_DEVICE_NAME != 0) && (USE_INFORMATION_FROM_UICR == 0)
    {
        char deviceName[DEVICE_NAME_MAX_SIZE] = { 0 };
        ble_gap_addr_t* btAddress = _peripheralController->btAddress();

        if((localDevicenameSize + 5) > DEVICE_NAME_MAX_SIZE)
        {
            localDevicenameSize = DEVICE_NAME_MAX_SIZE - 5;
        }

        memcpy(deviceName, DEVICE_NAME, localDevicenameSize);
        sprintf(&deviceName[localDevicenameSize], " %02X%02X", btAddress->addr[BLE_GAP_ADDR_LEN - 1], btAddress->addr[BLE_GAP_ADDR_LEN - 2]);
        DEVICE_NAME_DEBUG("[Gap] _gapInit() > deviceName: %s\r\n", deviceName);
        errCode = sd_ble_gap_device_name_set(securityMode, (const uint8_t*) deviceName, localDevicenameSize + 5);
        if(errCode != NRF_SUCCESS)
        {
            GAP_DEBUG("[Gap] _gapInit() > sd_ble_gap_device_name_set() FAILED! errCode: 0x%04x\r\n", errCode);
            ASSERT(false);
        }
    }
    #elif (USE_INFORMATION_FROM_UICR != 0)
    {
        char deviceName[DEVICE_NAME_MAX_SIZE] = { 0 };
        uint32_t serialNumber = 0;

        if((localDevicenameSize + 6) > DEVICE_NAME_MAX_SIZE)
        {
            localDevicenameSize = DEVICE_NAME_MAX_SIZE - 6;
        }

        serialNumber = UICR_GET_SERIAL_NUMBER;
        uint16_t serialCrc = crc16_compute((uint8_t*) &serialNumber, sizeof(uint32_t), NULL);
        if(errCode != NRF_SUCCESS)
        {
            GAP_DEBUG("[Gap] _gapInit() > crc16_compute failed ! (errCode: 0x%04x)\r\n", errCode);
            ASSERT(false);
        }
        memcpy(deviceName, DEVICE_NAME, localDevicenameSize);
        sprintf(&deviceName[localDevicenameSize], " %05u", serialCrc);
        DEVICE_NAME_DEBUG("[Gap] _gapInit() > deviceName: %s\r\n", deviceName);
        errCode = sd_ble_gap_device_name_set(securityMode, (const uint8_t*) deviceName, localDevicenameSize + 6);
        if(errCode != NRF_SUCCESS)
        {
            GAP_DEBUG("[Gap] _gapInit() > sd_ble_gap_device_name_set() FAILED! errCode: 0x%04x\r\n", errCode);
            ASSERT(false);
        }
    }
    #else
    {
        DEVICE_NAME_DEBUG("[Gap] _gapInit() > deviceName: %s\r\n", (const uint8_t*) DEVICE_NAME);
        errCode = sd_ble_gap_device_name_set(securityMode, (const uint8_t*) DEVICE_NAME, strlen(DEVICE_NAME));
        if(errCode != NRF_SUCCESS)
        {
            GAP_DEBUG("[Gap] _gapInit() > sd_ble_gap_device_name_set() FAILED! errCode: 0x%04x\r\n", errCode);
            ASSERT(false);
        }
    }
    #endif
    
    errCode = sd_ble_gap_appearance_set(BLE_APPEARANCE);
    if(errCode != NRF_SUCCESS)
    {
        GAP_DEBUG("[Gap] onConnParamsEvent() > sd_ble_gap_appearance_set() FAILED! errCode: 0x%04x\r\n", errCode);
        ASSERT(false);
    }

    // Setup the GAP connection's parameters
    _lowEnergyConnParams = stdConnParams;
    _highReactConnParams = specificConnParams;
    errCode = sd_ble_gap_ppcp_set(_lowEnergyConnParams);
    if(errCode != NRF_SUCCESS)
    {
        GAP_DEBUG("[Gap] _gapInit() > sd_ble_gap_ppcp_set() FAILED! errCode: 0x%04x\r\n", errCode);
        ASSERT(false);
    }

    // app_timer creation
    if(!_isTimerInitialized)
    {
        #if (GAP_USE_XF_TIMER != 0)
        this->startBehaviour();
        #else
        errCode = tools::SDKTools::appTimerCreate(&_bleBusyTimerId, APP_TIMER_MODE_SINGLE_SHOT, (void*) ble::busyTimeoutHandler);
        if(errCode != NRF_SUCCESS)
        {
            GAP_DEBUG("[Gap] _gapInit() > bleBusyTimer: createTimer() FAILED (error: 0x%04x)\r\n", errCode);
            ASSERT(false);
        }
        #endif // #if (GAP_USE_XF_TIMER != 0)
        _isTimerInitialized = true;
    }

    GAP_DEBUG("[Gap] _gapInit() > ... %s! (ErrCode: 0x%04x)\r\n", (errCode == NRF_SUCCESS ? "Done":"Failed"), errCode); 
    return errCode;
}

//------------------------------------------------------------------------------------------------------------
void Gap::_sendConnParamUpdateRequest()
{
    if(_isGapReadyToUpdateConnParam)
    {
        _isGapReadyToUpdateConnParam = false;
        uint32_t errCode = _changeConnectionParamForReactivity(_updateForHigherReactivity);
        if(errCode == NRF_ERROR_BUSY)
        {
            GAP_CPU_DEBUG("[Gap] _sendConnParamUpdateRequest() > NRF_ERROR_BUSY Procedure already in progress ...\r\n");
            _startGapBusyTimer();
        }
        #if (DEBUG_GAP_CPU_ENABLE != 0) && defined(DEBUG_NRF_USER)
        else if(errCode != NRF_SUCCESS)
        {
            GAP_CPU_DEBUG("[Gap] _sendConnParamUpdateRequest() > errCode: %d (not handled)\r\n", errCode);
        }
        #endif
    }
    else
    {
        GAP_CPU_DEBUG("[Gap] _sendConnParamUpdateRequest() > GAP not ready yet to update param ...\r\n");
        _startGapBusyTimer();
    }
}

//------------------------------------------------------------------------------------------------------------
uint32_t Gap::_changeConnectionParamForReactivity(bool highReactivity)
{
    ble_gap_conn_params_t* newConnParams = _lowEnergyConnParams;
    if(highReactivity && _highReactConnParams != NULL)
    {
       newConnParams = _highReactConnParams;
    }
    
    // Send the update request to the SoftDevice
    uint32_t errCode = ble_conn_params_change_conn_params(_peripheralController->connHandle(), newConnParams);
    #if (DEBUG_GAP_CPU_ENABLE != 0) && defined(DEBUG_NRF_USER)
    if(errCode == NRF_SUCCESS)
    {
        char minInter[8] = { 0 };
        char maxInter[8] = { 0 };
        sprintf(minInter, "%3.2f", newConnParams->min_conn_interval * 1.25);
        sprintf(maxInter, "%3.2f", newConnParams->max_conn_interval * 1.25);
        GAP_CPU_DEBUG("[Gap] _changeConnectionParamForReactivity() > `ble_conn_params_change_conn_params()` succeeded (min_interval = %s [ms] and max_interval = %s [ms])\r\n", minInter, maxInter);
    }
    else
    {
        GAP_CPU_DEBUG("[Gap] _changeConnectionParamForReactivity() > `ble_conn_params_change_conn_params()` FAILED! (errCode: 0x%04x)\r\n", errCode);
    }
    #endif  // #if (DEBUG_GAP_ENABLE != 0)
    return errCode;
}

//------------------------------------------------------------------------------------------------------------
uint32_t Gap::_checkConnectionParameters(bool highReactivity)
{
    // read the actual connection's parameters and checks the result
    ble_gap_conn_params_t gapConnParams;
    uint32_t errCode = sd_ble_gap_ppcp_get(&gapConnParams);
    GAP_CPU_DEBUG("[Gap] _checkConnectionParameters() > current params: min_interval = 0x%04x and max_interval = 0x%04x.\r\n",
              gapConnParams.min_conn_interval, gapConnParams.max_conn_interval);
    if(errCode == NRF_SUCCESS)
    {
        // Checks if the actual connection's parameters are as expected according to the reactivity mode.
        // If they don't match the expected ones, a `NRF_ERROR_INVALID_PARAM` error code is returned, which
        // means a new negotiation must be requested to the BLE stack.
        ble_gap_conn_params_t* desiredConnParams = _lowEnergyConnParams;
        if(highReactivity && _highReactConnParams != NULL)
        {
            desiredConnParams = _highReactConnParams;
        }   
        GAP_CPU_DEBUG("[Gap] _checkConnectionParameters() > desired params: min_interval = 0x%04x and max_interval = 0x%04x.\r\n",
                      desiredConnParams->min_conn_interval, desiredConnParams->max_conn_interval);
        
        if((gapConnParams.min_conn_interval < desiredConnParams->min_conn_interval || gapConnParams.min_conn_interval > desiredConnParams->max_conn_interval) ||
           (gapConnParams.max_conn_interval < desiredConnParams->min_conn_interval || gapConnParams.max_conn_interval > desiredConnParams->max_conn_interval))
        {
            errCode = NRF_ERROR_INVALID_PARAM;
        }
    }
    else
    {
        GAP_CPU_DEBUG("[Gap] _checkConnectionParameters() > sd_ble_gap_ppcp_get() FAILED! (errCode: 0x%04x)\r\n", errCode);
        return errCode;
    }
    GAP_CPU_DEBUG("[Gap] _checkConnectionParameters() > for %s mode: Connection's Parameters are %s range.\r\n", 
                  highReactivity ? "High Reactivity":"Low Energy", errCode == NRF_SUCCESS ? "in":"NOT in");
    return errCode;
}

// ------------------------------------------------------------------------------------------------------------------
void Gap::_setTxPowerToDefaultValue()
{
    if(!tools::SDKTools::setTxPower(BLE_GAP_TX_POWER_ROLE_CONN, _peripheralController->connHandle(), static_cast<int8_t>(TX_POWER_LEVEL)))
    {
        GAP_DEBUG("[Gap] _setTxPowerToDefaultValue() > tools::SDKTools::setTxPower() FAILED! \r\n");
        return;
    }
    GAP_DEBUG("[Gap] _setTxPowerToDefaultValue() > TX Power Level set to %d dBm for BLE_GAP_TX_POWER_ROLE_CONN role.\r\n", TX_POWER_LEVEL);
}

//------------------------------------------------------------------------------------------------------------
void Gap::_notifyConnParamUpdate(uint16_t connHandle)
{
    for(uint8_t i = 0; i < observersCount(); i++)
    {
        if(observer(i) != NULL)
        {
            observer(i)->onGapConnParamUpdate(this, connHandle);
        }
    }
}

//------------------------------------------------------------------------------------------------------------
void Gap::_notifyGapConnected(uint16_t connHandle)
{
    for(uint8_t i = 0; i < observersCount(); i++)
    {
        if(observer(i) != NULL)
        {
            observer(i)->onGapConnected(this, connHandle);
        }
    }
}

//------------------------------------------------------------------------------------------------------------
void Gap::_notifyGapDisconnected(uint16_t connHandle)
{
    for(uint8_t i = 0; i < observersCount(); i++)
    {
        if(observer(i) != NULL)
        {
            observer(i)->onGapDisconnected(this, connHandle);
        }
    }
}

//------------------------------------------------------------------------------------------------------------
void Gap::_notifyGapPasskeyDisplay(uint16_t connHandle, char* passkey)
{
    for(uint8_t i = 0; i < observersCount(); i++)
    {
        if(observer(i) != NULL)
        {
            observer(i)->onGapPasskeyDisplay(this, connHandle, passkey);
        }
    }
}

//------------------------------------------------------------------------------------------------------------
void Gap::_notifyGapAuthenticationStatus(uint16_t connHandle, ble_gap_evt_auth_status_t authStatus)
{
    for(uint8_t i = 0; i < observersCount(); i++)
    {
        if(observer(i) != NULL)
        {
            observer(i)->onGapAuthenticationStatus(this, connHandle, authStatus);
        }
    }
}

//------------------------------------------------------------------------------------------------------------
void Gap::_startGapBusyTimer()
{
    if(!_isTimeoutConnParamUpdateScheduled)
    {
        #if (GAP_USE_XF_TIMER != 0)
        getThread()->scheduleTimeout(1, APP_GAP_BLE_BUSY_DELAY, this);
        #else
        uint32_t errCode = tools::SDKTools::appTimerStart(_bleBusyTimerId, GAP_BLE_BUSY_DELAY, (void*) this);
        if(errCode != NRF_SUCCESS)
        {
            GAP_DEBUG("[Gap]_sendConnParamUpdateRequest() > bleBusyTimer: startTimer() ERROR: 0x%04x\r\n", errCode);
            return;
        }
        #endif
        _isTimeoutConnParamUpdateScheduled = true;
    }
}

//------------------------------------------------------------------------------------------------------------
void Gap::_disableGapBusyTimer()
{
    if(!_isTimeoutConnParamUpdateScheduled)
    {
        _isTimeoutConnParamUpdateScheduled = false;
        #if (GAP_USE_XF_TIMER != 0)
        getThread()->unscheduleTimeout(1, this);
        #else
        tools::SDKTools::appTimerStop(_bleBusyTimerId);
        #endif
    }
}

#endif  // #if defined(SOFTDEVICE_PRESENT)
