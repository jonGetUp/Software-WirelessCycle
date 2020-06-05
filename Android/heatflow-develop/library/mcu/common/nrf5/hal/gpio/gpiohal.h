/****************************************************************************************************************//**
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 *
 * \file    gpiohal.h
 *
 * \addtogroup LowHAL
 * @{
 *
 * \class   hal::GpioHal
 * \brief   Class of Hardware Abstraction Layer to handle the different GPIO's of the Nordic nRf5 series chips.
 *
 * Class of Hardware Abstraction Layer to handle the different GPIO's of the nRf5 series chips based on Nordic SDK
 * v15.0.0 and later. This file contains all functions necessary for the use of any GPIO. It starts with the function
 * `gpioInit()` to initialize all GPIO to disable unwanted current consumptions.
 *
 * \author  Patrice Rudaz
 * \date    June 2018
 ********************************************************************************************************************/
#pragma once


/* **************************************************************************************************************** */
/*                                                                                                                  */
/* Header files                                                                                                     */
/*                                                                                                                  */
/* **************************************************************************************************************** */
#include <stdint.h>
#include <nrf_gpio.h>


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
namespace hal 
{

    /* ************************************************************************************************************ */
    /*                                                                                                              */
    /* Class Declaration                                                                                            */
    /*                                                                                                              */
    /* ************************************************************************************************************ */
    class GpioHal
    {
    public:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PUBLIC DECLARATION SECTION                                                                               */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        /********************************************************************************************************//**
         * \brief   Initialiaze all GPIOs
         *
         * Setup all GPIOs to disable unwated current consumptions. The default value at startup is :
         *  - All GPIO as Input
         *  - disconnected
         *  - pulldown or puulup disabled
         *  - SENSE disabled
         ************************************************************************************************************/
        static void init();

        /********************************************************************************************************//**
         * \brief   Configure the PIN as an input
         *
         * This method configures the PIN specified by `pinNbr` as an input and setup its drive's capability and
         * connection's state.
         *
         * \param   pinNbr      Identify the pin to configure
         * \param   drive       Set the current drive capacity (high or low drain capability).
         * \param   pullSetup   Configure the input with an active pullup, pulldown or no pull at all.
         * \param   connect     Specify if the pin is connected or disconnected.
         ************************************************************************************************************/
        static void cfgPinInput(uint32_t pinNbr, uint32_t drive, uint32_t pullSetup, uint32_t connect);

        /********************************************************************************************************//**
         * \brief   Configure the PIN as an input with neither pullup or pulldown, standard drive and connected.
         *
         * This method configures the PIN specified by `pinNbr` as an input and takes `NOPULL`, `S0S1` and `CONNECTED`
         * as default values. It is dedicated to configure GPIO for button's usage.
         *
         * \param   pinNbr      Identify the pin to configure
         ************************************************************************************************************/
        static void cfgPinInputNoPull(uint32_t pinNbr);

        /********************************************************************************************************//**
         * \brief   Configure the PIN as an input using the pullup on chip, the standard drive and connected.
         *
         * This method configures the PIN specified by `pinNbr` as an input and takes `PULLUP`, `S0S1` and `CONNECT`
         * as default values. It is dedicated to configure GPIO for button's usage.
         *
         * \param   pinNbr      Identify the pin to configure
         ************************************************************************************************************/
        static void cfgPinInputPullup(uint32_t pinNbr);

        /********************************************************************************************************//**
         * \brief   Configure the PIN as an input using the pulldown on chip, the standard drive and connected.
         *
         * This method configures the PIN specified by `pinNbr` as an input and takes `PULLDOWN`, `S0S1` and `CONNECT`
         * as default values. It is dedicated to configure GPIO for button's usage.
         *
         * \param   pinNbr      Identify the pin to configure
         ************************************************************************************************************/
        static void cfgPinInputPullDown(uint32_t pinNbr);

        /********************************************************************************************************//**
         * \brief   Configure the PIN as an output
         *
         * This method configures the PIN specified by `pinNbr` as an output and setup its drive's capability and
         * connection's state.
         *
         * \param   pinNbr      Identify the pin to configure
         * \param   drive       Set the current drive capacity (high or low drain capability).
         * \param   pullSetup   Configure the output with an active pullup, pulldown or idsabled.
         * \param   connect     Specify if the pin is connected or disconnected.
         ************************************************************************************************************/
        static void cfgPinOutput(uint32_t pinNbr, uint32_t drive, uint32_t pullSetup, uint32_t connect);

        /********************************************************************************************************//**
         * \brief   Configure the PIN as an output with pulldown or pullup disabled
         *
         * This method configures the PIN specified by `pinNbr` as an output and takes `PULLDOWN`, the given drive
         * capability and `DISCONNECT` as default values.
         *
         * \param   pinNbr      Identify the pin to configure
         * \param   drive       Set the current drive capacity (high or low drain capability).
         ************************************************************************************************************/
        static void cfgQuickOutput(uint32_t pinNbr, uint32_t drive = NRF_GPIO_PIN_S0S1);

        /********************************************************************************************************//**
         * \brief   Clears the output PINs specified by their mask.
         *
         * \param   pinMask     Identify the pins to clear
         ************************************************************************************************************/
        static void pinMaskClear(uint64_t pinMask);

        /********************************************************************************************************//**
         * \brief   Set the output PINs specified by their mask.
         *
         * \param   pinMask     Identify the pins to set
         ************************************************************************************************************/
        static void pinMaskSet(uint64_t pinMask);

        /********************************************************************************************************//**
         * \brief   Toggle the output  PINs specified by their mask.
         *
         * \param   pinMask     Identify the pins to toggle
         ************************************************************************************************************/
        static void pinMaskToggle(uint64_t pinMask);

        /********************************************************************************************************//**
         * \brief   Set the output PIN at low level
         *
         * \param   pinNbr      Identify the pin to clear
         ************************************************************************************************************/
        static void pinClear(uint32_t pinNbr);

        /********************************************************************************************************//**
         * \brief   Set the output PIN at high level
         *
         * \param   pinNbr      Identify the pin to set
         ************************************************************************************************************/
        static void pinSet(uint32_t pinNbr);

        /********************************************************************************************************//**
         * \brief   Toggle the output PIN
         *
         * \param   pinNbr      Identify the pin to toggle
         ************************************************************************************************************/
        static void pinToggle(uint32_t pinNbr);

        /********************************************************************************************************//**
         * \brief   Write the given value to the output PIN
         *
         * \param   pinNbr      Identify the pin to write to
         * \param   value       The value to write to the output pin.
         ************************************************************************************************************/
        static void pinWrite(uint32_t pinNbr, uint8_t value);

        /********************************************************************************************************//**
         * \brief   Read the state of the input PIN and returns the `true` if the PIN is set and `false` otherwise.
         *
         * \param   pinNbr      Identify the pin to read
         ************************************************************************************************************/
        static bool pinRead(uint32_t pinNbr);

        /********************************************************************************************************//**
         * \brief   Read the input PINs specified by their mask.
         *
         * \param   pinMask     Identify the pins to read
         ************************************************************************************************************/
        static uint64_t pinMaskRead(uint64_t pinMask);

        #if defined(DEBUG_NRF_USER)
        /********************************************************************************************************//**
         * \brief   Initializes the Hardware GPIO assign to the pin number as an output.
         *
         * For DEBUG usage, this function allows the developer to initialize a specific GPIO as an output.
         *
         * \param   pinNbr      Identify the pin to configure
         ************************************************************************************************************/
        static void initDebugGPIO(uint8_t pinNbr);

        /********************************************************************************************************//**
         * \brief   Toggle the state of the DEBUG GPIO
         *
         * For DEBUG usage, toggles the state of GPIO...
         *
         * \param   pinNbr      Identify the pin to configure
         ************************************************************************************************************/
        static void toggleDebugGPIO(uint8_t pinNbr);
        #endif  // #if defined(DEBUG_NRF_USER)
    
    protected:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* CONSTRUCTOR SECTION                                                                                      */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        GpioHal()  {}
        ~GpioHal() {}

    };  // Class gpioHAL

}   // Namepsace hal

/** @} */
