/************************************************************************************************************
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 ************************************************************************************************************/

/************************************************************************************************************/
/* Header files                                                                                             */
/************************************************************************************************************/
#include "ledcontroller.h"
#include "gpio/ledhal.h"

#include <factory.h>
#include "critical/critical.h"


/************************************************************************************************************/
/* DEBUG COMPILATION OPTION & MACRO                                                                         */
/************************************************************************************************************/
#define DEBUG_LED_ENABLE                    0
#if (DEBUG_LED_ENABLE != 0) && defined(DEBUG_NRF_USER)
    #include <tools/logtrace.h>
    #define LED_DEBUG                       LOG_TRACE_DEBUG
#else
    #define LED_DEBUG(...)                  {(void) (0);}
#endif

// For State_machine debug
#define DEBUG_LED_ST_ENABLE                 0
#if (DEBUG_LED_ST_ENABLE != 0) && defined(DEBUG_NRF_USER)
    #include <tools/logtrace.h>
    #define LEDS_DEBUG                      LOG_TRACE_DEBUG
#else
    #define LEDS_DEBUG(...)                 {(void) (0);}
#endif

    
/************************************************************************************************************/
/* Namespace Declaration                                                                                    */
/************************************************************************************************************/
using gpio::LedController;

    
/************************************************************************************************************/
/* CONSTRUCTOR                                                                                              */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
LedController::LedController() : _blink(false)
{
    _currentState   = ST_INIT;

    _index          = 0;
    _isTimerActive  = false;
    
    _quickOnDelay   = LedController::DEFAULT_TIME_ON;
    _quickOffDelay  = 500 - _quickOnDelay;

    _stdOnDelay     = _quickOnDelay;
    _stdOffDelay    = 1000 - _stdOnDelay;

    _slowOnDelay    = _quickOnDelay;
    _slowOffDelay   = 4000 - _slowOnDelay;
}

// -----------------------------------------------------------------------------------------------------------
LedController::~LedController()
{
}


/************************************************************************************************************/
/* PUBLIC SECTION                                                                                           */
/************************************************************************************************************/
// -----------------------------------------------------------------------------------------------------------
void LedController::initialize(uint8_t index, uint16_t timeOn, uint16_t timeOff)
{
    _index   = index;
    _timeOn  = timeOn;
    _timeOff = timeOff;

    if(hal::LedHal::initLed(index))
    {
        LED_DEBUG("[LedController] initialize() > LED %#04u initialized with timeOn: %u and timeOff: %u [Ok]\r\n", index, timeOn, timeOff);
    }
    else
    {
        LED_DEBUG("[LedController] initialize() > LED %#04u initialized [Failed]\r\n", index);
    }
}

// -----------------------------------------------------------------------------------------------------------
void LedController::ledOn()
{
    _stopBlinking();
    if(_currentState == ST_LED_OFF)
    {
        _ST_LED_ON_Action();
        LED_DEBUG("[LedController] ledON() > LED %#04x set ON\r\n" ,_index);
    }
}

// -----------------------------------------------------------------------------------------------------------
void LedController::ledOff()
{
    _stopBlinking();
    if(_currentState == ST_LED_ON)
    {
        _ST_LED_OFF_Action();
        LED_DEBUG("[LedController] ledOFF() > LED %#04x set OFF\r\n" ,_index);
    }
}

// -----------------------------------------------------------------------------------------------------------
void LedController::setTimeforQuickStandardAndSlowBlinkingState(uint16_t timeOnQuick, uint16_t timeOffQuick, 
                                                                uint16_t timeOnStd,   uint16_t timeOffStd, 
                                                                uint16_t timeOnSlow,  uint16_t timeOffSlow)
{
    _quickOnDelay  = timeOnQuick;
    _quickOffDelay = timeOffQuick;
    LED_DEBUG("[LedController] setTimeforQuickStandardAndSlowBlinkingState() > quickOn = %d, quickOff = %d\r\n" ,timeOnQuick, timeOffQuick);
    _stdOnDelay    = timeOnStd;
    _stdOffDelay   = timeOffStd;
    LED_DEBUG("[LedController] setTimeforQuickStandardAndSlowBlinkingState() > standardOn = %d, standardOff = %d\r\n" ,timeOnStd, timeOffStd);
    _slowOnDelay   = timeOnSlow;
    _slowOffDelay  = timeOffSlow;
    LED_DEBUG("[LedController] setTimeforQuickStandardAndSlowBlinkingState() > slowOn = %d, slowOff = %d\r\n" ,timeOnSlow, timeOffSlow);
}

// -----------------------------------------------------------------------------------------------------------
void LedController::startBlinking(bool force)
{
    // The LED can not be ON for 0 [ms] !!!
    if(_timeOn == 0)
    {
        LED_DEBUG("[LedController] startBlinking() > Time ON is `0`, led#%u will not blink !\r\n", _index);
        _stopBlinking();
        return;
    }
    
    // Checs if the LED is allready blinking...
    if(_blink && !force)
    {
        LED_DEBUG("[LedController] startBlinking() > led#%u allreading blinking ...\r\n", _index);
        return; 
    }
    
    // Turn OFF any active timeout
    _stopBlinking();

    // Start blincking
    _ST_LED_ON_Action();
    getThread()->scheduleTimeout(Timeout_LED_id, _timeOn, this);
    _blink = true;
    _isTimerActive = true;
    LED_DEBUG("[LedController] startBlinking() > led%u should start blinking now...\r\n" ,_index);
}

// -----------------------------------------------------------------------------------------------------------
void LedController::startQuickBlinking()
{
    setTimeOnOff(_quickOnDelay, _quickOffDelay);
    startBlinking();
}

// -----------------------------------------------------------------------------------------------------------
void LedController::startStandardBlinking()
{
    setTimeOnOff(_stdOnDelay, _stdOffDelay);
    startBlinking();
}

// -----------------------------------------------------------------------------------------------------------
void LedController::startSlowBlinking()
{
    setTimeOnOff(_slowOnDelay, _slowOffDelay);
    startBlinking();
}


/************************************************************************************************************/
/* PROTECTED SECTION                                                                                        */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
EventStatus LedController::processEvent()
{
    switch(getCurrentEvent()->getEventType())
    {
        /****************************************************************************************************/
        /* Timeout event                                                                                    */
        /****************************************************************************************************/
        case IXFEvent::Timeout:
        {
            switch(getCurrentTimeout()->getId())
            {
                case Timeout_LED_id:
                {
                    LEDS_DEBUG("[LedController] processEvent() > Timeout_LED_ON_id received ...\r\n");
                    _isTimerActive = false;
                    if(_blink)
                    {
                        switch(_currentState) 
                        {
                            case ST_LED_OFF:
                                if(_timeOn > 0)
                                {
                                    LEDS_DEBUG("[LedController] processEvent() > -> ST_LED_ON from ST_LED_OFF.\r\n");
                                    _ST_LED_ON_Action();
                                    getThread()->scheduleTimeout(Timeout_LED_id, _timeOn, this);
                                    _isTimerActive = true;
                                }
                                break;

                            case ST_LED_ON:
                                if(_timeOff > 0)
                                {
                                    LEDS_DEBUG("[LedController] processEvent() > -> ST_LED_OFF from ST_LED_ON.\r\n");
                                    _ST_LED_OFF_Action();
                                    getThread()->scheduleTimeout(Timeout_LED_id, _timeOff, this);
                                    _isTimerActive = true;
                                }
                                break;
                            
                            default:
                                break;
                        }
                    }
                    break;
                }
                default:
                    break;
            }
            break;
        }

        /****************************************************************************************************/
        /* Initial event                                                                                    */
        /****************************************************************************************************/
        case IXFEvent::Initial:
        {
            if(_currentState == ST_INIT)
            {
                // Update State
                LEDS_DEBUG("[LedController] processEvent() > -> ST_LED_OFF from ST_INIT ((IXFEvent::Initial).\r\n");
                _ST_LED_OFF_Action();
            }
            break;
        }
        default:
            break;
    }

    return EventStatus::Consumed;
}

/************************************************************************************************************/
/* PRIVATE SECTION                                                                                          */
/************************************************************************************************************/
// -----------------------------------------------------------------------------------------------------------
void LedController::_stopBlinking()
{
    if(_blink) _blink = false;
    if(_isTimerActive)
    {
        this->getThread()->unscheduleTimeout(Timeout_LED_id, this);
        _isTimerActive = false;
    }
}

//------------------------------------------------------------------------------------------------------------
void LedController::_ST_LED_OFF_Action()
{
    LED_DEBUG("[LedController] _ST_LED_OFF_Action() > Turning OFF led#%u ...\r\n", _index);
    _currentState = ST_LED_OFF;
    hal::LedHal::setLedOff(_index);
}

// -----------------------------------------------------------------------------------------------------------
void LedController::_ST_LED_ON_Action()
{
    LED_DEBUG("[LedController] _ST_LED_ON_Action()  > Turning ON  led#%u ...\r\n", _index);
    _currentState = ST_LED_ON;
    hal::LedHal::setLedOn(_index);
}
