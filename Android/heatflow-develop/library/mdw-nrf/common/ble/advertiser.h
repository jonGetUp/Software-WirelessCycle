/****************************************************************************************************************//**
 * Copyright (C) Soprod SA & Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2018
 * Created by Thierry Hischier, Patrice Rudaz
 * All rights reserved.
 *
 * \file    advertiser.h
 *
 * \addtogroup BLE
 * \{
 *
 * \addtogroup peripheral
 * \{
 *
 * \class   ble::Advertiser
 * \brief   Class handling BLE advertisement feature provided by the Nordic SoftDevice (nRF5-SDK15)
 *
 * The Advertiser module manages completely the advertising feature in terms of BLE technology. As the advertisement
 * is a feature strongly related to a peripheral device, this module MUST only be used through the class 
 * \ref ble::PeripheralController
 *
 * The advertising interval of your peripheral affects the time to discovery and connect performance. Outside of ideal
 * conditions, for example when your app is no longer active in the foreground, the time to discovery becomes longer.
 * To maximize the probability of being discovered by an Apple device, the accessory must advertise at one of the listed
 * intervals exactly.
 *
 * The recommended advertising pattern and advertising intervals are:
 * - First, advertise at 20 ms intervals for at least 30 seconds
 * - If not discovered after 30 seconds, you may change to one of the following longer intervals: 152.5 ms, 211.25 ms,
 *   318.75 ms, 417.5 ms, 546.25 ms, 760 ms, 852.5 ms, 1022.5 ms, 1285 ms
 *
 * Actually there is no recommendation for Android device !
 *
 * Initial author: Thierry Hischier, Patrice Rudaz
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
#include <platform-config.h>

#if defined(SOFTDEVICE_PRESENT)
#include <ble-advertiser-config.h>
#include <ble-gap-config.h>
#include <ble_types.h>
#include <ble_advertising.h>

#include "ble/blecontroller.h"
#include "interface/bleobserver.h"

#if !defined(COMPANY_IDENTIFIER)
    #error "COMPANY_IDENTIFIER not defined for advertiser.h"
#elif defined(COMPANY_IDENTIFIER) && (COMPANY_IDENTIFIER != 0)
    #include "ble/manufacturerdata.h"
#endif

#if defined(__cplusplus)
extern "C" 
{
#endif
    namespace ble 
    {
        /********************************************************************************************************//**
         * \brief   Function for handling advertising events.
         *
         * This function will be called for advertising events which are passed to the application.
         *
         * \param   ble_adv_evt     Advertising event.
         ************************************************************************************************************/
        void advEventHandler(ble_adv_evt_t const bleAdvEvent);

        /********************************************************************************************************//**
         * \brief   Function for handling advertising errors.
         *
         * This function will be called for advertising errors which are passed to the application.
         *
         * \param   nrf_error   Numerical code identifying the occured error.
         ************************************************************************************************************/
        void advErrorHandler(uint32_t nrfError);

    }  // namespace ble
#if defined(__cplusplus)
}
#endif

/* **************************************************************************************************************** */
/*                                                                                                                  */
/* APPLICATION defintion                                                                                            */
/*                                                                                                                  */
/* **************************************************************************************************************** */

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
    class Advertiser 
    {
        friend void advEventHandler(ble_adv_evt_t const bleAdvEvent);
        friend void advErrorHandler(uint32_t nrfError);

    public:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* CONSTRUCTOR SECTION                                                                                      */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        Advertiser();
        virtual ~Advertiser();

        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PUBLIC DECLARATION SECTION                                                                               */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        /********************************************************************************************************//**
         * \brief   Advertising functionality initialization.
         *
         * Encodes the required advertising data and passes it to the stack.
         * Also builds a structure to be passed to the stack when starting advertising.
         *
         * \param[in]   bleController           Pointer to the controller of the BLE stack
         * \param[in]   whitelistModeEnabled    Enable or not the Whitelist advertising mode
         * \param[in]   directedSlowModeEnabled Enable or not the DIRECTED advertising mode
         * \param[in]   directedFastModeEnabled Enable or not the DIRECTED High Duty advertising mode
         * \param[in]   fastModeEnabled         Enable or not the FAST advertising mode
         * \param[in]   slowModeEnabled         Enable or not the SLOW advertising mode
         * \param[in]   extendedModeEnabled     Enable or not the extended advertising. Make sure this mode is set to 
         *                                      `false` to preserve advertising functionality similar to older SDKs.
         * \param[in]   connectionConfigTag     Parameter to keep track of what connection settings will be used if 
         *                                      the advertising results in a connection.
         * \param[in]   alternateTimings        Let use alternative timings for the different advertising modes.
         ************************************************************************************************************/
        void initialize(bool    whitelistModeEnabled        = (APP_ADV_WHITELIST_ENABLE && ((USE_MITM_BONDING != 0) || (USE_JUST_WORKS_BONDING != 0))),
                        bool    directedSlowModeEnabled     = APP_ADV_DIRECTED_SLOW_MODE_ENABLE,
                        bool    directedFastModeEnabled     = APP_ADV_DIRECTED_FAST_MODE_ENABLE,
                        bool    fastModeEnabled             = APP_ADV_FAST_MODE_ENABLE,
                        bool    slowModeEnabled             = APP_ADV_SLOW_MODE_ENABLE, 
                        bool    extendedEnabled             = APP_ADV_EXTENDED_MODE_ENABLE,
                        uint8_t connectionConfigTag         = APP_BLE_CONN_CFG_TAG,
                        bool    alternateTimings            = false);

        /********************************************************************************************************//**
         * \brief   Setup the advertising modes according to the given parameters
         *
         * \param[in]   whitelistModeEnabled    Enable or not the Whitelist advertising mode
         * \param[in]   directedSlowModeEnabled Enable or not the DIRECTED advertising mode
         * \param[in]   directedFastModeEnabled Enable or not the DIRECTED High Dutyadvertising mode
         * \param[in]   fastModeEnabled         Enable or not the FAST advertising mode
         * \param[in]   slowModeEnabled         Enable or not the SLOW advertising mode
         * \param[in]   extendedModeEnabled     Enable or not the extended advertising. Make sure this mode is set to 
         *                                      `false` to preserve advertising functionality similar to older SDKs.
         * \param[in]   updateAdvertisingSet    If set to `true`, new advertiser's parameters will be updated in SD.
         ************************************************************************************************************/
        void setupParameters(bool whitelistModeEnabled      = false,
                             bool directedSlowModeEnabled   = APP_ADV_DIRECTED_SLOW_MODE_ENABLE,
                             bool directedFastModeEnabled   = APP_ADV_DIRECTED_FAST_MODE_ENABLE,
                             bool fastModeEnabled           = APP_ADV_FAST_MODE_ENABLE,
                             bool slowModeEnabled           = APP_ADV_SLOW_MODE_ENABLE,
                             bool extendedEnabled           = APP_ADV_EXTENDED_MODE_ENABLE,
                             bool updateAdvertisingSet      = true);

        /********************************************************************************************************//**
         * \brief   Set up the record of UUIDs to be advertised.
         *
         * Set up the record of UUIDs to be advertised in the advertise slot or the scan response's one. In advertise
         * slot, only 1 adopted service is allowed. All UUIDs advertised in that slot takes space in term of bytes,
         * and all those bytes won't be avaialble for the device's name, which will be truncated.
         *
         * \param[in]   serviceUUID     The UUID of the service we want to advertise
         * \param[in]   uuidType        Defines if the service is an adopted or a private service. For adopted service,
         *                              `uuidType` takes the value `BLE_UUID_TYPE_BLE` (0x01). For private services,
         *                              the value is given by the `sd_ble_uuid_vs_add(...)` method.
         * \param[in]   inScanResponse  If set to `false`, the UUID is advertised in the advertisement data, otherwise
         *                              the scan response will be used.
         *
         * \return `true` if the UUID could be added in the desired record and `false` otherwise.
         ************************************************************************************************************/
        bool setAdvertiseUUID(uint16_t serviceUUID, uint8_t uuidType, bool inScanResponse);

        /********************************************************************************************************//**
         * \brief   Start advertising or restart advertising wihtout the use of whitelist, according to the dedicated
         *          parameter.
         *
         * You can start advertising in any of the advertising modes that you enabled during initialization. If
         * \c without_whitelist is set to \c true, this function temporarily disables whitelist advertising. Calling 
         * this function resets the current time-out countdown.
         *
         * \param   without_whitelist   To disable the whitelist advertising. Default is \c false
         * \param   advertisingMode     Advertising mode.
         *
         * \retval \ref NRF_SUCCESS On success, else an error code indicating reason for failure.
         * \retval \ref NRF_ERROR_INVALID_STATE
         ************************************************************************************************************/
        uint32_t startAdvertising(bool withoutWhitelist             = false,
                                  ble_adv_mode_t advertisingMode    = BLE_ADV_MODE_FAST);

        /********************************************************************************************************//**
         * \brief   Stop advertising.
         *
         * \retval \ref NRF_SUCCESS On success, else an error code indicating reason for failure.
         * \retval \ref NRF_ERROR_INVALID_STATE
         ************************************************************************************************************/
        inline uint32_t stopAdvertising()           { return sd_ble_gap_adv_stop(_advertisingInstance.adv_handle); }

        /********************************************************************************************************//**
         * \brief   Function for setting the peer address.
         *
         * The peer address must be set by the application upon receiving a \ref BLE_ADV_EVT_PEER_ADDR_REQUEST event.
         * Without the peer address, the directed advertising mode will not be run.
         *
         * \param   pPeerAddr   Pointer to a peer address.
         *
         * \retval \ref NRF_SUCCESS             Successfully stored the peer address pointer in the advertising module.
         * \retval \ref NRF_ERROR_INVALID_STATE If a reply was not expected.
         ************************************************************************************************************/
        uint32_t peerAddressReply(ble_gap_addr_t* pPeerAddr);

        /********************************************************************************************************//**
         * \brief   Function for setting a whitelist.
         *
         * The whitelist must be set by the application upon receiving a \ref BLE_ADV_EVT_WHITELIST_REQUEST event.
         * Without the whitelist, the whitelist advertising for fast and slow modes will not be run.
         *
         * \param   pGapAddrs   The list of GAP addresses to whitelist.
         * \param   addrCnt     The number of GAP addresses to whitelist.
         * \param   pGapIRKs    The list of peer IRK to whitelist.
         * \param   irkCnt      The number of peer IRK to whitelist.
         *
         * \retval \ref NRF_SUCCESS             If the operation was successful.
         * \retval \ref NRF_ERROR_INVALID_STATE If a call to this function was made without a
         *                                      BLE_ADV_EVT_WHITELIST_REQUEST event being received.
         ************************************************************************************************************/
        inline uint32_t whitelistReply(ble_gap_addr_t const *  pGapAddrs,
                                       uint32_t                addrCnt,
                                       ble_gap_irk_t  const *  pGapIRKs,
                                       uint32_t                irkCnt)
        {
            return ble_advertising_whitelist_reply(&_advertisingInstance, pGapAddrs, addrCnt, pGapIRKs, irkCnt);
        }
        
        /********************************************************************************************************//**
         * \brief   The method inform about the current handle of the advertising set.
         *
         * \return  The current advertising handle
         ************************************************************************************************************/
        inline uint8_t advertisingHandle()          { return _advertisingInstance.adv_handle; }

        /********************************************************************************************************//**
         * \brief   Retrieves the TX power actually in use.
         *
         * The TX power can be set -20 to +4 dBm. This method retrieves the actual value of the advertising field, 
         * which contains the Radio Transmit Power in dBm. That does not mean that the radio transmission is 
         * effectively powered at this value. This is only an advertising information.
         *
         * \note    The accepted values are -20dBm, -16dBm, -12dBm, -8dBm, -4dBm, 0dBm, 3dBm and 4dBm.
         *
         * \return  The actual value contained in the field representing the Radio Transmit Power in dBm. 
         ************************************************************************************************************/
        inline int8_t   txPowerLevel()              { return _txPowerLevel; }

        /********************************************************************************************************//**
         * \brief   The method let know if the device is advertising or not.
         *
         * \return  `true` if the device is advertising and `false` otherwise.
         ************************************************************************************************************/
        inline bool     isAdvertising() const       { return _advertisingInstance.adv_mode_current != BLE_ADV_MODE_IDLE; }
        
        /********************************************************************************************************//**
         * \brief   The method can be used to know if the Directed advertising's mode is enabled or not.
         *
         * \return  `true` if the Directed advertsing's mode using the whitelist s enabled and `false` otherwise.
         ************************************************************************************************************/
        inline bool     isDirectedModeEnabled()     { return _advertisingInstance.adv_modes_config.ble_adv_directed_enabled; }

        /********************************************************************************************************//**
         * \brief   The method can be used to know if the Fast advertising's mode is enabled or not.
         *
         * \return  `true` if the Fast advertsing's mode is enabled and `false` otherwise.
         ************************************************************************************************************/
        inline bool     isFastModeEnabled()         { return _advertisingInstance.adv_modes_config.ble_adv_fast_enabled; }

        /********************************************************************************************************//**
         * \brief   The method can be used to know if the Slow advertising's mode is enabled or not.
         *
         * \return  `true` if the Slow advertsing's mode is enabled and `false` otherwise.
         ************************************************************************************************************/
        inline bool     isSlowModeEnabled()         { return _advertisingInstance.adv_modes_config.ble_adv_slow_enabled; }

        /********************************************************************************************************//**
         * \brief   The method can be used to know if the advertising's mode using the whitelist is enabled or not.
         *
         * \return  `true` if the advertsing's mode using the whitelist is enabled and `false` otherwise.
         ************************************************************************************************************/
        inline bool     isWhitelistModeEnabled()    { return _advertisingInstance.adv_modes_config.ble_adv_whitelist_enabled; }

        /********************************************************************************************************//**
         * \brief   The method can be used to read the interval setting of the Fast advertising's mode.
         *
         * \return  The current interval setting in [ms].  
         ************************************************************************************************************/
        inline uint32_t fastModeInterval()          { return _advertisingInstance.adv_modes_config.ble_adv_fast_interval; }

        /********************************************************************************************************//**
         * \brief   The method can be used to read the timeout of the Fast advertising's mode.
         *
         * \return  The current timeout of the advertising's mode in [ms].  
         ************************************************************************************************************/
        inline uint32_t fastModeTimeout()           { return _advertisingInstance.adv_modes_config.ble_adv_fast_timeout; }

        /********************************************************************************************************//**
         * \brief   The method can be used to read the interval setting of the Slow advertising's mode.
         *
         * \return  The current interval setting in [ms].  
         ************************************************************************************************************/
        inline uint32_t slowModeInterval()          { return _advertisingInstance.adv_modes_config.ble_adv_slow_interval; }

        /********************************************************************************************************//**
         * \brief   The method can be used to read the timeout of the Slow advertising's mode.
         *
         * \return  The current timeout of the advertising's mode in [ms].  
         ************************************************************************************************************/
        inline uint32_t slowModeTimeout()           { return _advertisingInstance.adv_modes_config.ble_adv_slow_timeout; }

        /********************************************************************************************************//**
         * \brief   The method MUST be used to stop advertising without the whitelist.
         *
         * When the whitelist is enabled, the peripheral device will advertise and filter out (connection, scan or 
         * both according to the configuration of the filter) as soon as the whitelist contains some bonded device(s).
         * 
         * To let an other device pair to that peripheral, it needs to advertise wihtout the whitelist. Therefore, 
         * there is a flag in the advertising module to skip the whitelist. This flag will be reset only if a new 
         * device completes the pairing process. This method let reset this flag even if no new device has paired to
         * the advertising peripheral.
         *
         * \note    To work, this method NEEDS that the ble_advertising module to be modified. the method
         *          `ble_advertising_clear_without_whitelist_flag(ble_advertising_t * const p_advertising)` MUST be
         *          added to that module.
         ************************************************************************************************************/
        inline void     clearSkipWhitelistFlag()    { ble_advertising_clear_without_whitelist_flag(&_advertisingInstance); }
        
        /********************************************************************************************************//**
         * \brief   The method let change the interval of both Fast & Slow advertising's mode.
         *
         * \param   fastInterval    The new interval for the Fast advertising's mode, given in [ms].
         * \param   slowInterval    The new interval for the Slow advertising's mode, given in [ms].
         *
         * \return  `true` if the new settings are correct and could be set, `false` otherwise.
         ************************************************************************************************************/
        bool            updateIntervals(uint32_t fastInterval, uint32_t slowInterval);

        /********************************************************************************************************//**
         * \brief   The method let change the timeout of both Fast & Slow advertising's mode.
         *
         * \param   fastInterval    The new timeout for the Fast advertising's mode, given in [s].
         * \param   slowInterval    The new timeout for the Slow advertising's mode, given in [s].
         *
         * \return  `true` if the new settings are correct and could be set, `false` otherwise.
         ************************************************************************************************************/
        bool            updateTimeouts(uint8_t advertiseTimeout, uint32_t slowTimeout, uint32_t fastTimeout);

        /********************************************************************************************************//**
         * \brief   The method let change the advertising's field of the Radio Transmit Power.
         *
         * \param   txPowerLevel    The value to update the field with.
         *
         * \return  `true` if the new setting is correct and could be set, `false` otherwise.
         *
         * \note    The accepted values are -20, -16, -12, -8, -4, 0, and 4 dBm (and 8 dBm for the nRF52840)...
         ************************************************************************************************************/
        bool            setTxPowerLevel(int8_t txPowerLevel);

        /********************************************************************************************************//**
         * \brief   Enable or disable the advertising's mode using the whitelist.
         *
         * \param   enabled         `true` to enable that mode and `false` to disable it.
         ************************************************************************************************************/
        void            setWhitelistModeEnabled(bool enabled);

        /********************************************************************************************************//**
         * \brief   Enable or disable the Directed advertising's mode.
         *
         * \param   enabled         `true` to enable that mode and `false` to disable it.
         ************************************************************************************************************/
        void            setDirectedModeEnabled(bool enabled);

        /********************************************************************************************************//**
         * \brief   Enable or disable the Fast advertising's mode.
         *
         * \param   enabled         `true` to enable that mode and `false` to disable it.
         ************************************************************************************************************/
        void            setFastModeEnabled(bool enabled);

        /********************************************************************************************************//**
         * \brief   Enable or disable the Slow advertising's mode.
         *
         * \param   enabled         `true` to enable that mode and `false` to disable it.
         ************************************************************************************************************/
        void            setSlowModeEnabled(bool enabled);

    private:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PRIVATE DECLARATION SECTION                                                                              */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        // Attributes
        int8_t                      _txPowerLevel;
        uint8_t                     _timeout;
        ble_advertising_t           _advertisingInstance;

    #if defined(COMPANY_IDENTIFIER) && (COMPANY_IDENTIFIER != 0)
        ManufacturerData            _manufacturerData;
        ble_advdata_manuf_data_t    _manufacturerSpecificData;
    #endif
        ble_advdata_t               _advertiseData;
        ble_uuid_t                  _advertiseUUIDs[APP_ADV_MAX_ADV_UUIDS_COUNT];
        uint8_t                     _advertiseUUIDsCount;
        ble_advdata_t               _scanResponseData;
        ble_uuid_t                  _scanResponseUUIDs[APP_ADV_MAX_RSP_VENDOR_UUIDS_COUNT];
        uint8_t                     _scanResponseUUIDsCount;

        // Configure an advertising set. Set, clear or update advertising and scan response data.
        bool                        _updateAdvertisingSet();
    };

}    // namespace ble

#endif // #if defined(SOFTDEVICE_PRESENT)

/** \} */   // Group: Peripheral Controller

/** \} */   // Group: BLE
