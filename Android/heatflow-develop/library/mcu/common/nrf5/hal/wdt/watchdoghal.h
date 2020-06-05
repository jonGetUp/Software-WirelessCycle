/****************************************************************************************************************//**
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 *
 * \file    watchdoghal.h
 *
 * \addtogroup LowHAL
 * @{
 *
 * \class   hal::WatchdogHal
 * \brief   Handles the Hardware WatchDog module
 ********************************************************************************************************************/
#pragma once

/* **************************************************************************************************************** */
/*                                                                                                                  */
/* Header files                                                                                                     */
/*                                                                                                                  */
/* **************************************************************************************************************** */
#include <config/xf-config.h>
#include <sdk_config.h>
#include <platform-config.h>

#if (USE_WATCHDOG != 0)
#include <nrfx_wdt.h>


/* **************************************************************************************************************** */
/*                                                                                                                  */
/* APPLICATION defintion                                                                                            */
/*                                                                                                                  */
/* **************************************************************************************************************** */

/* **************************************************************************************************************** */
/* --- Extern "C" Section ---                                                                                       */
/*     Callback's section                                                                                           */
/* **************************************************************************************************************** */
#if defined(__cplusplus)
extern "C" 
{
#endif
    namespace hal 
    {
        /********************************************************************************************************//**
         * \brief   Function for handling when the watchdog is reached
         ************************************************************************************************************/
        void watchdogHandle(void);
    } // namespace hal
#if defined(__cplusplus)
}
#endif

/* **************************************************************************************************************** */
/*                                                                                                                  */
/* NAMESPACE Declaration                                                                                            */
/*                                                                                                                  */
/* **************************************************************************************************************** */
namespace hal 
{

    /* ************************************************************************************************************ */
    /*                                                                                                              */
    /* Class Declaration                                                                                            */
    /*                                                                                                              */
    /* ************************************************************************************************************ */
    class WatchdogHal
    {
        friend void watchdogHandle();
        
    public:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PUBLIC DECLARATION SECTION                                                                               */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        /********************************************************************************************************//**
         * \brief    Function to initialize the hardware watchdog
         *
         * \param[in] reload  time in millisecond of the watchdog timer delay
         ************************************************************************************************************/
        static void initialize();

        /********************************************************************************************************//**
         * \brief    Function to start the watchdog function
         ************************************************************************************************************/
        static void start();

        /********************************************************************************************************//**
         * \brief    Function to reset the watchdog timer
         *
         * Should be called everytime that something relevant happen to not reset the watch
         ************************************************************************************************************/
        static void feed();

    private:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PRIVATE DECLARATION SECTION                                                                              */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        static nrfx_wdt_config_t   _config;
        static nrfx_wdt_channel_id _channelId;
    };

} // namespace hal

#endif

/** @} */
