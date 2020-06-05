/****************************************************************************************************************//**
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 *
 * \file    sdktools.h
 *
 * \addtogroup Tools
 * \{
 *
 * \class   tools::SDKTools
 * \brief   Class agregating a couple of tool reéated to the SDK libraries and drivers provided by Nordic.
 *
 * Initial author: Patrice Rudaz
 * Creation date: 2016-06-16
 *
 * \author  Patrice RUDAZ
 * \date    July 2018
 ********************************************************************************************************************/
#pragma once

/* **************************************************************************************************************** */
/*                                                                                                                  */
/* Header files                                                                                                     */
/*                                                                                                                  */
/* **************************************************************************************************************** */
#include <stdlib.h> 
#include <stdint.h>


/* **************************************************************************************************************** */
/*                                                                                                                  */
/* TYPE definition                                                                                                  */
/*                                                                                                                  */
/* **************************************************************************************************************** */
typedef union
{
    int     i;
    float   f;
} intFloatUnion_t;

/* **************************************************************************************************************** */
/*                                                                                                                  */
/* NAMESPACE Declaration                                                                                            */
/*                                                                                                                  */
/* **************************************************************************************************************** */
namespace tools
{

    /* ************************************************************************************************************ */
    /*                                                                                                              */
    /* Class Declaration                                                                                            */
    /*                                                                                                              */
    /* ************************************************************************************************************ */
    class SDKTools
    {
    protected:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PROTECTED DECLARATION SECTION                                                                            */
        /* CONSTRUCTOR SECTION                                                                                      */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        SDKTools() {}
        ~SDKTools() {}

    public:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PUBLIC DECLARATION SECTION                                                                               */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        /********************************************************************************************************//**
         * \brief   Initialization the app_timer module
         *
         * \param   Type of function for passing events from the timer module to the scheduler.
         ************************************************************************************************************/
        static void init();

        /********************************************************************************************************//**
         * \brief   Returns the radio's transmit power.
         *
         * This function will return the current radio's transmit power.
         * Supported txPower values: -40dBm, -20dBm, -16dBm, -12dBm, -8dBm, -4dBm, 0dBm, +3dBm and +4dBm.
         *
         * \return  The radio's trsnmit power as an `int8_t`.
         ************************************************************************************************************/
        static int8_t getTxPower(uint8_t role);

        /********************************************************************************************************//**
         * \brief   Adjust the power of the radio hardware to the given value.
         *
         * This function will change the power used by the radio emitter to match the given value. It returns `false` 
         * if the given value is not valid or if the radio harware could not adjust the power, `true` otherwise.
         *
         * Supported txPower values: -40dBm, -20dBm, -16dBm, -12dBm, -8dBm, -4dBm, 0dBm, +3dBm and +4dBm.
         *
         * \param   role    Defines which role the TX power will be updated for. Allowed roRoles are
         *                  BLE_GAP_TX_POWER_ROLE_ADV, BLE_GAP_TX_POWER_ROLE_CON & BLE_GAP_TX_POWER_ROLE_SCAN_INIT
         * \param   handle  Depends on the choosen role and can be the connection's hanle, the advertising handle...
         * \param   txPower The value to set the radio transmit power !
         *
         * \return  `true` if the TX power could be updated or `false` otherwise.
         ************************************************************************************************************/
        static bool setTxPower(uint8_t role, uint16_t handle, int8_t txPower);

        /********************************************************************************************************//**
         * \brief   Set the device's name used in advertisement.
         *
         * This function will set the given data as the device's name. It returns `true` if it could change the name
         * successfully and `false` otherwise.
         *
         * \return  `true` if the name could be set successfully and `false` otherwise.
         ************************************************************************************************************/
        static bool setDeviceName(uint8_t* name, size_t nameLength);

        /********************************************************************************************************//**
         * \brief   Process the conversion from \c float to IEEE-11073 FLOAT (32 bits) or IEEE-11073 SFLOAT (16 bits).
         *
         * This function will convert any `float` value to IEEE-11073 32bit FLOAT (16-bit SFLOAT) and retrieve it as 
         * an array of `uint8_t` to be able to sent it through BLE.
         ************************************************************************************************************/
        static uint8_t floatToIEEE11073Float(float value, uint8_t precision, uint8_t* encodedValue);

        /********************************************************************************************************//**
         * \brief   Process the conversion from IEEE-11073 32bits FLOAT or IEEE-11073 16bits SFLOAT to a float value.
         *
         * This function will convert any IEEE-11073 32bits FLOAT (16bits SFLOAT) value to `float` value and retrieve
         * it.
         ************************************************************************************************************/
        static float IEEE11073ToFloat(uint8_t* ieee_float);

        /********************************************************************************************************//**
         * \brief   Process the conversion between float to IEEE-754 32bits float saved as an array of uint8_t.
         *
         * This function will convert any `float` value to IEEE-11073 32bit FLOAT and retrieve it as an array of 4
         * `uint8_t` to be able to sent it through BLE.
         ************************************************************************************************************/
        static uint8_t floatToUnsignedByteArray(float value, uint8_t* encodedValue);

        /********************************************************************************************************//**
         * \brief   Process the conversion from an array of uint8_t value to IEEE-754 32bits float
         *          value.
         *
         * This function will convert any array of `uint8_t` to an IEEE-754 32bits float value and retrieve it.
         *
         * \return  Return the given value as an IEEE-754 32bits float
         ************************************************************************************************************/
        static float unsignedByteArrayToFloat(uint8_t* encodedValue);
        
        static float unsignedInt16ToFloat(uint16_t value);

        /********************************************************************************************************//**
         * \brief   Compute the Greatest Common Divisor of two numbers
         *
         * \param   a   First number
         * \param   b   Second number
         ************************************************************************************************************/
        static uint16_t GCD(uint16_t a, uint16_t b);

        /********************************************************************************************************//**
         * \brief   Compute an division and rounded the value to the next unsigned integer value.
         *
         * \param   a   First number
         * \param   b   Second number
         ************************************************************************************************************/
        static uint32_t roundedDiv(uint64_t a, uint64_t b);

        /********************************************************************************************************//**
         * \brief   Compute an division and rounded the value to the next unsigned 64 bit integer value.
         *
         * \param   a   First number
         * \param   b   Second number
         ************************************************************************************************************/
        static uint64_t roundedDiv64(uint64_t a, uint64_t b);

        /********************************************************************************************************//**
         * \brief   Compute an division and rounded the value to the higher unsigned integer value.
         *
         * \param   a   First number
         * \param   b   Second number
         ************************************************************************************************************/
        static uint32_t ceiledDiv(uint64_t a, uint64_t b);

        /********************************************************************************************************//**
         * \brief   Compute an division and rounded the value to the higher unsigned integer value (64bits).
         *
         * \param   a   First number
         * \param   b   Second number
         ************************************************************************************************************/
        static uint64_t ceiledDiv64(uint64_t a, uint64_t b);
        
        /********************************************************************************************************//**
         * \brief   Checks if the given pointer has a valid address. 
         *
         * \return  `true` if the pointed address is invalid and `false` otherwise.
         ************************************************************************************************************/
        static bool isObjectNULL(void* object);
        
        
        static uint32_t appTimerCreate(void const* timerId, uint8_t timerMode, void* callback);
        static uint32_t appTimerStart(void* timerId, uint32_t timeout, void* context);
        static uint32_t appTimerStop(void* timerId);
        static uint32_t appTimerGetCounter();
        
        
    private:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PRIVATE DECLARATION SECTION                                                                              */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        /********************************************************************************************************//**
         * \brief   Iinitialization the app_timer module
         *
         * \param   Type of function for passing events from the timer module to the scheduler.
         *
         * \return  NRF_SUCCESS If no error occurs
         ************************************************************************************************************/
        static uint32_t _appTimerInit();
    
    };  // Class SDKTools

}  // Namepsace tools

/** \} */   // Group: Tools
