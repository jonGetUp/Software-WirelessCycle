/****************************************************************************************************************//**
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 *
 * \file    bleobserver.h
 *
 * \addtogroup BLE
 * \{
 *
 * \class   ble::BleObserver
 * \brief   Abstract class defining the generic Observer Interface dedicated to the BLE Controller...
 *
 * Initial author: Patrice Rudaz
 * Creation date: 2016-08-24
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

#include <ble.h>
#include <ble_types.h>


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
    class BleObserver
    {
    public:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PUBLIC DECLARATION SECTION                                                                               */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        /********************************************************************************************************//**
         * \brief   BLE event handler notification
         *
         * This function is called from the BLE Stack event interrupt handler after a BLE event has been received.
         * If the scheduler is used, this method is called from the main context. If not, is it called from the ISR 
         * context.
         *
         * \param   pBleEvent   The Bluetooth stack advertising event
         * \param   pContext    Pointer to any context...
         ************************************************************************************************************/
        virtual void onBleEvent(ble_evt_t const* pBleEvent, void* pContext) = 0;

        /********************************************************************************************************//**
         * \brief   SoftDevice system event handler notification.
         *
         * This function is called from the SD Stack event interrupt handler after a system event has been received.
         * If the scheduler is used, this method is called from the main context. If not, is it called from the ISR
         * context.
         *
         * \param   systemEvent The Bluetooth stack advertising event
         ************************************************************************************************************/
        virtual void onSystemEvent(uint32_t systemEvent) = 0;
    };

}   // namespace ble

/** \} */
