/* Copyright (c) 2018, HES-SO Valais
 * All rights reserved.
 */
#ifndef BLE_GAP_CONFIG_H
#define BLE_GAP_CONFIG_H

#include <app_util.h>
#include <app_timer.h>

// <<< Use Configuration Wizard in Context Menu >>>\n
#ifdef USE_APP_CONFIG
#include "app_config.h"
#endif
//==========================================================
// <h> GAP Connection's interval parameters

//==========================================================
// <h> High Reactivity mode

//==========================================================
// <o> Minimum acceptable connection interval [ms] - Minimum allowed is 7.5 [ms]
// ---------------------------------------------------------
// <i> Minimum acceptable connection interval for good reactivity. 
// <i> MUST be given in [ms].


#ifndef APP_REACT_MIN_CONN_INTERVAL
#define APP_REACT_MIN_CONN_INTERVAL         20
#define REACT_MIN_CONN_INTERVAL             MSEC_TO_UNITS(APP_REACT_MIN_CONN_INTERVAL, UNIT_1_25_MS)
#endif


//==========================================================
// <o> Maximum acceptable connection interval [ms] - Maximum allowed is 10 [s]
// ---------------------------------------------------------
// <i> Maximum acceptable connection interval for good reactivity. 
// <i> MUST be given in [ms].


#ifndef APP_REACT_MAX_CONN_INTERVAL
#define APP_REACT_MAX_CONN_INTERVAL         40
#define REACT_MAX_CONN_INTERVAL             MSEC_TO_UNITS(APP_REACT_MAX_CONN_INTERVAL, UNIT_1_25_MS)
#endif


//==========================================================
// </h>

//==========================================================
// <h> Low Power mode

//==========================================================
// <o> Minimum acceptable connection interval [ms] - Minimum allowed is 7.5 [ms]
// ---------------------------------------------------------
// <i> Minimum acceptable connection interval for good reactivity. 
// <i> MUST be given in [ms].


#ifndef APP_MIN_CONN_INTERVAL
#define APP_MIN_CONN_INTERVAL               100
#define MIN_CONN_INTERVAL                   MSEC_TO_UNITS(APP_MIN_CONN_INTERVAL, UNIT_1_25_MS)
#endif


//==========================================================
// <o> Maximum acceptable connection interval [ms] - Maximum allowed is 10 [s]
// ---------------------------------------------------------
// <i> Maximum acceptable connection interval for good reactivity. 
// <i> MUST be given in [ms].


#ifndef APP_MAX_CONN_INTERVAL
#define APP_MAX_CONN_INTERVAL               200
#define MAX_CONN_INTERVAL                   MSEC_TO_UNITS(APP_MAX_CONN_INTERVAL, UNIT_1_25_MS)
#endif


//==========================================================
// <o> Delay before switching back to Low Energy mode [s]
// ---------------------------------------------------------
// <i> Delay in seconds defining the time to wait before changing High Reactivity mode to Low Power consumption mode
// <i> MUST be given in [s].

#ifndef HIGH_REACTIVITY_DELAY_COUNTER
#define HIGH_REACTIVITY_DELAY_COUNTER       20
#endif


//==========================================================
// </h>

//==========================================================
// <e> GAP Connection's parameter negociation
#ifndef ENABLE_CHANGE_CONN_PARAM
#define ENABLE_CHANGE_CONN_PARAM            1
#endif


//==========================================================
// <o> Slave Latency


#ifndef SLAVE_LATENCY
#define SLAVE_LATENCY                       0
#endif


//==========================================================
// <o> Connection supervisory timeout [s]
// ---------------------------------------------------------
// <i> MUST be given in [s].


#ifndef APP_CONN_SUP_TIMEOUT
#define APP_CONN_SUP_TIMEOUT                4
#define CONN_SUP_TIMEOUT                    MSEC_TO_UNITS((APP_CONN_SUP_TIMEOUT*1000), UNIT_10_MS)
#endif


//==========================================================
// <o> Time from initiating event [s]
// ---------------------------------------------------------
// <i> Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (15 seconds).
// <i> MUST be given in [s].


#ifndef APP_FIRST_CONN_PARAMS_UPDATE_DELAY
#define APP_FIRST_CONN_PARAMS_UPDATE_DELAY  15
#define FIRST_CONN_PARAMS_UPDATE_DELAY      APP_TIMER_TICKS((APP_FIRST_CONN_PARAMS_UPDATE_DELAY*1000))
#endif


//==========================================================
// <o> Time between each call [s]
// ---------------------------------------------------------
// <i> Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds).
// <i> MUST be given in [s].


#ifndef APP_NEXT_CONN_PARAMS_UPDATE_DELAY
#define APP_NEXT_CONN_PARAMS_UPDATE_DELAY   30
#define NEXT_CONN_PARAMS_UPDATE_DELAY       APP_TIMER_TICKS((APP_NEXT_CONN_PARAMS_UPDATE_DELAY*1000))
#endif


//==========================================================
// <o> Number of attempts
// ---------------------------------------------------------
// <i> Number of attempts before giving up the connection parameter negotiation.


#ifndef MAX_CONN_PARAMS_UPDATE_COUNT
#define MAX_CONN_PARAMS_UPDATE_COUNT        3
#endif


//==========================================================
// <o> Delay before new negociation request [ms]
// ---------------------------------------------------------
// <i> This defines the delay to wait before requesting again a connection's parameter negociation
// <i> MUST be given in [ms].


#ifndef APP_GAP_BLE_BUSY_DELAY
#define APP_GAP_BLE_BUSY_DELAY              1000
#define GAP_BLE_BUSY_DELAY                  APP_TIMER_TICKS(APP_GAP_BLE_BUSY_DELAY)
#endif


//==========================================================
// <q> GAP_USE_XF_TIMER  - Use the XF's timer to decount the negociation request delay


#ifndef GAP_USE_XF_TIMER
#define GAP_USE_XF_TIMER                    0
#endif



// </e>

// </h>

//==========================================================
// <h> Radio Power mode

//==========================================================
// <o> Radio transmit power in dBm
// ---------------------------------------------------------
// <i> Power level for standard usage. (accepted values are -40, -20, -16, -12, -8, -4, 0, 3, and 4 dBm).

// <4=> +4 dBm
// <3=> +3 dBm
// <0=> 0 dBm
// <252=> -4 dBm
// <248=> -8 dBm
// <244=> -12 dBm
// <240=> -16 dBm
// <236=> -20 dBm
// <216=> -40 dBm

#ifndef TX_POWER_LEVEL
#define TX_POWER_LEVEL                      252
#endif



//==========================================================
// <o> Radio transmit power while skipping whitelist [dBm]
// ---------------------------------------------------------
// <i> Radio TX power to be used while skipping temporarely the whitelist. (accepted values are -40, -20, -16, -12, -8, -4, 0, 3, and 4 dBm).

// <4=> +4 dBm
// <3=> +3 dBm
// <0=> 0 dBm
// <252=> -4 dBm
// <248=> -8 dBm
// <244=> -12 dBm
// <240=> -16 dBm
// <236=> -20 dBm
// <216=> -40 dBm


#ifndef TX_POWER_LEVEL_WITHOUT_WHITELIST
#define TX_POWER_LEVEL_WITHOUT_WHITELIST    0
#endif


//==========================================================
// <q> TX_POWER_TEST_MODE  - Enable or disable the test mode of the Radio's transmit power feature...


#ifndef TX_POWER_TEST_MODE
#define TX_POWER_TEST_MODE                  0
#endif


//==========================================================
// </h>

#endif // BLE_GAP_CONFIG_H
