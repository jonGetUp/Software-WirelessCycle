/****************************************************************************************************************//**
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 *
 * \file    bleservicesobserver.h
 *
 * \addtogroup BLE
 * \{
 *
 * \class   ble::BleServicesObserver
 * \brief   Abstract class defining the generic Observer Interface dedicated to  BLE Adopted Services...
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
#include <ble-services-config.h>

/* **************************************************************************************************************** */
/*                                                                                                                  */
/* APPLICATION defintion                                                                                            */
/*                                                                                                                  */
/* **************************************************************************************************************** */

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
    class BleServicesObserver
    {
    public:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PUBLIC DECLARATION SECTION                                                                               */
        /*                                                                                                          */
        /* ******************************************************************************************************** */

    #if (IAS_SUPPORT != 0)
        /********************************************************************************************************//**
         * \brief   BLE advertising event handler notification
         *
         * This function is called from the BLE Stack event interrupt handler after a BLE advertising event has been
         * received. If the scheduler is used, this method is called from the main context. If not, is it called from
         * the ISR context.
         *
         * \param   ble_adv_evt The Bluetooth stack advertising event
         ************************************************************************************************************/
        virtual void onImmediateAlertChanged(uint8_t alertLevel) = 0;
    #endif
    };

}   // namespace ble

/** \} */
