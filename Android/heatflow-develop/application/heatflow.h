/***************************************************************************************************************//**
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 *
 * \file    heatflow.h
 * \class   HeatFlow
 * \brief   Main class of HeatFLow project...
 *
 * \mainpage HeatFlow
 *
 * # SDKUpdater
 *
 * This class has been developed for the Specific project called HeatFlow. It uses the eXecution Framework designed 
 * and developed by the HEI Sion, member of HES-SO Valais.
 *
 * The organisation of the software is shown in the following pictures:
 * \image html generalSchema.png
 * \htmlonly <style>div.image img[src="modules.png"]{width:800px;}</style> \endhtmlonly
 * \image html modules.png
 *
 * \author  Patrice Rudaz
 * \date    June 2019
 ********************************************************************************************************************/
#pragma once

/* **************************************************************************************************************** */
/*                                                                                                                  */
/* Header files                                                                                                     */
/*                                                                                                                  */
/* **************************************************************************************************************** */
#include <core/xfreactive.h>
#include <xf-config.h>
#include "boards.h"
#include <platform-config.h>

#if (USE_GPIOS != 0)
    #include "gpio/gpio.h"
    #include "gpio/interface/gpioobserver.h"
#endif

#if (USE_BUTTONS != 0)
    #include "button/button.h"
    #include "button/interface/buttonobserver.h"
#endif

#if defined(SOFTDEVICE_PRESENT)
    #include "application/config/ble-advertiser-config.h"
    #include "ble/interface/peripheralobserver.h"
    #include "ble/interface/bleservicesobserver.h"
    #include <ble-services-config.h>
    #include <ble-private-service-config.h>
    #include "ble/peripheralcontroller.h"
    
    #if (TBS_SUPPORT != 0)
        #include "tbs/tbsservice.h"
        #include "tbs/tbsobserver.h"
    #endif  // #if (TBS_SUPPORT != 0)
    
    #if (MHFS_SUPPORT != 0)
        #include "mhfs/mhfsdefinition.h"
        #include "mhfs/mhfsservice.h"
        #include "mhfs/mhfsobserver.h"
    #endif  // #if (MHFS_SUPPORT != 0)
#endif

#if (USE_FLASH != 0)
    #include "flash/flashcontroller.h"
    #include "flash/interface/flashObserver.h"
    #include "flash/interface/persister.h"
#endif

#if (USE_BATTERY_MANAGER != 0)
    #include "battery/interface/batteryobserver.h"
    #include "battery/batterymanager.h"
#endif

#if (USE_DFU != 0)
    #include "application/dfu.h"
#endif

#if (USE_LEDS != 0) && defined(LED_1) && defined(LED_2) && defined(LED_3)
    #include "gpio/ledcontroller.h"
#endif

#if (USE_UPTIME != 0)
    #include "uptime.h"
#endif // UPTIME_ACTIVE

#if (USE_PWM != 0) && (USE_TONE_MANAGER == 0) || (USE_MELODY_MANAGER == 0)
    #include "pwm/pwmhal.h"
#endif

#if (USE_DFU != 0)
    #include "application/dfu.h"
#endif

#if (USE_ADS111X != 0)
    #include "sensor/sensorcontroller.h"
#endif // #if (USE_ADS111X != 0)


/* **************************************************************************************************************** */
/*                                                                                                                  */
/* APPLICATION definition                                                                                           */
/*                                                                                                                  */
/* **************************************************************************************************************** */
#define NO_ACTIVITY_DELAY                 300   ///< Define the time to wait before going in system off [s]
#define NO_ACTIVITY_TIMEMOUT_STEP       30000   ///< Number of [ms] before a timer_tick
#define NO_ACTIVITY_DECREMENT              30   ///< Value to decrement the counter

/* **************************************************************************************************************** */
/*                                                                                                                  */
/* NAMESPACE Declaration                                                                                            */
/*                                                                                                                  */
/* **************************************************************************************************************** */
namespace application 
{
    /* ************************************************************************************************************ */
    /*                                                                                                              */
    /* Class Declaration                                                                                            */
    /*                                                                                                              */
    /* ************************************************************************************************************ */
    class HeatFlow
        : public XFReactive
    #if (USE_GPIOS != 0)
        , public gpio::GpioObserver
    #endif
    #if (USE_BUTTONS != 0)
        , public gpio::ButtonObserver
    #endif
    #if defined(SOFTDEVICE_PRESENT)
        , public ble::PeripheralObserver

        #if (MHFS_SUPPORT != 0)
        , public ble::MhfsObserver
        #endif  // #if (MHFS_SUPPORT != 0)

    #endif
    #if (USE_BATTERY_MANAGER != 0)
        , public battery::BatteryObserver
    #endif
    #if (USE_FLASH != 0) && (FLASH_CONTROLLER_TEST_MODE != 0)
        , public flash::FlashObserver
    #endif  // #if (USE_FLASH != 0) && (FLASH_CONTROLLER_TEST_MODE != 0)
    {
    public:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* CONSTRUCTOR SECTION                                                                                      */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        HeatFlow();
        virtual ~HeatFlow() {}

        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PUBLIC DECLARATION SECTION                                                                               */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        /********************************************************************************************************//**
         * \brief   Enumeration used to have a unique identifier for every button action.
         ************************************************************************************************************/
        enum
        {
            SWITCH_TO_FUNCTION_1 = 1,
            SWITCH_TO_FUNCTION_2 = 2,
            SWITCH_TO_FUNCTION_3 = 3,
            SWITCH_TO_MOTOR_INIT = 4
        } buttonActions;

        /********************************************************************************************************//**
         * \brief   Initializes the components needed by this class
         *
         * \warning Must be called before any other method of this class
         ************************************************************************************************************/
        void initialize();
        void bind(
        #if defined(SOFTDEVICE_PRESENT)
                      ble::PeripheralController* peripheralController
        #endif  // #if defined(SOFTDEVICE_PRESENT)
        #if (USE_FLASH != 0)
            #if defined(SOFTDEVICE_PRESENT)
                    ,
            #endif
                      flash::FlashController* flashController
        #endif  // #if (USE_FLASH != 0)
        #if (USE_BATTERY_MANAGER != 0)
            #if defined(SOFTDEVICE_PRESENT) || (USE_FLASH != 0)
                    ,
            #endif
                      battery::BatteryManager* batteryManager
        #endif  // #if (USE_BATTERY_MANAGER != 0)
        #if (USE_GPIOS != 0)
            #if defined(SOFTDEVICE_PRESENT) || (USE_FLASH != 0) || (USE_BATTERY_MANAGER != 0)
                    ,
            #endif
                      gpio::Gpio* gpioA, gpio::Gpio* gpioB
        #endif  // #if (USE_GPIOS != 0)
        #if (USE_LEDS != 0) && defined(LED_1) && defined(LED_2) && defined(LED_3)
            #if defined(SOFTDEVICE_PRESENT) || (USE_FLASH != 0) || (USE_BATTERY_MANAGER != 0) || (USE_GPIOS != 0)
                    ,
            #endif
                      gpio::LedController* ledA, gpio::LedController* ledB, gpio::LedController* ledC
            #if defined(BOARD_PCA10040) || defined(BOARD_PCA10056)
                    , gpio::LedController* ledD
            #endif  // #if defined(BOARD_PCA10040) || defined(BOARD_PCA10056)
        #endif  // #if (USE_LEDS != 0) && defined(LED_1)
        #if (USE_BUTTONS != 0)
            #if defined(SOFTDEVICE_PRESENT) || (USE_FLASH != 0) || (USE_BATTERY_MANAGER != 0) || (USE_GPIOS != 0) || ((USE_LEDS != 0) && defined(LED_1) && defined(LED_2) && defined(LED_3))
                    ,
            #endif
                      gpio::Button* buttonA, gpio::Button* buttonB, gpio::Button* buttonC
            #if defined(BOARD_PCA10040) || defined(BOARD_PCA10056)
                    , gpio::Button* buttonD
            #endif // #if defined(BOARD_PCA10040) || defined(BOARD_PCA10056)
        #endif  // #if (USE_BUTTONS != 0)
        #if (USE_PWM != 0)
            #if defined(SOFTDEVICE_PRESENT) || (USE_FLASH != 0) || (USE_BATTERY_MANAGER != 0) || (USE_GPIOS != 0) || ((USE_LEDS != 0) && defined(LED_1) && defined(LED_2) && defined(LED_3)) || (USE_BUTTONS != 0)
                    ,
            #endif
                      hal::PwmHal* pwmHAL
        #endif  // #if (USE_PWM != 0)
        #if (USE_DFU != 0)
            #if defined(SOFTDEVICE_PRESENT) || (USE_FLASH != 0) || (USE_BATTERY_MANAGER != 0) || (USE_GPIOS != 0) || ((USE_LEDS != 0) && defined(LED_1) && defined(LED_2) && defined(LED_3)) || (USE_BUTTONS != 0) || (USE_PWM != 0)
                    ,
            #endif
                      dfu::Dfu* dfu
        #endif  // #if (USE_DFU != 0)
        #if (USE_ADS111X != 0)
            #if defined(SOFTDEVICE_PRESENT) || (USE_FLASH != 0) || (USE_BATTERY_MANAGER != 0) || (USE_GPIOS != 0) || ((USE_LEDS != 0) && defined(LED_1) && defined(LED_2) && defined(LED_3)) || (USE_BUTTONS != 0) || (USE_PWM != 0) || (USE_DFU != 0)
                    ,
            #endif
                      sensor::SensorController* sensorCtrl
        #endif
        );

    protected:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PROTECTED DECLARATION SECTION                                                                            */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        /********************************************************************************************************//**
         * \brief   Timeout identifier(s) for this state machine
         *
         * This enumeration lists all identifier used with timeouts of the XF. And we have:
         *   - Timeout_NoActivity_id:
         *   Used to go back in Low Power mode... relatoed to connection's parameters.
         *
         *   - Timeout_ConnChangeParam_id
         *   Used when trying to update the connection's parameters !
         ************************************************************************************************************/
        typedef enum
        {
            TimeoutGenericId = 0,
            #if defined(SOFTDEVICE_PRESENT)
            TimeoutNoActivityId = 1,
            #endif  // #if defined(SOFTDEVICE_PRESENT)
            #if (USE_ADS111X)
            TimeoutAdsId,
            #endif  // #if (USE_ADS111X)
            TimeoutTopEnumId
        } eTimeoutId;

        /********************************************************************************************************//**
         * \brief   Enumeration used to have a unique identifier for every state in the state machine.
         ************************************************************************************************************/
        typedef enum
        {
            ST_UNKOWN = 0,
            ST_INIT,
            ST_IDLE,
            ST_BLE_ADVERTISING,
            ST_BLE_CONNECTED,
            ST_SYSTEM_OFF,
            ST_TOP_ENUM
        } eState;

        /********************************************************************************************************//**
         * \brief   Implements state machine behavior.
         *
         * Inherit from XFReactive.
         *
         * Executes the current event in its implemented behavior.
         ************************************************************************************************************/
        virtual EventStatus processEvent();

        eState _currentState;

        #if (USE_GPIOS != 0)
        virtual void onFallingEdge(gpio::Gpio* gpio);
        virtual void onRisingEdge(gpio::Gpio* gpio);
        #endif  // #if (USE_GPIOS != 0)

        #if (USE_BUTTONS != 0)
        virtual void onButtonSingleClick(gpio::Button* button);
        virtual void onButtonDoubleClick(gpio::Button* button);
        virtual void onButtonTripleClick(gpio::Button* button);
        virtual void onButtonLongPress(gpio::Button* button);
        virtual void onButtonVeryLongPress(gpio::Button* button);
        virtual void onButtonReleased(gpio::Button* button);

        virtual const gpio::Button* button(int index) const;
        gpio::Button* button(int index);
        #endif  // #if (USE_BUTTONS != 0)

        #if defined(SOFTDEVICE_PRESENT)
        virtual void onAdvertiseStopped(ble::PeripheralController* peripheral);
        virtual void onAdvertiseStateChanged(ble::PeripheralController* peripheral, ble_adv_evt_t bleAdvEvent);
        virtual void onAdvertiseError(ble::PeripheralController* peripheral, uint32_t nrfError);

        virtual void onPeripheralConnected(ble::PeripheralController* peripheral);
        virtual void onPeripheralDisconnected(ble::PeripheralController* peripheral);
        virtual void onPeripheralStateChanged(ble::PeripheralController* peripheral, uint8_t state);
        virtual void onPasskeyDisplay(ble::PeripheralController* peripheral, char* passkey);
        virtual void onAuthStatusUpdated(ble::PeripheralController* peripheral, ble_gap_evt_auth_status_t status);

        #if (MHFS_SUPPORT != 0)
        virtual void onConfigurationChange(ble::MhfsService* mhfsSrv, uint8_t adsId, uint16_t config);
        virtual void onCommandChange(ble::MhfsService* mhfsSrv, eMhfsCommandId commandId);
        #endif  // #if (MHFSS_SUPPORT != 0)
        #endif  // #if defined(SOFTDEVICE_PRESENT)

        #if (USE_BATTERY_MANAGER != 0)
        virtual void onBatteryStateChanged(battery::BatteryManager* batteryManager, 
                                           battery::BatteryState::eBatteryState state);
        virtual void onBatteryLevelChanged(battery::BatteryManager* batteryManager, uint8_t batteryLevel);
        #endif
        

    private:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PRIVATE DECLARATION SECTION                                                                              */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        // Attributes
        #if (USE_FLASH != 0)
        flash::FlashController*                     _flashController;
        #endif  // #if (USE_FLASH != 0)

        #if (USE_BATTERY_MANAGER != 0)
        battery::BatteryManager*                    _batteryManager; 
        #endif

        #if (USE_UPTIME != 0)
        UpTime                                      _upTime;
        #endif // UPTIME_ACTIVE

        // private attributes
        #if defined(SOFTDEVICE_PRESENT)
        ble::PeripheralController*                  _peripheralController;
        #endif  // #if defined(SOFTDEVICE_PRESENT)

        #if (USE_GPIOS != 0)
        gpio::Gpio*                                 _gpioA;
        gpio::Gpio*                                 _gpioB;
        #endif  //     #if (USE_GPIOS != 0)

        #if (USE_LEDS != 0) && defined(LED_1) && defined(LED_2) && defined(LED_3)
        gpio::LedController*                        _greenLed;
        gpio::LedController*                        _blueLed;
        gpio::LedController*                        _redLed;
        #if defined(BOARD_PCA10040) || defined(BOARD_PCA10056)
        gpio::LedController*                        _ledD;
        #endif  // #if defined(BOARD_PCA10040) || defined(BOARD_PCA10056)
        #endif  // #if (USE_LEDS != 0) && defined(LED_1)

        #if (USE_PWM != 0)
        hal::PwmHal*                                _pwmHAL;
        #endif  // #if (USE_PWM != 0)

        #if (USE_DFU != 0)
        dfu::Dfu*                                   _dfu;
        #endif  // #if (USE_DFU != 0)

        #if (USE_BUTTONS != 0) && (BUTTONS_NUMBER != 0)
        static const uint8_t                        _nbrButtons = BUTTONS_NUMBER;
        gpio::Button*                               _buttons[_nbrButtons];
        #endif


        #if (USE_ADS111X != 0)
        sensor::SensorController*                   _sensorCtrl;
        bool		                                _isTimeoutAdsScheduled;
        bool                                        _isMeasuring;
        #endif

        bool		                                _isTimeoutGenericScheduled;
        bool		                                _isTimeoutNoActivityScheduled;
        uint32_t                                    _noActivityCounter;
        bool                                        _isCharging;

        #if defined(SOFTDEVICE_PRESENT)
        bool                                        _isBleConnected;
        bool                                        _isBleAdvertising;
        bool                                        _dfuInProgress;
        bool                                        _updateForHigherReactivity;

        void 		                                _handleAdsTimeout(bool start);
        void 		                                _handleNoActivityTimeout(bool start);
        void                                        _startAdvertise(bool withoutWhitelist);
        void                                        _stopAdvertise();
        void                                        _onBleConnected();
        void                                        _onBleDisconnected();
        void                                        _updateSecurityLevelForServices(bool justWorks, bool mitm, bool whiteList);
        #endif  // #if defined(SOFTDEVICE_PRESENT)
	
        // state machine's Actions
        void		                                _ST_IDLE_Action();
        void		                                _ST_BLE_CONNECTED_Action();
        void 		                                _ST_SYSTEM_OFF_Action();
        
        void                                        _handleMeasurement(bool enable);
    };
}   // namespace application
