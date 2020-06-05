/****************************************************************************************************************//**
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 *
 * \file    gap.h
 *
 * \addtogroup BLE
 * \{
 *
 * \addtogroup peripheral
 * \{
 *
 * \class   ble::Gap
 * \brief   Class handling BLE GAP features provided by the Nordic SoftDevice
 *
 * Initial author: Patrice Rudaz
 * Creation date: 2016-08-24
 *
 * \author  Patrice Rudaz (patrice.rudaz@hevs.ch)
 * \date    February 2018
 ********************************************************************************************************************/
#pragma once

/* **************************************************************************************************************** */
/*                                                                                                                  */
/* Header files                                                                                                     */
/*                                                                                                                  */
/* **************************************************************************************************************** */
#include <platform-config.h>

#if defined(SOFTDEVICE_PRESENT)
#include <ble-gap-config.h>
#include <ble.h>
#include <ble_gap.h>
#include <ble_conn_params.h>
#include <peer_manager.h>

#if (GAP_USE_XF_TIMER != 0)
    #include <core/xfreactive.h>
#endif

#include "ble/interface/bleobserver.h"
#include "ble/interface/blegapobserver.h"
#include "ble/blecontroller.h"
#include "core/isubject.hpp"


#if defined(__cplusplus)
extern "C" 
{
#endif
    namespace ble 
    {
        /********************************************************************************************************//**
         * \brief   Function for handling the Connection Parameter events.
         *
         * This function will be called for all events in the Connection Parameters Module which are passed to the
         * application.
         *
         * \note    All this function does is to disconnect. This could have been done by simply setting the
         *          `disconnect_on_fail` configuration parameter, but instead we use the event handler mechanism to
         *          demonstrate its use.
         *
         * \param   pEvent  Event received from the Connection Parameters Module.
         ************************************************************************************************************/
        void onConnParamsEvent(ble_conn_params_evt_t* pEvent);
        
        /********************************************************************************************************//**
         * \brief   Connection Parameters module error handler.
         *
         * \param   nrfError    Error code containing information about what went wrong.
         ************************************************************************************************************/
        void onConnParamsErrorHandler(uint32_t nrfError);
        
        #if (GAP_USE_XF_TIMER == 0)
        /********************************************************************************************************//**
         * \brief   Function for waiting while the BLE stack is BUSY.
         *
         * \details This function will be called each time the BLE Busz timer expires.
         *
         * \param   pContext    Pointer used for passing some arbitrary information (context) from the
         *                      `app_start_timer()` call to the timeout handler.
         ************************************************************************************************************/
        void busyTimeoutHandler(void* pContext);
        #endif // #if (GAP_USE_XF_TIMER == 0)
    };  // namespace ble

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

    class Gap : public BleObserver, public ISubject<BleGapObserver, 1>
    #if (GAP_USE_XF_TIMER != 0)
        , public XFReactive
    #endif
    {
        friend void onConnParamsEvent(ble_conn_params_evt_t* pEvent);
        friend void onConnParamsErrorHandler(uint32_t nrfError);
        friend void busyTimeoutHandler(void* pContext);

    public:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* CONSTRUCTOR SECTION                                                                                      */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        Gap();
        virtual ~Gap();

        /* ******************************************************************************************************** */
        /*																										    */
        /* PUBLIC DECLARATION SECTION                                                                               */
        /*																										    */
        /* ******************************************************************************************************** */

        /********************************************************************************************************//**
         * \brief   Initialize the GAP (Generic Access Profile).
         *
         * \param   bleController           Pointer to the BLE stack controller object.
         * \param   peripheralController    Pointer to the BLE peripheral controller object.
         * \param   gapConnParams           Pointer to the current connection's parameters.
         * \param   securityMode            Pointer to the current security's settings.
         * \param   specificConnParams      Pointer to the a specific set of parameter's interval. Omit if `NULL`
         ************************************************************************************************************/
        void initialize(BleController*              bleController,
                        PeripheralController*       peripheralController,
                        ble_gap_conn_params_t*      stdConnParams,
                        ble_gap_conn_sec_mode_t*    securityMode,
                        ble_gap_conn_params_t*      specificConnParams = NULL);

        /********************************************************************************************************//**
         * \brief   Initialize the Connection Parameters module.
         ************************************************************************************************************/
        void connParamsInit();

        /********************************************************************************************************//**
         * \brief   Called each time the BLE busy Timer expires.
         *
         * This method will try to change the connection's parameter when the BLE stack get off of busy state.
         ************************************************************************************************************/
        void timeoutConnChangeParam();

        /********************************************************************************************************//**
         * \brief   Return `true` if the device is in high reactivity mode.
         *
         * This method will check the connection's parameters to determine if they are in a high reactivity mode. 
         * The "High Reactivity" mode means that the minimum and maximum ranges of the connection parameters are 
         * within a few tens of ms.
         *
         * \return `true` if the device is in high reactivity mode and `false` otherwise
         ************************************************************************************************************/
        inline bool isInHighReactivityMode()        { return (_checkConnectionParameters(true) == NRF_SUCCESS); }

        /********************************************************************************************************//**
         * \brief   Update the connection's parameter according to the desired reactivity mode.
         *
         * The peripheral device can update the connection's parameter to be more reactiv or to consumer less power.
         * This method, according to the given `highReactiv` parameter, will check the current connection's 
         * parameters. If the current parameters does not match the desired connection's parameters this function 
         * initiates a new negotiation by sending a request to the BLE stack.
         *
         * This function is useful for scenario where most of the time the application needs a relatively big 
         * connection interval, and just sometimes, for a temporary period requires shorter connection interval, for 
         * example to transfer a higher amount of data.
         *
         * In the central role this will initiate a Link Layer connection parameter update procedure, otherwise in 
         * the peripheral role, this will send the corresponding L2CAP request and wait for the central to perform 
         * the procedure. In both cases, and regardless of success or failure, the application will be informed of 
         * the result with a \ref BLE_GAP_EVT_CONN_PARAM_UPDATE event.
         *
         * \param   highReactivity      The handle of the current connection (to close).
         * \param   invalidParameter    Flag to select the reason of the deconnection.
         ************************************************************************************************************/
        void handleConnParamUpdate(bool highReactiv);

        /********************************************************************************************************//**
         * \brief   To close the current connection volontary !!!
         *
         * This call initiates the disconnection procedure, and its completion will be communicated to the 
         * application with a \ref BLE_GAP_EVT_DISCONNECTED event.
         * 
         * Only two reasons are accepted for forcing the peripheral to close the current connection. Those reasons
         * have an HCI flag to be pass with the disconnect request: \ref BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION 
         * and \ref BLE_HCI_CONN_INTERVAL_UNACCEPTABLE). If the given `invalidParameter` is set to `true`, the flag
         * `BLE_HCI_CONN_INTERVAL_UNACCEPTABLE` will be used.
         *
         * \param   connHandle          The handle of the current connection (to close).
         * \param   invalidParameter    Flag to select the reason of the deconnection.
         ************************************************************************************************************/
        void forceToDisconnect(uint16_t connHandle, bool invalidParameter = false);

        /********************************************************************************************************//**
         * \name    BleObserverInterface
         * \brief   BleObserver interface implementation
         * \{
         ************************************************************************************************************/
        virtual void onBleEvent(ble_evt_t const* pBleEvent, void* pContext);
        virtual void onSystemEvent(uint32_t systemEvent);
        /** \} */

    #if (GAP_USE_XF_TIMER != 0)
    protected:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PUBLIC DECALRATION SECTION                                                                               */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        /********************************************************************************************************//**
         * \brief   XFReactive interface implementation
         ************************************************************************************************************/
        virtual EventStatus processEvent();
    #endif // #if (USE_CALENDAR == 0)

    private:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PRIVATE DECLARATION SECTION                                                                              */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        // Attributes
        PeripheralController*               _peripheralController;

        bool                                _isTimerInitialized;
        bool                                _connParamUpdateInProgress;         ///< \brief `true` if a negotiation to uodate the connection's param is allready ongoing...
        bool                                _updateForHigherReactivity;         ///< \brief To know how to change the connection's parameters
        bool                                _isTimeoutConnParamUpdateScheduled; ///< \brief To know if a timeout is scheduled for the connection's parameter Update
        bool                                _isGapReadyToUpdateConnParam;       ///< \brief GAP is ready or not to send a request to change the connection's parameter
        ble_gap_conn_params_t*              _lowEnergyConnParams;               ///< \brief Connection's parameter for Low Energy Mode
        ble_gap_conn_params_t*              _highReactConnParams;               ///< \brief Connection's parameter for HighReactivity Mode

        /************************************************************************************************************
         * \brief   Function for initialiazing the GAP profile
         ************************************************************************************************************/
        uint32_t                            _gapInit(ble_gap_conn_params_t*     gapConnParams, 
                                                     ble_gap_conn_sec_mode_t*   securityMode,
                                                     ble_gap_conn_params_t*     specificConnParams = NULL);

        /************************************************************************************************************
         * \brief   Send a request to the peer device to update the connection's parameters
         ************************************************************************************************************/
        void                                _sendConnParamUpdateRequest();

        /************************************************************************************************************
         * \brief   Send a request to the SoftDevice to change the connection's parameter according to settings,
         *           whichMUST already be set.
         *
         * \param[in]   highReactivity  If set to `TRUE`, the connection's parameters are set to let the application
         *                              getting more reactivity. If set to `FALSE` the application will be in current
         *                              consumption safe mode.
         *
         * \return  The error code due to the request for a change in parameter of connection. These values can be:
         *          - NRF_SUCCESS                   The Connection Update procedure has been started successfully.
         *          - NRF_ERROR_INVALID_ADDR        Invalid pointer supplied.
         *          - NRF_ERROR_INVALID_PARAM       Invalid parameter(s) supplied, check parameter limits and
         *                                          constraints.
         *          - NRF_ERROR_BUSY                Procedure already in progress or not allowed at this time,
         *                                          process pending events and retry.
         *          - BLE_ERROR_INVALID_CONN_HANDLE Invalid connection handle supplied.
         *          - NRF_ERROR_NO_MEM              Not enough memory to complete operation.
         *
         * \see MIN_CONN_INTERVAL, MAX_CONN_INTRERVAL, REACT_MIN_CONN_INTERVAL and REACT_MAX_CONN_INTERVAL
         ************************************************************************************************************/
        uint32_t                            _changeConnectionParamForReactivity(bool highReactivity = false);

        /************************************************************************************************************
         * \brief   Checks if the active connection's parameters are the desired ones.
         *
         * Checks if the active connection's parameters are the desired ones. The desired connection's
         * parameters are saved in the glabal variable `m_new_conn_param`.
         *
         * \return  An `uint32_t` value:
         *          - NRF_SUCCESS if the connection's parameters are set to the desired values
         *          - NRF_ERROR_INVALID_PARAM if the connection's parameters are not set correctly
         *          - and any other NRF error code due to the call of \ref `sd_ble_gap_ppcp_get` method.
         ************************************************************************************************************/
        uint32_t                            _checkConnectionParameters(bool highReactivity = false);

        /********************************************************************************************************//**
         * \brief   Set the TX power to the default value, specified with TX_POWER_LEVEL
         *
         * This is done when disconnecting and at initialization of the Gap object.
         ************************************************************************************************************/
        void                                _setTxPowerToDefaultValue();

        /************************************************************************************************************
         * \brief   All functions notifying any observer of the GAP's event
         ************************************************************************************************************/
        void                                _notifyConnParamUpdate(uint16_t connHandle);
        void                                _notifyGapConnected(uint16_t connHandle);
        void                                _notifyGapDisconnected(uint16_t connHandle);
        void                                _notifyGapPasskeyDisplay(uint16_t connHandle, char* passkey);
        void                                _notifyGapAuthenticationStatus(uint16_t connHandle, ble_gap_evt_auth_status_t authStatus);
        void                                _startGapBusyTimer();
        void                                _disableGapBusyTimer();
    };  // Class Gap

}  // namespace ble

#endif  // #if defined(SOFTDEVICE_PRESENT)

/** \} */   // Group: Peripheral Controller

/** \} */   // Group: BLE
