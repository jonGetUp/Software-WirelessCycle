/****************************************************************************************************************//**
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 *
 * \file    scheduler.h
 *
 * \addtogroup BLE
 * \{
 *
 * \class   ble::Scheduler
 * \brief   SoftDevice App Scheduler wrapper
 *
 * The scheduler is used for transferring execution from the interrupt  context to the main context. You need to set
 * the USE_SD_APP_SCHEDULER define to 1 in ble-config.h header file to enable the scheduler.
 *
 * See
 * <a href="https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.sdk52.v0.9.1%2Fgroup__app__scheduler.html">SoftDevice app scheduler</a>
 * for more details.
 *
 * Initial author: Patrice Rudaz
 * Creation date: 2017-05-16
 *
 * \author  Patrice Rudaz (patrice.rudaz@hevs.ch)
 * \date    July 2018
 ********************************************************************************************************************/
#pragma once

/* **************************************************************************************************************** */
/*                                                                                                                  */
/* Header files                                                                                                     */
/*                                                                                                                  */
/* **************************************************************************************************************** */
#include <platform-config.h>

#if defined(SOFTDEVICE_PRESENT)

#include <ble.h>
#include <app_timer.h>


/* **************************************************************************************************************** */
/*                                                                                                                  */
/* NAMESPACE Declaration                                                                                            */
/*                                                                                                                  */
/* **************************************************************************************************************** */
namespace ble 
{

    /* ************************************************************************************************************ */
    /*                                                                                                              */
    /* Class Declaration                                                                                            */
    /*                                                                                                              */
    /* ************************************************************************************************************ */
    class Scheduler
    {
    public:
        /* ******************************************************************************************************** */
        /*																										    */
        /* PUBLIC DECLARATION SECTION                                                                               */
        /*																										    */
        /* ******************************************************************************************************** */

        /********************************************************************************************************//**
         *  \brief  Initialization of the queue of the `app_scheduler` modulde.
         ************************************************************************************************************/
        static void init();

        /********************************************************************************************************//**
         *  \brief  Should be called regularly in main loop
         *
         *  \note   Method is non-blocking.
         ************************************************************************************************************/
        static void schedule();
    };
    
}   // namespace ble

#endif  // #if defined(SOFTDEVICE_PRESENT)

/** \} */
