/* Copyright (c) 2018, HES-SO Valais
 * All rights reserved.
 */
#ifndef MDW_BLE_CONFIG_H
#define MDW_BLE_CONFIG_H

#include <nrf.h>
#include <nrf_soc.h>

// <<< Use Configuration Wizard in Context Menu >>>\n
#ifdef USE_APP_CONFIG
#include "app_config.h"
#endif
//==========================================================
// <h> General BLE Configuration Tool 

//==========================================================
// <o> BLE Observer's Priority - Application's BLE observer priority.
// ---------------------------------------------------------
// <i> Application's BLE observer priority. You shouldn't need to modify this value.


#ifndef APP_BLE_OBSERVER_PRIO
#define APP_BLE_OBSERVER_PRIO               2
#endif


//==========================================================
// <o> SOC Observer's Priority - Application's SoC observer priority.
// ---------------------------------------------------------
// <i> Application's SoC observer priority. You shouldn't need to modify this value.


#ifndef APP_SOC_OBSERVER_PRIO
#define APP_SOC_OBSERVER_PRIO               1
#endif


//==========================================================
// <o> SoftDevice BLE configuration
// ---------------------------------------------------------
// <i> A tag identifying the SoftDevice BLE configuration.


#ifndef APP_BLE_CONN_CFG_TAG
#define APP_BLE_CONN_CFG_TAG               1
#endif


//==========================================================
// <q> DC/DC converter modes.
// ---------------------------------------------------------
// <i> Enable or disable the usage of the DCDC converter module.


#ifndef APP_DCDC_CONVERTER_MODE
#define APP_DCDC_CONVERTER_MODE             0

#if (APP_DCDC_CONVERTER_MODE != 0)
    #define DCDC_CONVERTER_MODE             NRF_POWER_DCDC_ENABLE
#else
    #define DCDC_CONVERTER_MODE             NRF_POWER_DCDC_DISABLE
#endif
#endif

//==========================================================
// </h>

#endif // MDW_BLE_CONFIG_H
