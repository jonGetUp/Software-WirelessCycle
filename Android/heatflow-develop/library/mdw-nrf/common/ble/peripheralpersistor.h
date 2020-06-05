/****************************************************************************************************************//**
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 *
 * \file    peripheralpersistor.h
 *
 * \addtogroup BLE
 * \{
 *
 * \addtogroup peripheral
 * \{
 *
 * \class   ble::PeripheralPersistor
 * \brief   Class that handles peripheral's parameters to be saved and restore before and after DFU...
 *
 * Initial author: Patrice Rudaz
 * Creation date: 2017-05-17
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
#include <ble-advertiser-config.h>
#include <nordic_common.h>
#include <platform-config.h>
#include <stdint.h>


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
    class PeripheralPersistor
    {
    public:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* CONSTRUCTOR SECTION                                                                                      */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        inline PeripheralPersistor() : 
            _size(0), 
            _useJustWorks(USE_JUST_WORKS_BONDING), 
            _useMitm(USE_MITM_BONDING), 
            _useWhiteList(APP_ADV_WHITELIST_ENABLE),
            _reserved(0) {};

        /* ******************************************************************************************************** */
        /*																										    */
        /* PUBLIC DECLARATION SECTION                                                                               */
        /*																										    */
        /* ******************************************************************************************************** */
        /********************************************************************************************************//**
         * \brief   Serialisation of the class
         ************************************************************************************************************/
        inline uint32_t* data()                         { return reinterpret_cast<uint32_t*>(this); }

        /********************************************************************************************************//**
         * \brief   Gives the size of the current name of the device.
         ************************************************************************************************************/
        inline uint16_t nameLength() const              { return _size; }  

        /********************************************************************************************************//**
         * \brief   Returns the name used for advertising
         ************************************************************************************************************/
        inline const char* name()                       { return static_cast<const char*> (_name); }

        /********************************************************************************************************//**
         * \brief   Clear the current name of the watch used for advertising
         ************************************************************************************************************/
        inline void clearName()                         { memset(_name, '\0', DEVICE_NAME_MAX_SIZE); }

        /********************************************************************************************************//**
         * \brief   Set a new name for advertising...
         ************************************************************************************************************/
        inline void setName(char* name, uint16_t size)  
        { 
            clearName(); _size = size; memcpy(_name, name, MIN(size, DEVICE_NAME_MAX_SIZE)); 
        }

        /********************************************************************************************************//**
         * \name   SecuritySettings
         * \brief  Read out the security settings
         * \{
         ************************************************************************************************************/
        inline bool useJustWorks() const                { return _useJustWorks; }
        inline bool useMitm() const                     { return _useMitm; }
        inline bool useWhiteList() const                { return _useWhiteList; }
        /** \} */
        
        /********************************************************************************************************//**
         * \brief   Setup of the security settings
         ************************************************************************************************************/
        inline void setSecuritySettings(bool justWorks, bool mitm, bool whiteList)
        {
            _useJustWorks = justWorks; _useMitm = mitm; _useWhiteList = whiteList;
        }

    private:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PRIVATE DECLARATION SECTION                                                                              */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        // Attributes
        char            _name[DEVICE_NAME_MAX_SIZE];
        uint16_t        _size;

        // Stores the security configuration
        bool            _useJustWorks;
        bool            _useMitm;
        bool            _useWhiteList;

        // NOT USED
        uint8_t         _reserved;
    };

}  // Namespace ble

/** \} */   // Group: Peripheral Controller

/** \} */   // Group: BLE
