/****************************************************************************************************************//**
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 *
 * \file    ledcontroller.h
 *
 * \addtogroup GPIO
 * \{
 *
 * \defgroup LED
 * \{
 *
 * \class   LedController
 * \brief   Controller able to switch ON or OFF a specific LED
 *
 * # LED Controller
 *
 * This class handles LED by switching it ON or OFF. The selection of the LED to work with
 * depends of the hardware and it's set through the initialize() method which maps the MSP430
 * port.
 *
 * \image html ./images/LedControllerStateMachine.png "LED Controller State Machine"
 *
 * The switching between On and OFF is based on a very simple state machine.  The different states
 * of the transition are the following:
 *   - <i>ST_Init:</i>
 *
 *     This is the first state where every state machine MUST pass through and where every object and
 *     elements are initialized.
 *
 *   - <i>ST_OFF:</i>
 *
 *     The state where The LED is turned off. After a `TimerEvent` the state machine goes to `ST_ON`
 *     state.
 *
 *   - <i>ST_ON:</i>
 *
 *     The state where The LED is turned on. After a `TimerEvent` the state machine goes to `ST_OFF`
 *     state.
 *
 * Initial author: Patrice Rudaz
 * Creation date: 2016-01-09
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
#include "core/xfreactive.h"

#include <nrf.h>


/* **************************************************************************************************************** */
/*                                                                                                                  */
/* NAMESPACE Declaration                                                                                            */
/*                                                                                                                  */
/* **************************************************************************************************************** */
namespace gpio 
{

    /* ************************************************************************************************************ */
    /*                                                                                                              */
    /* Class Declaration                                                                                            */
    /*                                                                                                              */
    /* ************************************************************************************************************ */
    class LedController : public XFReactive
    {
    public:
        /********************************************************************************************************//**
         * \brief   Enumeration used to have a unique identifier for every state in the state machine.
         ************************************************************************************************************/
        typedef enum
        {
            ST_UNKNOWN = 0,
            ST_INIT,
            ST_LED_OFF,
            ST_LED_ON
        } eLedState;

        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* CONSTRUCTOR/DESTRUCTOR DECLARATION SECTION                                                               */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        LedController();
        virtual ~LedController();

        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PUBLIC DECLARATION SECTION                                                                               */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        /********************************************************************************************************//**
         * \brief   Initializes the components needed by this class
         *
         * This initialization method needs at least one parameter, which is the identifier of the LED to be
         * controlled. The different timing ON and OFF time can also be given here. If they are not defined, the 
         * default value of 1 second is applied for both of them.
         *
         * \param   index       The identifier of the LED to control
         * \param   timeOn      The delay the led should be ON, default value is 1000 [ms]
         * \param   timeOff     The time the led should be OFF, default value is 1000 [ms]
         *
         * \warning Must be called before any other method of this class
         * \note    Requires pre-existing instances of classes listed in parameter list
         ************************************************************************************************************/
        void initialize(uint8_t index, uint16_t timeOn = 1000, uint16_t timeOff = 1000);

        /********************************************************************************************************//**
         * \brief   Switch ON the LED.
         ************************************************************************************************************/
        void ledOn();

        /********************************************************************************************************//**
         * \brief   Turn OFF the LED.
         ************************************************************************************************************/
        void ledOff();

        /********************************************************************************************************//**
         * \brief   Replace the value of the length of time the LED will be turned ON.
         *
         * \param   timeOn          The new delay in millisecond
         ************************************************************************************************************/
        inline void setTimeOn(uint16_t timeOn)          { _timeOn  = timeOn; }

        /********************************************************************************************************//**
         * \brief   Returns the current delay set as time that the LED should be ON.
         ************************************************************************************************************/
        inline uint16_t timeOn() const                  { return _timeOn; }

        /********************************************************************************************************//**
         * \brief   Replace the value of the length of time the LED will be turned OFF.
         *
         * \param   timeOff         The new delay in millisecond
         ************************************************************************************************************/
        inline void setTimeOff(uint16_t timeOff)        { _timeOff  = timeOff; }

        /********************************************************************************************************//**
         * \brief   Returns the current delay set as time that the LED should be ON.
         ************************************************************************************************************/
        inline uint16_t timeOff() const                 { return _timeOff; }

        /********************************************************************************************************//**
         * \brief   Replace both values of the length of time the LED will be turned ON and OFF.
         *
         * \param   timeOn          The new delay in millisecond for the ON state
         * \param   timeOff         The new delay in millisecond for the OFF state
         ************************************************************************************************************/
        inline void setTimeOnOff(uint16_t timeOn, uint16_t timeOff) {
            _timeOn   = timeOn;
            _timeOff  = timeOff;
        }
        
        /********************************************************************************************************//**
         * \brief   Replace both values of the length of time the LED will be turned ON and OFF.
         *
         * \param   timeOnQuick     The new delay in millisecond for the ON state for quick blinking
         * \param   timeOffQuick    The new delay in millisecond for the OFF state for quick blinking
         * \param   timeOnStd       The new delay in millisecond for the ON state for standard blinking
         * \param   timeOffStd      The new delay in millisecond for the OFF state for standard blinking
         * \param   timeOnSlow      The new delay in millisecond for the ON state for slow blinking
         * \param   timeOffSlow     The new delay in millisecond for the OFF state for slow blinking
         ************************************************************************************************************/
        void setTimeforQuickStandardAndSlowBlinkingState(uint16_t timeOnQuick, uint16_t timeOffQuick, 
                                                         uint16_t timeOnStd,   uint16_t timeOffStd, 
                                                         uint16_t timeOnSlow,  uint16_t timeOffSlow);


        /********************************************************************************************************//**
         * \brief   Returns `true` or `false` according to the blinking state of the LED.
         ************************************************************************************************************/
        inline bool isBlinking() const      { return _blink; }

        /********************************************************************************************************//**
         * \brief   Start to blink the LED.
         *
         * This method will set ON the led and start the behaviour of the state machine by starting a timer with the 
         * `timeOn` value. This value should be set before.
         *
         * \param force If set, it forces the LED to start blinking again by restarting the timer...
         ************************************************************************************************************/
        void startBlinking(bool force = false);

        /********************************************************************************************************//**
         * \brief   Start to blink the LED quickly.
         *
         * This method will set ON the led and start the behaviour of the state machine by starting a timer with the 
         * `timeOn` value. This value is a default value for blinking quickly.
         ************************************************************************************************************/
        void startQuickBlinking();
        
        /********************************************************************************************************//**
         * \brief   Start to blink the LED.
         *
         * This method will set ON the led and start the behaviour of the state machine by starting a timer with the
         * `timeOn` value. This value is a default value for blinking in a standard way.
         ************************************************************************************************************/
        void startStandardBlinking();
        
        /********************************************************************************************************//**
         * \brief   Start to blink the LED.
         *
         * This method will set ON the led and start the behaviour of the state machine by starting a timer with the 
         * `timeOn` value. This value is a default value for blinking slowly.
         ************************************************************************************************************/
        void startSlowBlinking();
        

    protected:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PROTECTED DECLARATION SECTION                                                                            */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        /********************************************************************************************************//**
         * \brief   Timeout identifier(s) for this state machine
         ************************************************************************************************************/
        typedef enum
        {
            Timeout_LED_id  = 1,
            Timeout_end_of_list
        } eTimeoutId;

        /********************************************************************************************************//**
         * \brief   Implements state machine behavior.
         ************************************************************************************************************/
        virtual EventStatus processEvent();
        
        eLedState   _currentState;

        uint8_t     _index;
        uint16_t    _timeOn;
        uint16_t    _timeOff;
        
    private:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PRIVATE DECLARATION SECTION                                                                              */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        // Constants
        static const uint8_t DEFAULT_TIME_ON = 100;
    
        // Attributes
        bool        _blink;
        bool        _isTimerActive;

        uint16_t    _quickOnDelay;
        uint16_t    _quickOffDelay;

        uint16_t    _stdOnDelay;
        uint16_t    _stdOffDelay;

        uint16_t    _slowOnDelay;
        uint16_t    _slowOffDelay;

        // Methods
        void        _stopBlinking();
        void        _ST_LED_OFF_Action();
        void        _ST_LED_ON_Action();
    }; // class LedController

}   // namespace mdw

/** \} */   // Group: LED

/** \} */   // Group: GPIO
