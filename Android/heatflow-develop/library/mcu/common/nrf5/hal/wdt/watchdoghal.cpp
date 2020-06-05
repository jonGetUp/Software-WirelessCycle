/*************************************************************************************************************
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 *
 * \file    watchdoghal.cpp
 ************************************************************************************************************/

/************************************************************************************************************/
/* Header files                                                                                             */
/************************************************************************************************************/
#include "wdt/watchdoghal.h"
#include <watchdoghal-config.h>

#if (USE_WATCHDOG != 0)

#include <platform-config.h>

/************************************************************************************************************/
/* DEBUG COMPILATION OPTION & MACRO                                                                         */
/************************************************************************************************************/
#define DEBUG_WDT_ENABLE                    0
#if (DEBUG_WDT_ENABLE != 0) && defined(DEBUG_NRF_USER)
    #include <tools/logtrace.h>
    #define WDT_DEBUG                       LOG_TRACE_DEBUG
#else
    #define WDT_DEBUG(...)                  {(void) (0);}
#endif

/************************************************************************************************************/
/* Namespace's Declaration                                                                                  */
/************************************************************************************************************/
using hal::WatchdogHal;

/************************************************************************************************************/
/* Declaration section                                                                                      */
/************************************************************************************************************/
nrfx_wdt_config_t   WatchdogHal::_config;
nrfx_wdt_channel_id WatchdogHal::_channelId;

/************************************************************************************************************/
/* --- Extern "C" Section ---                                                                               */
/*     Callback's section                                                                                   */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
void hal::watchdogHandle(void)
{
    // reset the watch
    NVIC_SystemReset();
}


/************************************************************************************************************/
/* PUBLIC SECTION                                                                                           */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
void WatchdogHal::initialize()
{
    #if (USE_WATCHDOG != 0)
    _config.behaviour           = WDT_BEHAVIOUR;
    _config.reload_value        = WATCHDOG_DELAY;
    _config.interrupt_priority  = NRFX_WDT_CONFIG_IRQ_PRIORITY;

    uint32_t errCode            = NRF_SUCCESS;

    errCode = nrfx_wdt_init(&_config, hal::watchdogHandle);
    if(errCode != NRF_SUCCESS)
    {
        WDT_DEBUG("[Watchdog] initialize() > Battery service failed to initialize \r\n");
    }

    errCode = nrfx_wdt_channel_alloc(&_channelId);
    if(errCode != NRF_SUCCESS)
    {
        WDT_DEBUG("[Watchdog] initialize() > Battery service failed to allocate a service \r\n");
    }
    #endif  // #if (USE_WATCHDOG != 0)
}

//------------------------------------------------------------------------------------------------------------
void WatchdogHal::start()
{
    #if (USE_WATCHDOG != 0)
    WDT_DEBUG("[Watchdog] start() > start counting .. \r\n");
    nrfx_wdt_enable();
    #endif  // #if (USE_WATCHDOG != 0)
}

//------------------------------------------------------------------------------------------------------------
void WatchdogHal::feed()
{
    WDT_DEBUG("[Watchdog] feed() > reset watchdog counter \r\n");
    nrfx_wdt_channel_feed(_channelId);
}

#endif
