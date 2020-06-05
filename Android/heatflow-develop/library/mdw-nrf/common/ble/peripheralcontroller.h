/****************************************************************************************************************//**
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 *
 * \file    peripheralController.h
 *
 * \defgroup BLE Bluetooth Low Energy
 * \{
 *
 * \defgroup peripheral PeripheralController
 * \{
 *
 * \class   ble::PeripheralController
 * \brief   Any BLE communication starts with this class.
 *
 * # Peripheral Controller
 *
 * PeripheralController is the class that controls the entire process of a BLE communication.
 *
 * Within the PeripheralController, the application will have access to the all the BLE features needed by a device
 * playing the peripheral role. It starts from the advertisement to the connection itself including the security 
 * settings. The peripheralController handles any registered private service or adopted BLE service. It also
 * manages the connection's parameters and can also modify dynamically (if needed) the security settings applied on
 * each characteristic of the BLE services.
 *
 * It includes different modules such as Gap, Gatt, Advertiser or Security.
 *
 * Initial author: Patrice Rudaz
 * Creation date: 2017-01-15
 *
 * \author  Patrice Rudaz (patrice.rudaz@hevs.ch)
 * \date    July 2018
 ********************************************************************************************************************/
#pragma once

/* **************************************************************************************************************** */
/*                                                                                                                  */
/* Header files                                                                                                     */
/*                                                                                                                  */
/* **************************************************************************************************************** */
#if defined(SOFTDEVICE_PRESENT)
#include <ble.h>
#include <ble_advertising.h>
#include <ble_conn_params.h>
#include <ble_gap.h>
#include <peer_manager.h>

#include <ble-advertiser-config.h>
#include <ble-gap-config.h>
#include <ble-gatt-config.h>
#include <ble-security-config.h>
#include <ble-services-config.h>
#include <ble-private-service-config.h>

#include "ble/advertiser.h"
#include "ble/gap.h"
#include "ble/gatt.h"
#include "ble/interface/bleobserver.h"
#include "ble/interface/blegapobserver.h"
#include "ble/interface/bleservicesobserver.h"
#include "ble/interface/peripheralobserver.h"
#include "ble/peripheralpersistor.h"
#include "ble/security.h"
#include "ble/services/services.h"
#include "ble/services/blevendor.h"
#include "ble/services/privateservice.h"
#include "ble/blecontroller.h"

#include "core/isubject.hpp"
#include "tools/sdktools.h"


#if (USE_CALENDAR != 0)
    #include "calendar/interface/calendarobserver.h"
#else
    #include <core/xfreactive.h>
#endif  //#if (USE_CALENDAR != 0)

#if (USE_FLASH != 0)
    #include "flash/interface/persister.h"
#endif

#if defined(BLE_DB_DISCOVERY_ENABLED) && (BLE_DB_DISCOVERY_ENABLED != 0)
    #include "ble_db_discovery.h"
#endif  // defined(BLE_DB_DISCOVERY_ENABLED) && (BLE_DB_DISCOVERY_ENABLED != 0)

#if (ANCS_SUPPORT != 0)
    #include "ios/ancshandler.h"
#endif
#if (AMS_SUPPORT != 0)
    #include "ios/amshandler.h"
#endif

#if defined(__cplusplus)
extern "C" 
{
#endif
    namespace ble 
    {
        /********************************************************************************************************//**
         * \brief   Function for handling the timeout for the advertisement without the whitelist
         *
         * \details This function will be called each time the advertisement is restarting without the whitelist.
         *
         * \param   pContext    Pointer used for passing some arbitrary information (context) from the
         *                      `app_start_timer()` call to the timeout handler.
         ************************************************************************************************************/
        void advertiseWithoutWhitelistTimeoutHandler(void* pContext);

        
        #if defined(BLE_DB_DISCOVERY_ENABLED) && (BLE_DB_DISCOVERY_ENABLED != 0)
        /********************************************************************************************************//**
         * \brief   Function for dispatching the BLE DB Discovery events
         *
         * \details This function will be called each time the DB Discovery modules receive BLE DB Events
         *
         * \param   pDbDiscoveryEvent  Pointer used for passing the DB Discovery Event
         ************************************************************************************************************/
        void bleDbDiscoveryDispatch(ble_db_discovery_evt_t* pDbDiscoveryEvent);
        #endif // defined(BLE_DB_DISCOVERY_ENABLED) && (BLE_DB_DISCOVERY_ENABLED != 0)
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
    class PeripheralController : 
          public ISubject<ble::PeripheralObserver>
        , public BleGapObserver
        , public ble::BleServicesObserver
    #if (USE_FLASH != 0)
        , public flash::Persister
    #endif
    #if (USE_CALENDAR != 0)
        , public cal::CalendarObserver
    #else
        , public XFReactive
    #endif
    {

        #if (USE_CALENDAR == 0)
        friend void highReactivityTimeoutHandler(void* pContext);
        #endif
        friend void advertiseWithoutWhitelistTimeoutHandler(void* pContext);

        #if defined(BLE_DB_DISCOVERY_ENABLED) && (BLE_DB_DISCOVERY_ENABLED != 0)
        friend void bleDbDiscoveryDispatch(ble_db_discovery_evt_t* pDbDiscoveryEvent);
        #endif // defined(BLE_DB_DISCOVERY_ENABLED) && (BLE_DB_DISCOVERY_ENABLED != 0)

    public:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* CONSTRUCTOR SECTION                                                                                      */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        PeripheralController();
        virtual ~PeripheralController();

        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PUBLIC DECLARATION SECTION                                                                               */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        /********************************************************************************************************//**
         * \brief   Enumeration for the available states of the device.
         ************************************************************************************************************/
        typedef enum
        {
            PERIPHERAL_STATE_LOW_ENERGY = 0,        ///< The device is connected but in Low Energy mode meaning that not really reactive. Connection's interval is set to a long delay.
            PERIPHERAL_STATE_BUSY,                  ///< The BLE stack is working hard. The third application must wait until the device goes back in Low Energy or High Reactivity mode.
            PERIPHERAL_STATE_HIGH_REACTIVITY,       ///< The connection's interval is set to a short delay and the peripheral becomes very reactiv...
            PERIPHERAL_STATE_CHANGE_ERROR           ///< Something goes wrong, for sure !
        } peripheralState_e;

        /********************************************************************************************************//**
         * \brief   Initialization of the class
         *
         * This method initializes all parameters of the PeripheralController Class. Some parameters can be provided
         * to define the connection's parameter in both Low Power and High Reactivity mode. If any of these 
         * parameters is NULL, the connection's setting defined in <ble-gap-config.h> file will be applied.
         *
         * \param   lowPowerConnParams          The connection's settings to let the device going in Low Power mode 
         *                                      while connected. (Default value: NULL)
         * \param   highReactivityConnParams    The connection's settings to be in High Reactivity mode when 
         *                                      connected. (Default value: NULL)
         ************************************************************************************************************/
        void initialize(ble_gap_conn_params_t* lowPowerConnParams = NULL, ble_gap_conn_params_t* highReactivityConnParams = NULL);

        /********************************************************************************************************//**
         * \brief   Binds the components needed by this class
         ************************************************************************************************************/
        void bind(BleController* bleCtrl
        #if (USE_CALENDAR != 0)
                , cal::Calendar* calendar = NULL
        #endif
        #if (ANCS_SUPPORT != 0)
                , notification::ANCSHandler* ancsHandler = NULL
        #endif
        #if (AMS_SUPPORT != 0)
                , media::AMSHandler* amsHandler = NULL
        #endif
        );

        /********************************************************************************************************//**
         * \brief   Initialize the Security and the GAP profile.
         *
         * This method initialize the security settings and the connection's parameters using the ones stored in
         * FLASH.
         ************************************************************************************************************/
        void initSecurityAndGap();

        /********************************************************************************************************//**
         * \brief   Initialize the Security and the GAP profile.
         *
         * This method initialize the security settings and the connection's parameters using the given parameters.
         *
         * \param   useJustWorks        Enable/Disable the usage of the "Just Works" use case for the security.
         * \param   useMitm             Enable/Disable the usage of the "Men in the middle" use case for the security.
         * \param   useWhitelist        Enable/Disable the usage of the whitelist, which contains the bonded devices.
         * \param   highReactivityMode  If set to `true`, the GAP is initialized with the unique usage of the
         *                              "High Reactivity" mode (only short connection's interval). Otherwise, both
         *                              "Low Power" and "High Reactivity" modes will be available.
         ************************************************************************************************************/
        void initSecurityAndGap(bool  useJustWorks, bool useMitm, bool useWhitelist, bool highReactivityMode = false);

        /********************************************************************************************************//**
         * \brief   Set dinamically the Security and the GAP profile.
         *
         * This method MUST be used to update dynamically the security settings and the connection's parameters using 
         * the given parameters.
         *
         * \param   useJustWorks        Enable/Disable the usage of the "Just Works" use case for the security.
         * \param   useMitm             Enable/Disable the usage of the "Men in the middle" use case for the security.
         * \param   useWhitelist        Enable/Disable the usage of the whitelist, which contains the bonded devices.
         * \param   highReactivityMode  If set to `true`, the GAP will only use the "High Reactivity" mode (only 
         *                              short connection's interval). Otherwise, both "Low Power" and "High 
         *                              Reactivity" modes will be available.
         ************************************************************************************************************/
        void setSecuritySettings(bool useJustWorks, bool useMitm, bool useWhitelist, bool highReactivityMode = false);

        /********************************************************************************************************//**
         * \brief   Applies the new security settings to the characteristics.
         *
         * This method applies dynamically the new security settings to the characteristics of the registered services.
         ************************************************************************************************************/
        void updateSecurityForCharacteristics();

        /********************************************************************************************************//**
         * \brief   Retrieves the current setting according the usage of the "Just Works" use case.
         ************************************************************************************************************/
        inline bool                         useJustWorksBonding() const { return _persistor.useJustWorks(); }

        /********************************************************************************************************//**
         * \brief   Retrieves the current setting according the usage of the "Men in the middle" use case.
         ************************************************************************************************************/
        inline bool                         useMitmBonding() const      { return _persistor.useMitm(); }

        /********************************************************************************************************//**
         * \brief   Retrieves the current setting according the usage of the whitelist, the list of bonded devices.
         ************************************************************************************************************/
        inline bool                         useWhiteList() const        { return _persistor.useWhiteList(); }

        // Getters
        /********************************************************************************************************//**
         * \brief   Retrieves the connection's handle.
         * 
         * Retrieve the current connection's handle. If the device is not connected, \ref BLE_CONN_HANDLE_INVALID 
         * will be returned.
         ************************************************************************************************************/
        inline uint16_t                     connHandle()                { return _connHandle; }

        /********************************************************************************************************//**
         * \brief   Returns the connection's state: `true` if the device is connected or `false otherwise`.
         * 
         * \return  `true` if the device is connected or `false` otherwise.
         ************************************************************************************************************/
        inline bool                         isConnected() const         { return _connHandle != BLE_CONN_HANDLE_INVALID; }

        /********************************************************************************************************//**
         * \brief   Returns the bluetooth address of the device
         ************************************************************************************************************/
        inline ble_gap_addr_t*              btAddress()                 { return &_btaddress; }
        

        /********************************************************************************************************//**
         * \brief   Returns the current radio's transmit power used for advertising.
         *
         * \note    Supported values: -40dBm, -20dBm, -16dBm, -12dBm, -8dBm, -4dBm, 0dBm, +3dBm and +4dBm.
         ************************************************************************************************************/
        inline int8_t                       advTxPowerLevel()           { return _txPowerLevel(BLE_GAP_TX_POWER_ROLE_ADV); }

        /********************************************************************************************************//**
         * \brief   Returns the current radio's transmit power used for the connection.
         *
         * \note    Supported values: -40dBm, -20dBm, -16dBm, -12dBm, -8dBm, -4dBm, 0dBm, +3dBm and +4dBm.
         ************************************************************************************************************/
        inline int8_t                       connTxPowerLevel()          { return _txPowerLevel(BLE_GAP_TX_POWER_ROLE_CONN); }


        /********************************************************************************************************//**
         * \brief   Returns a pointer on the \see ble::Gap object related to the peripheral controller.
         *
         * \return  A pointer to the GAP object used by the current peripheral.
         ************************************************************************************************************/
        inline Gap*                         gap()                       { return &_gap; }

        /********************************************************************************************************//**
         * \brief   Returns a pointer on the \see ble::Gatt object related to the peripheral controller.
         *
         * \return  A pointer to the GATT object used by the current peripheral.
         ************************************************************************************************************/
        inline Gatt*                        gatt()                      { return &_gatt; }

        /********************************************************************************************************//**
         * \brief   Returns a pointer on the \see ble::Advertiser object related to the peripheral controller.
         *
         * \return  A pointer to the Advertiser object used by the current peripheral.
         ************************************************************************************************************/
        inline Advertiser*                  advertiser()                { return &_advertiser; }

        /********************************************************************************************************//**
         * \brief   Returns the security level. 
         *
         * \return  The current security level as an identifier. \ref ble::Security#getSecurityLevelId()
         ************************************************************************************************************/
        inline Security::eSecurityLevelId   securityLevelId() const     { return  _security.getSecurityLevelId(); }

        /********************************************************************************************************//**
         * \brief   Returns a pointer on BLE adopted services used by the peripheral controller.
         *
         * Return the pointer on the \see ble::Services object related to the peripheral controller. This object,
         * which can't be NULL, handles all BLE services adopted by the bluetooth SIG and used by the current device.
         *
         * \return  A pointer to the adopted services object.
         ************************************************************************************************************/
        inline Services*                    bleAdoptedServices()        { return &_bleServices; }

        /********************************************************************************************************//**
         * \brief   Returns a pointer on a specific BLE private service used by the peripheral.
         *
         * Return the pointer on the \ref ble::PrivateService identified by the given parameter. It returns NULL, if 
         * no service matches the given UUID
         *
         * \param   uuid    The UUID identifying the private service we want to work with
         *
         * \return  A pointer to the specified private service, or NULL if no service matches the given UUID 
         ************************************************************************************************************/
        PrivateService*                     getService(uint16_t uuid);

        /********************************************************************************************************//**
         * \brief   Registration of the given BLE private service to the current peripheral.
         *
         * This method MUST be used to register any \ref ble::PrivateService needed by the current peripheral. The
         * service id identified by the given parameter. It returns `true` if the process succeeds and `false` 
         * otherwise.
         *
         * \param   service Pointer to the private service to be stored.
         *
         * \return  `true` if the registration succeeds and `false` otherwise.
         ************************************************************************************************************/
        bool                                registerService(PrivateService* service);


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
         ************************************************************************************************************/
        inline void deleteBondingInformation(pm_peer_id_t peerId)       { _security.deleteBondingInformation(peerId); }

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
        inline void deleteAllBondingInformation()                       { _security.deleteAllBondingInformation(); }

        /********************************************************************************************************//**
         * \brief   Set the radio's transmit power.
         *
         * Adjust the radio's transmit power according to the given level. Supported tx_power values: -40dBm, -20dBm, 
         * -16dBm, -12dBm, -8dBm, -4dBm, 0dBm, +3dBm and +4dBm. 
         *
         * The power can be changed for connection or advertising's role. The initiator will have the same transmit 
         * power as the scanner. When a connection is created it will inherit the transmit power from the initiator
         * or advertiser leading to the connection.
         *
         * \param   level   Radio transmit power in dBm (see note for accepted values).
         * \param   role    The role to set the transmit power for, see \ref BLE_GAP_TX_POWER_ROLES for possible 
         *                  roles. Default: BLE_GAP_TX_POWER_ROLE_ADV
         *
         * \return  `true` if the transmit power changed successfully and `false` otherwise.
         ************************************************************************************************************/
        bool setTxPowerLevel(int8_t level, uint8_t role = BLE_GAP_TX_POWER_ROLE_ADV);

        /********************************************************************************************************//**
         * \name   SecurityObserverInterface
         * \brief  Callback methods coming from the Security.
         * \{
         ************************************************************************************************************/
        inline void onPeerManagerEvent(pm_evt_t const* pEvent)          { _security.onPeerManagerEvent(pEvent); }
        void onPasskeyId(bool correct)                                  {}
        void onBondingInformationDeleted()                              {}
        /** \} */

        #if (IAS_SUPPORT != 0)
        /// \brief  Immediate Alert Service Observer implementation
        virtual void onImmediateAlertChanged(uint8_t alertLevel);
        #endif


        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* ADVERTISEMENT RELATED                                                                                    */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        /********************************************************************************************************//**
         * \brief   Start advertising, according to the dedicated parameters.
         *
         * You can start advertising in any of the advertising modes that you enabled during initialization. By
         * default, the peripheral will start advertising in BLE_ADV_MODE_FAST mode. 
         * If \c withoutWhitelist is set to \c true, this function temporarily disables whitelist advertising.
         *
         * If \ref ADVERTISE_WHITELIST_EMPTY is defined and set to `1` and the usage of the whitelist is enabled, the
         * peripheral will start advertising only the the whitelist is not empty. In that case, if you want to bond
         * a device, you need to start advertising and skip the whitelist using the `withoutWhitelist` parameter.
         * (Same process to bond one more device)
         *
         * \param   withoutWhitelist    To disable the whitelist advertising. Default is \c false
         * \param   advertisingMode     Advertising mode.
         ************************************************************************************************************/
        void startAdvertising(bool withoutWhitelist = false, ble_adv_mode_t advertisingMode = BLE_ADV_MODE_FAST);

        /********************************************************************************************************//**
         * \brief   Stops the advertisement.
         ************************************************************************************************************/
        inline void stopAdvertising()                                   { _advertiser.stopAdvertising(); }

        /********************************************************************************************************//**
         * \brief   Handles the necessary actions when an advertisement skipping a whitelist ends.
         *
         * Advertising without the whitelist last not endlessly. It could be stopped by the timweout of a timer, it 
         * reach the last advertising mode and stops, or a third application can decide to stops it. This method can
         * be called manually or used as callback method of the timer to ends properly the advertisement. It clears
         * the `skip whitelist` flag and set the parameter back and starts advertising normally (if needed).
         ************************************************************************************************************/
        void backFromSkippingWhitelist();

        /********************************************************************************************************//**
         * \brief   Function for setting the peer address.
         *
         * The peer address must be set by the application upon receiving a \ref BLE_ADV_EVT_PEER_ADDR_REQUEST event.
         *
         * \param   pPeerAddr   Pointer to a peer address.
         *
         * \retval \ref NRF_SUCCESS             Successfully stored the peer address pointer in the advertising module.
         * \retval \ref NRF_ERROR_INVALID_STATE If a reply was not expected.
         ************************************************************************************************************/
        inline uint32_t advertisingPeerAddrReply(ble_gap_addr_t* peerAddr) 
        { 
            return _advertiser.peerAddressReply(peerAddr); 
        }

        /********************************************************************************************************//**
         * \name   AdvertiserObserverInterface
         * \brief  Callback methods coming from the Advertiser.
         * \{
         ************************************************************************************************************/
        void onAdvStateChanged(ble_adv_evt_t bleAdvEvent);
        void onAdvError(uint32_t nrfError);
        void onAdvWhitelistReq();
        inline void onAdvPeerAddrReq()                                  { _security.peerAddressRequest(); }
        /** \} */


        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* GAP RELATED                                                                                              */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        /********************************************************************************************************//**
         * \brief   Request the High Reactivity mode
         *
         * This method negotiates new connection parameters to make it more responsive. The High Reactivity mode 
         * means that the predefined settings for the connection intervals are of the order of tens of milliseconds.
         *
         * If the given parameter \c useHighReactivityTimer is set to `true` (default value), a timer will be started
         * for a predefined delay as soon as the process succeeded. When the timeout is reached, the peripheral goes
         * back to the Low Energy mode. \see ble::PeripheralController#setLowEnergyMode().
         *
         * \param   useHighReactivityTimer  Boolean flag to enable/disable the usage of a timer with the High 
         *                                  Reactivity mode.
         ************************************************************************************************************/
        void setHighReactivityMode(bool useHighReactivityTimer = true);

        /********************************************************************************************************//**
         * \brief   Request the High Reactivity mode
         *
         * This method negotiates new connection parameters to make it less energy consuming. The Low Energy mode 
         * means that the predefined settings for the connection intervals are of the order of several hundred of 
         * milliseconds.
         *
         * \see     ble::PeripheralController#setHighReactivityMode().
         ************************************************************************************************************/
        void setLowEnergyMode();

        /********************************************************************************************************//**
         * \brief   Reset the counter of the High Reactivity mode's timer.
         ************************************************************************************************************/
        #if (USE_CALENDAR != 0)
        inline void resetHighReactivityCounter()                        { _highReactivityCounter = HIGH_REACTIVITY_DELAY_COUNTER; }
        #else
        void resetHighReactivityCounter();
        #endif

        /********************************************************************************************************//**
         * \brief   Close the current connection
         *
         * This method closes the active connection. Only two reasons are accepted for forcing the peripheral to 
         * close the connection. Those reasons have an HCI flag to be pass with the disconnect request: 
         * \ref BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION and \ref BLE_HCI_CONN_INTERVAL_UNACCEPTABLE). If the given 
         * \c intervalUnacceptable is set to `true`, the flag `BLE_HCI_CONN_INTERVAL_UNACCEPTABLE` will be used.
         *
         * \param   intervalUnacceptable    Defines the reason to be invoked while closing the connection.
         ************************************************************************************************************/
        void disconnect(bool intervalUnacceptable = false);

        /********************************************************************************************************//**
         * \name   GapObserverInterface
         * \brief  Callback methods coming from the GAP.
         * \{
         ************************************************************************************************************/
        void         onGapConnParamEventFailed();
        virtual void onGapConnParamUpdate(Gap* gap, uint16_t connHandle);
        virtual void onGapConnParamUpdateError(Gap* gap, uint32_t errCode);
        virtual void onGapConnected(Gap* gap, uint16_t connHandle);
        virtual void onGapDisconnected(Gap* gap, uint16_t connHandle);
        virtual void onGapPasskeyDisplay(Gap* gap, uint16_t connHandle, char* passkey);
        virtual void onGapAuthenticationStatus(Gap* gap, uint16_t connHandle, ble_gap_evt_auth_status_t authStatus);
        /** \} */
        

        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* GATT RELATED                                                                                             */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        /********************************************************************************************************//**
         * \brief   Handles the GATT events...
         ************************************************************************************************************/
        inline void gattEventHandler(nrf_ble_gatt_t* pGatt, nrf_ble_gatt_evt_t const* pEvent) 
        { 
            _gatt.gattEventHandler(pGatt, pEvent); 
        }
        
        #if defined(BLE_DB_DISCOVERY_ENABLED) && (BLE_DB_DISCOVERY_ENABLED != 0)
        /********************************************************************************************************//**
         * \brief   To start a service discovery in the connected device.
         *
         * ToDo Add comment here
         *
         * \param   connHandle          Connection's handle.
         *
         * \retval  NRF_SUCCESS         If the discovery could start successfully
         * \retval  NRF_INVALID_STATE   If the current peripheral is not connected to any device.
         * \retval  ToDo
         ************************************************************************************************************/
        uint32_t startServiceDiscovery(uint16_t connHandle);
        #endif
        

        #if (USE_CALENDAR != 0)
        /********************************************************************************************************//**
         * \name    CalendarObserverInterface
         * \brief   Implementation of the cal::CalendarObserver callabck method.
         ************************************************************************************************************/
        virtual void onSecond(cal::Calendar* calendar);
        #endif

        #if (USE_FLASH != 0)
        /********************************************************************************************************//**
         * \name   FlashPersiterInheritance
         * \brief  Inherit from FlashPersiter.
         * \{
         ************************************************************************************************************/
        virtual uint32_t* save(uint16_t* size);
        virtual void restore(uint32_t* data);
        /** \} */
        #endif
        
        /********************************************************************************************************//**
         * \brief   DEBUG method converting peripheral's state in string...
         ************************************************************************************************************/
        static const char* state2String(peripheralState_e state)
        {
            switch(state)
            {
                case PERIPHERAL_STATE_LOW_ENERGY:       return "PERIPHERAL_STATE_LOW_ENERGY";
                case PERIPHERAL_STATE_BUSY:             return "PERIPHERAL_STATE_BUSY";
                case PERIPHERAL_STATE_HIGH_REACTIVITY:  return "PERIPHERAL_STATE_HIGH_REACTIVITY";
                case PERIPHERAL_STATE_CHANGE_ERROR:     return "PERIPHERAL_STATE_CHANGE_ERROR";
                default:                                return "PERIPHERAL_STATE_UNKNOWN";
            }
        }

    protected:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PROTECTED DECALRATION SECTION                                                                            */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        #if (USE_CALENDAR == 0)
        /********************************************************************************************************//**
         * \brief   XFReactive interface implementation
         ************************************************************************************************************/
        virtual EventStatus processEvent();
        #endif // #if (USE_CALENDAR == 0)

        #if defined(BLE_DB_DISCOVERY_ENABLED) && (BLE_DB_DISCOVERY_ENABLED != 0)
        void dbDiscoveryEventHandler(ble_db_discovery_evt_t* pDbDiscoveryEvent);
        #endif // defined(BLE_DB_DISCOVERY_ENABLED) && (BLE_DB_DISCOVERY_ENABLED != 0)

    private:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PRIVATE DECLARATION SECTION                                                                              */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        // Attributes
        PeripheralPersistor         _persistor;
        BleController*              _bleController;
        uint16_t                    _connHandle;
        ble_gap_conn_params_t       _loPowerConnParams;
        ble_gap_conn_params_t       _hiReactConnParams;
        ble_gap_conn_sec_mode_t     _securityMode;

        #if defined(BLE_DB_DISCOVERY_ENABLED) && (BLE_DB_DISCOVERY_ENABLED != 0)
        ble_db_discovery_t          _dbDiscovery;
        bool                        _newDiscoveryNeeded;
        #endif // defined(BLE_DB_DISCOVERY_ENABLED) && (BLE_DB_DISCOVERY_ENABLED != 0)

        uint16_t                    _highReactivityCounter;

        bool                        _skipWhitelist;
        bool                        _isHighReactivityModeTimerEnabled;
        bool                        _useHighReactivityTimer;

        ble_gap_addr_t              _btaddress;
        ble_gap_addr_t              _whitelistAddress[BLE_GAP_WHITELIST_ADDR_MAX_COUNT];
        ble_gap_irk_t               _whitelistIrks[BLE_GAP_WHITELIST_ADDR_MAX_COUNT];
        uint32_t                    _addressCount;
        uint32_t                    _irkCount;

        Advertiser                  _advertiser;
        Gap                         _gap;
        Gatt                        _gatt;
        Security                    _security;

        typedef struct 
        {
            bool isRequested    : 1;
            bool highReactivity : 1;
        } securityRequest_t;
        securityRequest_t           _securityRequest;


        Services                    _bleServices;
        static const uint8_t        _nbPrivateServices = (NUMBER_OF_HEI_PRIVATE_SERVICES + NUMBER_OF_SOPROD_PRIVATE_SERVICES);
        #if (NUMBER_OF_HEI_PRIVATE_SERVICES + NUMBER_OF_SOPROD_PRIVATE_SERVICES > 0)
        PrivateService*             _privateServices[_nbPrivateServices];
        #endif  // #if (NUMBER_OF_HEI_PRIVATE_SERVICES + NUMBER_OF_SOPROD_PRIVATE_SERVICES > 0)

        #if (USE_CALENDAR != 0)
        cal::Calendar*              _calendar;
        #endif
        #if (AMS_SUPPORT != 0)
        media::AMSHandler*          _amsHandler;
        #endif
        #if (ANCS_SUPPORT != 0)
        notification::ANCSHandler* _ancsHandler;
        #endif
        
        // Methods
        void _handleHighReactivityModeTimer(bool enable);
        inline int8_t               _txPowerLevel(uint8_t role)             { return  tools::SDKTools::getTxPower(role); }
        void                        _setAdvertiseForPrivateServices();

        void                        _notifyAdvState(ble_adv_evt_t bleAdvEvent);
        void                        _notifyAdvError(uint32_t errCode);
        void                        _notifyBleEvent(ble_evt_t* p_ble_evt);
        void                        _notifyConnectionState(bool connected);
        void                        _notifyGapState(peripheralState_e state);
        void                        _notifyPasskeyDisplay(char* passkey);
        void                        _notifyAuthStatus(ble_gap_evt_auth_status_t status);

        #if (IAS_SUPPORT != 0)
        void                        _notifyOnImmediateAlertStateChanged(uint8_t alertLevel);
        #endif        
    };

}   // namespace ble

#endif  // #if defined(SOFTDEVICE_PRESENT)

/** \} */   // Group: Peripheral Controller

/** \} */   // Group: BLE
