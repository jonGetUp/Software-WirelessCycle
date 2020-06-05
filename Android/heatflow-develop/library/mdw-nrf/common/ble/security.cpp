/************************************************************************************************************
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 ************************************************************************************************************/

/************************************************************************************************************/
/* Header files                                                                                             */
/************************************************************************************************************/
#include "security.h"

#if defined(SOFTDEVICE_PRESENT)
#include <sdk_config.h>

#include <app_timer.h>
#include <ble_conn_state.h>
#include <ble_hci.h>
#include <fds.h>

#if (USE_MITM_BONDING != 0)
    #include <nrf_soc.h>
#endif

#include <factory.h>
#include <ble-services-config.h>

#include "ble/peripheralcontroller.h"


/************************************************************************************************************/
/* DEBUG COMPILATION OPTION & MACRO                                                                         */
/************************************************************************************************************/
#define DEBUG_SECURITY_ENABLE               0
#if (DEBUG_SECURITY_ENABLE != 0) && defined(DEBUG_NRF_USER)
    #include <tools/logtrace.h>
    #define SEC_DEBUG                       LOG_TRACE_DEBUG
#else
    #define SEC_DEBUG(...)                  {(void) (0);}
#endif

#define DEBUG_SECURITY_WHITELIST_ENABLE     1
#if (DEBUG_SECURITY_WHITELIST_ENABLE != 0)
    #include <tools/logtrace.h>
    #define SECW_DEBUG                      LOG_TRACE_DEBUG
#else
    #define SECW_DEBUG(...)                 {(void) (0);}
#endif


/************************************************************************************************************/
/* Namespace's Declaration                                                                                  */
/************************************************************************************************************/
using ble::Security;


/************************************************************************************************************/
/* Declaration section                                                                                      */
/************************************************************************************************************/

/************************************************************************************************************/
/* --- Extern "C" Section ---                                                                               */
/*     Callback's section                                                                                   */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
void ble::peerManagerEventHandler(pm_evt_t const* pEvent)
{
    Factory::peripheralController.onPeerManagerEvent(pEvent);
}


/************************************************************************************************************/
/* CONSTRUCTOR                                                                                              */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
Security::Security()
{
    _peerManagerInitialized = false;
    _peerManagerRegistered  = false;
    _passkey                = NULL;
    setPasskeySize(PASS_KEY_SIZE);
}

//------------------------------------------------------------------------------------------------------------
Security::~Security()
{
    _peerManagerInitialized = false;
    _peerManagerRegistered  = false;
    free(_passkey);
}

/************************************************************************************************************/
/* PUBLIC SECTION                                                                                           */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
void Security::initialize(BleController*                bleController, 
                          PeripheralController*         peripheralController,
                          bool                          eraseBonds, 
                          bool                          useFixedPasskey)
{
    uint32_t errCode = NRF_SUCCESS;
    if(!bleController->addObserver(this))
    {
        SEC_DEBUG("[Security] initialize() > Impossible to add Security as Observer of bleCtrl !\r\n");
    }

    _peerId                 = PM_PEER_ID_INVALID;
    _peripheralController   = peripheralController;
    
    if(!_peerManagerInitialized)
    {
        errCode = pm_init();
        if(errCode != NRF_SUCCESS)
        {
            SEC_DEBUG("[Security] initialize() > pm_init() FAILED (Error: 0x%04x)\r\n", errCode);
            ASSERT(false);
        }
        else
        {
            SEC_DEBUG("[Security] initialize() > pm_init() SUCCEEDED !\r\n");
            _peerManagerInitialized = true;
        }
    } 

    if(eraseBonds)
    {
        deleteAllBondingInformation();
    }

    if(!_peerManagerRegistered)
    {
        errCode = pm_register(ble::peerManagerEventHandler);
        if(errCode != NRF_SUCCESS)
        {
            SEC_DEBUG("[Security] initialize() > pm_register() FAILED (Error: 0x%04x)\r\n", errCode);
            ASSERT(false);
        }
        else 
        {
            _peerManagerRegistered = true;
            SEC_DEBUG("[Security] initialize() > pm_register() SUCCEEDED !\r\n");
        }
    }
    SEC_DEBUG("[Security] initialize() > ... Init Done\r\n");
}

//------------------------------------------------------------------------------------------------------------
uint32_t Security::setupSecurityParameter(bool useJustWorksBonding, bool useMenInTheMiddleBonding, bool useFixedPasskey)
{
    uint32_t errCode = NRF_SUCCESS;

    if(!_peerManagerInitialized || !_peerManagerRegistered)
    {
        return NRF_ERROR_INVALID_STATE;
    }
    
    _securityLevel   = static_cast<eSecurityLevelId>((useJustWorksBonding ? 1 : 0) + (useMenInTheMiddleBonding ? 1 : 0));
    if(_securityLevel != noSecurityLevelId)
    {
        // Security parameters to be used for all security procedures.
        memset(&_securityParameter, 0, sizeof(ble_gap_sec_params_t));
        _securityParameter.bond           = useMenInTheMiddleBonding ? true : useJustWorksBonding;
        _securityParameter.mitm           = useMenInTheMiddleBonding;
        _securityParameter.lesc           = SEC_PARAM_LESC;
        _securityParameter.keypress       = SEC_PARAM_KEYPRESS;
        _securityParameter.io_caps        = SEC_PARAM_IO_CAPABILITIES;
        _securityParameter.oob            = SEC_PARAM_OOB;
        _securityParameter.min_key_size   = SEC_PARAM_MIN_KEY_SIZE;
        _securityParameter.max_key_size   = SEC_PARAM_MAX_KEY_SIZE;
        _securityParameter.kdist_own.enc  = 1;
        _securityParameter.kdist_own.id   = 1;
        _securityParameter.kdist_peer.enc = 1;
        _securityParameter.kdist_peer.id  = 1;

        errCode = pm_sec_params_set(&_securityParameter);
        if(errCode != NRF_SUCCESS)
        {
            SEC_DEBUG("[Security] initialize() > pm_sec_params_set() FAILED (Error: 0x%04x)\r\n", errCode);
            ASSERT(false);
        }

        // Handling of the whitelist...
        if(_useWhitelist || (_securityLevel == mitmLevelId))
        {
            // Create list of already bonded peers
            _createWhitelistPeers();
        }
        
        // Handling of the passkey...
        if((_securityLevel == mitmLevelId) && (useFixedPasskey))
        {
            // Use the fixed passkey `123456..` or the one stored in the flash
            SEC_DEBUG("[Security] initialize() > Generate a fixed passkey ...\r\n");
            generatePasskey();
        }
    }
    
    return errCode;
}

//------------------------------------------------------------------------------------------------------------
void Security::setPasskeySize(size_t size)
{
    if(_passkey != NULL)
    {
        free(_passkey);
    }
    _passkeySize = size;
    _passkey = new uint8_t[_passkeySize];
}

//------------------------------------------------------------------------------------------------------------
void Security::generatePasskey()
{
    uint32_t errCode = NRF_SUCCESS;

    #if (USE_INFORMATION_FROM_FLASH != 0)
    uint32_t passKeyChar = 0;
    for(uint8_t i = 0; i < _passkeySize; i++)
    {
        passKeyChar = *(uint32_t*)(PASSKEY_FLASH_ADDRESS + (i * 4));
        _passkey[i] = (char)(passKeyChar & 0xff);
    }
    #else
    for(uint8_t i = 0; i < _passkeySize; i++)
    {
        _passkey[i] = ((i + 1) + 0x30);
    }
    #endif
    SEC_DEBUG("[Security] generatePasskey() > Passkey: %s\r\n", _passkey);

    _bleOpt.gap_opt.passkey.p_passkey = _passkey;
    errCode = sd_ble_opt_set(BLE_GAP_OPT_PASSKEY, &_bleOpt);
    if(errCode != NRF_SUCCESS)
    {
        SEC_DEBUG("[Security] generatePasskey() > sd_ble_opt_set() FAILED (error: 0x%04xd\r\n", errCode);
    }
}

//------------------------------------------------------------------------------------------------------------
uint32_t Security::deleteBondingInformation(pm_peer_id_t peerId)
{
    uint32_t errCode = NRF_SUCCESS;
    if(peerId != PM_PEER_ID_INVALID)
    {
        errCode = pm_peer_delete(peerId);
        if(errCode != NRF_SUCCESS)
        {
            SEC_DEBUG("[Security] deletePeer() > pm_peer_delete() [FAILED] (error: 0x%04x)\r\n", errCode);
            ASSERT(false);
        }
    }
    else
    {
        errCode = NRF_ERROR_INVALID_PARAM;
    }
    return errCode;
}

//------------------------------------------------------------------------------------------------------------
void Security::deleteAllBondingInformation()
{
    uint32_t errCode = pm_peers_delete();
    if(errCode != NRF_SUCCESS)
    {
        SEC_DEBUG("[Security] deleteAllBondingInformation() > pm_peers_delete() FAILED (Error: 0x%04x)\r\n", errCode);
        ASSERT(false);
    }
    _clearWhitelistPeers();
}

//------------------------------------------------------------------------------------------------------------
uint32_t Security::getWhitelist(ble_gap_addr_t* pAddrs,
                                uint32_t*       pAddrCount,
                                ble_gap_irk_t*  pIrks,
                                uint32_t*       pIrkCount)
{
    uint32_t errCode = pm_whitelist_get(pAddrs, pAddrCount, pIrks, pIrkCount);
    if(errCode == NRF_SUCCESS)
    {
        SECW_DEBUG("[Security] getWhitelist() > pm_whitelist_get() returns %d addr and %d irk from the whitelist\r\n", 
                          *pAddrCount, *pIrkCount);
    }
    else
    {
        SEC_DEBUG("[Security] getWhitelist() > pm_whitelist_get() FAILED! (ErrCode: 0x%04x)\r\n", errCode);
        ASSERT(false);
    }
    return errCode;
}

//------------------------------------------------------------------------------------------------------------
void Security::getPeerList(pm_peer_id_t* pPeers, uint32_t* pPeersSize, pm_peer_id_list_skip_t skipId)
{
//    #warning "ToDo: rewrite this part using pm_peer_id_list method..."
    uint32_t peers2Copy = BLE_GAP_WHITELIST_ADDR_MAX_COUNT;
    uint32_t errCode    = pm_peer_id_list(pPeers, &peers2Copy, PM_PEER_ID_INVALID, skipId);
    if(errCode != NRF_SUCCESS)
    {
        SEC_DEBUG("[Security] getPeerList() > pm_peer_id_list() FAILED! (ErrCode: 0x%04x)\r\n", errCode);
        ASSERT(false);
    }
    
    // Update Peer List size
    *pPeersSize         = peers2Copy;

    SECW_DEBUG("[Security] getPeerList() > Peer's whitelist count: %d\r\n", peers2Copy);
}

//------------------------------------------------------------------------------------------------------------
bool Security::isWhitelistEmpty()
{
    if(_useWhitelist)
    {
        uint32_t count = pm_peer_count();
        SECW_DEBUG("[Security] isWhitelistEmpty() > Number of known peers (whitelist size): %d\r\n", count);
        return (count == 0);
    }
    else
    {
        SEC_DEBUG("[Security] isWhitelistEmpty() > Whitelist is not in use !\r\n");
    }
    return false;
}

//------------------------------------------------------------------------------------------------------------
uint32_t Security::peerAddressRequest()
{
    // Only Give peer address if we have a handle to the bonded peer.
    pm_peer_id_t peerId;
    uint32_t errCode = pm_peer_ranks_get(&peerId, NULL, NULL, NULL);
    if(peerId != PM_PEER_ID_INVALID)
    {
        pm_peer_data_bonding_t peerBondingData;
        SEC_DEBUG("[Security] peerAddressRequest() > peerId: %d\r\n", peerId);
        errCode = pm_peer_data_bonding_load(peerId, &peerBondingData);
        if((errCode != NRF_ERROR_NOT_FOUND) && (errCode == NRF_SUCCESS))
        {
            ble_gap_addr_t* peerAddr = &(peerBondingData.peer_ble_id.id_addr_info);
            errCode = _peripheralController->advertisingPeerAddrReply(peerAddr);
            if(errCode != NRF_SUCCESS)
            {
                SEC_DEBUG("[Security] peerAddressRequest() > ble_advertising_peer_addr_reply() ERROR: 0x%04x\r\n", errCode);
                ASSERT(false);
            }
            else
            {
                SEC_DEBUG("[Security] peerAddressRequest() > ble_advertising_peer_addr_reply() SUCCEEDED!\r\n");
            }
        }
        else
        {
            SEC_DEBUG("[Security] peerAddressRequest() > pm_peer_data_bonding_load() ERROR: 0x%04x\r\n", errCode);
        }
    }
    else
    {
        errCode = NRF_ERROR_INVALID_DATA;
    }
    return errCode;
}

//------------------------------------------------------------------------------------------------------------
Security::eSecurityLevelId Security::securityLevelFromConfig(bool useJustWorks, bool useMitm)
{
    if(useMitm)
    {
        return mitmLevelId;
    }
    else if(useJustWorks)
    {
        return justWorksLevelId;
    }
    return noSecurityLevelId;
}

//------------------------------------------------------------------------------------------------------------
void Security::onPeerManagerEvent(pm_evt_t const* pEvent)
{
    uint32_t errCode = NRF_SUCCESS;
    switch(pEvent->evt_id)
    {
        case PM_EVT_BONDED_PEER_CONNECTED:
        {
            SEC_DEBUG("[Security] onPeerManagerEvent() > Connected to previously bonded device\r\n");
            if((errCode = pm_peer_rank_highest(pEvent->peer_id)) != NRF_SUCCESS)
            {
                _peerId = pEvent->peer_id;
                SEC_DEBUG("[Security] peerManagerEventHandler() > pm_peer_rank_highest(%d) ERROR: 0x%04x\r\n", pEvent->peer_id, errCode);
            }
            break;    // PM_EVT_BONDED_PEER_CONNECTED
        }
        case PM_EVT_CONN_SEC_START:
        {
            SEC_DEBUG("[Security] onPeerManagerEvent() > PM_EVT_CONN_SEC_START\r\n");
            break;    // PM_EVT_CONN_SEC_START
        }
        case PM_EVT_CONN_SEC_SUCCEEDED:
        {
            SEC_DEBUG("[Security] onPeerManagerEvent() > PM_EVT_CONN_SEC_SUCCEEDED. Role: %d. connHandle: %d, Procedure: %d\r\n",
                      ble_conn_state_role(pEvent->conn_handle), pEvent->conn_handle, pEvent->params.conn_sec_succeeded.procedure);
            _peerId = pEvent->peer_id;
            errCode = pm_peer_rank_highest(pEvent->peer_id);
            if((errCode != NRF_ERROR_BUSY) && (errCode != NRF_SUCCESS))
            {
                SEC_DEBUG((char*) "[Security] peerManagerEventHandler() > pm_peer_rank_highest(%d) retVal: 0x%04x\r\n", pEvent->peer_id, errCode);
            }

            if(pm_peer_count() > BLE_GAP_WHITELIST_ADDR_MAX_COUNT)
            {
                pm_peer_id_t highestPeerId;
                uint32_t     highestRank;
                pm_peer_id_t lowestPeerId;
                uint32_t     lowestRank;
                errCode = pm_peer_ranks_get(&highestPeerId, &highestRank, &lowestPeerId, &lowestRank);
                SEC_DEBUG("[Security] onPeerManagerEvent() > PM_EVT_CONN_SEC_SUCCEEDED. pm_peer_ranks_get() > highest ID: %d (%d), lowest ID: %d (%d), current ID: %d\r\n",
                      highestPeerId, highestRank, lowestPeerId, lowestRank, pEvent->peer_id);
                if(errCode == NRF_SUCCESS && lowestPeerId != pEvent->peer_id)
                {
                    errCode = pm_peer_delete(lowestPeerId);
                    SEC_DEBUG("[Security] onPeerManagerEvent() > PM_EVT_CONN_SEC_SUCCEEDED. pm_peer_delete() > ID: %d ERROR: 0x%04x\r\n", lowestPeerId, errCode);
                }
            }
            
            #if defined(BLE_DB_DISCOVERY_ENABLED) && (BLE_DB_DISCOVERY_ENABLED != 0)
            errCode = _peripheralController->startServiceDiscovery(pEvent->conn_handle);
            if(errCode != NRF_SUCCESS)
            {
                SEC_DEBUG("[Security] onPeerManagerEvent() > Could not start discovering BLE services on peer device !  (errCode: 0x%04x)\r\n", errCode);
                ASSERT(false);
            }
            SEC_DEBUG("[Security] onPeerManagerEvent() > Started discovering BLE services on peer device...\r\n");
            #endif  // #if defined(BLE_DB_DISCOVERY_ENABLED) && (BLE_DB_DISCOVERY_ENABLED != 0)
            break;    // PM_EVT_CONN_SEC_SUCCEEDED
        }
        case PM_EVT_CONN_SEC_FAILED:
        {
            // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            // In some cases, when securing fails, it can be restarted directly. Sometimes it can be restarted, but
            // only after changing some Security Parameters. Sometimes, it cannot be restarted until the link is
            // disconnected and reconnected. Sometimes it is impossible to secure the link, or the peer device does
            // not support it. How to handle this error is highly application-dependent.
            // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            SEC_DEBUG("[Security] onPeerManagerEvent() > Connection security failed: role: %d, conn_handle: 0x%x, procedure: %d, error: %d. (PM_EVT_CONN_SEC_FAILED)\r\n",
                       ble_conn_state_role(pEvent->conn_handle), pEvent->conn_handle, pEvent->params.conn_sec_failed.procedure, pEvent->params.conn_sec_failed.error);
            switch(pEvent->params.conn_sec_failed.error)
            {
                case PM_CONN_SEC_ERROR_PIN_OR_KEY_MISSING:
                    SEC_DEBUG("[Security] onPeerManagerEvent() > PM_EVT_CONN_SEC_FAILED: PM_CONN_SEC_ERROR_PIN_OR_KEY_MISSING ...\r\n");
                    break;//PM_CONN_SEC_ERROR_PIN_OR_KEY_MISSING

                case PM_CONN_SEC_ERROR_MIC_FAILURE:
                    SEC_DEBUG("[Security] onPeerManagerEvent() > PM_EVT_CONN_SEC_FAILED: PM_CONN_SEC_ERROR_MIC_FAILURE\r\n");
                    break;//PM_CONN_SEC_ERROR_MIC_FAILURE

                case PM_CONN_SEC_ERROR_DISCONNECT :
                    SEC_DEBUG("[Security] onPeerManagerEvent() > PM_EVT_CONN_SEC_FAILED: PM_CONN_SEC_ERROR_DISCONNECT\r\n");
                    break;//PM_CONN_SEC_ERROR_DISCONNECT

                case PM_CONN_SEC_ERROR_SMP_TIMEOUT:
                    SEC_DEBUG("[Security] onPeerManagerEvent() > PM_EVT_CONN_SEC_FAILED: PM_CONN_SEC_ERROR_SMP_TIMEOUT\r\n");
                    break;//PM_CONN_SEC_ERROR_SMP_TIMEOUT

                default:
                    SEC_DEBUG("[Security] onPeerManagerEvent() > PM_EVT_CONN_SEC_FAILED, unhandled error: %d\r\n", pEvent->params.conn_sec_failed.error);
                    break;
            }
            break;    // PM_EVT_CONN_SEC_FAILED
        }
        case PM_EVT_CONN_SEC_CONFIG_REQ:
        {
            // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            // A connected peer (central) is trying to pair, but the Peer Manager already has a bond for that peer.
            // Setting allow_repairing to false rejects the pairing request. If this event is ignored
            // (pm_conn_sec_config_reply is not called in the event handler), the Peer Manager assumes
            // allow_repairing to be false.
            // `allow_repairing` takes part of the `platform_config` header file and let you choose if you want to
            // allow or not an already bonded device to pair again.
            // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            SEC_DEBUG("[Security] onPeerManagerEvent() > PM_EVT_CONN_SEC_CONFIG_REQ\r\n");

            // Reject pairing request from an already bonded peer.
            pm_conn_sec_config_t connSecConfig = { ALLOW_REPAIRING };
            pm_conn_sec_config_reply(pEvent->conn_handle, &connSecConfig);
            break;    // PM_EVT_CONN_SEC_CONFIG_REQ
        }
        case PM_EVT_CONN_SEC_PARAMS_REQ:
        {
            // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            // Security parameters (\ref ble_gap_sec_params_t) are needed for an ongoing security procedure. Reply 
            // with \ref pm_conn_sec_params_reply before the event handler returns. If no reply is sent, the 
            // parameters given in \ref pm_sec_params_set are used. If a peripheral connection, the central's 
            // sec_params will be available in the event.
            // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            SEC_DEBUG("[Security] onPeerManagerEvent() > PM_EVT_CONN_SEC_PARAMS_REQ\r\n");
//            errCode = pm_conn_sec_params_reply(pEvent->conn_handle, &_secParam, NULL);
//            if(errCode != NRF_SUCCESS) 
//            {
//                SEC_DEBUG("[Security] onPeerManagerEvent() > pm_conn_sec_params_reply() ERROR: 0x%04x\r\n", errCode);
//                ASSERT(false);
//            }
            break;
        }
        case PM_EVT_STORAGE_FULL:
        {
            // Run garbage collection on the flash.
            SEC_DEBUG("[Security] onPeerManagerEvent() > PM_EVT_STORAGE_FULL -> calling fds_gc()...\r\n");
            errCode = fds_gc();
            if(errCode == FDS_ERR_NO_SPACE_IN_QUEUES)
            {
                SEC_DEBUG("[Security] onPeerManagerEvent() > fds_gc() returns FDS_ERR_NO_SPACE_IN_QUEUES` ... YOU SHOULD INCREASE THE SIZE OF THE QUEUE IN `fds_config.h` !!!\r\n");
            }
            else if(errCode != NRF_SUCCESS)
            {
                SEC_DEBUG("[Security] onPeerManagerEvent() > fds_gc() ERROR: 0x%04x\r\n", errCode);
                ASSERT(false);
            }
            break;    //PM_EVT_STORAGE_FULL
        }
        case PM_EVT_ERROR_UNEXPECTED:
        {
            // Assert.
            if(pEvent->params.error_unexpected.error != NRF_SUCCESS)
            {
                SEC_DEBUG("[Security] onPeerManagerEvent() > PM_EVT_ERROR_UNEXPECTED: 0x%04x\r\n", pEvent->params.error_unexpected.error);
                ASSERT(false);
            }
            break;    //PM_EVT_ERROR_UNEXPECTED
        }
        case PM_EVT_PEER_DATA_UPDATE_SUCCEEDED:
        {
            SEC_DEBUG("[Security] onPeerManagerEvent() > PM_EVT_PEER_DATA_UPDATE_SUCCEEDED\r\n");
            if (     pEvent->params.peer_data_update_succeeded.flash_changed
                 && (pEvent->params.peer_data_update_succeeded.data_id == PM_PEER_DATA_ID_BONDING))
            {
                SEC_DEBUG("[Security] onPeerManagerEvent() > New Bond, add the peer to the whitelist if possible. _whitelistPeerCount: %d, MAX_PEERS_WLIST: %d\r\n", _whitelistPeersCount + 1, BLE_GAP_WHITELIST_ADDR_MAX_COUNT);
                
                // Note: You should check on what kind of white list policy your application should use.
                _createWhitelistPeers();
//                #warning "ToDo: use whitelist_set(PM_PEER_ID_LIST_SKIP_NO_ID_ADDR);"
//                if (_whitelistPeersCount < BLE_GAP_WHITELIST_ADDR_MAX_COUNT)
//                {
//                    // Bonded to a new peer, add it to the whitelist.
//                    _whitelistPeers[_whitelistPeersCount++] = pEvent->peer_id;

//                    // The whitelist has been modified, update it in the Peer Manager.
//                    errCode = pm_device_identities_list_set(_whitelistPeers, _whitelistPeersCount);
//                    if((errCode != NRF_ERROR_NOT_SUPPORTED) && (errCode != NRF_SUCCESS))
//                    {
//                        SEC_DEBUG("[Security] onPeerManagerEvent() > pm_device_identities_list_set() FAILED ! (ErrCode: 0x%04x)\r\n", errCode);
//                    }

//                    errCode = pm_whitelist_set(_whitelistPeers, _whitelistPeersCount);
//                    if (errCode != NRF_SUCCESS)
//                    {
//                        SEC_DEBUG("[Security] onPeerManagerEvent() > pm_whitelist_set() FAILED ! (ErrCode: 0x%04x)\r\n", errCode);
//                        ASSERT(false);
//                    }
//                }
            }
            break;    //PM_EVT_PEER_DATA_UPDATE_SUCCEEDED
        }
        case PM_EVT_PEER_DATA_UPDATE_FAILED:
        {
            // Assert.
            if(pEvent->params.peer_data_update_failed.error != NRF_SUCCESS)
            {
                SEC_DEBUG("[Security] onPeerManagerEvent() > PM_EVT_PEER_DATA_UPDATE_FAILED: 0x%04x -> ASSERT !\r\n", pEvent->params.peer_data_update_failed.error);
                ASSERT(false);
            }
            break;    //PM_EVT_PEER_DATA_UPDATE_FAILED
        }
        case PM_EVT_PEER_DELETE_SUCCEEDED:
        {
            SEC_DEBUG("[Security] onPeerManagerEvent() > PM_EVT_PEER_DELETE_SUCCEEDED for peerId #%d\r\n", pEvent->peer_id);            
            break;    //PM_EVT_PEER_DELETE_SUCCEEDED
        }
        case PM_EVT_PEER_DELETE_FAILED:
        {
            // Assert.
            if(pEvent->params.peer_delete_failed.error != NRF_SUCCESS)
            {
                SEC_DEBUG("[Security] onPeerManagerEvent() > PM_EVT_PEER_DELETE_FAILED: 0x%04x -> ASSERT !\r\n", pEvent->params.peer_delete_failed.error);
                ASSERT(false);
            }
            break;    //PM_EVT_PEER_DELETE_FAILED
        }
        case PM_EVT_PEERS_DELETE_SUCCEEDED:
        {
            SEC_DEBUG("[Security] onPeerManagerEvent() > PM_EVT_PEERS_DELETE_SUCCEEDED\r\n");
            // FIXME advertising_start();
            break;
        }
        case PM_EVT_PEERS_DELETE_FAILED:
        {
            // Assert.
            if(pEvent->params.peers_delete_failed_evt.error != NRF_SUCCESS)
            {
                SEC_DEBUG("[Security] onPeerManagerEvent() > PM_EVT_PEERS_DELETE_FAILED: 0x%04x\r\n", pEvent->params.peers_delete_failed_evt.error);
                ASSERT(false);
            }
            break;    //PM_EVT_PEERS_DELETE_FAILED
        }
        case PM_EVT_LOCAL_DB_CACHE_APPLIED:
        {
            SEC_DEBUG("[Security] onPeerManagerEvent() > PM_EVT_LOCAL_DB_CACHE_APPLIED\r\n");
            break;    //PM_EVT_LOCAL_DB_CACHE_APPLIED
        }
        case PM_EVT_LOCAL_DB_CACHE_APPLY_FAILED:
        {
            SEC_DEBUG("[Security] onPeerManagerEvent() > PM_EVT_LOCAL_DB_CACHE_APPLY_FAILED\r\n");
            // The local database has likely changed, send service changed indications.
            pm_local_database_has_changed();
            break;    //PM_EVT_LOCAL_DB_CACHE_APPLY_FAILED
        }
        case PM_EVT_SERVICE_CHANGED_IND_SENT:
        {
            SEC_DEBUG("[Security] onPeerManagerEvent() > PM_EVT_SERVICE_CHANGED_IND_SENT\r\n");
            break;    //PM_EVT_SERVICE_CHANGED_IND_SENT
        }
        case PM_EVT_SERVICE_CHANGED_IND_CONFIRMED:
        {
            SEC_DEBUG("[Security] onPeerManagerEvent() > PM_EVT_SERVICE_CHANGED_IND_CONFIRMED\r\n");
            break;    //PM_EVT_SERVICE_CHANGED_IND_CONFIRMED
        }
        case PM_EVT_SLAVE_SECURITY_REQ:
        {
            // The peer (peripheral) has requested link encryption, which has been enabled.
            SEC_DEBUG("[Security] onPeerManagerEvent() > PM_EVT_SLAVE_SECURITY_REQ\r\n");
            break;  // PM_EVT_SLAVE_SECURITY_REQ
        }
        case PM_EVT_FLASH_GARBAGE_COLLECTED:
        {
            // The flash has been garbage collected (By FDS), possibly freeing up space.
            SEC_DEBUG("[Security] onPeerManagerEvent() > PM_EVT_FLASH_GARBAGE_COLLECTED\r\n");
            break;
        }
        case PM_EVT_FLASH_GARBAGE_COLLECTION_FAILED:
        {
            SEC_DEBUG("[Security] onPeerManagerEvent() > PM_EVT_FLASH_GARBAGE_COLLECTION_FAILED\r\n");
            break;
        }
    }
}

//------------------------------------------------------------------------------------------------------------
void Security::onBleEvent(ble_evt_t const* pBleEvent, void* pContext)
{
    switch(pBleEvent->header.evt_id)
    {
        case SD_BLE_GAP_DISCONNECT:
        {
            SEC_DEBUG("[Security] onBleEvent() > SD_BLE_GAP_DISCONNECT ...\r\n");
            _peerId = PM_PEER_ID_INVALID;
        }
        case BLE_GAP_EVT_AUTH_STATUS:
        {
            switch(pBleEvent->evt.gap_evt.params.auth_status.auth_status)
            {
                case BLE_GAP_SEC_STATUS_SUCCESS:
                {
                    SEC_DEBUG("[Security] onBleEvent() > Authentication succeeded ...\r\n");
                    return;
                }
                case BLE_GAP_SEC_STATUS_PASSKEY_ENTRY_FAILED:
                {
                    SEC_DEBUG("[Security] onBleEvent() > Authentication canceled by the user ...\r\n");
                    break;
                }
                case BLE_GAP_SEC_STATUS_CONFIRM_VALUE:
                {
                    SEC_DEBUG("[Security] onBleEvent() > Wrong passkey entered ...\r\n");
                    break;
                }
                default:
                    return;
            }
            break;
        }
        default:
            break;
    }
}

//------------------------------------------------------------------------------------------------------------
void Security::onSystemEvent(uint32_t sysEvt)
{
}

/************************************************************************************************************/
/* PRIVATE SECTION                                                                                          */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
void Security::_createWhitelistPeers()
{
    memset(_whitelistPeers, PM_PEER_ID_INVALID, sizeof(_whitelistPeers));
    _whitelistPeersCount = (sizeof(_whitelistPeers) / sizeof(pm_peer_id_t));

    getPeerList(_whitelistPeers, &_whitelistPeersCount, PM_PEER_ID_LIST_SKIP_NO_ID_ADDR);
    SECW_DEBUG("[Security] _createWhitelistPeers() > getPeerList() returns _whitelistPeersCount: %d (_whitelistPeers.size(): %d)\r\n", 
                _whitelistPeersCount, sizeof(_whitelistPeers) / sizeof(pm_peer_id_t));

    ret_code_t errCode = pm_whitelist_set(_whitelistPeers, _whitelistPeersCount);
    if(errCode == NRF_SUCCESS)
    {
        SECW_DEBUG("[Security] _createWhitelistPeers() > pm_whitelist_set() succeeded!\r\n");
    }
    else
    {
        SEC_DEBUG("[Security] _createWhitelistPeers() > pm_whitelist_set() FAILED! (ErrCode: 0x%04x)\r\n", errCode);
        ASSERT(false);
    }

    // Setup the device identies list.
    // Some SoftDevices do not support this feature.
    errCode = pm_device_identities_list_set(_whitelistPeers, _whitelistPeersCount);
    if(errCode != NRF_ERROR_NOT_SUPPORTED && errCode != NRF_SUCCESS)
    {
        SEC_DEBUG("[Security] _createWhitelistPeers() > pm_device_identities_list_set() FAILED! (ErrCode: 0x%04x)\r\n", errCode);
        ASSERT(false);
    }
    else if(errCode == NRF_SUCCESS)
    {
        SECW_DEBUG("[Security] _createWhitelistPeers() > pm_device_identities_list_set() succeeded!\r\n");
    }
    else if(errCode == NRF_ERROR_NOT_SUPPORTED)
    {
        SECW_DEBUG("[Security] _createWhitelistPeers() >Whitelist not supported !\r\n");
    }
}

//------------------------------------------------------------------------------------------------------------
void Security::_clearWhitelistPeers()
{
    // Nothing to do here...
    SEC_DEBUG("[Security] _clearWhitelistPeers() > Size of whitelist: %d\r\n", pm_peer_count());
}

//------------------------------------------------------------------------------------------------------------
void Security::_setSecurityLevelIdFromInt(uint8_t level)
{
    switch(level)
    {
        case 1:
            _securityLevel = justWorksLevelId;
            break;

        case 2:
            _securityLevel = mitmLevelId;
            break;

        default:
            _securityLevel = noSecurityLevelId;
            break;
    }

    SEC_DEBUG("[Security] _setSecurityLevelIdFromInt() > Secusrity level set to %d\r\n", _securityLevel);
}

#endif // #if defined(SOFTDEVICE_PRESENT)
