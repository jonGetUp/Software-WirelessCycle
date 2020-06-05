/****************************************************************************************************************//**
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 *
 * \file    blecontroller.h
 *
 * \addtogroup BLE
 * \{
 *
 * \class   ble::BleController
 * \brief   Class handling BLE stack and its features provided by the Nordic SoftDevice (SDK v15.0.0)
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
#include <stdint.h>
#include <ble-config.h>
#include <core/isubject.hpp>

#include "interface/bleobserver.h"

#include <ble.h>
#include <ble_gap.h>

#if defined(__cplusplus)
extern "C" 
{
#endif
    namespace ble 
    {
        /********************************************************************************************************//**
         * \brief   Function for dispatching a BLE stack event to all modules with a BLE stack event handler.
         ************************************************************************************************************/
        void bleEventHandler(ble_evt_t const * pBleEvt, void* pContext);
        
        /********************************************************************************************************//**
         * \brief   Function for dispatching SoftDevice SoC events to all modules.
         ************************************************************************************************************/
        void sysEventHandler(uint32_t sysEvt, void* pContext);
    }
#if defined(__cplusplus)
}
#endif

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
class Factory;

namespace ble 
{

    /* ************************************************************************************************************ */
    /*                                                                                                              */
    /* Class Declaration                                                                                            */
    /*                                                                                                              */
    /* ************************************************************************************************************ */
    class BleController : public ISubject<BleObserver>
    {
        friend class ::Factory;
        friend void bleEventHandler(ble_evt_t* pBleEvt, void* pContext);
        friend void sysEventHandler(uint32_t sysEvt, void* pContext);

    public:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* CONSTRUCTOR SECTION                                                                                      */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        virtual ~BleController();

        /* ******************************************************************************************************** */
        /*																										    */
        /* PUBLIC DECLARATION SECTION                                                                               */
        /*																										    */
        /* ******************************************************************************************************** */

        /********************************************************************************************************//**
         * \brief   Initialize the SoftDevice.
         ************************************************************************************************************/
        void initialize();

        /********************************************************************************************************//**
         * \brief   Read the Bluetooth Address of the device.
         *
         * Read the BT-Address of the device and retrieves the number of read bytes.
         *
         * \param[out]  address Pointer to a bluetooth address defined as a `ble_gap_addr_t` structure
         *
         * \return      The size of the BT address.
         ************************************************************************************************************/
        uint8_t readBtAddress(ble_gap_addr_t* address);

        /********************************************************************************************************//**
         * \brief   Notify all registered BleObservers on BLE event coming from the SoftDevice
         ************************************************************************************************************/
        void notifyObserversOnBleEvent(ble_evt_t const * pBleEvt);

        /********************************************************************************************************//**
         * \brief   Notify all registered BleObservers on System event coming from the SoftDevice
         ************************************************************************************************************/
        void notifyObserversOnSystemEvent(uint32_t sysEvt);

    private:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PRIVATE DECLARATION SECTION                                                                              */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        // Attributes
    
    
    
        // Methods

        // MUST be a Singleton
        BleController();

        uint32_t _bleStackInit();
    };

}   // namespace ble

/** \} */
