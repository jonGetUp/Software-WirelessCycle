/************************************************************************************************************
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 ************************************************************************************************************/

/************************************************************************************************************/
/* Header files                                                                                             */
/************************************************************************************************************/
#include "gattcharacteristic.h"

#if defined(SOFTDEVICE_PRESENT)
#include <ble.h>
#include <ble_srv_common.h>
#include <ble_types.h>
#include <nrf_assert.h>

#include <string.h>


/************************************************************************************************************/
/* DEBUG COMPILATION OPTION & MACRO                                                                         */
/************************************************************************************************************/
#define DEBUG_GATT_CHARACTERISTIC_ENABLE    0
#if (DEBUG_GATT_CHARACTERISTIC_ENABLE != 0) && defined(DEBUG_NRF_USER)
    #include <tools/logtrace.h>
    #define GSC_DEBUG                       LOG_TRACE_DEBUG
#else
    #define GSC_DEBUG(...)                  {(void) (0);}
#endif


/************************************************************************************************************/
/* Namespace's Declaration                                                                                  */
/************************************************************************************************************/
using ble::GattCharacteristic;
    
/************************************************************************************************************/
/* Declaration section                                                                                      */
/************************************************************************************************************/


/************************************************************************************************************/
/* --- Extern "C" Section ---                                                                               */
/*     Callback's section                                                                                   */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------


/************************************************************************************************************/
/* CONSTRUCTOR                                                                                              */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
GattCharacteristic::GattCharacteristic()
{
    // Some initialisation
    _isNotificationIndicationEnable = false;
    _hvxType                        = BLE_GATT_HVX_INVALID;

    memset(&_charMetaData, 0, sizeof(ble_gatts_char_md_t));
    memset(&_attrMetaData, 0, sizeof(ble_gatts_attr_md_t));
    memset(&_cccdMetaData, 0, sizeof(ble_gatts_attr_md_t));
    memset(&_attrCharValue, 0, sizeof(ble_gatts_attr_t));
}

//------------------------------------------------------------------------------------------------------------
GattCharacteristic::~GattCharacteristic()
{
}

/************************************************************************************************************/
/* PUBLIC SECTION                                                                                           */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
uint32_t GattCharacteristic::addCharacteristic(uint16_t serviceHandler)
{
    // Add characteristic to the service
    uint32_t errCode = sd_ble_gatts_characteristic_add(serviceHandler, &_charMetaData, &_attrCharValue, &_charHandle);
    if(errCode != NRF_SUCCESS)
    {
        GSC_DEBUG("[GattCharacteristic] addCharacteristic() > characteristic 0x%04x [FAILED] (error: 0x%04x)\r\n",
                  _bleUUID.uuid, errCode);
        ASSERT(false);
    }
    else
    {
        GSC_DEBUG("[GattCharacteristic] addCharacteristic() > characteristic 0x%04x (valueHandle: 0x%04x) [OK].\r\n",
                  _bleUUID.uuid, _charHandle.value_handle);
    }
    return errCode;
}

//------------------------------------------------------------------------------------------------------------
void GattCharacteristic::setDescriptor(uint16_t charUUID, uint8_t uuidType, uint8_t* description, size_t charDescLen)
{
    _bleUUID.type                           = uuidType;
    _bleUUID.uuid                           = charUUID;

    _charMetaData.p_char_user_desc          = description;
    _charMetaData.char_user_desc_max_size   = charDescLen;
    _charMetaData.char_user_desc_size       = charDescLen;
    _charMetaData.p_user_desc_md            = NULL;
    _charMetaData.p_char_pf                 = NULL;
    _charMetaData.p_sccd_md                 = NULL;
}

//------------------------------------------------------------------------------------------------------------
void GattCharacteristic::setDefinition(uint8_t len, bool lenVariable, uint8_t* value)
{
    if(lenVariable)
    {
        _attrMetaData.vlen                  = 1;
        _lastWrittenLen                     = 1;
    }
    else
    {
        _attrMetaData.vlen                  = 0;
        _lastWrittenLen                     = len;
    }

    _attrCharValue.p_uuid                   = &_bleUUID;
    _attrCharValue.p_attr_md                = &_attrMetaData;
    _attrCharValue.init_len                 = 1;
    _attrCharValue.init_offs                = 0;
    _attrCharValue.max_len                  = len;
    _attrCharValue.p_value                  = value;
}

//------------------------------------------------------------------------------------------------------------
void GattCharacteristic::setProperties(bool writeWithoutResponse,
                                       bool read, bool notify, bool indicate, bool write,
                                       uint8_t inSdStack)
{
    _charMetaData.char_props.notify         = 0;
    _charMetaData.char_props.indicate       = 0;
    if(notify)
    {
        _hvxType                            = BLE_GATT_HVX_NOTIFICATION;
        _charMetaData.char_props.notify     = 1;
    }
    else if(indicate)
    {
        _hvxType                            = BLE_GATT_HVX_INDICATION;
        _charMetaData.char_props.indicate   = 1;
    }
    _charMetaData.char_props.read           = (read ? 1 : 0);
    _charMetaData.char_props.write          = (write ? 1 : 0);
    _charMetaData.char_props.write_wo_resp  = (writeWithoutResponse ? 1 : 0);
    _charMetaData.p_cccd_md                 = ((notify || indicate) ? &_cccdMetaData : NULL);

    _attrMetaData.vloc                      = inSdStack;
}

//------------------------------------------------------------------------------------------------------------
void GattCharacteristic::setSecurity(bool readAuth, bool writeAuth, bool cccdWriteAuth, Security::eSecurityLevelId securityLevel)
{
    // Configure the permission for read property of the characteristic
    if(readAuth)
    {
        _attrMetaData.rd_auth = 1;
    }
    else 
    {
        _attrMetaData.rd_auth = 0;
    }

    // Configure the permission for write property of the characteristic
    if(writeAuth)
    {
        _attrMetaData.wr_auth = 1;
    }
    else
    {
        _attrMetaData.wr_auth = 0;
    }

    // Configure the permission for notify/indicate property of the characteristic
    _cccdMetaData.rd_auth     = 0;
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&_cccdMetaData.read_perm);
    if(cccdWriteAuth)
    {
        _cccdMetaData.wr_auth = 1;
    }
    else
    {
        _cccdMetaData.wr_auth = 0;
    }
    
    updateSecurityLevel(securityLevel);
}

//------------------------------------------------------------------------------------------------------------
void GattCharacteristic::updateSecurityLevel(Security::eSecurityLevelId securityLevel)
{
    // Configure the permission for read property of the characteristic
    if(_charMetaData.char_props.read == 0)
    {
        BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&_attrMetaData.read_perm);
    }
    // No security needed
    else if(_attrMetaData.rd_auth == 0 || securityLevel == Security::noSecurityLevelId)
    {
        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&_attrMetaData.read_perm);
    }
    else 
    {
        switch(securityLevel)
        {
            case Security::justWorksLevelId:
                BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&_attrMetaData.read_perm);
                break;
            
            case Security::mitmLevelId:
                BLE_GAP_CONN_SEC_MODE_SET_ENC_WITH_MITM(&_attrMetaData.read_perm);
                break;
        
            default:
                // No security, already set OPEN
                break;
        }
    }

    // Configure the permission for write property of the characteristic
    if(_charMetaData.char_props.write == 0)
    {
        BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&_attrMetaData.write_perm);
    }
    else if(_attrMetaData.wr_auth == 0 || securityLevel == Security::noSecurityLevelId)
    {
        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&_attrMetaData.write_perm);
    }
    else
    {
        switch(securityLevel)
        {
            case Security::justWorksLevelId:
                BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&_attrMetaData.write_perm);
                break;
            
            case Security::mitmLevelId:
                BLE_GAP_CONN_SEC_MODE_SET_ENC_WITH_MITM(&_attrMetaData.write_perm);
                break;
            
            default:
                // No security, already set OPEN
                break;
        }
    }
    
    // Configure the permission for notify/indicate property of the characteristic
    if((_charMetaData.char_props.notify == 0) && (_charMetaData.char_props.indicate == 0))
    {
        BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&_cccdMetaData.write_perm);
    }
    else
    {
        _cccdMetaData.vloc = _attrMetaData.vloc;
        if(_cccdMetaData.wr_auth == 0 || securityLevel == Security::noSecurityLevelId)
        {
            BLE_GAP_CONN_SEC_MODE_SET_OPEN(&_cccdMetaData.write_perm);
        }
        else
        {
            switch(securityLevel)
            {
                case Security::justWorksLevelId:
                    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&_cccdMetaData.write_perm);
                    break;
                
                case Security::mitmLevelId:
                    BLE_GAP_CONN_SEC_MODE_SET_ENC_WITH_MITM(&_cccdMetaData.write_perm);
                    break;
            
            default:
                // No security, already set OPEN
                break;
            }
        }
    }
}

//------------------------------------------------------------------------------------------------------------
void GattCharacteristic::enableNotificationIndication(uint16_t cccdValue)
{
    if((_charMetaData.char_props.notify == 0) && (_charMetaData.char_props.indicate == 0))
    {
        _isNotificationIndicationEnable = false;
        GSC_DEBUG("[GattCharacteristic] enableNotificationIndication() > _isNotificationIndicationEnable = false\r\n");
    }
    else
    {
        _isNotificationIndicationEnable = ((cccdValue & _hvxType) != 0);
        GSC_DEBUG("[GattCharacteristic] enableNotificationIndication() > _isNotificationIndicationEnable is %s\r\n", (((cccdValue & _hvxType) != 0) ? "ENABLED" : "DISABLED"));
    }
}

//------------------------------------------------------------------------------------------------------------
uint32_t GattCharacteristic::getValue(ble_gatts_value_t* pValue)
{
    uint32_t errCode = NRF_SUCCESS;
    if(_attrMetaData.vloc == BLE_GATTS_VLOC_STACK)
    {
        errCode = sd_ble_gatts_value_get(BLE_CONN_HANDLE_INVALID, _charHandle.value_handle, pValue);
        if(errCode != NRF_SUCCESS)
        {
            GSC_DEBUG("[GattCharacteristic] getValue() > sd_ble_gatts_value_get() Error (char 0x%04x, errCode: 0x%04x)\r\n", _bleUUID.uuid, errCode);
        }

        if(pValue->len != _lastWrittenLen)
        {
            errCode = NRF_ERROR_DATA_SIZE;
            GSC_DEBUG("[GattCharacteristic] getValue() > sd_ble_gatts_value_get() Error (char 0x%04x, errCode: NRF_ERROR_DATA_SIZE)\r\n", _bleUUID.uuid);
        }
    }
    else
    {
        pValue->len     = _lastWrittenLen;
        pValue->offset  = _attrCharValue.init_offs;
        pValue->p_value = _attrCharValue.p_value;
    }
    return errCode;
}

//------------------------------------------------------------------------------------------------------------
uint32_t GattCharacteristic::setValue(const uint16_t size, uint8_t* pValue)
{
    uint32_t errCode = NRF_ERROR_DATA_SIZE;

    if((_attrMetaData.vlen && (size <= _attrCharValue.max_len)) || (size == _attrCharValue.max_len))
    {
        errCode = NRF_SUCCESS;
        if(_attrMetaData.vloc == BLE_GATTS_VLOC_STACK)
        {
            ble_gatts_value_t pCharVal = { size, 0, pValue };
            errCode = sd_ble_gatts_value_set(BLE_CONN_HANDLE_INVALID, _charHandle.value_handle, &pCharVal);
            if(errCode != NRF_SUCCESS)
            {
                GSC_DEBUG("[GattCharacteristic] setValue() > 0x%04x sd_ble_gatts_value_set Error: 0x%04x\r\n", _bleUUID.uuid, errCode);
            }
        }
        else
        {
            memset(_attrCharValue.p_value, 0, _attrCharValue.max_len);
            memcpy(_attrCharValue.p_value, pValue, size);
        }
    }

    if(errCode == NRF_SUCCESS)
    {
        _lastWrittenLen = static_cast<uint8_t>(size & 0xff);
    }
    else
    {
        GSC_DEBUG("[GattCharacteristic] setValue() > Error on characteristic 0x%04x (errCode: 0x%04x)\r\n", _bleUUID.uuid, errCode);
    }
    return errCode;
}

//------------------------------------------------------------------------------------------------------------
uint32_t GattCharacteristic::notifyIndicateValue(uint16_t connHandle, const uint16_t size, uint8_t* pValue)
{
    uint32_t errCode = setValue(size, pValue);
    if(errCode == NRF_SUCCESS)
    {
        if(_isNotificationIndicationEnable)
        {
            GSC_DEBUG("[GattCharacteristic] notifyIndicateValue() > size: %d\r\n", size);
            errCode = sendValue(connHandle, _hvxType, size, pValue);
        }
    }
    else
    {
        GSC_DEBUG("[GattCharacteristic] setnotifyIndicateValueValue() > Error on characteristic 0x%04x (errCode: 0x%04x)\r\n", _bleUUID.uuid, errCode);
    }
    return errCode;
}

//------------------------------------------------------------------------------------------------------------
uint32_t GattCharacteristic::sendValue(uint16_t connHandle, uint8_t hvxType, const uint16_t size, uint8_t* pValue)
{
    uint32_t errCode = NRF_ERROR_INVALID_STATE;
    uint16_t len     = size;

    if(connHandle != BLE_CONN_HANDLE_INVALID)
    {
        ble_gatts_hvx_params_t hvxParams;
        memset(&hvxParams, 0, sizeof(ble_gatts_hvx_params_t));
        hvxParams.handle    = _charHandle.value_handle;
        hvxParams.type      = hvxType;
        hvxParams.offset    = 0;
        hvxParams.p_len     = &len;
        hvxParams.p_data    = pValue;
        errCode             = sd_ble_gatts_hvx(connHandle, &hvxParams);
#if (DEBUG_GATT_CHARACTERISTIC_ENABLE != 0)
        if(errCode == NRF_ERROR_RESOURCES) 
        {
            GSC_DEBUG("[GattCharacteristic] sendValue() > sd_ble_gatts_hvx(): Too many notifications queued. [FAILED] ! (errCode: NRF_ERROR_RESOURCES)\r\n");
        }
        else if(errCode != NRF_SUCCESS)
        {
            GSC_DEBUG("[GattCharacteristic] sendValue() > sd_ble_gatts_hvx() [FAILED] ! (errCode: 0x%04x)\r\n", errCode);
            GSC_DEBUG("                     +-> len: %d, hvxType: %d, charHandle: 0x%04x\r\n", len, hvxType, _charHandle.value_handle);
        }
    }
    else
    {
        GSC_DEBUG("[GattCharacteristic] sendValue() > connHandle is INVALID !\r\n");
#endif  // #if (DEBUG_GATT_CHARACTERISTIC_ENABLE != 0)
    }
    return errCode;
}

#endif    // #if defined(SOFTDEVICE_PRESENT)
