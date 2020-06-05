/************************************************************************************************************
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 ************************************************************************************************************/

/************************************************************************************************************/
/* Header files                                                                                             */
/************************************************************************************************************/
#include "mhfsservice.h"
#include <string.h>


#if defined(SOFTDEVICE_PRESENT) && (MHFS_SUPPORT != 0)

/************************************************************************************************************/
/* DEBUG COMPILATION OPTION & MACRO                                                                         */
/************************************************************************************************************/
#define DEBUG_MHFS_SERVICE_ENABLE            0
#if (DEBUG_MHFS_SERVICE_ENABLE != 0) && defined(DEBUG_NRF_USER)
    #include <tools/logtrace.h>
    #define MHFS_DEBUG                       LOG_TRACE_DEBUG
#else
    #define MHFS_DEBUG(...)                  {(void) (0);}
#endif

/************************************************************************************************************/
/* Namespace                                                                                                */
/************************************************************************************************************/
using ble::MhfsService;


/************************************************************************************************************/
/* Constructor | Destructor                                                                                 */
/************************************************************************************************************/
// -----------------------------------------------------------------------------------------------------------
MhfsService::MhfsService() {}

// -----------------------------------------------------------------------------------------------------------
MhfsService::~MhfsService() {}

    
/************************************************************************************************************/
/* PUBLIC SECTION                                                                                           */
/************************************************************************************************************/
// -----------------------------------------------------------------------------------------------------------
uint32_t MhfsService::adsValuesUpdated(uint8_t adsId, uint8_t mux, uint8_t pga, bool isFloat, uint32_t value)
{
    static mhfsAdsRecord_t adsValues;

    adsValues.params.adsId      = adsId;
    adsValues.params.mux        = mux;
    adsValues.params.pga        = pga;
    adsValues.params.isFloat    = isFloat;
    adsValues.value.rawValue    = value;
    #if (DEBUG_MHFS_SERVICE_ENABLE != 0) && defined(DEBUG_NRF_USER)
    MHFS_DEBUG("[MhfsService] adsValueUpdated() > config: 0x%02x, value: 0x%08x; ", adsValues.params, adsValues.value);
    uint8_t arrData[sizeof(mhfsAdsRecord_t)] = { 0 };
    memcpy(arrData, (uint8_t*) &adsValues, sizeof(mhfsAdsRecord_t));
    for(uint8_t i = 0; i < sizeof(mhfsAdsRecord_t); i++)
    {
        MHFS_DEBUG("%02x ",arrData[i]);
    }
    MHFS_DEBUG("\r\n");
    #endif // #if (DEBUG_MHFS_SERVICE_ENABLE != 0) && defined(DEBUG_NRF_USER)
    return _adsValuesChar.notifyIndicateValue(_connHandle, sizeof(mhfsAdsRecord_t), (uint8_t*) &adsValues);
}

// -----------------------------------------------------------------------------------------------------------
uint32_t MhfsService::adsConfigurationUpdated(uint8_t adsId, uint16_t config)
{
    static mhfsAdsConfiguration_t adsConfig;
    
    adsConfig.index         = adsId;
    adsConfig.configuration = config;
    MHFS_DEBUG("[MhfsService] adsConfigurationUpdated() > ADS #%02d, configuration: 0x%04x\r\n", adsId, config);
    return _adsValuesChar.notifyIndicateValue(_connHandle, sizeof(mhfsAdsConfiguration_t), (uint8_t*) &adsConfig);
}

// -----------------------------------------------------------------------------------------------------------
mhfsAdsRecord_t* MhfsService::adsValuesCharValue()
{
    if(_adsValuesChar.charVal() != NULL)
    {
        return reinterpret_cast<mhfsAdsRecord_t*>(_adsValuesChar.charVal());
    }
    MHFS_DEBUG("[MhfsService] adsValuesCharValue() > Null Pointer Error !\r\n");
    return NULL;
}

// -----------------------------------------------------------------------------------------------------------
mhfsAdsConfiguration_t* MhfsService::adsConfigurationCharValue()
{
    if(_adsConfigurationChar.charVal() != NULL)
    {
        return reinterpret_cast<mhfsAdsConfiguration_t*>(_adsConfigurationChar.charVal());
    }
    MHFS_DEBUG("[MhfsService] adsConfigurationCharValue() > Null Pointer Error !\r\n");
    return NULL;
}

// -----------------------------------------------------------------------------------------------------------
mhfsCommand_t* MhfsService::commandCharValue()
{
    if(_commandChar.charVal() != NULL)
    {
        return reinterpret_cast<mhfsCommand_t*>(_commandChar.charVal());
    }
    MHFS_DEBUG("[MhfsService] commandCharValue() > Null Pointer Error !\r\n");
    return NULL;
}

// -----------------------------------------------------------------------------------------------------------
void MhfsService::bind(BleController* bleCtrl)
{
    bleCtrl->addObserver(this);
}

// -----------------------------------------------------------------------------------------------------------
void MhfsService::updateSecurityForCharacteristics(Security::eSecurityLevelId securityLevel)
{
    _securityLevel = securityLevel;
    
    _adsConfigurationChar.updateSecurityLevel(_securityLevel);
    _adsValuesChar.updateSecurityLevel(_securityLevel);
    _commandChar.updateSecurityLevel(_securityLevel);
}

// -----------------------------------------------------------------------------------------------------------
void MhfsService::onBleEvent(ble_evt_t const* pBleEvent, void* pContext)
{
    switch(pBleEvent->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
        {
            _connHandle = pBleEvent->evt.gap_evt.conn_handle;
            break;
        }
        case BLE_GAP_EVT_DISCONNECTED:
        {
            _connHandle = BLE_CONN_HANDLE_INVALID;
            break;
        }
        case BLE_GATTS_EVT_WRITE:
        {
            MHFS_DEBUG("[MhfsService] onBleEvent() > BLE_GATTS_EVT_WRITE\r\n");
            onWrite(&pBleEvent->evt.gatts_evt.params.write);
            break;
        }
        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
        {
            uint16_t requestHandle = (pBleEvent->evt.gatts_evt.params.authorize_request.type == BLE_GATTS_AUTHORIZE_TYPE_READ ? \
                                      pBleEvent->evt.gatts_evt.params.authorize_request.request.read.handle : pBleEvent->evt.gatts_evt.params.authorize_request.request.write.handle);
            
            if(requestHandle == adsValuesCharHandle()->value_handle         || requestHandle == adsValuesCharHandle()->cccd_handle          ||
               requestHandle == adsConfigurationCharHandle()->value_handle  || requestHandle == adsConfigurationCharHandle()->cccd_handle   ||
               requestHandle == commandCharHandle()->value_handle           || requestHandle == commandCharHandle()->cccd_handle)
            {
                ble_gatts_rw_authorize_reply_params_t authReply;
                memset(&authReply, 0, sizeof(ble_gatts_rw_authorize_reply_params_t));
                uint8_t reqType = handleReadWriteAuthorizeRequest(pBleEvent, &authReply);

                // Checks the read request first
                if((authReply.params.read.gatt_status == BLE_GATT_STATUS_SUCCESS) && (reqType == BLE_GATTS_AUTHORIZE_TYPE_READ))
                {
                    MHFS_DEBUG("[MhfsService] onBleEvent() > BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST: Forwarding pBleEvent to onRead method for 0x%04x...\r\n", pBleEvent->evt.gatts_evt.params.authorize_request.request.read.uuid.uuid);
                    onRead(&pBleEvent->evt.gatts_evt.params.authorize_request.request.read);
                }

                // If the write operation is authorized, forward the write event
                if((authReply.params.write.gatt_status == BLE_GATT_STATUS_SUCCESS) && (reqType == BLE_GATTS_AUTHORIZE_TYPE_WRITE))
                {
                    MHFS_DEBUG("[MhfsService] onBleEvent() > BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST: Updating charValue on SD for 0x%04x...\r\n", pBleEvent->evt.gatts_evt.params.authorize_request.request.write.uuid.uuid);
                    authReply.params.write.len    = pBleEvent->evt.gatts_evt.params.authorize_request.request.write.len;
                    authReply.params.write.offset = pBleEvent->evt.gatts_evt.params.authorize_request.request.write.offset;
                    authReply.params.write.p_data = pBleEvent->evt.gatts_evt.params.authorize_request.request.write.data;
                }

                // Send the BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST reply with the corresponding gatt code (success or error)
                ret_code_t ret = sd_ble_gatts_rw_authorize_reply(pBleEvent->evt.gap_evt.conn_handle, &authReply);
                if(ret != NRF_SUCCESS)
                {
                    MHFS_DEBUG("[MhfsService] onBleEvent() > BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST: sd_ble_gatts_rw_authorize_reply() FAILED (errCode: 0x%04x)\r\n", ret);
                    ASSERT(false);
                    return;
                }

                // If the write operation is authorized, forward the write event
                if((authReply.params.write.gatt_status == BLE_GATT_STATUS_SUCCESS) && (reqType == BLE_GATTS_AUTHORIZE_TYPE_WRITE))
                {
                    MHFS_DEBUG("[MhfsService] onBleEvent() > BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST: Forwarding pBleEvent to onWrite method for 0x%04x...\r\n", pBleEvent->evt.gatts_evt.params.authorize_request.request.write.uuid.uuid);
                    onWrite(&pBleEvent->evt.gatts_evt.params.authorize_request.request.write);
                }
            }
            break;
        }
        default:
            break;
    }
}

// -----------------------------------------------------------------------------------------------------------
void MhfsService::onSystemEvent(uint32_t systemEvent)
{
    // Nothing to do here !
}

// -----------------------------------------------------------------------------------------------------------
bool MhfsService::addObserver(MhfsObserver* observer)
{
    return ISubject<MhfsObserver>::addObserver(observer);
}    


/************************************************************************************************************/
/* PROTECTED SECTION                                                                                        */
/************************************************************************************************************/
// -----------------------------------------------------------------------------------------------------------
void MhfsService::onRead(ble_gatts_evt_read_t const* pEvtRead)
{
    // ---- ADS Configuration ----
    if(pEvtRead->handle == _adsConfigurationChar.handle()->value_handle)
    {
         MHFS_DEBUG("[MhfsService] onRead() > Read ADS Configuration. (handle: 0x%04x)\r\n", pEvtRead->handle);
    }
    // ---- Command ----
    else if(pEvtRead->handle == _commandChar.handle()->value_handle)
    {
         MHFS_DEBUG("[MhfsService] onRead() > Command. (handle: 0x%04x)\r\n", pEvtRead->handle);
    }
}

// -----------------------------------------------------------------------------------------------------------
void MhfsService::onWrite(ble_gatts_evt_write_t const* pEvtWrite)
{
    // ---- CCCD: ADS Values ----
    if((pEvtWrite->handle == _adsValuesChar.handle()->cccd_handle) && (pEvtWrite->len == BLE_CCCD_VALUE_LEN))
    {
        uint16_t cccdValue = uint16_decode(pEvtWrite->data);
        MHFS_DEBUG("[MhfsService] onWrite() > Notification for ADS#1 Value 1: %d (handle: %d)\r\n", cccdValue, pEvtWrite->handle);
        _adsValuesChar.enableNotificationIndication(cccdValue);
    }

    // ---- ADS Configuration ----
    else if((pEvtWrite->handle == _adsConfigurationChar.handle()->value_handle) && (pEvtWrite->len == sizeof(mhfsAdsConfiguration_t)))
    {
        MHFS_DEBUG("[MhfsService] onWrite() > ADS Configuration: ADS #%02d, config: 0x%04x\r\n", ((mhfsAdsConfiguration_t*) pEvtWrite->data)->index, ((mhfsAdsConfiguration_t*) pEvtWrite->data)->configuration);
        _notifyAdsConfigurationChange((mhfsAdsConfiguration_t*) pEvtWrite->data);
    }
    // ---- ADS Command ----
    else if((pEvtWrite->handle == _commandChar.handle()->value_handle) && (pEvtWrite->len == sizeof(mhfsCommand_t)))
    {
        MHFS_DEBUG("[MhfsService] onWrite() > Command: Action=%d\r\n", ((mhfsCommand_t*) pEvtWrite->data)->commandId);
        _notifyCommandChange((mhfsCommand_t*) pEvtWrite->data);
    }
    else
    {
        MHFS_DEBUG("[MhfsService] onWrite() > Unknown handle: 0x%04x (handle: 0x%02x, len: %d)\r\n", pEvtWrite->uuid.uuid, pEvtWrite->handle, pEvtWrite->len);
    }
}


/************************************************************************************************************/
/* PRIVATE SECTION                                                                                          */
/************************************************************************************************************/
// -----------------------------------------------------------------------------------------------------------
uint32_t MhfsService::_initService()
{
    uint32_t errCode    = NRF_SUCCESS;

    // Initialize service structure
    _connHandle         = BLE_CONN_HANDLE_INVALID;

    // Add Service
    _advertiseUUID      = ADVERTISE_MHFS_UUID;
    _serviceUuid.uuid   = MHFS_SERVICE_UUID;
    errCode             = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &_serviceUuid, &_serviceHandle);
    if(errCode == NRF_SUCCESS)
    {
        // Call add characteristic functions
        errCode        |= _addAdsConfigurationChar(&_adsConfigurationChar);
        errCode        |= _addAdsValuesChar(&_adsValuesChar);
        errCode        |= _addCommandChar(&_commandChar);
    }

    return errCode;
}

// -----------------------------------------------------------------------------------------------------------
uint32_t MhfsService::_addAdsConfigurationChar(GattCharacteristic* bleChar)
{
    #if(USE_USER_DESCRIPTION_FOR_CHAR != 0)
    uint8_t desc[] = MHFS_ADS_CONFIGURATION_CHAR_DESC;
    bleChar->setDescriptor(MHFS_ADS_CONFIGURATION_CHAR, uuidType(), desc, sizeof(desc) - 1);
    #else
    bleChar->setDescriptor(MHFS_ADS_CONFIGURATION_CHAR, uuidType(), NULL, 0);
    #endif
    
    bleChar->setDefinition((uint8_t) sizeof(mhfsAdsConfiguration_t),                                            // Buffer's size of the characteristic's value
                           false,                                                                               // `true` means that the charactersitic's value length can be variable from 1 to `char_len`
                           (uint8_t*) &_adsConfigurationCharValue);                                             // Pointer to the buffer of the characteristic's value
    memset(&_adsConfigurationCharValue, 0, bleChar->charLen());                                                 // Initialise the value of the characteristic
    bleChar->setProperties(false, true, false, false, true, BLE_GATTS_VLOC_USER);                               // set up the properties: writeWithoutResponse, read, notify, indicate, write & inSdStack
    bleChar->setSecurity(false, true, false, _securityLevel);                                                   // set up the security: readAuth, writeAuth, cccdWriteAuth
    return bleChar->addCharacteristic(_serviceHandle);
}

// -----------------------------------------------------------------------------------------------------------
uint32_t MhfsService::_addAdsValuesChar(GattCharacteristic* bleChar)
{
    #if(USE_USER_DESCRIPTION_FOR_CHAR != 0)
    uint8_t desc[] = MHFS_ADS_VAL_CHAR_DESC;
    bleChar->setDescriptor(MHFS_ADS_VAL_CHAR, uuidType(), desc, sizeof(desc) - 1);
    #else
    bleChar->setDescriptor(MHFS_ADS_VAL_CHAR, uuidType(), NULL, 0);
    #endif
    
    bleChar->setDefinition((uint8_t) sizeof(mhfsAdsRecord_t),                                                   // Buffer's size of the characteristic's value
                           false,                                                                                // `true` means that the charactersitic's value length can be variable from 1 to `char_len`
                           NULL);                                                                               // Pointer to the buffer of the characteristic's value
    bleChar->setProperties(false, true, true, false, false, BLE_GATTS_VLOC_STACK);                              // set up the properties: writeWithoutResponse, read, notify, indicate, write & inSdStack
    bleChar->setSecurity(false, false, false, _securityLevel);                                                  // set up the security: readAuth, writeAuth, cccdWriteAuth
    return bleChar->addCharacteristic(_serviceHandle);
}

// -----------------------------------------------------------------------------------------------------------
uint32_t MhfsService::_addCommandChar(GattCharacteristic* bleChar)
{
    #if(USE_USER_DESCRIPTION_FOR_CHAR != 0)
    uint8_t desc[] = MHFS_CMD_CHAR_DESC;
    bleChar->setDescriptor(MHFS_CMD_CHAR, uuidType(), desc, sizeof(desc) - 1);
    #else
    bleChar->setDescriptor(MHFS_CMD_CHAR, uuidType(), NULL, 0);
    #endif

    bleChar->setDefinition((uint8_t) sizeof(mhfsCommand_t),                                                     // Buffer's size of the characteristic's value
                           false,                                                                               // `true` means that the charactersitic's value length can be variable from 1 to `char_len`
                           (uint8_t*) &_commandCharValue);                                                      // Pointer to the buffer of the characteristic's value
    memset(&_commandCharValue, 0, bleChar->charLen());                                                          // Initialise the value of the characteristic
    bleChar->setProperties(false, true, false, false, true, BLE_GATTS_VLOC_USER);                               // set up the properties: writeWithoutResponse, read, notify, indicate, write & inSdStack
    bleChar->setSecurity(false, true, false, _securityLevel);                                                   // set up the security: readAuth, writeAuth, cccdWriteAuth
    return bleChar->addCharacteristic(_serviceHandle);
}

// -----------------------------------------------------------------------------------------------------------
void MhfsService::_notifyAdsConfigurationChange(mhfsAdsConfiguration_t* adsConfiguration)
{
    for(uint8_t i = 0; i < observersCount(); i++)
    {
        if(observer(i) != NULL)
        {
            static_cast<MhfsObserver*>(observer(i))->onConfigurationChange(this, 
                                                                           adsConfiguration->index, 
                                                                           adsConfiguration->configuration);
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
void MhfsService::_notifyCommandChange(mhfsCommand_t* adsCommand)
{
    for(uint8_t i = 0; i < observersCount(); i++)
    {
        if(observer(i) != NULL)
        {
            static_cast<MhfsObserver*>(observer(i))->onCommandChange(this, 
                                                                     adsCommand->commandId);
        }
    }
}

#endif  // #if (MHFS_SUPPORT != 0)
