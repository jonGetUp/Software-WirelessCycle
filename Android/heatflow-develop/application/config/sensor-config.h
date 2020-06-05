/* Copyright (c) 2018, HES-SO Valais
 * All rights reserved.
 *
 * \author  Patrice Rudaz (patrice.rudazhevs.ch)
 * \date    March 2019
 */
#ifndef SENSOR_CONFIG_H
#define SENSOR_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>\n
#ifdef USE_APP_CONFIG
#include "app_config.h"
#endif


//==========================================================
// <h> Flash Controller Configuration Tool 

//==========================================================
// <q> SENSOR_CONTROLLER_ASYNC_MODE  - Asynchronous mode enabled/disabled
// ---------------------------------------------------------
// <i> In asynchronous mode, between each mesure the CPU goes back in wait for an event mode


#ifndef SENSOR_CONTROLLER_ASYNC_MODE
    #define SENSOR_CONTROLLER_ASYNC_MODE    1
#endif


//==========================================================
// <q> SENSOR_CONTROLLER_HANDLES_LED  - If `true`, the controller blinks a LED according to the state of its state-machine


#ifndef SENSOR_CONTROLLER_HANDLES_LED
    #define SENSOR_CONTROLLER_HANDLES_LED   0
#endif


//==========================================================
// <q> SENSOR_CONTROLLER_TEST_MODE  - Enable or disable the test mode of the Sensor Controller


#ifndef SENSOR_CONTROLLER_TEST_MODE
#define SENSOR_CONTROLLER_TEST_MODE         0
#endif


//==========================================================
// </h>

#endif // SENSOR_CONFIG_H
