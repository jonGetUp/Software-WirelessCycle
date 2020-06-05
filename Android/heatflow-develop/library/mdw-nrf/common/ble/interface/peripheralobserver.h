/****************************************************************************************************************//**
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 *
 * \file    peripheralobserver.h
 *
 * \addtogroup BLE
 * \{
 *
 * \class   ble::PeripheralObserver
 * \brief   Abstract class defining the generic Observer Interface dedicated to BLE Peripheral...
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
#include <ble_advertising.h>

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
    class PeripheralController;
    class PeripheralObserver
    {
    public:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PUBLIC DECLARATION SECTION                                                                               */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        /********************************************************************************************************//**
         * \name    AdvertiseEvtHandler Advertising's states handler
         * \brief   BLE advertising event handler notification
         * \{
         ************************************************************************************************************/
        // ----------------------------------------------------------------------------------------------------------
        /********************************************************************************************************//**
         * \brief   Called when the advertising stops.
         ************************************************************************************************************/
        virtual void onAdvertiseStopped(PeripheralController* peripheral) = 0;

        // ----------------------------------------------------------------------------------------------------------
        /********************************************************************************************************//**
         * \brief   Called when the advertising's state has changed
         *
         * This function is called from the BLE Stack event interrupt handler after a BLE advertising event has been
         * received. If the scheduler is used, this method is called from the main context. If not, is it called from
         * the ISR context.
         *
         * \param   ble_adv_evt The Bluetooth stack advertising event
         ************************************************************************************************************/
        virtual void onAdvertiseStateChanged(PeripheralController* peripheral, ble_adv_evt_t bleAdvEvent) = 0;

        // ----------------------------------------------------------------------------------------------------------
        /********************************************************************************************************//**
         * \brief   Called when an error occurs during advertising.
         *
         * \param   nrfError    Error code identifying what has 
         ************************************************************************************************************/
        virtual void onAdvertiseError(PeripheralController* peripheral, uint32_t nrfError) = 0;
        /** \} */

        /********************************************************************************************************//**
         * \name    GAPConnParamUpdate GAP Connection Parameter Update Observers
         * \brief   GAP Connection Parameter Callback's
         * \{
         ************************************************************************************************************/
        /********************************************************************************************************//**
         * \brief   BLE stack event handler notification
         *
         * This function is called from the BLE Stack event interrupt handler after a BLE stack event has been received.
         * If the scheduler is used, this method is called from the main context. If not, is it called from the ISR
         * context.
         *
         * \param   ble_adv_evt The Bluetooth stack advertising event
         ************************************************************************************************************/
        // -------------------------------------------------------------------------------------------------------
        virtual void onPeripheralConnected(PeripheralController* peripheral) = 0;

        // -------------------------------------------------------------------------------------------------------
        virtual void onPeripheralDisconnected(PeripheralController* peripheral) = 0;

        // -------------------------------------------------------------------------------------------------------
        virtual void onPeripheralStateChanged(PeripheralController* peripheral, uint8_t state) = 0;
        
        // -------------------------------------------------------------------------------------------------------
        virtual void onPasskeyDisplay(PeripheralController* peripheral, char* passkey) = 0;
        
        // -------------------------------------------------------------------------------------------------------
        virtual void onAuthStatusUpdated(PeripheralController* peripheral, ble_gap_evt_auth_status_t status) = 0;
        /** \} */


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
        // -------------------------------------------------------------------------------------------------------
        virtual void onPeripheralImmediateAlertChanged(PeripheralController* peripheral, uint8_t alertLevel) = 0;
    #endif
    };

}  // namespace ble

/** \} */
