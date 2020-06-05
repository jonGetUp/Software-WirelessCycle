/****************************************************************************************************************//**
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Thomas Sterren, Patrice Rudaz
 * All rights reserved.
 *
 * \file    mcu.h
 * \brief   External C method to halt the MCU when it's in DEBUG mode.
 *
 * Initial author: Thomas Sterren
 * Creation date: 2016-06-07
 *
 * \author  Patrice Rudaz
 * \date    August 2018
 ********************************************************************************************************************/
#ifndef MCU_H
#define MCU_H

#if defined(__cplusplus)
extern "C"
{
#endif
    /************************************************************************************************************//**
     * \brief   Will halt the MCU when in debug mode.
     ****************************************************************************************************************/
    extern void breakOnDebug(void);
#if defined(__cplusplus)
}
#endif

#endif // MCU_H
