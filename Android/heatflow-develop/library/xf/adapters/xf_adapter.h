/****************************************************************************************************************//**
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2018
 * Created by Thomas Sterren (thomas.sterren@hevs.ch) & Patrice Rudaz (patrice.rudaz@hevs.ch)
 * All rights reserved.
 *
 * \file    xf_adapter.h
 *
 * \addtogroup XF eXecution Framework
 * \{
 *
 * \brief   Definition file for xf_adapter.cpp
 *
 * This adapter is dedicated to the nRF51822 Nordic chip. This file take care of the hardware specification to match 
 * the resources needed by the XF.
 *
 * Initial author:  Patrice Rudaz
 * Creation date:   2013-09-19
 *
 * \author  Patrice RUDAZ
 * \date    July 2018
 ********************************************************************************************************************/
#pragma once

/* **************************************************************************************************************** */
/*                                                                                                                  */
/* Header files                                                                                                     */
/*                                                                                                                  */
/* **************************************************************************************************************** */
#include <xf-config.h>
#include "core/xftimeoutmanager.h"


/* **************************************************************************************************************** */
/*                                                                                                                  */
/* Declaration section                                                                                              */
/*                                                                                                                  */
/* **************************************************************************************************************** */
#if defined( __cplusplus )
extern "C" 
{
#endif // __cplusplus
    extern uint8_t  _is_nested_critical_region; ///< \brief To keep the nested number of call of critical region entry
    extern bool     _is_xfTimer_started;        ///< \brief To know if the XF timer is already running
    extern uint32_t _xf_rct_value;              ///< \brief Stores the RTC value when the app timer is started

    
    /************************************************************************************************************//**
     * \brief   Timeout's handler for the XF timer.
     *
     * This function will be called each time the XF's timer expires. It is also the callback method of the 
     * `app_timer` used by the XF_Adapter for nRF5 series.
     *
     * \param   pContext    Pointer used for passing some arbitrary information (context) from the app_start_timer() 
     *                      call to the timeout handler.
     ****************************************************************************************************************/
    void xfTimerEvent(void* p_context);
    
#if defined( __cplusplus )
}
#endif // __cplusplus

static const uint64_t xf_timer_clock_frequency     =  APP_TIMER_CLOCK_FREQ;
static const uint64_t xf_timer_rtc_frequency_ms    = (APP_TIMER_CONFIG_RTC_FREQUENCY + 1) * 1000;
static const uint64_t xf_timer_rtc_frequency_tms   =  xf_timer_rtc_frequency_ms * 10;
static const uint32_t xf_timer_max_counter_value   =  XF_TIMER_MAX_COUNTER_VAL;


/* **************************************************************************************************************** */
/*                                                                                                                  */
/* PUBLIC DECLARATION SECTION                                                                                       */
/*                                                                                                                  */
/* **************************************************************************************************************** */
/****************************************************************************************************************//**
 * \brief   Initialize the timer needed by the XF.
 *
 * This method initializes the timer used by the XF to perform some time-out events. In this case,
 * we use an `app_timer` (Nordic SoftDevice) to realize the timer. This timer is based on a RTC in
 * in counter mode.
 *
 * The RTC will run off a low-frequency clock (LFCLK) running at 32.768 kHz. The COUNTER
 * resolution will therefore be 30.517 탎. For more information, see nRF51_Reference_manual.pdf
 ********************************************************************************************************************/
void xfTimerInit();

/****************************************************************************************************************//**
 * \brief   Starts the XF's timer.
 *
 * This method starts the XF's timer. As the resolution is about 30 탎, we have to set-up a trick to correct the 
 * deviation of the timer. The chosen solution is a kind of inhibition.
 *
 * The timer's frequency is 32.768 kHz. This means that we need 32.768 timer's tick to reach the millisecond, which 
 * is the base time we need for the XF. Now we have to choose between 32 or 33 for the value of the timer's counter: 
 * to be able to do some inhibition, we need a base time less than 1 ms and so we'll take 32 for the timer's counter. 
 * According to this decision, we can compute the deviation of time of the timer for 1 ms: 
 *
 *                                         1 [ms] - (32 / 32.768) = 0.0234375!
 *
 * Let's try with a time base of 2 ms: the counter's value will be choose between 65 and 66 (2 [ms] * 32.768 = 65.536). 
 * We'll take 65 and the deviation becomes: 2 [ms] - (65 / 32.768) = 0.00817! This seems to be better than the one 
 * with 1 ms as base time.
 *
 * Now we have to find when a tick must be inhibited to correct the time's deviation. This is done by resolving the 
 * equation `a * x = b * (x + 1)` where `a` is the base time and `b` is (tickCounter / 32.768). So for a 1 ms base 
 * time, we have to skip a timer's tick every 42 ticks and the deviation will be 7.8125 탎. For 2 ms base time, 
 * we'll skip a timer's tick every 121 ticks and the deviation will be of 4.3945 탎.
 *
 * \param   timerTicks    The amount of RTC ticks to wait before sending a timeout event...
 *
 * \see     xfTimerInit
 ********************************************************************************************************************/
void xfTimerStart(unsigned int timerTicks);

/****************************************************************************************************************//**
 * \brief   Stops the XF's timer.
 *
 * If the queue of TimeoutEvent is empty, we simply stops the timer. This can save current consumption.
 *
 * \see     xfTimerInit
 ********************************************************************************************************************/
void xfTimerStop();

/****************************************************************************************************************//**
 * \brief   Stops and restarts the XF's timer.
 *
 * This method stops the XF's timer (if necessary) and starts it right after with the given duration. 
 *
 * \param   duration    The amount of RTC ticks to wait before sending a timeout event...
 *
 * \see     xfTimerStart
 ********************************************************************************************************************/
inline void xfTimerRestart(uint32_t duration)       { xfTimerStop(); xfTimerStart(duration); }

/****************************************************************************************************************//**
 * \brief	Returns the elapsed time of actually running timer in tenths of milliseconds.
 ********************************************************************************************************************/
uint32_t xfTimerGetElapsedTime();

/****************************************************************************************************************//**
 * \brief	Returns the elapsed time of actually running timer in XF's timer ticks.
 ********************************************************************************************************************/
inline uint32_t xfTimerGetElapsedTimerTicks();


/****************************************************************************************************************//**
 * \brief   Convert the given interval value to XF's timer tick counter.
 *
 * \param   interval    The timeout to be converted
 *
 * \return  The amount of tick that the XF's timer has to count before firing a timeout event.
 ********************************************************************************************************************/
unsigned int interval2TimerTick(int interval);


/****************************************************************************************************************//**
 * \brief   Called whenever there are actually no more events to execute.
 ********************************************************************************************************************/
void xfHookOnIdle();

/****************************************************************************************************************//**
 * \brief   Called before dispatch of the next event.
 ********************************************************************************************************************/
#if (XF_HOOK_PREEXECUTE_ENABLED != 0)
    void xfPreExecuteHook();
#endif // XF_HOOK_PREEXECUTE_ENABLED

/****************************************************************************************************************//**
 * \brief   Called after dispatch of an event.
 ********************************************************************************************************************/
#if (XF_HOOK_POSTEXECUTE_ENABLED != 0)
    void xfPostExecuteHook();
#endif // XF_HOOK_POSTEXECUTE_ENABLED

/** \} */
