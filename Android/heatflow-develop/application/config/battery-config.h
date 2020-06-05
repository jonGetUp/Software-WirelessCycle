/********************************************************************************************************************
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2018
 * All rights reserved.
 *
 * \file    battery-config.h
 * \author  Patrice Rudaz (patrice.rudaz@hevs.ch)
 * \date    February 2018
 ********************************************************************************************************************/
#ifndef BATTERY_CONFIG_H
#define BATTERY_CONFIG_H

#include <sdk_config.h>
#include <saadc-config.h>
#include <platform-config.h>
#include <nrf_saadc.h>


// <<< Use Configuration Wizard in Context Menu >>>\n
#ifdef USE_APP_CONFIG
    #include "app_config.h"
#endif

//==========================================================
// <h> Battery Manager Configurator


//==========================================================
// <o> APP_BATT_SAMPLES  - Number of samples
// <i> Number of samples taken during a measurement

#ifndef APP_BATT_SAMPLES
    #define APP_BATT_SAMPLES                5
#endif


//==========================================================
// <h> Battery Manager Delays


//==========================================================
// <q> APP_BATT_USE_XF_TIMER   - You can choose to use a Low Power Timer provided by a third party or the XF's timer

#ifndef APP_BATT_USE_XF_TIMER
    #define APP_BATT_USE_XF_TIMER           0
#endif


//==========================================================
// <o> APP_BATT_LEVEL_UPDATE_DELAY  - Delay between two Battery level measurement [m]
// <i> Time to wait between two measurement of the battery level given in minutes [m] (default 60 min (1 hour))

#ifndef APP_BATT_LEVEL_UPDATE_DELAY
    #define APP_BATT_LEVEL_UPDATE_DELAY     60
    
    #define BATT_MEASURING_TIMEOUT_DELAY    (APP_BATT_LEVEL_UPDATE_DELAY * 60 * 1000)                                
#endif


//==========================================================
// <o> APP_BATT_SHIPPING_UPDATE_DELAY  - Delay between two Battery level measurement [h] in shipping mode
// <i> Time to wait between two measurement of the battery level given in hours [h] when the device is in shipping mode (default 24 hours)

#ifndef APP_BATT_SHIPPING_UPDATE_DELAY
    #define APP_BATT_SHIPPING_UPDATE_DELAY  60
    
    #define BATT_MEASURING_SHIPPING_DELAY   (APP_BATT_SHIPPING_UPDATE_DELAY * 3600 * 1000)                                
#endif


//==========================================================
// <o> APP_BATT_SAMPLE_DELAY  - Delay between measurement [ms]
// <i> Time to wait between each sample of measurement in ms.

#ifndef APP_BATT_SAMPLE_DELAY
    #define APP_BATT_SAMPLE_DELAY           25
#endif


//==========================================================
// <o> APP_BATT_PREPARE_GPIO_DELAY  - Delay until GPIO is prepared [ms]
// <i> Time to wait until the GPIO is prepared to start the measure in ms.

#ifndef APP_BATT_PREPARE_GPIO_DELAY
    #define APP_BATT_PREPARE_GPIO_DELAY     200
#endif


//==========================================================
// <o> APP_BATT_MANAGER_SDT_DELAY  - Standard delay for the Battery Manager [ms]

#ifndef APP_BATT_MANAGER_STD_DELAY
    #define APP_BATT_MANAGER_STD_DELAY      20
#endif

//==========================================================
// </h>



//==========================================================
// <e> APP_USE_FAKE_BATT_LEVEL  - Battery's simulation
// <i> Simulates that a battery is used in the product...

#ifndef APP_USE_FAKE_BATT_LEVEL
    #define APP_USE_FAKE_BATT_LEVEL         0
#endif


//==========================================================
// <o> APP_FAKE_BATT_LEVEL  - Battery's level
// <i> This is the battery level used when the simulation is enabled

#ifndef APP_FAKE_BATT_LEVEL
    #define APP_FAKE_BATT_LEVEL             75
#endif

//==========================================================
// </e>


//==========================================================
// <o> APP_BATT_LOW_BATT_THRESHOLD  - Battery's level triggering the Low Batt level

#ifndef APP_BATT_LOW_BATT_THRESHOLD
    #define APP_BATT_LOW_BATT_THRESHOLD     10
#endif


//==========================================================
// <o> APP_BATT_SHIPPING_LOW_BATT_THRESHOLD  - Battery's level triggering the Low Batt level when in shipping mode

#ifndef APP_BATT_SHIPPING_LOW_THRESHOLD
    #define APP_BATT_SHIPPING_LOW_THRESHOLD 25
#endif


//==========================================================
// <e> APP_BATT_USE_EOL_MODE  - Enable/disable the usage of the End Of Life modes. 

#ifndef APP_BATT_USE_EOL_MODE
    #define APP_BATT_USE_EOL_MODE           1
#endif


//==========================================================
// <o> APP_BATT_EOL1  - Battery's level triggering the EOL Mode 1

#ifndef APP_BATT_EOL1
    #define APP_BATT_EOL1                   25
#endif

//==========================================================
// <o> APP_BATT_EOL2  - Battery's level triggering the EOL Mode 2

#ifndef APP_BATT_EOL2
    #define APP_BATT_EOL2                   15
#endif

//==========================================================
// <o> APP_BATT_EOL3  - Battery's level triggering the EOL Mode 3

#ifndef APP_BATT_EOL3
    #define APP_BATT_EOL3                   10
#endif

//==========================================================
// </e>


//==========================================================
// <o> APP_BATT_SAADC_RESOLUTION  - Battery's SAADC resolution

// <0=> 8 bit 
// <1=> 10 bit 
// <2=> 12 bit 
// <3=> 14 bit 


#ifndef APP_BATT_SAADC_RESOLUTION
    #define APP_BATT_SAADC_RESOLUTION       2        
#endif


//==========================================================
// <q> APP_BATT_USE_INTERNAL_VDD_INPUT  - No GPIO will be used.
// <i> No GPIO will be used. The SAADC will take the internal VDD reference input to measure the battery level.

#ifndef APP_BATT_USE_INTERNAL_VDD_INPUT
    #define APP_BATT_USE_INTERNAL_VDD_INPUT 1
#endif


//==========================================================
// <q> APP_BATT_USE_BATT_CARACTERISATION  - Use a specific chargeable caracterisation curve.

#ifndef APP_BATT_USE_BATT_CARACTERISATION
    #define APP_BATT_USE_BATT_CARACTERISATION 0
#endif


//==========================================================
// </h>

// -----------------------------------------------------------------------------------------------
// 

#define BATT_AFTER_CHARGE_MEAS_DELAY        600000
#define BATT_SAADC_VMAX                     3.0f

    
#if   (APP_BATT_SAADC_RESOLUTION == 0)
    #define BATT_SAADC_VALMAX               255u
#elif (APP_BATT_SAADC_RESOLUTION == 1)
    #define BATT_SAADC_VALMAX               1023u
#elif (APP_BATT_SAADC_RESOLUTION == 2)
    #define BATT_SAADC_VALMAX               4095u
#else
    #define BATT_SAADC_VALMAX               16383u
#endif

#if (APP_BATT_USE_INTERNAL_VDD_INPUT != 0)
    #define BATT_LEVEL_MEASURE_GPIO         NRF_SAADC_INPUT_VDD
    #define BATT_LEVEL_SAADC_GAIN           NRF_SAADC_GAIN1_6
    #define BATT_SAADC_INTERNAL_VMAX        3.6f

    #if   (APP_BATT_SAADC_RESOLUTION == 0)
        #define BATT_SAADC_VALMIN           155u
    #elif (APP_BATT_SAADC_RESOLUTION == 1)
        #define BATT_SAADC_VALMIN           625u
    #elif (APP_BATT_SAADC_RESOLUTION == 2)
        #define BATT_SAADC_VALMIN           2500u
    #else
        #define BATT_SAADC_VALMIN           10000u
    #endif
#else
    #define BATT_LEVEL_MEASURE_GPIO         GPIO_CHG_MES
    #define BATT_LEVEL_SAADC_GAIN           NRF_SAADC_GAIN1_5

    #define BATT_PC1                        6750                        // charged at 88.5%
    #define BATT_PC2                        7650                        // charged at 95%
    #define BATT_PC3                        9450                        // charged at 99%
    #define BATT_PC4                        9930                        // charged at 100%

    // R13=4.0M, R14=10.0M //
    #define BATT_C0_INT                     19773u
    #define BATT_C1_INT                     17831u
    #define BATT_C2_INT                     10263u
    #define BATT_C3_INT                     24837u

    #define BATT_SCALE_C0_I                 100u
    #define BATT_SCALE_C1                   100u
    #define BATT_SCALE_C2                   500u
    #define BATT_SCALE_C3                   1u

    #if   (APP_BATT_SAADC_RESOLUTION == 0)
        #define BATT_SAADC_VALMIN           207u
    #elif (APP_BATT_SAADC_RESOLUTION == 1)
        #define BATT_SAADC_VALMIN           833u
    #elif (APP_BATT_SAADC_RESOLUTION == 2)
    #define BATT_SAADC_VALMIN               3336u
    #else
        #define BATT_SAADC_VALMIN           13346u
    #endif
#endif

#endif // BATTERY_CONFIG_H
