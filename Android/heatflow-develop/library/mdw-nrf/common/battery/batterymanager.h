/****************************************************************************************************************//**
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Cyril Praz, Patrice Rudaz
 * All rights reserved.
 *
 * \file    batterymanager.h
 *
 * \defgroup Battery
 * \{
 *
 * \class   BatteryManager
 * \brief   Class to manage the battery
 *
 * Here is the class to handle everything about the battery
 *
 * \author  Patrice Rudaz (patrice.rudaz@hevs.ch)
 * \version 2.0.0
 * \date    February 2018
 ********************************************************************************************************************/
#pragma once

/* **************************************************************************************************************** */
/*                                                                                                                  */
/* Header files                                                                                                     */
/*                                                                                                                  */
/* **************************************************************************************************************** */
#include <platform-config.h>


#if (USE_BATTERY_MANAGER != 0)
#include <battery-config.h>
#include <ble-services-config.h>

#include "isubject.hpp"

#include "interface/batteryobserver.h"
#include "batterymanagerpersistor.h"
#include "batterystate.h"

#if (USE_FLASH != 0)
    #include "flash/interface/persister.h"
#endif  // #if (USE_FLASH != 0)

#if (USE_SAADC != 0)
    #include "saadc/saadchal.h"
    #include "saadc/interface/saadcobserver.h"
#endif  // #if (USE_SAADC != 0)

#if !defined(USE_CHARGER)
    #error "USE_CHARGER not defined !"
#elif (USE_CHARGER != 0)
    #include "charger/chargerstate.h"
    #include "charger/interface/chargerobserver.h"
    #if (USE_STBC02 != 0)
        #include "stbc02/charger.h"
    #else
        #include "bq51003bq25101/charger.h"
    #endif
    
    #if (USE_CALENDAR != 0)
    #include "calendar/calendar.h"
    #include "calendar/interface/calendarobserver.h"
    #endif  // #if (USE_CALENDAR != 0)
#endif  // #if (USE_CHARGER != 0)

#if !defined(APP_BATT_USE_XF_TIMER)
    #error "APP_BATT_USE_XF_TIMER not defined !"
#elif (APP_BATT_USE_XF_TIMER != 0)
    #include <core/xfreactive.h>
#else
    #if defined(__cplusplus)
    extern "C" 
    {
    #endif
        namespace battery 
        {
            /****************************************************************************************************//**
             * \brief   Function for handling the battery measurement delay
             *
             * \details This function will be called each time the battery measurement delay timer expires
             *
             * \param   pContext    Pointer used for passing some arbitrary information (context) from the
             *                      `app_start_timer()` call to the timeout handler.
             ********************************************************************************************************/
            void battManagerTimeoutHandler(void* pContext);
        } // namespace battery
    #if defined(__cplusplus)
    }
    #endif
#endif  // #if (APP_BATT_USE_XF_TIMER != 0)

/* **************************************************************************************************************** */
/*                                                                                                                  */
/* APPLICATION definition                                                                                           */
/*                                                                                                                  */
/* **************************************************************************************************************** */

/* **************************************************************************************************************** */
/*                                                                                                                  */
/* NAMESPACE Declaration                                                                                            */
/*                                                                                                                  */
/* **************************************************************************************************************** */
namespace battery 
{
    /* ************************************************************************************************************ */
    /*                                                                                                              */
    /* Class Declaration                                                                                            */
    /*                                                                                                              */
    /* ************************************************************************************************************ */
    class BatteryManager : public ISubject<BatteryObserver>
    #if (APP_BATT_USE_XF_TIMER != 0)
        , public XFReactive
    #endif  // #if (APP_BATT_USE_XF_TIMER != 0)
    #if (USE_FLASH != 0)
        , public flash::Persister
    #endif  // #if (USE_FLASH != 0)
    #if (USE_SAADC != 0)
        , public hal::SaadcObserver
    #endif  // USE_SAADC
    #if (USE_CHARGER != 0)
        , public ChargerObserver
        #if (USE_CALENDAR != 0)
        , public cal::CalendarObserver
        #endif  // #if (USE_CALENDAR != 0)
    #endif  // #if (USE_CHARGER != 0)
    {
        #if (APP_BATT_USE_XF_TIMER == 0)
        friend void battManagerTimeoutHandler(void* pContext);
        #endif  // #if (APP_BATT_USE_XF_TIMER == 0)

    public:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* CONSTRUCTOR SECTION                                                                                      */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        BatteryManager();
        virtual ~BatteryManager();

        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PUBLIC DECLARATION SECTION                                                                               */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        /********************************************************************************************************//**
         * \brief   Initializes the component
         *
         * \warning Must be called first !
         ************************************************************************************************************/
        void initialize();

        /********************************************************************************************************//**
         * \brief   Binds the components needed by this class
         ************************************************************************************************************/
        void bind(
        #if (USE_SAADC != 0)
            hal::SaadcHal* saadcHal
        #endif
        #if (USE_CHARGER != 0) 
          , Charger* charger
            #if (USE_CALENDAR != 0)
          , cal::Calendar* calendar
            #endif
        #endif
        );

        /********************************************************************************************************//**
         * \brief   Disable the battery manager and start monitoring the battery state
         ************************************************************************************************************/
        void disable();

        /********************************************************************************************************//**
         * \brief   Enable the battery manager and start monitoring the battery state
         ************************************************************************************************************/
        void enable();

        /********************************************************************************************************//**
         * \brief   Get the Battery Level (in %)
         ************************************************************************************************************/
        inline uint8_t getBatteryLevel() const              { return _batteryLevel; }

        /********************************************************************************************************//**
         * \brief   Getter to retrieve the current battery state
         ************************************************************************************************************/
        BatteryState::eBatteryState getBatteryState() const { return _batteryState; }

        #if (USE_CHARGER != 0)
        /********************************************************************************************************//**
         * \brief   Get the number of charging cycles
         ************************************************************************************************************/
        inline uint32_t getChargeCounter() const        { return _persistentParams.chargingCyclesCounter(); }
        #endif  // #if (USE_CHARGER != 0)

        /************************************************************************************************************
         * \brief   Enter or leave the shipping mode to reduce the power consumption to a strictly minimum
         *
         * \param   enter   Is set to `true`, the BatteryManager will get in shipping mode and it will leave this
         *                  mode if this parameter is set to `false`.
         ************************************************************************************************************/
        void performShippingMode(bool enter);

        #if (USE_FLASH != 0)
        /********************************************************************************************************//**
         * \name   FlashPersiterInheritance
         * \brief  Inherit from FlashPersiter.
         * \{
         ************************************************************************************************************/
        virtual uint32_t* save(uint16_t* size);
        virtual void restore(uint32_t* data);
        /** \} */
        #endif  // #if (USE_FLASH != 0)


    protected:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PROTECTED DECLARATION SECTION                                                                            */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        /********************************************************************************************************//**
         * \brief   Enumeration used to have a unique identifier for every state in the state machine.
         ************************************************************************************************************/
        typedef enum
        {
            ST_UNKNOWN,
            ST_INIT,
            ST_IDLE,
            ST_PREPARE_GPIO,
            ST_WAITING,
            ST_MEASURING,
            ST_MEASURE_DONE
        } eMainState;

        #if (APP_BATT_USE_XF_TIMER != 0)
        /********************************************************************************************************//**
         * \brief   Timeout identifier(s) for this state machine
         ************************************************************************************************************/
        typedef enum
        {
            TimeoutBatteryId  = 1,
            TimeoutChargerId,
            TimeoutTopEnumId
        } eTimeoutId;

        /********************************************************************************************************//**
         * \brief   Implements state machine behavior.
         ************************************************************************************************************/
        virtual EventStatus processEvent();
        #endif // #if (APP_BATT_USE_XF_TIMER != 0)

        /********************************************************************************************************//**
         * \brief   Called when the `app_timer` of the Battery Manager notify a timeout
         ************************************************************************************************************/
        void onBatteryManagerTimeout();

        #if (USE_SAADC != 0)
        /********************************************************************************************************//**
         * \brief   Called when a battery measure is received
         ************************************************************************************************************/
        virtual void onMeasureDone(hal::SaadcHal* saadcHal, nrf_saadc_value_t* buff, uint16_t buffSize);
        #endif  // #if (USE_SAADC != 0)

        #if (USE_CHARGER != 0)
        /********************************************************************************************************//**
         * \name    ChargerObserverImplementation Implementation of the Charger's observer callback methods
         * \brief   Callback methods for the Charger
         * \{
         *
         * \brief   Called when a battery start charging
         ************************************************************************************************************/
        virtual void onChargerStateChanged(Charger* charger, ChargerState::eState state);
        
        /********************************************************************************************************//**
         * \brief   Called when the device is put on its chargin's base
         ************************************************************************************************************/
        virtual void onChargerAvailable(Charger* charger, bool available);
        /** \} */

        #if (USE_CALENDAR != 0)
        /********************************************************************************************************//**
         * \brief   Called on each second that has passed !!!
         ************************************************************************************************************/
        virtual void onSecond(cal::Calendar* calendar);
        #endif  // #if (USE_CALENDAR != 0)
        #endif  // #if (USE_CHARGER != 0)

    private:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PRIVATE DECLARATION SECTION                                                                              */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        // Attributes
        eMainState                  _currentState;
        BatteryState::eBatteryState _batteryState;
        uint8_t                     _batteryLevel;
        int16_t                     _batteryLevelBuffer[APP_BATT_SAMPLES];
        bool                        _isTimerInitialized;
        bool                        _isTimerEnabled;
        uint8_t                     _measurementCounter;

        // Persistent parameters
        BatteryManagerPersistor     _persistentParams;

        // Delays
        uint32_t                    _measuringWaitingDelay;
        uint32_t                    _measuringShippingDelay;
        uint32_t                    _measuringDelayAfterCharging;
        uint32_t                    _inBetweenSamplesDelay;
        uint32_t                    _prepareGPIODelay;
        uint32_t                    _standardDelay;

        #if (USE_SAADC != 0)
            hal::SaadcHal*          _saadcHal;
            #if (APP_BATT_USE_BATT_CARACTERISATION == 0)
            float                   _a;
            uint64_t                _b;
            #endif
        #endif
        bool                        _isSaadcInitialized;

        #if (USE_CHARGER != 0)
            uint16_t                _chargeTime;
            bool                    _charging;
            Charger*                _charger;
            #if (APP_BATT_USE_XF_TIMER != 0)
                bool                _isChargerTimerEnabled;
            #elif (USE_CALENDAR != 0)
                cal::Calendar*      _calendar;
            #endif  // #if (USE_CALENDAR != 0)
        #endif  // #if (USE_CHARGER != 0)

        // Methods
        bool _initMeasurer();
        void _getBattLevelInPercent(int16_t battLevel);
        void _startBatteryMeasurement();
        bool _measure();
        bool _startTimer(uint32_t delay);
        bool _stopTimer();
        void _updateBatteryState();
        void _updateBatteryState(bool isInShippingMode);

        #if (USE_CHARGER != 0)
            /****************************************************************************************************//**
             * \brief Function to convert a battery percentage into a charge time
             ********************************************************************************************************/
            uint16_t _percentToChargeTime();

            /****************************************************************************************************//**
             * \brief Function to convert a charge time into a battery percentage
             ********************************************************************************************************/
            uint8_t _chargeTimeToPercent();
            
            /****************************************************************************************************//**
             * \brief Function to do what is needed when a charger's timeout is coming...
             ********************************************************************************************************/
            void _chargerTimerHandler();
        #endif  // #if (USE_CHARGER != 0)

        /********************************************************************************************************//**
         * \brief Notify any observer if the battery's level changed
         ************************************************************************************************************/
        void _notifyBattLevelChanged();

        /********************************************************************************************************//**
         * \brief Notify any observer if the battery's state changed
         ************************************************************************************************************/
        void _notifyBattStateChanged();
    };

} // namespace battery

#endif // #if (USE_BATTERY_MANAGER != 0)

/** \} */   // Group: Battery
