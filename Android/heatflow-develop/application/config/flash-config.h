/* Copyright (c) 2018, HES-SO Valais
 * All rights reserved.
 *
 * \author  Patrice Rudaz (patrice.rudazhevs.ch)
 * \date    April 2018
 */
#ifndef FLASH_CONFIG_H
#define FLASH_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>\n
#ifdef USE_APP_CONFIG
#include "app_config.h"
#endif


//==========================================================
// <h> Flash Controller Configuration Tool 

//==========================================================
// <o> Number max of dirty records.
// ---------------------------------------------------------
// <i> If this amount of dirty records is reached, a Garbage Collection should be run


#ifndef FLASH_MAX_NB_DIRTY_RECORDS
    #define FLASH_MAX_NB_DIRTY_RECORDS      4
#endif


//==========================================================
// <o> Number max of persister.
// ---------------------------------------------------------
// <i> Maximum authorized amount of persister in the application.


#ifndef FLASH_MAX_PERSISTER
    #define FLASH_MAX_PERSISTER             12
#endif


//==========================================================
// <q> FLASH_CONTROLLER_TEST_MODE  - Enable or disable the test mode of the Flash Controller


#ifndef FLASH_CONTROLLER_TEST_MODE
#define FLASH_CONTROLLER_TEST_MODE          0
#endif


//==========================================================
// </h>

#endif // FLASH_CONFIG_H
