/************************************************************************************************************
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2018
 * Created by Markus Salzmann, Patrice Rudaz & Thomas Sterren
 * All rights reserved.
 ************************************************************************************************************/

/************************************************************************************************************/
/* Header files                                                                                             */
/************************************************************************************************************/
#include "xf_adapter.h"

#if (NRF_PWR_MGMT_ENABLED != 0)

    #if defined( __cplusplus )
    extern "C" 
    {
    #endif  
        #include <nrf_pwr_mgmt.h>
    #if defined( __cplusplus )
    }
    #endif  

#else
    #include <nrf_soc.h>
//    #include <cmsis_armcc.h>
#endif  // #if (NRF_PWR_MGMT_ENABLED != 0)

#if (XF_TIMER_TICK_ON_PIO != 0)
    #include "hal/gpio/gpiohal.h"
#endif

#if !defined(APP_SCHEDULER_ENABLED)
    #error "APP_SCHEDULER not defined for sdkTools"
#elif (APP_SCHEDULER_ENABLED != 0)
    #include <ble/scheduler.h>
#endif

#include "tools/sdktools.h"



/************************************************************************************************************/
/* DEBUG COMPILATION OPTION & MACRO                                                                         */
/************************************************************************************************************/
#define DEBUG_XF_ADAPTER_ENABLE             0
#if (DEBUG_XF_ADAPTER_ENABLE != 0) && defined(DEBUG_NRF_USER)
    #include <tools/logtrace.h>
    #define XFA_DEBUG                       LOG_TRACE_DEBUG
#else
    #define XFA_DEBUG(...)                  {(void) (0);}
#endif


/************************************************************************************************************/
/* --- Extern "C" Section ---                                                                               */
/*     Callback's section                                                                                   */
/************************************************************************************************************/
void xfTimerEvent(void* p_context)
{
    // App timer is no more running
    _is_xfTimer_started = false;

    // Execute callback method
    if(p_context == NULL)
    {
        XFA_DEBUG("[XFAdapter] xfTimerEvent() > p_context != XFTimeoutManager::getInstance() !!!\r\n");
        XFTimeoutManager::getInstance()->timeout();
    }
    else
    {
        reinterpret_cast<XFTimeoutManager*>(p_context)->timeout();
    }
    
#if (XF_TIMER_TICK_ON_PIO != 0)
    hal::GpioHal::pinToggle(XF_TIMER_TICK_ON_PIO);
#endif
}


/************************************************************************************************************/
/* Declaration section                                                                                      */
/************************************************************************************************************/
#define FPU_EXCEPTION_MASK 0x0000009F
APP_TIMER_DEF(_xfTimerId);

uint8_t     _is_nested_critical_region              = 0;
bool        _is_xfTimer_started                     = false;
uint32_t    _xf_rtc_value                           = 0;


/************************************************************************************************************/
/* PUBLIC SECTION                                                                                           */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
void xfTimerInit()
{
    // Create the advertise interval timer
    uint32_t errCode = tools::SDKTools::appTimerCreate(&_xfTimerId, APP_TIMER_MODE_SINGLE_SHOT, (void*) xfTimerEvent);
    if(errCode != NRF_SUCCESS)
    {
        XFA_DEBUG("[XFAdapter] xfTimerInit() > SDKTools::appTimerCreate() FAILED ! (errCode: 0x%04x)\r\n", errCode);
        ASSERT(false);
    }

#if (NRF_PWR_MGMT_ENABLED != 0)
    // New in SDK 15, initialization of the Power Manager
    errCode = nrf_pwr_mgmt_init();
    if(errCode != NRF_SUCCESS)
    {
        XFA_DEBUG("[XFAdapter] xfTimerInit() > nrf_pwr_mgmt_init() FAILED ! (errCode: 0x%04x)\r\n", errCode);
        ASSERT(false);
    }
#endif  // #if (NRF_PWR_MGMT_ENABLED != 0)

#if (XF_TIMER_TICK_ON_PIO != 0)
    hal::GpioHal::cfgPinOutput(XF_TIMER_TICK_ON_PIO,
                               GPIO_PIN_CNF_DRIVE_H0H1,
                               GPIO_PIN_CNF_PULL_Disabled,
                               GPIO_PIN_CNF_INPUT_Disconnect);
#endif
    XFA_DEBUG("[XFAdapter] xfTimerInit() > done !\r\n");
}

// -----------------------------------------------------------------------------------------------------------
void xfTimerStart(unsigned int timerTicks)
{
	// If timerTicks is 0 or lower, directly return timeout without starting a timer
	if(timerTicks <= 0)
	{
        XFA_DEBUG("[XFAdapter] xfTimerStart() > duration: 0 => XFTimeoutManager::getInstance()->timeout()\r\n");
		XFTimeoutManager::getInstance()->timeout();
        return;
	}
    
    if(!_is_xfTimer_started)
    {
        // Start an app timer & store actual RTC value at timer start
        //uint32_t ticks = timerTicks & xf_timer_max_counter_value;
        if(tools::SDKTools::appTimerStart(_xfTimerId, 
                                          timerTicks < XF_TIMER_MIN_TIMEOUT_TICKS ? XF_TIMER_MIN_TIMEOUT_TICKS : (uint32_t) timerTicks, 
                                          (void*) XFTimeoutManager::getInstance()) == NRF_SUCCESS)
        {
            // App timer is now started
            _xf_rtc_value       = tools::SDKTools::appTimerGetCounter();
            _is_xfTimer_started = true;
            XFA_DEBUG("[XFAdapter] xfTimerStart() > timerTicks: %10lu (rtc_value: %10lu)\r\n", timerTicks, _xf_rtc_value);
        }
        else
        {
            XFA_DEBUG("[XFAdapter] xfTimerStart() > SDKTools::appTimerStart() FAILED !!! <-------\r\n");
            ASSERT(false);
        }
    }
    else
    {
        XFA_DEBUG("[XFAdapter] xfTimerStart() > XF Timer already stated\r\n");
    }
}

// -----------------------------------------------------------------------------------------------------------
void xfTimerStop()
{
    if(_is_xfTimer_started)
    {
        if(tools::SDKTools::appTimerStop(_xfTimerId) == NRF_SUCCESS)
        {
            _is_xfTimer_started = false;
        }
        else
        {
            XFA_DEBUG("[XFAdapter] xfTimerStop() > SDKTools::appTimerStop() FAILED !\r\n");
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
uint32_t xfTimerGetElapsedTime()
{
	// Compute difference between starting and ending RTC value
    uint32_t elapsedTimeInTicks = (tools::SDKTools::appTimerGetCounter() - _xf_rtc_value) & xf_timer_max_counter_value; 
	
	// Convert RTC ticks to tenths of milliseconds and return that value
	return ROUNDED_DIV((uint64_t) elapsedTimeInTicks * xf_timer_rtc_frequency_tms, xf_timer_clock_frequency);
}

// -----------------------------------------------------------------------------------------------------------
inline uint32_t xfTimerGetElapsedTimerTicks()
{
    // Compute and returns the difference between starting and ending RTC value
    return (tools::SDKTools::appTimerGetCounter() - _xf_rtc_value) & xf_timer_max_counter_value;
}

// -----------------------------------------------------------------------------------------------------------
unsigned int interval2TimerTick(int interval)
{
    uint64_t timerTicks = ROUNDED_DIV((uint64_t) interval * xf_timer_clock_frequency, xf_timer_rtc_frequency_ms);
    XFA_DEBUG("[XFAdapter] interval2TimerTick() > interval : %10lu  -> timerTick: %10lu\r\n", interval, timerTicks);
    return (unsigned int) (timerTicks & 0x00000000ffffffff);
}

// -----------------------------------------------------------------------------------------------------------
void xfHookOnIdle()
{
#if (NRF_PWR_MGMT_ENABLED != 0)
    // New in SDK 15
    (void) nrf_pwr_mgmt_run();
#else
    // Wait in low power mode until something happens
    //  if( sd_app_evt_wait() != NRF_SUCCESS ) { ASSERT(false); }
    //    __WFI();
    //    __SEV();    // Clear event register
    //    __WFE();    // Wait for event
    __set_FPSCR(__get_FPSCR() & ~(FPU_EXCEPTION_MASK));
    (void) __get_FPSCR();
    NVIC_ClearPendingIRQ(FPU_IRQn);

    (void) sd_app_evt_wait();
#endif  // #if (NRF_PWR_MGMT_ENABLED != 0)
}

// -----------------------------------------------------------------------------------------------------------
#if (XF_HOOK_PREEXECUTE_ENABLED != 0)
void xfPreExecuteHook()
{
#if (APP_SCHEDULER_ENABLED != 0)
    XFA_DEBUG("[XFAdapter] xfPreExecuteHook() > ble::Scheduler::schedule() [in]...\r\n");
    ble::Scheduler::schedule();
    XFA_DEBUG("[XFAdapter] xfPreExecuteHook() > ble::Scheduler::schedule() [out]...\r\n");
#endif // APP_SCHEDULER_ENABLED
}
#endif // XF_HOOK_PREEXECUTE_ENABLED

// -----------------------------------------------------------------------------------------------------------
#if (XF_HOOK_POSTEXECUTE_ENABLED != 0)
void xfPostExecuteHook()
{
}
#endif // XF_HOOK_POSTEXECUTE_ENABLED
