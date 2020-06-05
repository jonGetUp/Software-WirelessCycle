/************************************************************************************************************
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 ************************************************************************************************************/

/************************************************************************************************************/
/* Header files                                                                                             */
/************************************************************************************************************/
#include "gpio/gpio.h"
#include "hal/gpio/gpiohal.h"
#include <nrf.h>


/************************************************************************************************************/
/* DEBUG COMPILATION OPTION & MACRO                                                                         */
/************************************************************************************************************/
#define DEBUG_GPIO_ENABLE                   0
#if (DEBUG_GPIO_ENABLE != 0) && defined(DEBUG_NRF_USER)
    #include <tools/logtrace.h>
    #define GPIO_DEBUG                      LOG_TRACE_DEBUG
#else
    #define GPIO_DEBUG(...)                 {(void) (0);}
#endif

/************************************************************************************************************/
/* Namespace Declaration                                                                                    */
/************************************************************************************************************/
using hal::GpioHal;
using gpio::Gpio;

    
/************************************************************************************************************/
/* PUBLIC SECTION                                                                                           */
/************************************************************************************************************/
// -----------------------------------------------------------------------------------------------------------
void Gpio::initialize(uint8_t pinNumber, GpioId::eGpioId gpioId, nrf_gpio_pin_pull_t pullSetup)
{
    _gpioId         = gpioId;
    _pinNumber      = pinNumber;
    _pinMask        = 1 << pinNumber;
    _pullSetup      = pullSetup;
    _isDebouncing   = false;
    
    if(pullSetup == NRF_GPIO_PIN_PULLDOWN)
    {
        _state      = Gpio::GpioFallingEdge;
    }
    else if(pullSetup == NRF_GPIO_PIN_PULLUP)
    {
        _state      = Gpio::GpioRisingEdge;
    }
    else 
    {
        _state      = Gpio::UnknownGpioState;
    }
    _lastState      = _state;

    // Configure the GPIO as an input
    hal::GpioHal::cfgPinInput(_pinNumber, GPIO_PIN_CNF_DRIVE_S0S1, pullSetup, GPIO_PIN_CNF_INPUT_Connect);
    if(pullSetup == NRF_GPIO_PIN_PULLDOWN)
    {
        GPIO_DEBUG("[Gpio] initialize() > %s (PIN %d) set as Input PullDown.\r\n", toString(), _pinNumber);
        hal::GpioHal::cfgPinInputPullDown(_pinNumber);
    }
    else if(pullSetup == NRF_GPIO_PIN_PULLUP)
    {
        GPIO_DEBUG("[Gpio] initialize() > %s (PIN %d) set as Input PullUp.\r\n", toString(), _pinNumber);
        hal::GpioHal::cfgPinInputPullup(_pinNumber);
    }
    else
    {
        GPIO_DEBUG("[Gpio] initialize() > %s (PIN %d) set as Input without any PullUp or PullDown.\r\n", toString(), _pinNumber);
        hal::GpioHal::cfgPinInputNoPull(_pinNumber);
    }
}

// -----------------------------------------------------------------------------------------------------------
void Gpio::setState(Gpio::eGpioState state)
{
    if(state != _state)
    {
        _lastState  = _state;
        _state      =  state;
        GPIO_DEBUG("[Gpio] setState() > %s Edge on %s with PIN %d (rtc counter: %lu)\r\n", 
                   (state == GpioFallingEdge ? "Falling":"Rising "), toString(), _pinNumber, 
                   (state == GpioFallingEdge ? _rtcCounterFalling:_rtcCounterRising));
        _notifyObservers();
    }
}


/************************************************************************************************************/
/* PRIVATE SECTION                                                                                          */
/************************************************************************************************************/
// -----------------------------------------------------------------------------------------------------------
void Gpio::_notifyObservers()
{
    for(uint8_t i = 0; i < observersCount(); i++)
    {
        if(observer(i) != NULL)
        {
            if(_state == GpioFallingEdge)
            {
                observer(i)->onFallingEdge(this);
            }
            else
            {
                observer(i)->onRisingEdge(this);
            }
        }
    }
}
