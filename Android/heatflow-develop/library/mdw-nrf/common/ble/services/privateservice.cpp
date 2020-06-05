/************************************************************************************************************
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 ************************************************************************************************************/

/************************************************************************************************************/
/* Header files                                                                                             */
/************************************************************************************************************/
#include "privateservice.h"

#if defined(SOFTDEVICE_PRESENT)
#include <ble.h>
#include <ble_types.h>

#include <factory.h>

/************************************************************************************************************/
/* DEBUG COMPILATION OPTION & MACRO                                                                         */
/************************************************************************************************************/
#define DEBUG_PRIVATE_SERVICE_ENABLE        0
#if (DEBUG_PRIVATE_SERVICE_ENABLE != 0) && defined(DEBUG_NRF_USER)
    #include <tools/logtrace.h>
    #define PSRV_DEBUG                      LOG_TRACE_DEBUG
#else
    #define PSRV_DEBUG(...)                 {(void) (0);}
#endif

/************************************************************************************************************/
/* Namespace's Declaration                                                                                  */
/************************************************************************************************************/
using ble::PrivateService;

/************************************************************************************************************/
/* --- Extern "C" Section ---                                                                               */
/*     Callback's section                                                                                   */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------

/************************************************************************************************************/
/* CONSTRUCTOR                                                                                              */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
PrivateService::PrivateService()    {}

//------------------------------------------------------------------------------------------------------------
PrivateService::~PrivateService()   {}

/************************************************************************************************************/
/* PUBLIC SECTION                                                                                           */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
void PrivateService::initializeService(uint8_t uuidType, Security::eSecurityLevelId securityLevel)
{
    _advertiseUUID      = 0;
    _securityLevel      = securityLevel;
    _serviceUuid.type   = uuidType;

    // ---- Service's initalization ----
    uint32_t errCode = _initService();
    if(errCode != NRF_SUCCESS)
    {
        PSRV_DEBUG("[PrivateServices] initializeService() > _initService() failed ! (Error: 0x%04x)\r\n", errCode);
    }
    else
    {
        PSRV_DEBUG("[PrivateServices] initializeService() > Done !\r\n");
    }
    
}

/************************************************************************************************************/
/* PROTECTED SECTION                                                                                        */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
uint8_t PrivateService::handleReadWriteAuthorizeRequest(ble_evt_t const*                        pBleEvent,
                                                        ble_gatts_rw_authorize_reply_params_t*  pAuthReply)
{
    const uint8_t authorizeOp = pBleEvent->evt.gatts_evt.params.authorize_request.type;
    PSRV_DEBUG("[PrivateServices] handleReadWriteAuthorizeRequest() > BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST type %s on 0x%04x\r\n", (authorizeOp == BLE_GATTS_AUTHORIZE_TYPE_READ ? "READ" : "WRITE"), pBleEvent->evt.gatts_evt.params.authorize_request.request.read.uuid.uuid);
    memset(pAuthReply, 0, sizeof(ble_gatts_rw_authorize_reply_params_t));
    pAuthReply->type = authorizeOp;

    // Check the status of the BLE link
    pm_conn_sec_status_t status;
    ret_code_t retCode = pm_conn_sec_status_get(pBleEvent->evt.gatts_evt.conn_handle, &status);
    if(retCode == NRF_SUCCESS)
    {
        PSRV_DEBUG("[PrivateServices] handleReadWriteAuthorizeRequest() > pm_conn_sec_status_get:%s connected,%s bonded,%s encrypted,%s mitm\r\n",
                   (status.connected ? "" : " NOT"), (status.bonded ? "" : " NOT"), (status.encrypted ? "" : " NOT"), (status.mitm_protected ? "" : " NO"));
        // Only write operations are supported
        if(authorizeOp == BLE_GATTS_AUTHORIZE_TYPE_READ || authorizeOp == BLE_GATTS_AUTHORIZE_TYPE_WRITE)
        {
            // Checks if we need some authentication
            if(_securityLevel > Security::noSecurityLevelId)
            {
                PSRV_DEBUG("[PrivateServices] handleReadWriteAuthorizeRequest() > Using JUST_WORKS or MITM security use case.\r\n");
                // FIXME: -> For now on, PeerManager does not handle the `bonded status` correctly
                if(/*status.bonded &&*/ status.encrypted)
                {
                    // The link is secured so the read/write is authorized
                    PSRV_DEBUG("[PrivateServices] handleReadWriteAuthorizeRequest() > The link is secured (gatt_status = BLE_GATT_STATUS_SUCCESS)\r\n");
                    _setAuthorisationReply(authorizeOp, BLE_GATT_STATUS_SUCCESS, pAuthReply);
                }
                else
                {
                    // The link is not secure so the read or write request cannot be authorized (error 0x08).
                    // Return this specific error code will force the connected central device to send a security request.
                    PSRV_DEBUG("[PrivateServices] handleReadWriteAuthorizeRequest() > The link is not secured (gatt_status = BLE_GATT_STATUS_ATTERR_INSUF_AUTHORIZATION)\r\n");
                    _setAuthorisationReply(authorizeOp, BLE_GATT_STATUS_ATTERR_INSUF_AUTHORIZATION, pAuthReply);
                }
            }
            else
            {
                // We don't need any security
                PSRV_DEBUG("[PrivateServices] handleReadWriteAuthorizeRequest() > We don't need any security. (gatt_status = BLE_GATT_STATUS_SUCCESS)\r\n");
                _setAuthorisationReply(authorizeOp, BLE_GATT_STATUS_SUCCESS, pAuthReply);
            }
        }
        else
        {
            // The operation is not supported. Only read or write operations are supported
            PSRV_DEBUG("[PrivateServices] handleReadWriteAuthorizeRequest() > The operation is not supported. (gatt_status = BLE_GATT_STATUS_ATTERR_REQUEST_NOT_SUPPORTED)\r\n");
            _setAuthorisationReply(authorizeOp, BLE_GATT_STATUS_ATTERR_REQUEST_NOT_SUPPORTED, pAuthReply);
        }
    }
    return authorizeOp;
}

//------------------------------------------------------------------------------------------------------------
void PrivateService::_setAuthorisationReply(uint8_t                                 type,
                                            uint16_t                                gattStatus,
                                            ble_gatts_rw_authorize_reply_params_t*  reply)
{
    if(type == BLE_GATTS_AUTHORIZE_TYPE_READ)
    {
        reply->params.read.gatt_status = gattStatus;
    }
    else if(type == BLE_GATTS_AUTHORIZE_TYPE_WRITE)
    {
        reply->params.write.gatt_status = gattStatus;
        reply->params.write.update      = 1;
    }
    else if(type == BLE_GATTS_AUTHORIZE_TYPE_INVALID)
    {
        reply->params.read.gatt_status  = BLE_GATT_STATUS_ATTERR_INVALID;
        reply->params.write.gatt_status = BLE_GATT_STATUS_ATTERR_INVALID;
        PSRV_DEBUG("[PrivateServices] _setAuthorisationReply() > gatt_status = BLE_GATT_STATUS_ATTERR_INVALID\r\n");
    }
}


#endif    // #if defined(SOFTDEVICE_PRESENT)
