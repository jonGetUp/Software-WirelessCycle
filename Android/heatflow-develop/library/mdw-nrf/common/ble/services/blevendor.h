/****************************************************************************************************************//**
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz (patrice.rudaz@hevs.ch)
 * All rights reserved.
 *
 * \file    bleVendor.h
 *
 * \addtogroup BLE
 * \{
 *
 * \addtogroup prSrv Private Services
 * \{
 *
 * \class   ble::BleVendor
 * \brief   Class handling the registration of private services, it handles also the vendor ID.
 *
 * Initial author: Patrice Rudaz
 * Creation date: 2018-01-31
 *
 * \author  Patrice Rudaz
 * \date    July 2018
 ********************************************************************************************************************/
#pragma once

/* **************************************************************************************************************** */
/*                                                                                                                  */
/* Header files                                                                                                     */
/*                                                                                                                  */
/* **************************************************************************************************************** */
#if defined(SOFTDEVICE_PRESENT)
#include <ble-services-config.h>
#include "ble/services/privateservice.h"

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
    /* **************************************************************************************************************/
    class BleVendor
    {
    public:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* CONSTRUCTOR SECTION                                                                                      */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        BleVendor();
        virtual ~BleVendor();

        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PUBLIC DECLARATION SECTION                                                                               */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        /********************************************************************************************************//**
         * \brief  Register a new Vendor Specific base UUID.
         *
         * The usage of a BLE private service needs a vendor ID. This method MUST be used to register once a new 
         * vendor. A vendor can only be registered one time.
         *
         * This call enables the application to add a Vendor Specific base UUID to the BLE stack's table, for later
         * use with all other modules and APIs. This then allows the application to use the shorter, 24-bit 
         * \ref ble_uuid_t format when dealing with both 16-bit and 128-bit UUIDs without having to check for lengths 
         * and having split code paths. This is accomplished by extending the grouping mechanism that the Bluetooth 
         * SIG standard base UUID uses for all other 128-bit UUIDs. The type field in the \ref ble_uuid_t structure
         * is an index (relative to \ref BLE_UUID_TYPE_VENDOR_BEGIN) to the table populated by multiple calls to this 
         * function, and the UUID field in the same structure contains the 2 bytes at indexes 12 and 13. The number 
         * of possible 128-bit UUIDs available to the application is therefore the number of Vendor Specific UUIDs 
         * added with the help of this function times 65536, although restricted to modifying bytes 12 and 13 for 
         * each of the entries in the supplied array.
         *
         * Bytes 12 and 13 of the provided UUID will not be used, since those are always replaced by the 16-bit uuid 
         * of the service.
         *
         * \param   serviceUUIDs    UUID of the private service for the vendor to register
         * \param   ownerMask       Identifier of the vendor to register
         ************************************************************************************************************/
        void registerVendor(ble_uuid128_t* serviceUUIDs, uint16_t ownerMask);

        /********************************************************************************************************//**
         * \brief  Returns the index related to the specific vendor.
         ************************************************************************************************************/
        inline uint8_t uuidType()                   { return _uuidType; }

        /********************************************************************************************************//**
         * \brief  Returns `true` if the specific vendor is already registered and `false` otherwise.
         ************************************************************************************************************/
        inline bool isVendorRegistered() const      { return _isVendorRegistered; }

    private:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PRIVATE DECLARATION SECTION                                                                              */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        // Attributes
        uint8_t     _uuidType;
        bool        _isVendorRegistered;

    };  // class BleVendor

}   // namespace ble

#endif /* SOFTDEVICE_PRESENT    */

/** \} */   // Group: Private Services

/** \} */   // Group: BLE
