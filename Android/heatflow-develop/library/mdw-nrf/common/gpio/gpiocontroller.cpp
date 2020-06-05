/************************************************************************************************************
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 ************************************************************************************************************/

/************************************************************************************************************/
/* Header files                                                                                             */
/************************************************************************************************************/
#include "gpiocontroller.h"

#include <factory.h>
#include <sdk_config.h>

#include "tools/sdktools.h"


/************************************************************************************************************/
/* DEBUG COMPILATION OPTION & MACRO                                                                         */
/************************************************************************************************************/
#define DEBUG_GPIO_CONTROLLER_ENABLE        0
#if (DEBUG_GPIO_CONTROLLER_ENABLE != 0) && defined(DEBUG_NRF_USER)
    #include <tools/logtrace.h>
    #define PIOC_DEBUG                      LOG_TRACE_DEBUG
#else
    #define PIOC_DEBUG(...)                 {(void) (0);}
#endif

#define DEBUG_GPIO_CONTROLLER_TE_ENABLE     0
#if (DEBUG_GPIO_CONTROLLER_TE_ENABLE == 1)
    #include <tools/logtrace.h>
    #define PIOCTE_DEBUG                    LOG_TRACE_DEBUG
#else
    #define PIOCTE_DEBUG(...)               {(void) (0);}
#endif

/************************************************************************************************************/
/* Object Declaration                                                                                       */
/************************************************************************************************************/
APP_TIMER_DEF(_gpioHandlerTimerId);


/************************************************************************************************************/
/* Namespace Declaration                                                                                    */
/************************************************************************************************************/
using gpio::GpioController;


/************************************************************************************************************/
/* --- Extern "C" Section ---                                                                               */
/*     Callback's section                                                                                   */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
void gpio::gpioControllerTaskEventHandler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
#if (USE_GPIOS != 0) || (USE_CHARGER != 0)
    // Save the RTC1 counter value
    static uint32_t counterValue = 0;
    counterValue = tools::SDKTools::appTimerGetCounter();
    Factory::gpioController.gpioteEventHandler(pin, action, counterValue);
#endif
}

//------------------------------------------------------------------------------------------------------------
void gpio::gpioControllerTimeoutHandler(void* pContext)
{
    if(pContext != NULL)
    {
        (reinterpret_cast<GpioController*>(pContext))->timerEventHandler();
    }
}


/************************************************************************************************************/
/* PUBLIC SECTION                                                                                           */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
void GpioController::initialize(uint32_t timerTickDelay, uint32_t resetValue)
{
    // Initializations
    BaseController::initialize();
    for(int i = 0; i < NB_MAX_GPIOS; i++)
    {
        _timerEnabled[i]    = false;
        _gpios[i]           = NULL;
    }
    
    _timerTickDelay         = timerTickDelay;
    _timerTickDelayValue    = APP_TIMER_TICKS(timerTickDelay);
    _resetDelayValue        = resetValue;
}

//--------------------------------------------------------------------------------------------------------------
bool GpioController::registerGpio(Gpio* gpio, bool useTimer, uint32_t debounceDelay, nrfx_gpiote_evt_handler_t eventHandler)
{
    if(_nbRegisteredObject >= NB_MAX_GPIOS)
    {
        PIOC_DEBUG("[GpioController] registerGpio() > No more GPIO can be registered ! The controller is full\r\n");
        return false;
    }
        
    _gpios[_nbRegisteredObject]  = gpio;
    _pinMaskForRegisteredObject |= gpio->pinMask();
    if(useTimer)
    {
        _gpioControllerTimerInit();
        setDelay(_nbRegisteredObject, debounceDelay);
        _timerEnabled[_nbRegisteredObject] = true;
    }
    _nbRegisteredObject++;

    nrfx_gpiote_in_config_t gpioConfig;
    gpioConfig.sense             = NRF_GPIOTE_POLARITY_TOGGLE;
    gpioConfig.pull              = gpio->pullSetup();
    gpioConfig.is_watcher        = false;
    gpioConfig.hi_accuracy       = false;
    uint32_t errCode             = nrfx_gpiote_in_init(gpio->pinNumber(), &gpioConfig, eventHandler);
    if(errCode != NRF_SUCCESS)
    {
        if(errCode == NRF_ERROR_NO_MEM)
        {
            PIOC_DEBUG("[GpioController] registerGpio() > nrfx_gpiote_in_init()failed for %s (PIN %d). Error: NRF_ERROR_NO_MEM\r\n", gpio->toString(), gpio->pinNumber());
            PIOC_DEBUG("                                  +-> Open sdk_config.h and increase the value of GPIOTE_CONFIG_NUM_OF_LOW_POWER_EVENTS !\r\n\r\n");
        }
        else
        {
            PIOC_DEBUG("[GpioController] registerGpio() > nrfx_gpiote_in_init() failed for %s (PIN %d). Error: 0x%04x\r\n", gpio->toString(), gpio->pinNumber(), errCode);
        }
        return false;
    }
    else
    {
        nrfx_gpiote_in_event_enable(gpio->pinNumber(), true);
    }
    return true;
}

//------------------------------------------------------------------------------------------------------------
void GpioController::unregisterGpio(Gpio* gpio)
{
    // Un-init the GPIOTE for the given gpio
    nrfx_gpiote_in_uninit(gpio->pinNumber());

    // remove the given gpio form the controller
    _removeGpio(gpio);
}

//------------------------------------------------------------------------------------------------------------
uint8_t GpioController::pinNumber(uint8_t index) const
{
    if(index < _nbRegisteredObject)
    {
        return _gpios[index]->pinNumber();
    }
    return 255;
}

//------------------------------------------------------------------------------------------------------------
uint8_t GpioController::indexFromPinNumber(uint8_t pinNumber) const
{
    for(uint8_t i = 0; i < _nbRegisteredObject; i++)
    {
        if(_gpios[i]->pinNumber() == pinNumber)
        {
            return i;
        }
    }
    return 255;
}

//------------------------------------------------------------------------------------------------------------
uint8_t GpioController::indexFromId(uint8_t gpioId) const
{
    for(uint8_t i = 0; i < _nbRegisteredObject; i++)
    {
        if(_gpios[i]->gpioId() == gpioId)
        {
            return i;
        }
    }
    PIOC_DEBUG("[GpioController] indexFromId(%02d) > Nothing found !\r\n", gpioId);
    return 255;
}

//------------------------------------------------------------------------------------------------------------
uint32_t GpioController::pinMask(uint8_t index) const
{
    if(index < _nbRegisteredObject)
    {
        return _gpios[index]->pinMask();
    }
    PIOC_DEBUG("[GpioController] pinMask(%02d) > Out of bound index: %d!\r\n", index);
    return 0;
}

//------------------------------------------------------------------------------------------------------------
bool GpioController::isTimerEnabled(uint8_t index) const
{
    if(index < _nbRegisteredObject)
    {
        return _timerEnabled[index];
    }
    PIOC_DEBUG("[GpioController] isTimerEnabled(%02d) > Out of bound index: %d!\r\n", index);
    return false;
}

//------------------------------------------------------------------------------------------------------------
void GpioController::enableController()
{
    _gpioControllerInit();
}

//------------------------------------------------------------------------------------------------------------
void GpioController::stateChanged(uint8_t index, Gpio::eGpioState gpioState)
{
    if(_gpios[index] != NULL)
    {
        _gpios[index]->setState(gpioState);
        PIOC_DEBUG("[GpioController] stateChanged() > %s (%d), gpioState: %d\r\n", _gpios[index]->toString(), index, gpioState);
    }
}

//------------------------------------------------------------------------------------------------------------
void GpioController::setHandlerTimerActive(bool active)
{
    _isHandlerTimerActive = active;
}

//------------------------------------------------------------------------------------------------------------
void GpioController::gpioteEventHandler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action, uint32_t rtcCounterValue)
{
    // Check which gpios generates an event
    uint8_t gpioIdx  = indexFromPinNumber(pin);
    PIOCTE_DEBUG("[PIOCTE] gpioIdx %02d (pin: %02d), action: %02d\r\n", gpioIdx, pin, action);

    if(gpioIdx != 255)
    {
        const bool gpioIsSet = nrfx_gpiote_in_is_set(pin);
        
        // Checks if the debouncer is activated
        if(isTimerEnabled(gpioIdx))
        {
            // Update debounce's process flag
            _gpios[gpioIdx]->updateDebouncerFlag(true);
            
            // Checks if the Debouncer's timer is running
            if(!_isHandlerTimerActive)                                                                                  
            {
                if(tools::SDKTools::appTimerStart(_gpioHandlerTimerId, _timerTickDelayValue, this) == NRF_SUCCESS)
                {
                    _isHandlerTimerActive = true;
                    PIOC_DEBUG("[GpioController] gpioteEventHandler() > Starts the app_timer\r\n");
                }
                else
                {
                    PIOC_DEBUG("[GpioController] gpioteEventHandler() > Error while starting the debouncer's timer\r\n");
                    ASSERT(false);
                }
            }
        }
        else
        {
            // Update the State...
            resetDelayCounter(gpioIdx, _resetDelayValue);
            if(gpioIsSet)
            {
                PIOCTE_DEBUG("[PIOCTE] %s rising.\r\n", _gpios[gpioIdx]->toString());
                stateChanged(gpioIdx, gpio::Gpio::GpioRisingEdge);
            }
            else
            {
                PIOCTE_DEBUG("[PIOCTE] %s falling.\r\n", _gpios[gpioIdx]->toString());
                stateChanged(gpioIdx, Gpio::GpioFallingEdge);
            }
        }

        // Storing the RTC counter value
        if(gpioIsSet)
        {
            PIOCTE_DEBUG("[PIOCTE] %s > setRtcCounterValue (%lu) for RISING edge...\r\n", _gpios[gpioIdx]->toString(), rtcCounterValue);
            _gpios[gpioIdx]->setRtcCounterValue(rtcCounterValue, false);
        }
        else
        {
            PIOCTE_DEBUG("[PIOCTE] %s > setRtcCounterValue (%lu) for FALLING edge...\r\n", _gpios[gpioIdx]->toString(), rtcCounterValue);
            _gpios[gpioIdx]->setRtcCounterValue(rtcCounterValue, true);
        }
    }
}

//------------------------------------------------------------------------------------------------------------
void GpioController::timerEventHandler()
{
    bool stopAppTimer = true;
    for(int i = 0; i < _nbRegisteredObject; i++)
    {
        uint8_t pin = pinNumber(i);
        if(pin < NUMBER_OF_PINS && isTimerEnabled(i))
        {
            incrDelayCounter(i, _timerTickDelay);
            PIOC_DEBUG("[GpioController] timerEventHandler() > delayCounter[%d] = %04d.\r\n", i, _delayCounters[i]);
            
            // checks debouncer's delay
            if((delayCounters(i) >= delay(i)))
            {
                _gpios[i]->updateDebouncerFlag(false);
                if(nrfx_gpiote_in_is_set(pin))
                {
                    PIOCTE_DEBUG("[PIOCTE] timerEventHandler() > Rising Edge detected on %s.\r\n", _gpios[i]->toString());
                    stateChanged(i, gpio::Gpio::GpioRisingEdge);
                }
                else
                {
                    PIOCTE_DEBUG("[PIOCTE] timerEventHandler() > Falling Edge detected on %s.\r\n", _gpios[i]->toString());
                    stateChanged(i, gpio::Gpio::GpioFallingEdge);
                }
            }
            else
            {
                stopAppTimer = false;
            }
        }
    }

    if(stopAppTimer)
    {
        tools::SDKTools::appTimerStop(_gpioHandlerTimerId);
        _isHandlerTimerActive = false;
        for(int i = 0; i < _nbRegisteredObject; i++)
        {
            resetDelayCounter(i, _resetDelayValue);
            _gpios[i]->updateDebouncerFlag(false);
        }
        PIOC_DEBUG("[GpioController] gpioControllerTimeoutHandler() > Stops the app_timer\r\n");
    }
}

/************************************************************************************************************/
/*                                             PRIVATE SECTION                                              */
/************************************************************************************************************/
// -----------------------------------------------------------------------------------------------------------
inline void GpioController::_removeGpio(Gpio* gpio)
{
    Gpio* pToMove                               = NULL;
    bool useTimer                               = false;
    for(uint8_t i = _nbRegisteredObject; i > 0; i--)
    {
        if(_gpios[i - 1] == gpio)
        {
            _gpios[i - 1]                       = pToMove;
            _timerEnabled[i - 1]                = useTimer;
            _pinMaskForRegisteredObject        &= ~gpio->pinMask();
            _nbRegisteredObject--;
            break;
        }
        else if(pToMove == NULL)
        {
            pToMove                             = _gpios[i - 1];
            useTimer                            = _timerEnabled[i - 1];
            _gpios[i - 1]                       = NULL;
            _timerEnabled[i - 1]                = false;
        }
    }
    
    if(pToMove != NULL)
    {
        _gpios[_nbRegisteredObject - 1]         = pToMove;
        _timerEnabled[_nbRegisteredObject - 1]  = useTimer;
    }
}

//------------------------------------------------------------------------------------------------------------
inline void GpioController::_clearAllGpios()
{
    for(uint8_t i = _nbRegisteredObject; i > 0; i--)
    {
        // Un-init the GPIOTE for the given gpio
        nrfx_gpiote_in_uninit(_gpios[i - 1]->pinNumber());

        _gpios[i - 1]                           = NULL;
        _timerEnabled[i - 1]                    = false;
    }
    _nbRegisteredObject                         = 0;
    _pinMaskForRegisteredObject                 = 0;
}

//------------------------------------------------------------------------------------------------------------
inline void GpioController::_gpioControllerTimerInit()
{
    if(!_isHandlerTimerInitialized)
    {
        uint32_t errCode = tools::SDKTools::appTimerCreate(&_gpioHandlerTimerId,
                                                           APP_TIMER_MODE_REPEATED,
                                                           (void*) gpio::gpioControllerTimeoutHandler);
        if(errCode == NRF_SUCCESS)
        {
            _isHandlerTimerInitialized          = true;
        }
        else
        {
            PIOC_DEBUG("[GpioController] _gpioControllerTimerInit() > errCode: 0x%04x\r\n", errCode);
        }
    }
}

//------------------------------------------------------------------------------------------------------------
inline void GpioController::_gpioControllerInit()
{
    for(uint8_t i = 0; i < NB_MAX_GPIOS; i++)
    {
        _delayCounters[i]                   = 0;
    }
}
