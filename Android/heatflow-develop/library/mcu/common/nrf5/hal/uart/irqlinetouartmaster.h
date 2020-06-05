/****************************************************************************************************************//**
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2018
 * Created by Thomas Sterren (thomas.sterren@hevs.ch) & Patrice Rudaz (patrice.rudaz@hevs.ch)
 * All rights reserved.
 *
 * \file    irqlinetouertmaster.h
 *
 * \addtogroup LowHAL
 * @{
 *
 * \class   hal::IrqLineToUartMaster
 * \brief   Handles IRQ line to UART master.
 *
 * When UART is used with Hardware Control, it should be usefull to have a line for the slave to request a 
 * communication transfer to the master. In fact, the MCU needs to inform the UART master prior to send data to it.
 *
 * Initial author: Thomas Sterren
 * Creation date: 2016-06-07
 *
 * \author  Patrice Rudaz
 * \date    July 2018
 ********************************************************************************************************************/

#ifndef HAL_IRQLINETOUARTMASTER_H
#define HAL_IRQLINETOUARTMASTER_H

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
    class IrqLineToUartMaster
    {
    public:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PUBLIC DECLARATION SECTION                                                                               */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        /********************************************************************************************************//**
         * \brief   Initialiaze the class.
         ************************************************************************************************************/
        static void initialize();

        /********************************************************************************************************//**
         * \brief   Indicate UART master that we want to send data.
         ************************************************************************************************************/
        static void set();

        /********************************************************************************************************//**
         * \brief   Indicate UART master that we are done with sending data.
         ************************************************************************************************************/
        static void clear();
    };

}   // namespace hal

#endif // HAL_IRQLINETOUARTMASTER_H

/** @} */
