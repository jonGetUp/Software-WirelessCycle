/****************************************************************************************************************//**
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 *
 * \file    gatt.h
 *
 * \addtogroup BLE
 * \{
 *
 * \addtogroup peripheral
 * \{
 *
 * \class   ble::Gatt
 * \brief   Class handling BLE GATT features provided by the Nordic SoftDevice (SDK v14.2.0)
 *
 * Class for negotiating and keeping track of GATT connection parameters and updating the data length.
 *
 * Initial author: Patrice Rudaz
 * Creation date: 2018-01-16
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
#include <nrf_ble_gatt.h>

#include <ble-gatt-config.h>
#include "ble/interface/blegattobserver.h"
#include "core/isubject.hpp"


#if defined(__cplusplus)
extern "C" 
{
#endif
    namespace ble 
    {
        /********************************************************************************************************//**
         * \brief   Function for dispatching a BLE GATT event.
         ************************************************************************************************************/
        void onGattEvent(nrf_ble_gatt_t* pGatt, nrf_ble_gatt_evt_t const* pEvent);
        
    };  // namespace ble
#if defined(__cplusplus)
}
#endif


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

    class Gatt : public ISubject<BleGattObserver>
    {
    public:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* CONSTRUCTOR SECTION                                                                                      */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        Gatt();
        virtual ~Gatt();

        /* ******************************************************************************************************** */
        /*																										    */
        /* PUBLIC DECLARATION SECTION                                                                               */
        /*																										    */
        /* ******************************************************************************************************** */

        /********************************************************************************************************//**
         * \brief   Initialize the GATT module.
         ************************************************************************************************************/
        void initialize();
        
        /********************************************************************************************************//**
         * \brief   Notify all registered BleGattObservers on GATT event coming from the SoftDevice
         ************************************************************************************************************/
        void gattEventHandler(nrf_ble_gatt_t* pGatt, nrf_ble_gatt_evt_t const* pEvent);


    private:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PRIVATE DECLARATION SECTION                                                                              */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        // Attributes
        nrf_ble_gatt_t _gatt;

        // Methods
        void _notifyObserverOnMtuUpdated(uint16_t connHandle, uint16_t mtuEffective);
        void _notifyObserverOnDataLengthUpdated(uint16_t connHandle, uint8_t dataLength);
    
    };  // Class Gatt

}   // namespace ble

#endif  // #if defined(SOFTDEVICE_PRESENT)

/** \} */   // Group: Peripheral Controller

/** \} */   // Group: BLE
