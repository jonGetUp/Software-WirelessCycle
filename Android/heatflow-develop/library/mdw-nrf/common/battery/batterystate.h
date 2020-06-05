/****************************************************************************************************************//**
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Cyril Praz, Patrice Rudaz
 * All rights reserved.
 *
 * \file    batterystate.h
 *
 * \addtogroup Battery
 * \{
 *
 * \class   BatteryState
 * \brief   Class to manage the different state available during the life time of a battery
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
#include <string.h>
#include <stdint.h>

/* **************************************************************************************************************** */
/*                                                                                                                  */
/* APPLICATION definition                                                                                           */
/*                                                                                                                  */
/* **************************************************************************************************************** */

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
    class BatteryState
    {
    public:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PUBLIC DECLARATION SECTION                                                                               */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        /********************************************************************************************************//**
         * \brief   Battery State enumeration
         ************************************************************************************************************/
        typedef enum
        {
            BATT_STATE_UNKNOWN = 0,
            BATT_STATE_OK,                          ///< \brief The battery is functional
            BATT_STATE_LOW_BAT,                     ///< \brief If the system doesn't use the EOL modes, this state indicates that the battery is empty.
            BATT_STATE_EOL_FIRST,                   ///< \brief First state of EOL mode:  Some heavy consumer should be only use when needed
            BATT_STATE_EOL_SECOND,                  ///< \brief Second state of EOL mode: all energy consumers should be only use when needed
            BATT_STATE_EOL_THIRD,                   ///< \brief Third and last state of EOL mode: Heavy energy consumer should be shut off.
            BATT_STATE_SHIPPING_MODE_CHARGED,       ///< \brief The battery is in shipping mode and charged
            BATT_STATE_SHIPPING_MODE_DISCHARGED,    ///< \brief The battery is in shipping mode but discharged
            BATT_STATE_CHARGING
        } eBatteryState;
        
        /********************************************************************************************************//**
         * \brief   Returns the eBatteryState matching the given state value
         *
         * \param   state   The charger's state to find a match.
         ************************************************************************************************************/
        static eBatteryState toBatteryState(uint8_t state)
        {
            switch(state)
            {
                case 1:     return BATT_STATE_OK;
                case 2:     return BATT_STATE_LOW_BAT;
                case 3:     return BATT_STATE_EOL_FIRST;
                case 4:     return BATT_STATE_EOL_SECOND;
                case 5:     return BATT_STATE_EOL_THIRD;
                case 6:     return BATT_STATE_SHIPPING_MODE_CHARGED;
                case 7:     return BATT_STATE_SHIPPING_MODE_DISCHARGED;
                case 8:     return BATT_STATE_CHARGING;
                default:    return BATT_STATE_UNKNOWN;
            }
        }
        
        /********************************************************************************************************//**
         * \brief   Returns the string which describes the given state.
         *
         * \param   state    The given state to translate in string.
         ************************************************************************************************************/
        static const char* toString(eBatteryState state)
        {
            switch(state)
            {
                case BATT_STATE_OK:                         return "BATT_STATE_OK";
                case BATT_STATE_LOW_BAT:                    return "BATT_STATE_LOW_BAT";
                case BATT_STATE_EOL_FIRST:                  return "BATT_STATE_EOL_FIRST";
                case BATT_STATE_EOL_SECOND:                 return "BATT_STATE_EOL_SECOND";
                case BATT_STATE_EOL_THIRD:                  return "BATT_STATE_EOL_THIRD";
                case BATT_STATE_CHARGING:                   return "BATT_STATE_CHARGING";
                case BATT_STATE_SHIPPING_MODE_CHARGED:      return "BATT_STATE_SHIPPING_MODE_CHARGED";
                case BATT_STATE_SHIPPING_MODE_DISCHARGED:   return "BATT_STATE_SHIPPING_MODE_DISCHARGED";
                default:                                    return "BATT_STATE_UNKNOWN";
            }
        }
    };

} // namespace battery

/** \} */   // Group: Battery
