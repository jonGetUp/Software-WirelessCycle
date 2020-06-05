/* Copyright (c) 2018, HES-SO Valais
 * All rights reserved.
 */
#ifndef HAL_WDT_CONFIG_H
#define HAL_WDT_CONFIG_H

#include <sdk_config.h>
#include <nrf_wdt.h>

// <<< Use Configuration Wizard in Context Menu >>>\n
#ifdef USE_APP_CONFIG
#include "app_config.h"
#endif

// -----------------------------------------------------------------------------------------------
// Watchdog related
#if (WDT_CONFIG_BEHAVIOUR == 1)
    #define	WDT_BEHAVIOUR                   NRF_WDT_BEHAVIOUR_RUN_SLEEP
#elif   (WDT_CONFIG_BEHAVIOUR == 8)
    #define	WDT_BEHAVIOUR                   NRF_WDT_BEHAVIOUR_RUN_HALT
#elif   (WDT_CONFIG_BEHAVIOUR == 9)
    #define	WDT_BEHAVIOUR                   NRF_WDT_BEHAVIOUR_RUN_SLEEP_HALT
#else
    #define	WDT_BEHAVIOUR                   NRF_WDT_BEHAVIOUR_PAUSE_SLEEP_HALT
#endif

#define WDT_IRQ_PRIORITY					WDT_CONFIG_IRQ_PRIORITY

#endif // HAL_WDT_CONFIG_H
