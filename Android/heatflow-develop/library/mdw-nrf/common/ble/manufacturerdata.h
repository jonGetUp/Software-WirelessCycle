/****************************************************************************************************************//**
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 *
 * \file    manufacturerdata.h
 *
 * \addtogroup BLE
 * \{
 *
 * \addtogroup peripheral
 * \{
 *
 * \class   ble::ManufacturerData
 * \brief   Class handling information about the manufacturer of the device. THoses informations are strongly related
 *          to the BLE advertisement feature.
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
#if defined(SOFTDEVICE_PRESENT)
#include <stdint.h>
#include <boards.h>
//#include <platform-config.h>
#include <firmware-revision.h>
#include <hardware-revision.h>

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
    class ManufacturerData
    {
        
    public:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* CONSTRUCTOR SECTION                                                                                      */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        inline ManufacturerData(uint16_t modelIdentifier        = MODEL_IDENTIFIER, 
                                uint8_t  firmwareMajorRevision  = FIRMWARE_REVISION_MAJOR,
                                uint8_t  firmwareMinorRevision  = FIRMWARE_REVISION_MINOR,
                                uint8_t  hardwareMajorRevision  = HARDWARE_REVISION_MAJOR,
                                uint8_t  hardwareMinorRevision  = HARDWARE_REVISION_MINOR) : 
                _modelIdentifier(modelIdentifier), 
                _firmwareMajorRevision(firmwareMajorRevision), 
                _firmwareMinorRevision(firmwareMinorRevision), 
                _hardwareMajorRevision(hardwareMajorRevision),
                _hardwareMinorRevision(hardwareMinorRevision) {}
                    
        /* ******************************************************************************************************** */
        /*																										    */
        /* PUBLIC DECLARATION SECTION                                                                               */
        /*																										    */
        /* ******************************************************************************************************** */
        /********************************************************************************************************//**
         * \brief   Serialize the content of the class and give back a pointer to the data.
         ************************************************************************************************************/
        inline uint8_t* data()                                          { return reinterpret_cast<uint8_t*>(this); }

        /********************************************************************************************************//**
         * \brief   Retrieves the Model ID of the device.
         ************************************************************************************************************/
        inline uint16_t modelIdentifier() const                         { return _modelIdentifier; }

        /********************************************************************************************************//**
         * \brief   Set a new Model ID for the device
         ************************************************************************************************************/
        inline void setModelIdentifier(uint16_t modelIdentifier)        { _modelIdentifier = modelIdentifier; }


        /********************************************************************************************************//**
         * \brief   Retrieves the Major Revision Number of the firmware
         ************************************************************************************************************/
        inline uint8_t  firmwareMajorRevision() const                   { return _firmwareMajorRevision; }

        /********************************************************************************************************//**
         * \brief   Retrieves the Minor Revision Number of the firmware
         ************************************************************************************************************/
        inline uint8_t  firmwareMinorRevision() const                   { return _firmwareMinorRevision; }

        /********************************************************************************************************//**
         * \brief   Retrieves the Revision Number of the firmware
         ************************************************************************************************************/
        inline uint16_t firmwareRevision() const                        
        { 
            return (((uint16_t) _firmwareMajorRevision) << 8) + (uint16_t) _firmwareMinorRevision; 
        }

        /********************************************************************************************************//**
         * \brief   Set the Revision Number of the firmware
         ************************************************************************************************************/
        inline void setFirmwareRevision(uint8_t major, uint8_t minor)   
        { 
            _firmwareMajorRevision = major;  
            _firmwareMinorRevision = minor; 
        }


        /********************************************************************************************************//**
         * \brief   Retrieves the Major Revision Number of the hardware
         ************************************************************************************************************/
        inline uint8_t  hardwareMajorRevision() const                   { return _hardwareMajorRevision; }

        /********************************************************************************************************//**
         * \brief   Retrieves the Minor Revision Number of the hardware
         ************************************************************************************************************/
        inline uint8_t  hardwareMinorRevision() const                   { return _hardwareMinorRevision; }

        /********************************************************************************************************//**
         * \brief   Retrieves the Revision Number of the hardware
         ************************************************************************************************************/
        inline uint16_t hardwareRevision() const                        
        { 
            return (((uint16_t) _hardwareMajorRevision) <<8) + (uint16_t) _hardwareMinorRevision; 
        }

        /********************************************************************************************************//**
         * \brief   Set the Revision Number of the hardware
         ************************************************************************************************************/
        inline void setHardwareRevision(uint8_t major, uint8_t minor) 
        {
            _hardwareMajorRevision = major;
            _hardwareMinorRevision = minor; 
        }

        /********************************************************************************************************//**
         * \brief   Some Company Identifier obtained by the Bluetooth SIG
         ************************************************************************************************************/
        static const uint16_t SOPROD_COMPANY_ID = 0x0512;
        static const uint16_t HEI_COMPANY_ID    = 0x025A;
        static const uint16_t NORDIC_COMPANY_ID = 0x0059;
       
    private:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PRIVATE DECLARATION SECTION                                                                              */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        // Attributes
        int16_t     _modelIdentifier;
        int8_t      _firmwareMajorRevision;
        int8_t      _firmwareMinorRevision;
        int8_t      _hardwareMajorRevision;
        int8_t      _hardwareMinorRevision;
    };

}    // namespace ble

#endif // #if defined(SOFTDEVICE_PRESENT) 

/** \} */   // Group: Peripheral Controller

/** \} */   // Group: BLE
