/*************************************************************************************************************
 * Copyright (C) Soprod SA, Sion. 2018
 * Created by Thierry Hischier (thischier@soprod.ch)
 *
 * \file    timerdriver.h
 * \brief   Class that manages a TIMER peripheral
 *
 * Initial author: Thierry Hischier
 * Creation date: 2018-06-07
 ************************************************************************************************************/
#pragma once


/************************************************************************************************************/
/*                                                                                                          */
/* Header files                                                                                             */
/*                                                                                                          */
/************************************************************************************************************/
#include "platform-config.h"
#include "timerinstance.h"

#include <nrfx_timer.h>
#include <nrf_gpio.h>

#if defined(__cplusplus)
extern "C" 
{
#endif
namespace timer
{
    /*********************************************************************************************************
     * \brief         TIMER's event handler.
     *
     * This function will be called each time the used TIMER throws an event.
     *
     * \param        The event type of the used RTC.
     * \param   pContext     The event type of the used RTC.
     ********************************************************************************************************/
    void timerEventHandler(nrf_timer_event_t timerEvent, void* pContext);
}   // namespace timer
#if defined(__cplusplus)
}
#endif

/************************************************************************************************************/
/*                                                                                                          */
/* TYPEDEF DECLARATION                                                                                      */
/*                                                                                                          */
/************************************************************************************************************/

/************************************************************************************************************/
/*                                                                                                          */
/* Namespace Declaration                                                                                    */
/*                                                                                                          */
/************************************************************************************************************/
namespace timer {

/************************************************************************************************************/
/*                                                                                                          */
/* Class Declaration                                                                                        */
/*                                                                                                          */
/************************************************************************************************************/
class TimerDriver
{
    friend void timerEventHandler(nrf_timer_event_t timerEvent, void* pContext);
    
public:
    /********************************************************************************************************/
    /*                                            PUBLIC SECTION                                            */
    /********************************************************************************************************/
    TimerDriver();
    virtual ~TimerDriver();

    void initialize();
    void bind(TIMERInstance::eTIMERInstance instance);

    uint32_t getTimerValue();

    void clearTimer();

    void setCompareValueInMiliseconds(nrf_timer_cc_channel_t compareChannel, uint32_t compareValue, bool enableInterrupt);
    void setExtendedCompareValueInMiliseconds(nrf_timer_cc_channel_t compareChannel, uint32_t compareValue, nrf_timer_short_mask_t timerShortMask, bool enableInterrupt);
    void setCompareValueInMicroseconds(nrf_timer_cc_channel_t compareChannel, uint32_t compareValue, bool enableInterrupt);
    void setExtendedCompareValueInMicroseconds(nrf_timer_cc_channel_t compareChannel, uint32_t compareValue, nrf_timer_short_mask_t timerShortMask, bool enableInterrupt);

    void enableTimer();
    void disableTimer();

protected:
    /********************************************************************************************************/
    /*                                        PROTECTED SECTION                                             */
    /********************************************************************************************************/

private:
    /********************************************************************************************************/
    /*                                         PRIVATE SECTION                                              */
    /********************************************************************************************************/
    // Attributes
    nrfx_timer_t                          _timerInstance;
    nrfx_timer_config_t                   _timerConfig;

    // Methods

};  // class PwmDriver

};  // namespace pwm
