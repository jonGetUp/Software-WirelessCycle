/****************************************************************************************************************//**
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 *
 * \file    security.h
 *
 * \addtogroup BLE
 * \{
 *
 * \addtogroup peripheral
 * \{
 *
 * \class   ble::Security
 * \brief   Class handling BLE Security features provided by the Nordic SoftDevice SDK v15
 *
 * Initial author: Patrice Rudaz
 * Creation date: 2017-01-26
 *
 * \author  Patrice Rudaz
 * \date    April 2018
 ********************************************************************************************************************/
#pragma once

/* **************************************************************************************************************** */
/*                                                                                                                  */
/* Header files                                                                                                     */
/*                                                                                                                  */
/* **************************************************************************************************************** */
#include <platform-config.h>

#if defined(SOFTDEVICE_PRESENT)
#include <ble.h>
#include <peer_manager.h>
#include <ble-security-config.h>
#include <ble-services-config.h>

#include "ble/interface/bleobserver.h"

#include "ble/blecontroller.h"


/* **************************************************************************************************************** */
/* --- Extern "C" Section ---                                                                                       */
/*     Callback's section                                                                                           */
/* **************************************************************************************************************** */
#if defined(__cplusplus)
extern "C"
{
#endif
    namespace ble 
    {
        /********************************************************************************************************//**
         * \brief   Function for handling the Peer Manager Events.
         *
         * \details This function handle the Peer Manager Events.
         *
         * \param   pEvent  Pointer to the event's handle
         ************************************************************************************************************/
        void peerManagerEventHandler(pm_evt_t const* pEvent);
    } // namespace ble

#if defined(__cplusplus)
}
#endif


/* **************************************************************************************************************** */
/*                                                                                                                  */
/* NAMESPACE Declaration                                                                                            */
/*                                                                                                                  */
/* **************************************************************************************************************** */
namespace ble 
{

    /* ************************************************************************************************************ */
    /*                                                                                                              */
    /* Class Declaration                                                                                            */
    /*                                                                                                              */
    /* ************************************************************************************************************ */
    class PeripheralController;
 
    class Security : public BleObserver
    {
        friend void peerManagerEventHandler(pm_evt_t const* pEvent);

    public:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* CONSTRUCTOR SECTION                                                                                      */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        Security();
        virtual ~Security();

        /* ******************************************************************************************************** */
        /*																										    */
        /* PUBLIC DECLARATION SECTION                                                                               */
        /*																										    */
        /* ******************************************************************************************************** */
        /********************************************************************************************************//**
         * \brief   Security Level's identifier(s).
         *
         * This enumeration lists all identifier used to configure the security levels. We can have:
         *   - noSecurityLevelId:
         *   This level is used during production to skip any bonding/pairong process.
         *
         *   - justWorksLevelId:
         *   This level use the JUST_WORKS use's case as defined by the Bluetooth SIG.
         *
         *   - mitmLevelId:
         *   This level use the MAN_IN_THE_MIDDLE use's case as defined by the Bluetooth SIG. This level will need a
         *   a passkey, which can be generated automatically, can be hard-coded or can be read from UICR.
         *   Used to turn off the backlight !
         ************************************************************************************************************/
        typedef enum
        {
            noSecurityLevelId = 0,          ///< \brief No security needed
            justWorksLevelId,               ///< \brief Bonding/pairing process needed using the JUST_WORKS scenario
            mitmLevelId,                    ///< \brief Bonding/pairing process using the MAN_IN_THE_MIDDLE scenario
        } eSecurityLevelId;

        /********************************************************************************************************//**
         * \return  The security level identifier, which can be:
         *            - `noSecurityLevelId (0)`
         *            - `justWorksLevelId (1)`, when using teh JUST_WORKS scenario for bonding/pairing process.
         *            - `mitmLevelId (2)`, when using the MITM scenario for bonding/pairing process.
         ************************************************************************************************************/
        inline eSecurityLevelId getSecurityLevelId() const              { return _securityLevel; }

        /********************************************************************************************************//**
         * \brief   Set the security level identifier.
         ************************************************************************************************************/
        inline void             setSecurityLevelId(eSecurityLevelId id) { _securityLevel = id; }

        /********************************************************************************************************//**
         * \return  `true` if the usage of the whitelist is enabled and `false` otherwise.
         ************************************************************************************************************/
        inline bool             isWhitelistUsageEnabled() const         { return _useWhitelist; }

        /********************************************************************************************************//**
         * \brief   Enable/disable the usage of the whitelist.
         ************************************************************************************************************/
        inline void             setWhitelistUsage(bool enable)          { _useWhitelist = enable; }

        /********************************************************************************************************//**
         * \brief   Returns the pointer to the passkey...
         ************************************************************************************************************/
        inline uint8_t*         getPasskey()                            { return _passkey; }

        /********************************************************************************************************//**
         * \brief   Returns the size of the stored passkey...
         ************************************************************************************************************/
        inline size_t           passkeySize()                           { return _passkeySize; }

        /********************************************************************************************************//**
         * \brief   Returns the pointer on the common BLE Option type, wrapping the module specific options.
         ************************************************************************************************************/
        inline ble_opt_t*       getBleOpt()                             { return &_bleOpt; }

        /********************************************************************************************************//**
         * \brief   Function for manually informing that the local database has changed.
         *
         * This function sends a service changed indication to all bonded and/or connected peers that subscribe to 
         * this indication. If a bonded peer is not connected, the indication is sent when it reconnects. 
         ************************************************************************************************************/
        inline void             localDatabaseHasChanged()               { pm_local_database_has_changed(); }

        /********************************************************************************************************//**
         * \brief   Set the size of the passkey.
         *
         * Set the size of the passkey to be used in security process. The passkey can be:
         *  - generated as a random number
         *  - hard-coded. Here the hardcoded passkey is 12345.. up to the given size!
         *  - can be read from the flash. To enable this feature, the Security class needs the 
         *    \see USE_INFORMATION_FROM_FLASH to be defined and set to 1.
         *
         * \param   size    The size of the passkey to be used.
         ************************************************************************************************************/
        void                    setPasskeySize(size_t size);

        /********************************************************************************************************//**
         * \brief   Initialize the security module using the `peer manager` provided by Nordic.
         *
         * \param   bleController           Pointer to the controller of the BLE Stack
         * \param   peripheralController    Pointer to the peripheral controller
         * \param   eraseBondedDevice       Indicates whether bonding information should be cleared from 
         *                                  persistent storage during initialization of the Device Manager.
         * \param   useFixedPasskey         Defines if the pairing passkey is fixed or not. Fixed means either 
         *                                  stored in the flash at production time or a fixed `123456` passkey. 
         *                                  This option is defined by \ref USE_INFORMATION_FROM_FLASH
         ************************************************************************************************************/
        void initialize(BleController*              bleController,
                        PeripheralController*       peripheralController,
                        bool                        eraseBondedDevice   = true,
                        bool                        useFixedPasskey     = false);
                        
        /********************************************************************************************************//**
         * \brief   Set up the security rights and acces according of the security level.
         *
         * \param   useJustWorksBonding         Indicates whether bonding process should use the Just Works 
         *                                      use-case or not.
         * \param   useMenInTheMiddleBonding    Indicates whether bonding process should use the Man In The Middle
         *                                      use-case or not.
         * \param   useFixedPasskey             Defines if the pairing passkey is fixed or not. Fixed means either
         *                                      stored in the flash at production time or a fixed `123456` passkey.
         *                                      This option is defined by \ref USE_INFORMATION_FROM_FLASH.
         ************************************************************************************************************/
        uint32_t setupSecurityParameter(bool useJustWorksBonding        = false,
                                        bool useMenInTheMiddleBonding   = false,
                                        bool useFixedPasskey            = false);

        /********************************************************************************************************//**
         * \brief   Function for generating a random passkey.
         *
         * This function will be called each time a new random passkey is required.
         ************************************************************************************************************/
        void generatePasskey();

        /********************************************************************************************************//**
         * \brief   Function for deleting the bonding information.
         *
         * This function deletes every piece of data that is associated with the specified peer and frees the peer ID
         * to be used for another peer. The deletion happens asynchronously, and the peer ID is not freed until the 
         * data is deleted. When the operation finishes, a \ref PM_EVT_PEER_DELETE_SUCCEEDED or 
         * \ref PM_EVT_PEER_DELETE_FAILED event is sent.
         *
         * \warning Use this function only when not connected to or connectable for the peer that is being deleted. 
         *          If the peer is or becomes connected or data is manually written in flash during this procedure 
         *          (until the success or failure event happens), the behavior is undefined.
         *
         * \param   peerId  Peer ID to be freed and have all associated data deleted.
         *
         * \retval NRF_SUCCESS                      If the bonding information has been deleted successfully.
         * \retval NRF_ERROR_INVALID_PARAM          If the given peer ID is not valid.
         ************************************************************************************************************/
        uint32_t deleteBondingInformation(pm_peer_id_t peerId);

        /********************************************************************************************************//**
         * \brief   Function for deleting the all bonding information stored for all peers.
         *
         * This function sends either a \ref PM_EVT_PEERS_DELETE_SUCCEEDED or a \ref PM_EVT_PEERS_DELETE_FAILED event. 
         * In addition, a \ref PM_EVT_PEER_DELETE_SUCCEEDED or \ref PM_EVT_PEER_DELETE_FAILED event is sent for each 
         * deleted peer.
         *
         * \warning Use this function only when not connected or connectable. If a peer is or becomes connected or a 
         *          \ref PM_PEER_DATA_FUNCTIONS function is used during this procedure (until  the success or failure 
         *          event happens), the behavior is undefined.
         ************************************************************************************************************/
        void deleteAllBondingInformation();

        /********************************************************************************************************//**
         * \brief   Function for getting the whitelist from the Peer Mananger.
         *
         * The function retrieves the whitelist of GAP addresses and IRKs that was previously set by 
         * \ref pm_whitelist_set.
         *
         * To retrieve only GAP addresses or only IRKs, provide only one of the buffers. If a buffer is provided,
         * its size must be specified.
         *
         * \param[out]      pAddrs      The buffer where to store GAP addresses. Pass NULL to retrieve only IRKs 
         *                              (in that case, \p p_irks must not be NULL).
         * \param[in,out]   pAddrCount  In:     The size of the \p p_addrs buffer.
         *                                      May be NULL if and only if \p p_addrs is NULL.
         *                              Out:    The number of GAP addresses copied into the buffer.
         *                                      If \p p_addrs is NULL, this parameter remains unchanged.
         * \param[out]      pIrks       The buffer where to store IRKs. Pass NULL to retrieve only GAP addresses 
         *                              (in that case, \p p_addrs must not NULL).
         * \param[in,out]   pIrkCount   In:     The size of the \p p_irks buffer.
         *                                      May be NULL if and only if \p p_irks is NULL.
         *                              Out:    The number of IRKs copied into the buffer.
         *                                      If \p p_irks is NULL, this paramater remains unchanged.
         *
         * \retval NRF_SUCCESS                      If the whitelist was successfully retrieved.
         * \retval BLE_ERROR_GAP_INVALID_BLE_ADDR   If a peer has an address that cannot be used for
         *                                          whitelisting (this error can occur only
         *                                          when using the S13x SoftDevice v2.x).
         * \retval NRF_ERROR_NULL                   If a required parameter is NULL.
         * \retval NRF_ERROR_NO_MEM                 If the provided buffers are too small.
         * \retval NRF_ERROR_NOT_FOUND              If the data for any of the cached whitelisted peers
         *                                          cannot be found. It might have been deleted.
         * \retval NRF_ERROR_INVALID_STATE          If the Peer Manager is not initialized.
         ************************************************************************************************************/
        uint32_t getWhitelist(ble_gap_addr_t*   pAddrs,
                              uint32_t*         pAddrCount,
                              ble_gap_irk_t*    pIrks,
                              uint32_t*         pIrkCount);
                              
        /********************************************************************************************************//**
         * \brief   Fetch the list of peer manager peer IDs.
         *
         * The function returns a list of IDs related to all already paired peer devices. This function starts 
         * searching from the first peer identifier. IDs ordering is the same as for \ref pm_next_peer_id_get(). If 
         * the peer id is invalid, the function starts searching from the first ID. 
         *
         * Only those IDs that match \p skipId are added to the list. The number of returned elements is determined
         * by \p pPeersSize.
         *
         * \param[inout]    pPeers      The buffer where to store the list of peer IDs. The memory for this buffer 
         *                              must be allocated!
         * \param[inout]    pPeersSize  In:     The size of the \p p_peers buffer.
         *                              Out:    The number of peers copied in the buffer.
         * \param[in]       skipId      It determines which peer ID will be added to list.
         ************************************************************************************************************/
        void getPeerList(pm_peer_id_t* pPeers, uint32_t* pPeersSize, pm_peer_id_list_skip_t skipId);

        /********************************************************************************************************//**
         * \brief   Function to know if the whitelist is empty or not.
         *
         * \return  `true`  If the whitelist is empty and flase otherwise.
         ************************************************************************************************************/
        bool isWhitelistEmpty();

        /********************************************************************************************************//**
         * \brief   Retrieves the BT addresse of a bonded peer device in advertisment when whitelist's usage is
         *          enabled.
         *
         * \retval  \ref NRF_SUCCESS                If the data was read successfully.
         * \retval  \ref NRF_ERROR_INVALID_DATA     If peer device's identifier is invalid.
         * \retval  \ref NRF_ERROR_INVALID_PARAM    If the the data type or the peer ID was invalid or unallocated,
         *                                          or if the length in \p p_length was not a multiple of 4.
         * \retval  \ref NRF_ERROR_INVALID_STATE    If the Peer Manager is not initialized or a reply was not expected..
         * \retval  \ref NRF_ERROR_NULL             If a pointer parameter was NULL.
         * \retval  \ref NRF_ERROR_NOT_FOUND        If no stored data was found for this peer ID/data ID combination.
         * \retval  \ref NRF_ERROR_DATA_SIZE        If the provided buffer was not large enough.
         ************************************************************************************************************/
        uint32_t peerAddressRequest();

        /********************************************************************************************************//**
         * \brief   Retrieves a value for the security level related to the desired configuration of bonding process.
         *
         * According to the desired bonding/pairing process (JUST_WORKS or MITM), the method retrieves a value for
         * the Security Level Identifier. This value can be used to set up the complete security module.
         * This feature has been added to security module to let any application dynamically adjust its security level.
         *
         * \param   useJustWorks    Setup for JUST_WORKS bonding/pairing scenario
         * \param   useMitm         Setup for MITM (Man In The Middle) bonding/pairing scenario
         ************************************************************************************************************/
        eSecurityLevelId securityLevelFromConfig(bool useJustWorks, bool useMitm);

        /********************************************************************************************************//**
         * \brief   This method cares about the Peer Manager incoing event !
         ************************************************************************************************************/
        void onPeerManagerEvent(pm_evt_t const* pEvent);
        
        /********************************************************************************************************//**
         * \name    BleObserverInterface
         * \brief   BleObserver interface implementation
         * \{
         ************************************************************************************************************/
        virtual void onBleEvent(ble_evt_t const* pBleEvent, void* pContext);
        virtual void onSystemEvent(uint32_t systemEvent);
        /** \} */

    private:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PRIVATE DECLARATION SECTION                                                                              */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        // Attributes
        PeripheralController*       _peripheralController;
        ble_gap_sec_params_t        _securityParameter;

        pm_peer_id_t                _peerId;
        pm_peer_id_t                _whitelistPeers[BLE_GAP_WHITELIST_ADDR_MAX_COUNT];
        uint32_t                    _whitelistPeersCount;
        eSecurityLevelId            _securityLevel;
        bool                        _useWhitelist;
        bool                        _peerManagerInitialized;
        bool                        _peerManagerRegistered;

        uint8_t*                    _passkey;
        size_t                      _passkeySize;
        ble_opt_t                   _bleOpt;

        // Methods
        /************************************************************************************************************
         * \brief   Function to create a list of peers currently in the whitelist.
         ************************************************************************************************************/
        void _createWhitelistPeers();

        /************************************************************************************************************
         * \brief   Function to clear the list of peers currently in the whitelist.
         ************************************************************************************************************/
        void _clearWhitelistPeers();

        /************************************************************************************************************
         * \brief   Function to match the securtiy level identifier from a given int value
         ************************************************************************************************************/
        void _setSecurityLevelIdFromInt(uint8_t level);
    };

}   // namespace ble

#endif // #if defined(SOFTDEVICE_PRESENT)

/** \} */   // Group: Peripheral Controller

/** \} */   // Group: BLE
