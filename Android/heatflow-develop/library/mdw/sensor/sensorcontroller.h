/****************************************************************************************************************//**
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 *
 * \file        sensorcontroller.h
 *
 * \addtogroup  Sensor
 * \{
 *
 * \class    SensorController
 * \brief    Controller able to handle the ADS1118 sensor.
 *
 * This class handles the heat flow measurement using temperatures measured by ADS1118 sensors. It 
 * will handle the activation of measurements amp. Finally, it can turn on or OFF the metering. 
 *
 * It's very important to respect defined timings between the power ON of the chip and the startup
 * of the sensors.
 *
 * \image html ./images/SensorControllerStateMachine.png "ADS Controller State Machine"
 *
 * The different states of the transition are the following:
 *   - <i>ST_Init:</i>
 *
 *     This is the first state where every state machine MUST pass through and where every object and
 *     elements are initialized.
 *
 *   - <i>ST_POWERED_OFF:</i>
 *
 *     The state where The sensor is turned off. After a `TimerEvent` the state machine goes to `ST_ON`
 *     state.
 *
 *   - <i>ST_POWERED_ON:</i>
 *
 *     The state where The LED is turned on. After a `TimerEvent` the state machine goes to `ST_OFF`
 *     state.
 *
 *   - <i>ST_SENSOR_ACTIVE:</i>
 *
 *     The state where The LED is turned on. After a `TimerEvent` the state machine goes to `ST_OFF`
 *     state.
 *
 *
 * \author    Patrice RUDAZ
 * \version    2.0
 * \date    February 2019
 ********************************************************************************************************************/
#pragma once

/* **************************************************************************************************************** */
/*                                                                                                                  */
/* Header files                                                                                                     */
/*                                                                                                                  */
/* **************************************************************************************************************** */
#include <boards.h>
#include <platform-config.h>
#include "core/xfreactive.h"

#if (USE_ADS111X != 0)
    #include "ads111X/ads1118.h"
    #include <ble-private-service-config.h>
    #include <sensor-config.h>
    
    #if (MHFS_SUPPORT != 0)
        #include "mhfs/mhfsservice.h"
        #include "mhfs/mhfsdefinition.h"
    #endif
#endif

#if (SENSOR_CONTROLLER_HANDLES_LED != 0) && (USE_LEDS != 0) && (LED_2 != 0)
    #include "gpio/ledcontroller.h"
#endif

#include "XF/sensorevent.h"

#include <nrfx_spi.h>
#include <nrfx_spim.h>

#if defined(__cplusplus)
extern "C" 
{
#endif
    namespace sensor 
    {
        /********************************************************************************************************//**
         * \brief   Event handler for the SPIM driver.
         *
         * This function will be called each time the SPIM driver throws an event.
         *
         * \param   pEvent      Pointer used for passing the SPIM event.
         * \param   pContext    Pointer used for passing some arbitrary information (context).
         ************************************************************************************************************/
        #if (USE_SPI != 0)
            void spiSensorEventHandler(nrfx_spi_evt_t const* pEvent, void* pContext);
        #else
            void spiEventHandler(nrfx_spim_evt_t const* pEvent, void* pContext);
        #endif
    }
#if defined(__cplusplus)
}
#endif



/* **************************************************************************************************************** */
/*                                                                                                                  */
/* NAMESPACE Declaration                                                                                            */
/*                                                                                                                  */
/* **************************************************************************************************************** */
namespace sensor 
{

    /* ************************************************************************************************************ */
    /*                                                                                                              */
    /* Class Declaration                                                                                            */
    /*                                                                                                              */
    /* ************************************************************************************************************ */
    class SensorController : public XFReactive
    {
        #if (USE_SPI != 0)
        friend void spiSensorEventHandler(nrfx_spi_evt_t const* pEvent, void* pContext);
        #else
        friend void spiEventHandler(nrfx_spim_evt_t const* pEvent, void* pContext);
        #endif
    public:
        /********************************************************************************************************//**
         * \brief   Enumeration used to have a unique identifier for every state in the state machine.
         ************************************************************************************************************/
        typedef enum
        {
            ST_UNKOWN        = 0,       ///< Unkown state
            ST_INIT,                    ///< Initial state
            ST_IDLE,                    ///< No measurement
            ST_DELAY,                   ///< Waiting some time
            ST_MEASUREMENT,             ///< The measurement is active
            ST_SHUTDOWN
        } eSensorControllerState;

        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* CONSTRUCTOR SECTION                                                                                      */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        SensorController();
        virtual ~SensorController();


        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PUBLIC DECLARATION SECTION                                                                               */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        /********************************************************************************************************//**
         * \brief    This method initialize the sensor's controller
         *
         * \warning        Must be called before any other method of this class
         * \note        Requires pre-existing instances of classes listed in parameter list
         ************************************************************************************************************/
        void initialize();
        
        /********************************************************************************************************//**
         * \brief   Binds the components needed by this class
         ************************************************************************************************************/
        void bind(
        #if (USE_ADS111X != 0)
                  ADS1118*              ads1118_a
            #if (USE_ADS111X > 1)
                , ADS1118*              ads1118_b
            #endif
        #endif // #if (USE_ADS111X != 0)
        #if (SENSOR_CONTROLLER_HANDLES_LED != 0) && (USE_LEDS != 0) && (LED_2 != 0)
            #if (USE_ADS111X != 0)
                ,
            #endif // (USE_ADS111X != 0)
                  gpio::LedController*  led
        #endif // #if (USE_LEDS != 0) && (LED_2 != 0)
        );
        
        /********************************************************************************************************//**
         * \brief    Set up the state to reach and start a timer for the state machine.
         *
         * \param   newState    The state that the sensor has to reach.
         ************************************************************************************************************/
        void setState(eSensorControllerState newState);
        
        /********************************************************************************************************//**
         * \brief    Get the state of the state machine.
         ************************************************************************************************************/
        inline eSensorControllerState getState() const      { return _currentState; }

        /********************************************************************************************************//**
         * \brief    Method handling the NRFX_SPI_EVENT_DONE or NRFX_SPIM_EVENT_DONE event...
         ************************************************************************************************************/
        void onSpiDoneEvent(void const* pEvent);

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
            TimeoutSensorId = 1,                                      ///< Timeout id for sensor to switch between POWERED_ON State and SENSOR's enabling
            TimeoutEndOfList
        } eTimeoutId;

        /********************************************************************************************************//**
         * \brief    Implements state machine behavior.
         ************************************************************************************************************/
        virtual EventStatus processEvent();
        
        eSensorControllerState              _currentState;                ///< Attribute indicating currently active state of the state machine
        eSensorControllerState              _stateToReach;                ///< Attribute indicating state of the state machine to reach

    private:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PRIVATE DECLARATION SECTION                                                                              */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        //ADC Driver
        #if (USE_SPI != 0)
        nrfx_spi_t                          _spi;
        nrfx_spi_config_t                   _spiConfig;
        nrfx_spi_xfer_desc_t                _spiXferDescriptor;
        #else
        nrfx_spim_t                         _spi;
        nrfx_spim_config_t                  _spiConfig;
        nrfx_spim_xfer_desc_t               _spiXferDescriptor;
        #endif
    
        #if (USE_ADS111X != 0)
        ADS1118*                            _ads1118_a;
            #if (USE_ADS111X > 1)
        ADS1118*                            _ads1118_b; 
            #endif
        #endif // #if (USE_ADS111X != 0)
        
        #if (SENSOR_CONTROLLER_HANDLES_LED != 0) && (USE_LEDS != 0) && (LED_2 != 0)
        gpio::LedController*                _led;
        #endif // #if (USE_LEDS != 0) && (LED_2 != 0)

        SensorEvent                         _xfSensorEvt;                   // Default event to update the State Machine

        // Local variables
        bool                                _isSensorTimeoutActive;        // to know if a action/event is on process
        bool                                _isMeasureActive;

        // local methods
        void                                _handleActivityTimeout( bool start, int delay );
        void                                _stopMeasurement();

        void _spiInit(nrfx_spi_evt_handler_t handler, void* context);
        bool _spiTransfer(uint8_t* tx, size_t sTx, uint8_t* rx, size_t sRx);
        void _spiUnit();
        void _setSpiFrequency(uint16_t frequency);
        
        uint16_t _adsTransfer(ADS1118* ads);
        uint16_t _readFromSensor(ADS1118* ads, bool configHasChanged);


        // state machine's Actions
        void _ST_IDLE_Action();             // Action to be done in ST_IDLE mode
        void _ST_DELAY_Action();            // Action to be done in ST_DELAY mode
        void _ST_MEASUREMENT_Action();      // Action to be done in ST_MEASUREMENT mode
        void _ST_SHUTDOWN_Action();

        #if (USE_ADS111X != 0)
        void _readTemperature(ADS1118* ads, const char* dbgMsg);
        void _readVolts(ADS1118* ads, uint8_t inputs, const char* dbgMsg);
        void _readAdcValue(ADS1118* ads, uint8_t inputs, const char* dbgMsg);
        #endif // #if (USE_ADS111X != 0)
        
        #if defined(SENSOR_VDD_ENABLE)
        void _handleVddSensorChain(bool enable);
        #endif  // #if defined(SENSOR_VDD_ENABLE)
        
        void debugPrintFloat(float value, const char* msg, const char* unit);
    };

}   // namespace sensor 

/** \} */   // Group: Sensor
