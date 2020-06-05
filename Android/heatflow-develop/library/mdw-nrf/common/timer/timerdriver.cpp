/*************************************************************************************************************
 * Copyright (C) Soprod SA, Sion. 2018
 * Created by Thierry Hischier (thischier@soprod.ch)
 *
 * \file    pwmdriver.cpp
 * \brief   Contains all necessary functions and event handlers to drive a TIMER peripheral
 *
 * Initial author: Thierry Hischier
 * Creation date: 2018-03-16
 ************************************************************************************************************/

/************************************************************************************************************/
/*                                                                                                          */
/* Header files                                                                                             */
/*                                                                                                          */
/************************************************************************************************************/
#include "timerdriver.h"
#include "app_util_platform.h"
#include "factory.h"

#include "hal/gpio/gpiohal.h"
#include "gpio/gpioid.h"

/************************************************************************************************************/
/* DEBUG COMPILATION OPTION & MACRO                                                                         */
/************************************************************************************************************/
#define DEBUG_TIMER_DRIVER_ENABLE           1
#if (DEBUG_TIMER_DRIVER_ENABLE != 0)
    #include <tools/logtrace.h>
    #define TMRD_DEBUG                      LOG_TRACE_DEBUG
#else
    #define TMRD_DEBUG(...)                 {(void) (0);}
#endif


/************************************************************************************************************/
/* Namespace                                                                                                */
/************************************************************************************************************/
using timer::TimerDriver;

/************************************************************************************************************/
/* --- Extern "C" Section ---                                                                               */
/*     Callback's section                                                                                   */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
void timer::timerEventHandler(nrf_timer_event_t timerEvent, void* pContext)
{
    
}
    
/************************************************************************************************************/
/* Declaration section                                                                                      */
/************************************************************************************************************/
    
/************************************************************************************************************/
/* Constructor | Destructor                                                                                 */
/************************************************************************************************************/
// -----------------------------------------------------------------------------------------------------------
TimerDriver::TimerDriver() {}

// -----------------------------------------------------------------------------------------------------------
TimerDriver::~TimerDriver() {}


/************************************************************************************************************/
/*                                              PUBLIC SECTION                                              */
/************************************************************************************************************/
// -----------------------------------------------------------------------------------------------------------
void TimerDriver::initialize()
{
    _timerConfig.frequency = (nrf_timer_frequency_t)NRFX_TIMER_DEFAULT_CONFIG_FREQUENCY;
    _timerConfig.mode = (nrf_timer_mode_t)NRFX_TIMER_DEFAULT_CONFIG_MODE;
    _timerConfig.bit_width = (nrf_timer_bit_width_t)NRFX_TIMER_DEFAULT_CONFIG_BIT_WIDTH;
    _timerConfig.interrupt_priority = NRFX_TIMER_DEFAULT_CONFIG_IRQ_PRIORITY;
    _timerConfig.p_context = NULL;
    TMRD_DEBUG("[TimerDriver] initialize() > Initialise Done...\r\n");
}

// -----------------------------------------------------------------------------------------------------------
void TimerDriver::bind(TIMERInstance::eTIMERInstance instance)
{
    switch(instance)
    {
        #if (NRFX_TIMER0_ENABLED == 1)
        case TIMERInstance::TIMER0:
            _timerInstance.p_reg = NRF_TIMER0;
            _timerInstance.instance_id = NRFX_TIMER0_INST_IDX;
            _timerInstance.cc_channel_count = TIMER0_CC_NUM;
            break;
        #endif

        #if (NRFX_TIMER1_ENABLED == 1)
        case TIMERInstance::TIMER1:
            _timerInstance.p_reg = NRF_TIMER1;
            _timerInstance.instance_id = NRFX_TIMER1_INST_IDX;
            _timerInstance.cc_channel_count = TIMER1_CC_NUM;
            break;
        #endif

        #if (NRFX_TIMER2_ENABLED == 1)
        case TIMERInstance::TIMER2:
            _timerInstance.p_reg = NRF_TIMER2;
            _timerInstance.instance_id = NRFX_TIMER2_INST_IDX;
            _timerInstance.cc_channel_count = TIMER2_CC_NUM;
            break;
        #endif
        
        #if (NRFX_TIMER3_ENABLED == 1)
        case TIMERInstance::TIMER1:
            _timerInstance.p_reg = NRF_TIMER3;
            _timerInstance.instance_id = NRFX_TIMER3_INST_IDX;
            _timerInstance.cc_channel_count = TIMER3_CC_NUM;
            break;
        #endif

        #if (NRFX_TIMER4_ENABLED == 1)
        case TIMERInstance::TIMER2:
            _timerInstance.p_reg = NRF_TIMER4;
            _timerInstance.instance_id = NRFX_TIMER4_INST_IDX;
            _timerInstance.cc_channel_count = TIMER4_CC_NUM;
            break;
        #endif

        default:
            // do nothing
            break;
    }
    TMRD_DEBUG("[TimerDriver] bind() > Bind Done...\r\n");
}

// -----------------------------------------------------------------------------------------------------------
uint32_t TimerDriver::getTimerValue()
{
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
void TimerDriver::clearTimer()
{
    
}

// -----------------------------------------------------------------------------------------------------------
void TimerDriver::setCompareValueInMiliseconds(nrf_timer_cc_channel_t compareChannel, uint32_t compareValue, bool enableInterrupt)
{
    uint32_t timeInTicks = nrfx_timer_ms_to_ticks(&_timerInstance, compareValue);
    nrfx_timer_compare(&_timerInstance, compareChannel, timeInTicks, enableInterrupt);
}

// -----------------------------------------------------------------------------------------------------------
void TimerDriver::setExtendedCompareValueInMiliseconds(nrf_timer_cc_channel_t compareChannel, uint32_t compareValue, nrf_timer_short_mask_t timerShortMask, bool enableInterrupt)
{
    
}

// -----------------------------------------------------------------------------------------------------------
void TimerDriver::setCompareValueInMicroseconds(nrf_timer_cc_channel_t compareChannel, uint32_t compareValue, bool enableInterrupt)
{
    uint32_t timeInTicks = nrfx_timer_us_to_ticks(&_timerInstance, compareValue);
    nrfx_timer_compare(&_timerInstance, compareChannel, timeInTicks, enableInterrupt);
}

// -----------------------------------------------------------------------------------------------------------
void TimerDriver::setExtendedCompareValueInMicroseconds(nrf_timer_cc_channel_t compareChannel, uint32_t compareValue, nrf_timer_short_mask_t timerShortMask, bool enableInterrupt)
{
    
}

// -----------------------------------------------------------------------------------------------------------
void TimerDriver::enableTimer()
{
    
}

// -----------------------------------------------------------------------------------------------------------
void TimerDriver::disableTimer()
{
    
}
