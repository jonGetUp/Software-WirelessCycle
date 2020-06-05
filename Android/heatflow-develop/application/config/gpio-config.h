/********************************************************************************************************************
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2018
 * All rights reserved.
 *
 * \file    gpio-config.h
 * \brief   Defines and settings for the GPIO objects...
 *
 * \author  Patrice Rudaz (patrice.rudaz@hevs.ch)
 * \date    26.02.2018
 ********************************************************************************************************************/
#ifndef MDW_GPIO_CONFIG_H
#define MDW_GPIO_CONFIG_H

#include <nrf_gpio.h>
#include <boards.h>
#include <math.h>

// <<< Use Configuration Wizard in Context Menu >>>\n
#ifdef USE_APP_CONFIG
#include "app_config.h"
#endif
//==========================================================
// <h> GPIO defines

//==========================================================
// <o> Sense Setup  - GPIO sensing mechanism
// <i> Enumerator used for selecting the pin to sense high or low level on the pin input.
// <i> Avialable values:
// <i> - Disable : GPIO_PIN_CNF_SENSE_Disabled (0UL) (default)
// <i> - High level: GPIO_PIN_CNF_SENSE_High (2UL)
// <i> - Low level: GPIO_PIN_CNF_SENSE_Low (3UL)

// <0=> Disable 
// <1=> High level
// <2=> Low level

#ifndef APP_GPIO_SENSE_SETUP
    #define APP_GPIO_SENSE_SETUP            0
    #if (APP_GPIO_SENSE_SETUP == 0)
        #define GPIO_SENSE_SETUP            NRF_GPIO_PIN_NOSENSE
    #elif (APP_GPIO_SENSE_SETUP == 1)
        #define GPIO_SENSE_SETUP            NRF_GPIO_PIN_SENSE_HIGH
    #else
        #define GPIO_SENSE_SETUP            NRF_GPIO_PIN_SENSE_LOW
    #endif
#endif


//==========================================================
// <o> GPIO pull configuration
// <i> Enumerator used for selecting the pin to be pulled down or up at the time of pin configuration.
// <i> Avaialble setup:
// <i> - No pull: GPIO_PIN_CNF_PULL_Disabled (0UL) (default)
// <i> - Pull down on pin: GPIO_PIN_CNF_PULL_Pulldown (1UL)
// <i> - Pull up on pin: GPIO_PIN_CNF_PULL_Pullup (3UL)

// <0=> No pull
// <1=> Pull down
// <2=> Pull up


#ifndef APP_GPIO_PULL_SETUP
    #define APP_GPIO_PULL_SETUP             2
    #if (APP_GPIO_PULL_SETUP == 0)
        #define GPIO_PULL_SETUP             NRF_GPIO_PIN_NOPULL
    #elif (APP_GPIO_PULL_SETUP == 1)
        #define GPIO_PULL_SETUP             NRF_GPIO_PIN_PULLDOWN
    #else
        #define GPIO_PULL_SETUP             NRF_GPIO_PIN_PULLUP
    #endif
#endif

//==========================================================
// </h>


//==========================================================
// <h> GPIO Controller defines

//==========================================================
// <o> Maximum number of GPIOs used.
// <i> Maximum number of users of the GPIOTE handler.

#ifndef APP_GPIOTE_MAX_USERS
    #define APP_GPIOTE_MAX_USERS            5
    #define NB_MAX_GPIOS                    MAX(APP_GPIOTE_MAX_USERS, BUTTONS_NUMBER)
#endif


//==========================================================
// <o> GPIOs delay in [ms]
// <i> Standard delay used by the GPIO's controller to consider an effective state's change

#ifndef APP_GPIO_MS_DELAY
    #define APP_GPIO_MS_DELAY               250
#endif


//==========================================================
// <o> Timer tick for GPIO debouncer [ms]
// <i> Standard delay used by the GPIO's controller to consider an effective state's change

#ifndef APP_GPIO_HANDLER_MS_TIMER_TICK
    #define APP_GPIO_HANDLER_MS_TIMER_TICK  100
#endif

//==========================================================
// <o> GPIO Debouncer delay [ms]

#ifndef APP_GPIO_DEBOUNCE_MS_DELAY
    #define APP_GPIO_DEBOUNCE_MS_DELAY      150
#endif


//==========================================================
// </h>

//==========================================================
// <e> GPIO_INIT_OUT_AND_IN - GPIOs initializazion process
// <i> If `true`, all GPIOs are, at initialization, set first as clearded output and then as disconnected input.
// <i> If `false`, they are only set as input disconnected


#ifndef GPIO_INIT_OUT_AND_IN
    #define GPIO_INIT_OUT_AND_IN            0
#endif


//==========================================================
// <o> GPIO_INIT_OUT_AND_IN_DELAY - Delay [ms]
// <i> Delay in [ms] to wait before setting all GPIOs as disconnected inputs

#ifndef GPIO_INIT_OUT_AND_IN_DELAY
    #define GPIO_INIT_OUT_AND_IN_DELAY      1000
#endif    

//==========================================================
// <e> GPIO debug on a specific pin
// <i> Enable or disable the DEBUG feature on a specific pin number.

#ifndef APP_DEBUG_GPIO_PIN_XX
    #define APP_DEBUG_GPIO_PIN_XX           0
#endif


//==========================================================
// <o> GPIO debug pin number
// <i> Defines the PIN number de enable debug on... 


#ifndef APP_DEBUG_PIN_NUMBER
    #define APP_DEBUG_PIN_NUMBER            10
#endif

//==========================================================
// </e>

#endif // MDW_GPIO_CONFIG_H
