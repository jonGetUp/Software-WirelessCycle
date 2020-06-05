/********************************************************************************************************************
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2018
 * All rights reserved.
 *
 *
 * \author  Patrice Rudaz (patrice.rudaz@hevs.ch)
 * \version 2.0.0
 * \date    March 2018
 ********************************************************************************************************************/
#ifndef MDW_SAADC_CONFIG_H
#define MDW_SAADC_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>\n
#ifdef USE_APP_CONFIG
#include "app_config.h"
#endif


//==========================================================
// <h> SAADC defines


//==========================================================
// <q> SAADC_CONFIG_LP_MODE  - Enabling low power mode

#ifndef APP_SAADC_CONFIG_LP_MODE
    #define APP_SAADC_CONFIG_LP_MODE        1
#endif

//==========================================================
// <q> SAADC_BURST_MODE
// <i> Set to 1 to enable BURST mode, otherwise set to 0.

#ifndef APP_SAADC_BURST_MODE
    #define APP_SAADC_BURST_MODE            1
#endif


//==========================================================
// <o> SAADC_CALIBRATION_INTERVAL
// <i> Determines how often the SAADC should be calibrated relative to NRF_DRV_SAADC_EVT_DONE event.
// <i> E.g. value 5 will make the SAADC calibrate every fifth time the NRF_DRV_SAADC_EVT_DONE is received.

#ifndef APP_SAADC_CALIBRATION_INTERVAL
    #define APP_SAADC_CALIBRATION_INTERVAL  5
#endif


//==========================================================
// <o> SAADC_CONFIG_RESOLUTION  - Resolution

// <0=> 8 bit
// <1=> 10 bit
// <2=> 12 bit
// <3=> 14 bit

#ifndef APP_SAADC_CONFIG_RESOLUTION
    #define APP_SAADC_CONFIG_RESOLUTION     2
#endif


//==========================================================
// <o> SAADC_CONFIG_OVERSAMPLE  - Sample period

// <0=> Disabled
// <1=> 2x
// <2=> 4x
// <3=> 8x
// <4=> 16x
// <5=> 32x
// <6=> 64x
// <7=> 128x
// <8=> 256x

#ifndef APP_SAADC_CONFIG_OVERSAMPLE
    #define APP_SAADC_CONFIG_OVERSAMPLE     3
#endif


//==========================================================
// </h>


// SAADC Set in Low Power Mode ...
// Oversampling and BURST mode are disabled, SAMPLES_IN_BUFFER is set to 1
#if defined(APP_SAADC_CONFIG_LP_MODE) && (APP_SAADC_CONFIG_LP_MODE != 0)
    #undef  APP_SAADC_BURST_MODE
    #define APP_SAADC_BURST_MODE            0

    #undef  APP_SAADC_CONFIG_OVERSAMPLE
    #define APP_SAADC_CONFIG_OVERSAMPLE     0
#endif

// Number of samples in channel's buffers
#ifndef APP_SAADC_SAMPLES_IN_BUFFER
    #if   (APP_SAADC_CONFIG_OVERSAMPLE == 0)
        #define APP_SAADC_SAMPLES_IN_BUFFER 1
    #elif (APP_SAADC_CONFIG_OVERSAMPLE == 1)
        #define APP_SAADC_SAMPLES_IN_BUFFER 2
    #elif (APP_SAADC_CONFIG_OVERSAMPLE == 2)
        #define APP_SAADC_SAMPLES_IN_BUFFER 4
    #elif (APP_SAADC_CONFIG_OVERSAMPLE == 3)
        #define APP_SAADC_SAMPLES_IN_BUFFER 8
    #elif (APP_SAADC_CONFIG_OVERSAMPLE == 4)
        #define APP_SAADC_SAMPLES_IN_BUFFER 16
    #elif (APP_SAADC_CONFIG_OVERSAMPLE == 5)
        #define APP_SAADC_SAMPLES_IN_BUFFER 32
    #elif (APP_SAADC_CONFIG_OVERSAMPLE == 6)
        #define APP_SAADC_SAMPLES_IN_BUFFER 64
    #elif (APP_SAADC_CONFIG_OVERSAMPLE == 7)
        #define APP_SAADC_SAMPLES_IN_BUFFER 128
    #elif (APP_SAADC_CONFIG_OVERSAMPLE == 8)
        #define APP_SAADC_SAMPLES_IN_BUFFER 256
    #endif
#endif

#endif // MDW_BUTTON_CONFIG_H
