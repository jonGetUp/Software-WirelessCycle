/************************************************************************************************************
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 ************************************************************************************************************/

/************************************************************************************************************/
/* Header files                                                                                             */
/************************************************************************************************************/
#include "basecontroller.h"

#include <button-config.h>

#include <sdk_config.h>
#include <nrfx_gpiote.h>


/************************************************************************************************************/
/* DEBUG COMPILATION OPTION & MACRO                                                                         */
/************************************************************************************************************/
#define DEBUG_BASE_CONTROLLER_ENABLE        0
#if (DEBUG_BASE_CONTROLLER_ENABLE != 0) && defined(DEBUG_NRF_USER)
    #include <tools/logtrace.h>
    #define BASC_DEBUG                      LOG_TRACE_DEBUG
#else
    #define BASC_DEBUG(...)                 {(void) (0);}
#endif


/************************************************************************************************************/
/* Namespace Declaration                                                                                    */
/************************************************************************************************************/
using gpio::BaseController;


/************************************************************************************************************/
/* PUBLIC SECTION                                                                                           */
/************************************************************************************************************/
// -----------------------------------------------------------------------------------------------------------
void BaseController::initialize()
{
    // Initializations
    for(int i = 0; i < NB_MAX_GPIOS; i++)
    {
        _delayCounters[i]       = 0;
        _delays[i]              = APP_GPIO_MS_DELAY;
    }
    _nbRegisteredObject         = 0;
    _pinMaskForRegisteredObject = 0;

    _isHandlerTimerActive       = false;
    _isHandlerTimerInitialized  = false;

    if(!nrfx_gpiote_is_init())
    {
        uint32_t errorCode      = nrfx_gpiote_init();
        if(errorCode != NRF_SUCCESS)
        {
            BASC_DEBUG("[BaseController] initialize() > nrfx_gpiote_init() failed with error code: 0x%04x!\r\n", errorCode);
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
uint32_t BaseController::delayCounters(uint8_t index) const
{
    if(index < _nbRegisteredObject)
    {
        return _delayCounters[index];
    }
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
void BaseController::setDelay(uint8_t index, uint32_t delay)
{
    if(index < NB_MAX_GPIOS)
    {
        _delays[index] = delay;
        BASC_DEBUG("[BaseController] setDelay() > new delay for gpio %d is %d\r\n", index, delay);
    }
}

// -----------------------------------------------------------------------------------------------------------
uint32_t BaseController::delay(uint8_t index) const
{
    if(index < _nbRegisteredObject)
    {
        return _delays[index];
    }
    BASC_DEBUG("[BaseController] delay() > index %d is out of bound ! (_nbRegisteredObject = %d)\r\n", index, _nbRegisteredObject);
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
void BaseController::resetDelayCounter(uint8_t index, uint8_t resetVal)
{
    if(index < _nbRegisteredObject)
    {
        _delayCounters[index] = resetVal;
    }
}

// -----------------------------------------------------------------------------------------------------------
void BaseController::incrDelayCounter(uint8_t index, uint32_t offset)
{
    if(index < _nbRegisteredObject)
    {
        _delayCounters[index] += offset;
    }
}
