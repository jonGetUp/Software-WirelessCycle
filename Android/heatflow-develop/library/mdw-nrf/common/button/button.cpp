/************************************************************************************************************
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 ************************************************************************************************************/

/************************************************************************************************************/
/* Header files                                                                                             */
/************************************************************************************************************/
#include "button.h"

#include <nrf.h>
#include <string.h>


/************************************************************************************************************/
/* DEBUG COMPILATION OPTION & MACRO                                                                         */
/************************************************************************************************************/
#define DEBUG_BUTTON_ENABLE                 1
#if (DEBUG_BUTTON_ENABLE == 1) && defined(DEBUG_NRF_USER)
    #include <tools/logtrace.h>
    #define BTN_DEBUG                       LOG_TRACE_DEBUG
#else
    #define BTN_DEBUG(...)                  {(void) (0);}
#endif

    
/************************************************************************************************************/
/* Namespace Declaration                                                                                    */
/************************************************************************************************************/
using gpio::Button;

/************************************************************************************************************/
/* PUBLIC SECTION                                                                                           */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
void Button::initialize(uint8_t pinNumber, ButtonId::eButtonId buttonId, nrf_gpio_pin_pull_t pullSetup)
{
    // Configure GPIO
    _gpio.initialize(pinNumber, ButtonId::toGpioId(buttonId), pullSetup);
    
    _buttonId   = buttonId;
    _state      = Button::ButtonReleased;
    _lastState  = _state;
    BTN_DEBUG("[Button] initialize() > PIN %d set as %s.\r\n", pinNumber, toString());
}

//------------------------------------------------------------------------------------------------------------
bool Button::setState(Button::eButtonState state)
{
    if(state != _state)
    {
        _lastState = _state;
        _state     =  state;
        BTN_DEBUG("[Button] setState() > %s (PIN %02d) -> `%s`\r\n", toString(), pinNumber(), stateToString(state));
        _notifyObservers(_state);

        // for SAV
        switch((uint8_t) _state)
        {
            case ButtonSingleClick:
                _persistentParams.incrSingleClickCounter();
                break;

            case ButtonDoubleClick:
                _persistentParams.incrDoubleClickCounter();
                break;

            case ButtonTripleClick:
                _persistentParams.incrTripleClickCounter();
                break;

            case ButtonLongPress:
                _persistentParams.decrSingleClickCounter();
                _persistentParams.incrLongPressCounter();
                break;

            case ButtonVeryLongPress:
                _persistentParams.decrLongPressCounter();
                _persistentParams.incrVeryLongPressCounter();
                break;

            case ButtonReleased:
                _persistentParams.incrReleaseCounter();
                break;
        }
        return true;
    }
    return false;
}

#if (USE_FLASH != 0)
//------------------------------------------------------------------------------------------------------------
uint32_t* Button::save(uint16_t* size)
{
    *size = sizeof(ButtonPersistor) / 4;
    BTN_DEBUG("[Button] save() > saving parameters for %s: single click = %d, double click = %d, triple click = %d, long press = %d, very-long press = %d, release = %d\r\n",
              toString(),
              _persistentParams.singleClickCounter(), _persistentParams.doubleClickCounter(), _persistentParams.tripleClickCounter(),
              _persistentParams.longPressCounter(), _persistentParams.veryLongPressCounter(), _persistentParams.releaseCounter());
    return _persistentParams.data();
}

//------------------------------------------------------------------------------------------------------------
void Button::restore(uint32_t* data)
{
    memcpy(_persistentParams.data(), data, sizeof(ButtonPersistor));
    BTN_DEBUG("[Motor] restore() > restoring parameters for %s: single click = %d, double click = %d, ...\r\n",
              toString(),
              _persistentParams.singleClickCounter(), _persistentParams.doubleClickCounter());
}
#endif // #if (USE_FLASH != 0)

/************************************************************************************************************/
/*                                             PRIVATE SECTION                                              */
/************************************************************************************************************/
// -----------------------------------------------------------------------------------------------------------
void Button::_notifyObservers(Button::eButtonState state)
{
    for(uint8_t i = 0; i < observersCount(); i++)
    {
        if(observer(i) != NULL)
        {
            switch(state)
            {
                case ButtonReleased:      observer(i)->onButtonReleased(this);      break;
                case ButtonSingleClick:   observer(i)->onButtonSingleClick(this);   break;
                case ButtonDoubleClick:   observer(i)->onButtonDoubleClick(this);   break;
                case ButtonTripleClick:   observer(i)->onButtonTripleClick(this);   break;
                case ButtonLongPress:     observer(i)->onButtonLongPress(this);     break;
                case ButtonVeryLongPress: observer(i)->onButtonVeryLongPress(this); break;
                default: break;
            }
        }
    }
}
