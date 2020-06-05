/****************************************************************************************************************//**
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2018
 * All rights reserved.
 *
 * \file    batteryobserver.h
 *
 * \addtogroup Battery
 * \{
 *
 * \class   BatteryObserver
 * \brief   Interface definition dedicated to the Battery...
 *
 * Initial author: Cyril Praz
 * Creation date: 2016-12-27
 *
 * \author  Patrice Rudaz (patrice.rudaz@hevs.ch)
 * \version 2.0.0
 * \date    February 2018
 ********************************************************************************************************************/
#pragma once

/* **************************************************************************************************************** */
/*                                                                                                                  */
/* Header files                                                                                                     */
/*                                                                                                                  */
/* **************************************************************************************************************** */
#include "battery/batterystate.h"
#include <stdint.h>


/* **************************************************************************************************************** */
/*                                                                                                                  */
/* NAMESPACE Declaration                                                                                            */
/*                                                                                                                  */
/* **************************************************************************************************************** */
namespace battery 
{

    /* ************************************************************************************************************ */
    /*                                                                                                              */
    /* Class Declaration                                                                                            */
    /*                                                                                                              */
    /* ************************************************************************************************************ */
    class BatteryManager;

    class BatteryObserver
    {
    public:
        /********************************************************************************************************** */
        /*                                                                                                          */
        /* PUBLIC DECLARATION SECTION                                                                               */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        /********************************************************************************************************//**
         * \name    BatteryObservers Battery's Observers
         * \{
         ************************************************************************************************************/
        // ----------------------------------------------------------------------------------------------------------
        /********************************************************************************************************//**
         * \brief   Battery state Callback's method
         *
         * Method used to notity any observer that the battery's state has changed...
         *
         * \param batteryManager    Pointer on the battery manager
         * \param state             The new state the battery just goes into...
         ************************************************************************************************************/
        virtual void onBatteryStateChanged(BatteryManager* batteryManager, 
                                           BatteryState::eBatteryState state = BatteryState::BATT_STATE_UNKNOWN) = 0;
        
        // ----------------------------------------------------------------------------------------------------------
        /********************************************************************************************************//**
         * \brief   Battery level Callback's method
         *
         * Method used to notity any observer that the battery's level (in percent) has changed...
         *
         * \param batteryManager    Pointer on the battery manager
         * \param state             The new level of the battery
         ************************************************************************************************************/
        virtual void onBatteryLevelChanged(BatteryManager* batteryManager, uint8_t batteryLevel) = 0;
        /** \} */
    };

}   // namespace battery

/** \} */   // Group: Battery
