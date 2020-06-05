/********************************************************************************************************************
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2018
 * All rights reserved.
 *
 * \file    pwm-config.h
 * \brief   Defines and settings for the PwmHal objects...
 *
 * Initial author: Patrice Rudaz
 * Creation date: 2018-08-02
 *
 * \author  Patrice Rudaz
 * \date    August 2018
 ********************************************************************************************************************/
#pragma once


/* **************************************************************************************************************** */
/*                                                                                                                  */
/* Header files                                                                                                     */
/*                                                                                                                  */
/* **************************************************************************************************************** */

// <<< Use Configuration Wizard in Context Menu >>>\n
#ifdef USE_APP_CONFIG
#include "app_config.h"
#endif


//==========================================================
// <h> PWM HAL defines


//==========================================================
// <o> Maximum frequency allowed [Hz].

#ifndef APP_PWM_MAX_FREQUENCY
    #define APP_PWM_MAX_FREQUENCY           16000000
#endif


//==========================================================
// <o> Maximum of sequences allowed.

#ifndef APP_PWM_MAX_SEQUENCES_NUMBER
    #define APP_PWM_MAX_SEQUENCES_NUMBER    2
#endif


//==========================================================
// </h>

