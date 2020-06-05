/* Copyright (c) 2018, HES-SO Valais
 * All rights reserved.
 */
#ifndef BLE_ADVERTISER_CONFIG_H
#define BLE_ADVERTISER_CONFIG_H

#include <app_util.h>
#include <ble_advdata.h>

// <<< Use Configuration Wizard in Context Menu >>>\n
#ifdef USE_APP_CONFIG
#include "app_config.h"
#endif
//==========================================================
// <h> General Advertising parameters 

//==========================================================
// <q> ADVERTISE_FOREVER  - The device will always advertise as soon as it's disconnected


#ifndef ADVERTISE_FOREVER
    #define ADVERTISE_FOREVER                   1
#endif

//==========================================================
// <q> START_ADVERTISE_AUTO  - The device will start advertising automatically as soon as it's ready or disconnected.
// <i>This behaviour is handled by the PeripheralController. The third application doesn't have to take care of this !


#ifndef START_ADVERTISE_AUTO
    #define START_ADVERTISE_AUTO                1
#endif

//==========================================================
// <q> ADVERTISE_WHITELIST_EMPTY  - The device won't advertise if the whitelist is empty
// <i> This option is available only if the security settings make usage of the whitelist.

#ifndef ADVERTISE_WHITELIST_EMPTY
    #define ADVERTISE_WHITELIST_EMPTY           0
#endif

//==========================================================
// <o> APP_ADV_NAME_TYPE  - Include full device name in advertising data. Alternate values are:
// <i>BLE_ADVDATA_NO_NAME,    Include no device name in advertising data.
// <i>BLE_ADVDATA_SHORT_NAME, Include short device name in advertising data.
// <i>BLE_ADVDATA_FULL_NAME,  Include full device name in advertising data.
 
// <0=> BLE_ADVDATA_NO_NAME 
// <1=> BLE_ADVDATA_SHORT_NAME 
// <2=> BLE_ADVDATA_FULL_NAME 


#ifndef APP_ADV_NAME_TYPE
    #define APP_ADV_NAME_TYPE                   2
    #if     (APP_ADV_NAME_TYPE == 0)
        #define ADVERTISE_NAME_TYPE             BLE_ADVDATA_NO_NAME
    #elif   (APP_ADV_NAME_TYPE == 1)
        #define ADVERTISE_NAME_TYPE             BLE_ADVDATA_SHORT_NAME
    #else
        #define ADVERTISE_NAME_TYPE             BLE_ADVDATA_FULL_NAME
    #endif
#endif

//==========================================================
// <q> ADVERTISE_SHORT_NAME_LENGTH  - Length of short device name (if short type is specified).


#ifndef ADVERTISE_SHORT_NAME_LENGTH
    #define ADVERTISE_SHORT_NAME_LENGTH         0
#endif

//==========================================================
// <o> ADVERTISE_APPAERANCE_ENABLE  - Enable or disable the BLE APPEARANCE in advertisement

// <0=> false 
// <1=> true 


#ifndef ADVERTISE_APPAERANCE_ENABLE
    #define ADVERTISE_APPAERANCE_ENABLE         1
#endif


//==========================================================
// <o> ADVERTISE_INCLUDE_DEVICE_ADDR  - Determines if LE Bluetooth Device Address shall be included. Default value is `false`

// <0=> false 
// <1=> true 


#ifndef ADVERTISE_INCLUDE_DEVICE_ADDR
    #define ADVERTISE_INCLUDE_DEVICE_ADDR       0
#endif
    
// </h> 

//==========================================================
// <h> Advertising parameters related to Security

//==========================================================
// <o> APP_ADV_WHITELIST_ENABLE - Enable the usage of the white list.
// <0=> false 
// <1=> true 
#ifndef APP_ADV_WHITELIST_ENABLE
    #define APP_ADV_WHITELIST_ENABLE            1
#endif


//==========================================================
// <o> APP_ADV_DIRECTED_SLOW_MODE_ENABLE - Enable the directed advertising mode.
// <0=> false 
// <1=> true 

#ifndef APP_ADV_DIRECTED_SLOW_MODE_ENABLE
    #define APP_ADV_DIRECTED_SLOW_MODE_ENABLE   0
#endif


//==========================================================
// <o> APP_ADV_DIRECTED_FAST_MODE_ENABLE - Enable the directed High Duty advertising mode.
// <0=> false 
// <1=> true 

#ifndef APP_ADV_DIRECTED_FAST_MODE_ENABLE
    #define APP_ADV_DIRECTED_FAST_MODE_ENABLE   0
#endif


//==========================================================
// <o> APP_ADV_FAST_MODE_ENABLE - Enable the fast advertising mode.

// <0=> false 
// <1=> true 
#ifndef APP_ADV_FAST_MODE_ENABLE
    #define APP_ADV_FAST_MODE_ENABLE            1
#endif


//==========================================================
// <o> APP_ADV_SLOW_MODE_ENABLE - Enable the slow advetising mode.

// <0=> false 
// <1=> true 
#ifndef APP_ADV_SLOW_MODE_ENABLE
    #define APP_ADV_SLOW_MODE_ENABLE            1
#endif


//==========================================================
// <o> APP_ADV_EXTENDED_MODE_ENABLE - Enable the extended mode.

// <0=> false 
// <1=> true 
#ifndef APP_ADV_EXTENDED_MODE_ENABLE
    #define APP_ADV_EXTENDED_MODE_ENABLE        0
#endif

// </h>

//==========================================================
// <h> Advertising Timings

//==========================================================
// <o> APP_ADV_DIRECTED_TIMEOUT -  DIRECTED mode timeout in [s]
// <i> The advertising timeout for the DIRECTED mode(in seconds).
#ifndef ADV_DIRECTED_TIMEOUT
    #define ADV_DIRECTED_TIMEOUT                5
    #define APP_ADV_DIRECTED_TIMEOUT            MSEC_TO_UNITS((ADV_DIRECTED_TIMEOUT*1000), UNIT_10_MS)
#endif

//==========================================================
// <o> APP_ADV_FAST_TIMEOUT -  FAST mode timeout in [s]
// <i> The advertising timeout for the FAST mode(in seconds).
#ifndef ADV_FAST_TIMEOUT
    #define ADV_FAST_TIMEOUT                    5
    #define APP_ADV_FAST_TIMEOUT                MSEC_TO_UNITS((ADV_FAST_TIMEOUT*1000), UNIT_10_MS)
#endif

//==========================================================
// <o> APP_ADV_SLOW_TIMEOUT -  SLOW mode timeout in [s]
// <i> The advertising timeout for the SLOW mode(in seconds).
#ifndef ADV_SLOW_TIMEOUT
    #define ADV_SLOW_TIMEOUT                    25
    #define APP_ADV_SLOW_TIMEOUT                MSEC_TO_UNITS((ADV_SLOW_TIMEOUT*1000), UNIT_10_MS)
#endif

//==========================================================
// <o> ADV_DIRECTED_MODE_INTERVAL -  DIRECTED mode interval in [ms]
// <i> The advertising interval for DIRECTED mode given in [ms](in units of 0.625 ms).
// <i> The recommended advertising pattern and advertising intervals are:
// <i>  - First, advertise at 20 ms intervals for at least 30 seconds
// <i>  - If not discovered after 30 seconds, you may change to one of the following longer intervals: 152.5 ms, 211.25 ms, 318.75 ms, 417.5 ms, 546.25 ms, 760 ms, 852.5 ms, 1022.5 ms, 1285 ms
// <0=> 20 ms
// <1=> 152.50 ms
// <2=> 211.25 ms
// <3=> 318.75 ms
// <4=> 417.50 ms
// <5=> 546.25 ms
// <6=> 760.00 ms
// <7=> 852.50 ms
// <8=> 1022.50 ms
// <9=> 1285 ms


#ifndef ADV_DIRECTED_MODE_INTERVAL
    #define ADV_DIRECTED_MODE_INTERVAL          0

    #if   (ADV_DIRECTED_MODE_INTERVAL == 0)
        #define APP_ADV_DIRECTED_INTERVAL       32
    #elif (ADV_DIRECTED_MODE_INTERVAL == 1)
        #define APP_ADV_DIRECTED_INTERVAL       244
    #elif (ADV_DIRECTED_MODE_INTERVAL == 2)
        #define APP_ADV_DIRECTED_INTERVAL       338
    #elif (ADV_DIRECTED_MODE_INTERVAL == 3)
        #define APP_ADV_DIRECTED_INTERVAL       510
    #elif (ADV_DIRECTED_MODE_INTERVAL == 4)
        #define APP_ADV_DIRECTED_INTERVAL       668
    #elif (ADV_DIRECTED_MODE_INTERVAL == 5)
        #define APP_ADV_DIRECTED_INTERVAL       874
    #elif (ADV_DIRECTED_MODE_INTERVAL == 6)
        #define APP_ADV_DIRECTED_INTERVAL       1216
    #elif (ADV_DIRECTED_MODE_INTERVAL == 7)
        #define APP_ADV_DIRECTED_INTERVAL       1364
    #elif (ADV_DIRECTED_MODE_INTERVAL == 8)
        #define APP_ADV_DIRECTED_INTERVAL       1636
    #else
        #define APP_ADV_DIRECTED_INTERVAL       2056
    #endif
#endif

//==========================================================
// <o> ADV_FAST_MODE_INTERVAL -  FAST mode interval in [ms]
// <i> The advertising interval for FAST mode given in [ms](in units of 0.625 ms).
// <i> The recommended advertising pattern and advertising intervals are:
// <i>  - First, advertise at 20 ms intervals for at least 30 seconds
// <i>  - If not discovered after 30 seconds, you may change to one of the following longer intervals: 152.5 ms, 211.25 ms, 318.75 ms, 417.5 ms, 546.25 ms, 760 ms, 852.5 ms, 1022.5 ms, 1285 ms
// <0=> 20 ms
// <1=> 152.50 ms
// <2=> 211.25 ms
// <3=> 318.75 ms
// <4=> 417.50 ms
// <5=> 546.25 ms
// <6=> 760.00 ms
// <7=> 852.50 ms
// <8=> 1022.50 ms
// <9=> 1285 ms


#ifndef ADV_FAST_MODE_INTERVAL
    #define ADV_FAST_MODE_INTERVAL              0

    #if   (ADV_FAST_MODE_INTERVAL == 0)
        #define APP_ADV_FAST_INTERVAL           32
    #elif (ADV_FAST_MODE_INTERVAL == 1)
        #define APP_ADV_FAST_INTERVAL           244
    #elif (ADV_FAST_MODE_INTERVAL == 2)
        #define APP_ADV_FAST_INTERVAL           338
    #elif (ADV_FAST_MODE_INTERVAL == 3)
        #define APP_ADV_FAST_INTERVAL           510
    #elif (ADV_FAST_MODE_INTERVAL == 4)
        #define APP_ADV_FAST_INTERVAL           668
    #elif (ADV_FAST_MODE_INTERVAL == 5)
        #define APP_ADV_FAST_INTERVAL           874
    #elif (ADV_FAST_MODE_INTERVAL == 6)
        #define APP_ADV_FAST_INTERVAL           1216
    #elif (ADV_FAST_MODE_INTERVAL == 7)
        #define APP_ADV_FAST_INTERVAL           1364
    #elif (ADV_FAST_MODE_INTERVAL == 8)
        #define APP_ADV_FAST_INTERVAL           1636
    #else
        #define APP_ADV_FAST_INTERVAL           2056
    #endif
#endif

//==========================================================
// <o> APP_ADV_SLOW_INTERVAL -  SLOW mode interval in [ms]
// <i> Slow advertising interval (in units of 0.625 ms).
// <i> The recommended advertising pattern and advertising intervals are:
// <i>  - First, advertise at 20 ms intervals for at least 30 seconds
// <i>  - If not discovered after 30 seconds, you may change to one of the following longer intervals: 152.5 ms, 211.25 ms, 318.75 ms, 417.5 ms, 546.25 ms, 760 ms, 852.5 ms, 1022.5 ms, 1285 ms
// <0=> 20 ms
// <1=> 152.50 ms
// <2=> 211.25 ms
// <3=> 318.75 ms
// <4=> 417.50 ms
// <5=> 546.25 ms
// <6=> 760.00 ms
// <7=> 852.50 ms
// <8=> 1022.50 ms
// <9=> 1285 ms


#ifndef ADV_SLOW_MODE_INTERVAL
    #define ADV_SLOW_MODE_INTERVAL              6

    #if   (ADV_SLOW_MODE_INTERVAL == 0)
        #define APP_ADV_SLOW_INTERVAL           32
    #elif (ADV_SLOW_MODE_INTERVAL == 1)
        #define APP_ADV_SLOW_INTERVAL           244
    #elif (ADV_SLOW_MODE_INTERVAL == 2)
        #define APP_ADV_SLOW_INTERVAL           338
    #elif (ADV_SLOW_MODE_INTERVAL == 3)
        #define APP_ADV_SLOW_INTERVAL           510
    #elif (ADV_SLOW_MODE_INTERVAL == 4)
        #define APP_ADV_SLOW_INTERVAL           668
    #elif (ADV_SLOW_MODE_INTERVAL == 5)
        #define APP_ADV_SLOW_INTERVAL           874
    #elif (ADV_SLOW_MODE_INTERVAL == 6)
        #define APP_ADV_SLOW_INTERVAL           1216
    #elif (ADV_SLOW_MODE_INTERVAL == 7)
        #define APP_ADV_SLOW_INTERVAL           1364
    #elif (ADV_SLOW_MODE_INTERVAL == 8)
        #define APP_ADV_SLOW_INTERVAL           1636
    #else
        #define APP_ADV_SLOW_INTERVAL           2056
    #endif
#endif


//==========================================================
// <o> APP_ALT_ADV_DIRECTED_TIMEOUT -  DIRECTED mode alternative timeout in [s]
// <i> The advertising timeout for the DIRECTED mode(in seconds).
#ifndef ADV_ALT_DIRECTED_TIMEOUT
#define ADV_ALT_DIRECTED_TIMEOUT                5
    #define APP_ALT_ADV_DIRECTED_TIMEOUT        MSEC_TO_UNITS((ADV_DIRECTED_TIMEOUT*1000), UNIT_10_MS)
#endif

//==========================================================
// <o> APP_ALT_ADV_FAST_TIMEOUT -  FAST mode alternative timeout in [s]
// <i> The advertising timeout for the FAST mode(in seconds).
#ifndef ADV_ALT_FAST_TIMEOUT
#define ADV_ALT_FAST_TIMEOUT                    5
    #define APP_ALT_ADV_FAST_TIMEOUT            MSEC_TO_UNITS((ADV_FAST_TIMEOUT*1000), UNIT_10_MS)
#endif

//==========================================================
// <o> APP_ALT_ADV_SLOW_TIMEOUT -  SLOW mode alternative timeout in [s]
// <i> The advertising timeout for the SLOW mode(in seconds).
#ifndef ADV_ALT_SLOW_TIMEOUT
#define ADV_ALT_SLOW_TIMEOUT                    25
    #define APP_ALT_ADV_SLOW_TIMEOUT            MSEC_TO_UNITS((ADV_SLOW_TIMEOUT*1000), UNIT_10_MS)
#endif

//==========================================================
// <o> ADV_ALT_DIRECTED_MODE_INTERVAL -  DIRECTED mode alternative interval in [ms]
// <i> The advertising interval for DIRECTED mode given in [ms](in units of 0.625 ms).
// <i> The recommended advertising pattern and advertising intervals are:
// <i>  - First, advertise at 20 ms intervals for at least 30 seconds
// <i>  - If not discovered after 30 seconds, you may change to one of the following longer intervals: 152.5 ms, 211.25 ms, 318.75 ms, 417.5 ms, 546.25 ms, 760 ms, 852.5 ms, 1022.5 ms, 1285 ms
// <0=> 20 ms
// <1=> 152.50 ms
// <2=> 211.25 ms
// <3=> 318.75 ms
// <4=> 417.50 ms
// <5=> 546.25 ms
// <6=> 760.00 ms
// <7=> 852.50 ms
// <8=> 1022.50 ms
// <9=> 1285 ms


#ifndef ADV_ALT_DIRECTED_MODE_INTERVAL
#define ADV_ALT_DIRECTED_MODE_INTERVAL          1

    #if   (ADV_ALT_DIRECTED_MODE_INTERVAL == 0)
        #define APP_ALT_ADV_DIRECTED_INTERVAL   32
    #elif (ADV_ALT_DIRECTED_MODE_INTERVAL == 1)
        #define APP_ALT_ADV_DIRECTED_INTERVAL   244
    #elif (ADV_ALT_DIRECTED_MODE_INTERVAL == 2)
        #define APP_ALT_ADV_DIRECTED_INTERVAL   338
    #elif (ADV_ALT_DIRECTED_MODE_INTERVAL == 3)
        #define APP_ALT_ADV_DIRECTED_INTERVAL   510
    #elif (ADV_ALT_DIRECTED_MODE_INTERVAL == 4)
        #define APP_ALT_ADV_DIRECTED_INTERVAL   668
    #elif (ADV_ALT_DIRECTED_MODE_INTERVAL == 5)
        #define APP_ALT_ADV_DIRECTED_INTERVAL   874
    #elif (ADV_ALT_DIRECTED_MODE_INTERVAL == 6)
        #define APP_ALT_ADV_DIRECTED_INTERVAL   1216
    #elif (ADV_ALT_DIRECTED_MODE_INTERVAL == 7)
        #define APP_ALT_ADV_DIRECTED_INTERVAL   1364
    #elif (ADV_ALT_DIRECTED_MODE_INTERVAL == 8)
        #define APP_ALT_ADV_DIRECTED_INTERVAL   1636
    #else
        #define APP_ALT_ADV_DIRECTED_INTERVAL   2056
    #endif
#endif

//==========================================================
// <o> ADV_FAST_MODE_INTERVAL -  FAST mode alternative interval in [ms]
// <i> The advertising interval for FAST mode given in [ms](in units of 0.625 ms).
// <i> The recommended advertising pattern and advertising intervals are:
// <i>  - First, advertise at 20 ms intervals for at least 30 seconds
// <i>  - If not discovered after 30 seconds, you may change to one of the following longer intervals: 152.5 ms, 211.25 ms, 318.75 ms, 417.5 ms, 546.25 ms, 760 ms, 852.5 ms, 1022.5 ms, 1285 ms
// <0=> 20 ms
// <1=> 152.50 ms
// <2=> 211.25 ms
// <3=> 318.75 ms
// <4=> 417.50 ms
// <5=> 546.25 ms
// <6=> 760.00 ms
// <7=> 852.50 ms
// <8=> 1022.50 ms
// <9=> 1285 ms


#ifndef ADV_ALT_FAST_MODE_INTERVAL
#define ADV_ALT_FAST_MODE_INTERVAL              9

    #if   (ADV_ALT_FAST_MODE_INTERVAL == 0)
        #define APP_ALT_ADV_FAST_INTERVAL       32
    #elif (ADV_ALT_FAST_MODE_INTERVAL == 1)
        #define APP_ALT_ADV_FAST_INTERVAL       244
    #elif (ADV_ALT_FAST_MODE_INTERVAL == 2)
        #define APP_ALT_ADV_FAST_INTERVAL       338
    #elif (ADV_ALT_FAST_MODE_INTERVAL == 3)
        #define APP_ALT_ADV_FAST_INTERVAL       510
    #elif (ADV_ALT_FAST_MODE_INTERVAL == 4)
        #define APP_ALT_ADV_FAST_INTERVAL       668
    #elif (ADV_ALT_FAST_MODE_INTERVAL == 5)
        #define APP_ALT_ADV_FAST_INTERVAL       874
    #elif (ADV_ALT_FAST_MODE_INTERVAL == 6)
        #define APP_ALT_ADV_FAST_INTERVAL       1216
    #elif (ADV_ALT_FAST_MODE_INTERVAL == 7)
        #define APP_ALT_ADV_FAST_INTERVAL       1364
    #elif (ADV_ALT_FAST_MODE_INTERVAL == 8)
        #define APP_ALT_ADV_FAST_INTERVAL       1636
    #else
        #define APP_ALT_ADV_FAST_INTERVAL       2056
    #endif
#endif

//==========================================================
// <o> APP_ADV_SLOW_INTERVAL -  SLOW mode alternative interval in [ms]
// <i> Slow advertising interval (in units of 0.625 ms).
// <i> The recommended advertising pattern and advertising intervals are:
// <i>  - First, advertise at 20 ms intervals for at least 30 seconds
// <i>  - If not discovered after 30 seconds, you may change to one of the following longer intervals: 152.5 ms, 211.25 ms, 318.75 ms, 417.5 ms, 546.25 ms, 760 ms, 852.5 ms, 1022.5 ms, 1285 ms
// <0=> 20 ms
// <1=> 152.50 ms
// <2=> 211.25 ms
// <3=> 318.75 ms
// <4=> 417.50 ms
// <5=> 546.25 ms
// <6=> 760.00 ms
// <7=> 852.50 ms
// <8=> 1022.50 ms
// <9=> 1285 ms


#ifndef ADV_ALT_SLOW_MODE_INTERVAL
#define ADV_ALT_SLOW_MODE_INTERVAL              2

    #if   (ADV_ALT_SLOW_MODE_INTERVAL == 0)
        #define APP_ALT_ADV_SLOW_INTERVAL       32
    #elif (ADV_ALT_SLOW_MODE_INTERVAL == 1)
        #define APP_ALT_ADV_SLOW_INTERVAL       244
    #elif (ADV_ALT_SLOW_MODE_INTERVAL == 2)
        #define APP_ALT_ADV_SLOW_INTERVAL       338
    #elif (ADV_ALT_SLOW_MODE_INTERVAL == 3)
        #define APP_ALT_ADV_SLOW_INTERVAL       510
    #elif (ADV_ALT_SLOW_MODE_INTERVAL == 4)
        #define APP_ALT_ADV_SLOW_INTERVAL       668
    #elif (ADV_ALT_SLOW_MODE_INTERVAL == 5)
        #define APP_ALT_ADV_SLOW_INTERVAL       874
    #elif (ADV_ALT_SLOW_MODE_INTERVAL == 6)
        #define APP_ALT_ADV_SLOW_INTERVAL       1216
    #elif (ADV_ALT_SLOW_MODE_INTERVAL == 7)
        #define APP_ALT_ADV_SLOW_INTERVAL       1364
    #elif (ADV_ALT_SLOW_MODE_INTERVAL == 8)
        #define APP_ALT_ADV_SLOW_INTERVAL       1636
    #else
        #define APP_ALT_ADV_SLOW_INTERVAL       2056
    #endif
#endif

//==========================================================
// <o> APP_ADV_WHITELIST_SKIPPED_DELAY - Duration of advertisement when the whitelist is skipped [s]

#ifndef APP_ADV_WHITELIST_SKIPPED_DELAY
    #define APP_ADV_WHITELIST_SKIPPED_DELAY     120
    #define ADV_WITHOUT_WHITELIST_DELAY         APP_TIMER_TICKS((APP_ADV_WHITELIST_SKIPPED_DELAY*1000))
#endif


// </h>

//==========================================================
// <h> Advertising parameter related to specific vendor and private services

//==========================================================
// <o> APP_ADV_MAX_ADV_UUIDS_COUNT
#ifndef APP_ADV_MAX_ADV_UUIDS_COUNT
    #define APP_ADV_MAX_ADV_UUIDS_COUNT         1
#endif

//==========================================================
// <o> APP_ADV_MAX_RSP_VENDOR_UUIDS_COUNT
#ifndef APP_ADV_MAX_RSP_VENDOR_UUIDS_COUNT
    #define APP_ADV_MAX_RSP_VENDOR_UUIDS_COUNT  4
#endif

//==========================================================
// </h>

//==========================================================
// <q> ADVERTISE_TEST_MODE  - Enable or disable the test mode of the Advertiser Module


#ifndef ADVERTISE_TEST_MODE
    #define ADVERTISE_TEST_MODE                 0
#endif

#endif // BLE_ADVERTISER_CONFIG_H
