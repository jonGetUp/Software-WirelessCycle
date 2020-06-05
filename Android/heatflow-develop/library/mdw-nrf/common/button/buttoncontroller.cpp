/************************************************************************************************************
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 ************************************************************************************************************/

/************************************************************************************************************/
/* Header files                                                                                             */
/************************************************************************************************************/
#include "buttoncontroller.h"

#if (USE_BUTTONS != 0)

#include <factory.h>
#include <assert-config.h>
#include <tools/sdktools.h>


/************************************************************************************************************/
/* DEBUG COMPILATION OPTION & MACRO                                                                         */
/************************************************************************************************************/
#define DEBUG_BUTTON_CONTROLLER_ENABLE      0
#if (DEBUG_BUTTON_CONTROLLER_ENABLE != 0) && defined(DEBUG_NRF_USER)
    #include <tools/logtrace.h>
    #define BC_DEBUG                        LOG_TRACE_DEBUG
#else
    #define BC_DEBUG(...)                   {(void) (0);}
#endif

#define DEBUG_BUTTON_CONTROLLER_DEEP_ENABLE 0
#if (DEBUG_BUTTON_CONTROLLER_DEEP_ENABLE != 0) && defined(DEBUG_NRF_USER)
    #include <tools/logtrace.h>
    #define BCD_DEBUG                       LOG_TRACE_DEBUG
#else
    #define BCD_DEBUG(...)                  {(void) (0);}
#endif

#define DEBUG_BUTTON_CTRL_EVENT_ENABLE      0
#if (DEBUG_BUTTON_CTRL_EVENT_ENABLE != 0) && defined(DEBUG_NRF_USER)
    #include <tools/logtrace.h>
    #define BCE_DEBUG                       LOG_TRACE_DEBUG
#else
    #define BCE_DEBUG(...)                  {(void) (0);}
#endif

#define DEBUG_BUTTON_CTRL_TIMER_ENABLE      0
#if (DEBUG_BUTTON_CTRL_TIMER_ENABLE != 0) && defined(DEBUG_NRF_USER)
    #include <tools/logtrace.h>
    #define BCT_DEBUG                       LOG_TRACE_DEBUG
#else
    #define BCT_DEBUG(...)                  {(void) (0);}
#endif

#define DEBUG_BUTTON_CTRL_TIMER_DEEP_ENABLE 0
#if (DEBUG_BUTTON_CTRL_TIMER_DEEP_ENABLE != 0) && defined(DEBUG_NRF_USER)
    #include <tools/logtrace.h>
    #define BCTD_DEBUG                      LOG_TRACE_DEBUG
#else
    #define BCTD_DEBUG(...)                 {(void) (0);}
#endif


/************************************************************************************************************/
/* Object Declaration                                                                                       */
/************************************************************************************************************/
APP_TIMER_DEF(_buttonHandlerTimerId);


/************************************************************************************************************/
/* Namespace Declaration                                                                                    */
/************************************************************************************************************/
using gpio::ButtonController;


/************************************************************************************************************/
/* --- Extern "C" Section ---                                                                               */
/*     Callback's section                                                                                   */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
void gpio::btnControllerTaskEventHandler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    #if (USE_BUTTONS != 0)
    // Save the RTC1 counter value
    static uint32_t counterValue = 0;
    counterValue = tools::SDKTools::appTimerGetCounter();
    Factory::buttonController.gpioController()->gpioteEventHandler(pin, action, counterValue);
    #endif
}

//------------------------------------------------------------------------------------------------------------
void gpio::btnCtrlTimeoutHandler(void* pContext)
{
    #if (USE_BUTTONS != 0)
    if(pContext != NULL)
    {
        (reinterpret_cast<ButtonController*>(pContext))->timerEventHandler();
    }
    #endif
}


/************************************************************************************************************/
/* PUBLIC SECTION                                                                                           */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
void ButtonController::initialize()
{
    // Initializations
    _gpioController.initialize((APP_BUTTON_DEBOUNCE_MS_DELAY / 2));
    for(uint8_t i = 0; i < NB_MAX_BUTTONS; i++)
    {
        _buttons[i]             = NULL;
        _clickCounters[i]       = _resetClickCounterValue;
    }

    _buttonTimerTickDelay       = APP_BUTTON_MS_TIMER_TICK;
    _buttonTimerTickDelayValue  = APP_TIMER_TICKS(APP_BUTTON_MS_TIMER_TICK);
    
    BCT_DEBUG("[BCT] initialize() > _buttonTimerTickDelay      = %d [ms]\r\n", _buttonTimerTickDelay);
    BCT_DEBUG("[BCT] initialize() > _resetDelayCounterValue    = %d [ms]\r\n", _resetDelayCounterValue);
}

//------------------------------------------------------------------------------------------------------------
bool ButtonController::registerButton(Button* button)
{
    _buttons[_nbRegisteredObject++] = button;
    _pinMaskForRegisteredObject    |= button->pinMask();
    BC_DEBUG("[ButtonController] registerButton() > pinMask = 0x%08x, _pinMaskForRegisteredButtons = 0x%#08x\r\n", button->pinMask(), _pinMaskForRegisteredObject);

    _gpioController.registerGpio(button->gpio(), true, APP_BUTTON_DEBOUNCE_MS_DELAY, gpio::btnControllerTaskEventHandler);
    button->gpio()->addObserver(this);
    return true;
}

//------------------------------------------------------------------------------------------------------------
uint8_t ButtonController::pinNumber(uint8_t index) const
{
    if(index < nbOfRegisteredButtons())
    {
        return _buttons[index]->pinNumber();
    }
    return 255;
}

//------------------------------------------------------------------------------------------------------------
uint8_t ButtonController::indexFromPinNumber(uint8_t pinNumber) const
{
    for(uint8_t i = 0; i < _nbRegisteredObject; i++)
    {
        if(_buttons[i]->pinNumber() == pinNumber)
        {
            return i;
        }
    }
    BCD_DEBUG("[BCD] indexFromPinNumber(%02d) > Nothing found !\r\n", pinNumber);
    return 255;
}

//------------------------------------------------------------------------------------------------------------
uint8_t ButtonController::indexFromId(uint8_t buttonId) const
{
    for(uint8_t i = 0; i < _nbRegisteredObject; i++)
    {
        if(_buttons[i]->buttonId() == buttonId)
        {
            return i;
        }
    }
    BCD_DEBUG("[BCD] indexFromId(%02d) > Nothing found !\r\n", buttonId);
    return 255;
}

//------------------------------------------------------------------------------------------------------------
uint32_t ButtonController::pinMask(uint8_t index) const
{
    if(index < _nbRegisteredObject)
    {
        BC_DEBUG("[ButtonController] pinMask(%d) > pinMask: b%b\r\n", index, _buttons[index]->pinMask());
        return _buttons[index]->pinMask();
    }
    return 0;
}

//------------------------------------------------------------------------------------------------------------
uint8_t ButtonController::clickCounter(uint8_t index) const
{
    if(index < _nbRegisteredObject)
    {
        BCD_DEBUG("[BCD] clickCounter(%d) > counter: %d\r\n", index, _clickCounters[index]);
        return _clickCounters[index];
    }
    return 255;
}

//------------------------------------------------------------------------------------------------------------
void ButtonController::resetClickCounter(uint8_t index)
{
    if(index < _nbRegisteredObject)
    {
        _clickCounters[index] = _resetClickCounterValue;
        BCD_DEBUG("[ButtonController] resetClickCounter(%d) > counter: %d\r\n", index, _clickCounters[index]);
    }
}

//------------------------------------------------------------------------------------------------------------
void ButtonController::incrClickCounter(uint8_t index)
{
    if(index < _nbRegisteredObject)
    {
        _clickCounters[index]++;
        BCD_DEBUG("[BCD] clickCounter(%d): %d\r\n", index, _clickCounters[index]);
    }
}

//------------------------------------------------------------------------------------------------------------
void ButtonController::resetCounters()
{
    for(uint8_t i = 0; i < _nbRegisteredObject; i++)
    {
        _delayCounters[i] = 0;
        _clickCounters[i] = _resetClickCounterValue;
        BCD_DEBUG("[BCD] resetCounters() > clickCounter[%d]: %d, delayCounters[%d]: %d\r\n", i, _clickCounters[i], i, delayCounters(i));
    }
}

//------------------------------------------------------------------------------------------------------------
void ButtonController::enableController()
{
    resetCounters();
    _buttonControllerTimerInit();
}

//------------------------------------------------------------------------------------------------------------
void ButtonController::buttonPressed(uint8_t index, Button::eButtonState buttonState)
{
    if((index < _nbRegisteredObject) && (_buttons[index] != NULL)) 
    {
        BCD_DEBUG("[BCD] %s on %s\r\n", Button::stateToString(buttonState), _buttons[index]->toString());
        _buttons[index]->setState(buttonState);
    }
}

//------------------------------------------------------------------------------------------------------------
void ButtonController::buttonReleased(uint8_t index)
{
    if((index < _nbRegisteredObject) && (_buttons[index] != NULL))
    {
        BCD_DEBUG("[BCD] RELEASED on %s\r\n", _buttons[index]->toString());
        _buttons[index]->setState(Button::ButtonReleased);
    }
}

//------------------------------------------------------------------------------------------------------------
inline uint32_t ButtonController::getRtcCounterValue(uint8_t index, bool onRelease) const
{
    if((index < _nbRegisteredObject) && (_buttons[index] != NULL))
    {
        if(onRelease)
        {
            return _buttons[index]->getRtcCounterValueOnReleaseEvent();
        }
        else
        {
            return _buttons[index]->getRtcCounterValueOnClickEvent();
        }
    }
    return 0;
}

//------------------------------------------------------------------------------------------------------------
void ButtonController::timerEventHandler()
{
    #if (USE_BUTTONS != 0)
    uint8_t pin = 255;
    uint8_t i   = 0;

    // Loop on all registered Buttons
    BCTD_DEBUG("[BCTD] +\r\n");
    for(i = 0; i < _nbRegisteredObject; i++)
    {
        pin = pinNumber(i);
        if(pin < NUMBER_OF_PINS)
        {
            incrDelayCounter(i, _buttonTimerTickDelay);
            BCT_DEBUG("[BCT] delayCounters %s: %d\r\n",_buttons[i]->toString(), delayCounters(i));
            
            // ************* Handling "PRESS Action" *************
            #if (BTN_IS_ACTIVE_HIGH != 0)
            if(nrfx_gpiote_in_is_set(pin))
            #else
            if(!nrfx_gpiote_in_is_set(pin))
            #endif
            {
                if(clickCounter(i) == 0)
                {
                    // Check for LONG or VERY LONG Press
                    if(delayCounters(i) > ButtonLongPressDelay)
                    {
                        buttonPressed(i, Button::ButtonVeryLongPress);
                        BCT_DEBUG("[BCT] VERY_LONG PRESS on button %s\r\n", _buttons[i]->toString());
                    }
                    else if(delayCounters(i) > ButtonSingleClickDelay)
                    {
                        buttonPressed(i, Button::ButtonLongPress);
                        BCT_DEBUG("[BCT] LONG PRESS on button %s\r\n", _buttons[i]->toString());
                    }
                }
            }
            
            // ************* Handling "RELEASE Action" *************
            else
            {
                // Checks the timings for a double or triple click
                if(delayCounters(i) >= ButtonDoubleClickDelay)
                {
                    resetClickCounter(i);
                    BCT_DEBUG("[BCT] RELEASE on %s\r\n", _buttons[i]->toString());
                    buttonReleased(i);
                }
            }
        }
    }
    BCTD_DEBUG("[BCTD] -\r\n");

    bool stop = true;
    for(i = 0; i < _nbRegisteredObject; i++)
    {
        if(!_isButtonReleased(i))
        {
            BCTD_DEBUG("[BCTD] %d: P\r\n", i);
            stop = false;
            break;
        }
        else if((delayCounters(i) > _resetDelayCounterValue) && (delayCounters(i) < ButtonDoubleClickDelay))
        {
            BCTD_DEBUG("[BCTD] delay(%d): %d\r\n", i, delayCounters(i));
            stop = false;
            break;
        }
    }

    if(stop)
    {
        _stopButtonControllerTimer();
        BCTD_DEBUG("[BCTD] Should have stopped the timer\r\n");
        resetCounters();
    }
    #endif
}


/************************************************************************************************************/
/* PROTECTED SECTION                                                                                        */
/************************************************************************************************************/
// -----------------------------------------------------------------------------------------------------------
void ButtonController::onFallingEdge(gpio::Gpio* gpio) 
{
    if(gpio == NULL)
    {
        BCE_DEBUG("[BCE] onFallingEdge() > NULL pointer error !!!\r\n");
        ASSERT(false);
        return;
    }
    
    uint8_t buttonIdx = indexFromId(gpio->gpioId());
    #if (BTN_IS_ACTIVE_HIGH != 0)
    // Handles the "Button Released" gesture
    _onGpioStateChanged(buttonIdx, true);
    #else
    // Handles the "Button Pressed" gesture
    _onGpioStateChanged(buttonIdx, false);
    #endif  // #if (BTN_IS_ACTIVE_HIGH != 0)
}

//------------------------------------------------------------------------------------------------------------
void ButtonController::onRisingEdge(gpio::Gpio* gpio)
{
    if(gpio == NULL)
    {
        BCE_DEBUG("[BCE] onRisingEdge() > NULL pointer error !!!\r\n");
        ASSERT(false);
        return;
    }
    
    uint8_t buttonIdx = indexFromId(gpio->gpioId());
    #if (BTN_IS_ACTIVE_HIGH != 0)
    // Handles the "Button Pressed" gesture
    _onGpioStateChanged(buttonIdx, false);
    #else
    // Handles the "Button Released" gesture
    _onGpioStateChanged(buttonIdx, true);
    #endif  // #if (BTN_IS_ACTIVE_HIGH != 0)
}


/************************************************************************************************************/
/* PRIVATE SECTION                                                                                          */
/************************************************************************************************************/
// -----------------------------------------------------------------------------------------------------------
void ButtonController::_buttonControllerTimerInit()
{
    if(!_isHandlerTimerInitialized)
    {
        uint32_t errCode = tools::SDKTools::appTimerCreate(&_buttonHandlerTimerId,
        #if (BUTTON_TIMER_IN_SINGLE_SHOT != 0)
                                                        APP_TIMER_MODE_SINGLE_SHOT,
        #else
                                                        APP_TIMER_MODE_REPEATED,
        #endif
                                                        (void*) gpio::btnCtrlTimeoutHandler);
        if(errCode == NRF_SUCCESS)
        {
            _isHandlerTimerInitialized = true;
            BC_DEBUG("[ButtonController] _buttonControllerTimerInit() > Succeeded\r\n");
        }
        else
        {
            BC_DEBUG("[ButtonController] _buttonControllerTimerInit() > errCode: 0x%04x\r\n", errCode);
        }
    }
}

//------------------------------------------------------------------------------------------------------------
void ButtonController::_startButtonControllerTimer()
{
    if(!_isHandlerTimerActive)
    {
        BCT_DEBUG("[BCT] Starting the `app_timer` ...\r\n");
        uint32_t errCode = tools::SDKTools::appTimerStart(_buttonHandlerTimerId, _buttonTimerTickDelayValue, this);
        if(errCode == NRF_SUCCESS)                
        {
            _isHandlerTimerActive = true;
        }
        else
        {
            BCT_DEBUG("[BCT] Error while starting the debouncer's timer ! (errCode: 0x%04x)\r\n", errCode);
            ASSERT(false);
        }
    }
}

//------------------------------------------------------------------------------------------------------------
void ButtonController::_stopButtonControllerTimer()
{
    if(_isHandlerTimerActive)
    {
        BCT_DEBUG("[BCT] Stopping the `app_timer` ...\r\n");
        uint32_t errCode = tools::SDKTools::appTimerStop(_buttonHandlerTimerId);
        if(errCode == NRF_SUCCESS)
        {
            _isHandlerTimerActive = false;
        }
        else
        {
            BCT_DEBUG("[BCT] Error while stopping the button's app_timer ! (errCode: 0x%04x)\r\n", errCode);
            ASSERT(false);
        }
    }
}

//------------------------------------------------------------------------------------------------------------
void ButtonController::_onGpioStateChanged(uint8_t index, bool buttonRelease)
{
    if(index != 255)
    {
        static uint8_t clickCount = 0;
        incrDelayCounter(index, _resetDelayCounterValue);

        // ************* Handling the "Button Released" gesture *************
        if(buttonRelease)
        {
            BCE_DEBUG("[BCE] Release action on %s.\r\n", _buttons[index]->toString());
            
            // checks if we are in a double or triple click action...
            if(delayCounters(index) > ButtonSingleClickDelay)
            {
                BCE_DEBUG("[BCE] %s is released. Delay counter: %d\r\n", _buttons[index]->toString(), delayCounters(index));
                resetClickCounter(index);
                resetDelayCounter(index);
                buttonReleased(index);
            }
        }

        // ************* Handling the "Button Pressed" gesture *************
        else
        {
            BCE_DEBUG("[BCE] Press action on %s ...\r\n", _buttons[index]->toString());
            
            // checks if we are in a double or triple click action...
            if(delayCounters(index) < ButtonSingleClickDelay)
            {
                incrClickCounter(index);
                clickCount = clickCounter(index);
                BCE_DEBUG("[BCE] Incr. click counter for %s: %d\r\n", _buttons[index]->toString(), clickCount);

                // Handle the Single, Double and triple click
                if(clickCount == 0)
                {
                    buttonPressed(index, Button::ButtonSingleClick);
                    BCE_DEBUG("[BCE] SINGLE CLICK on %s\r\n",  _buttons[index]->toString());
                }
                else if(clickCount == 1)
                {
                    buttonPressed(index, Button::ButtonDoubleClick);
                    BCE_DEBUG("[BCE] DOUBLE CLICK on %s\r\n",  _buttons[index]->toString());
                }
                else if(clickCount == 2)
                {
                    buttonPressed(index, Button::ButtonTripleClick);
                    BCE_DEBUG("[BCE] TRIPLE CLICK on %s\r\n",  _buttons[index]->toString());
                    resetClickCounter(index);
                }
                
                // This is for the double-click detection
                resetDelayCounter(index);
            }
        }
        
        // Starts the timer if needed.
        _startButtonControllerTimer();
    }
}

#endif  // #if (USE_BUTTONS != 0)
