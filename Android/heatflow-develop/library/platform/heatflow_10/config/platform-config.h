/****************************************************************************************************************//**
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 *
 * \file    platform-config.h
 * \brief   This file is used to enable/disable all the different peripherals and features for PCA10040 project
 *
 * Initial author: Patrice Rudaz
 * Creation date: 2018-01-11
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
#include <sdk_config.h>


// ------------------------------------------------------------------------------------------------------------------
#ifndef UNUSED
    #define UNUSED(x) ((void)0)
#endif // UNUSED


/* **************************************************************************************************************** */
/*                                                                                                                  */
/* LOG TRACE ENABLER                                                                                                */
/*                                                                                                                  */
/* **************************************************************************************************************** */
// ------------------------------------------------------------------------------------------------------------------
#define LOG_TRACE_RTT_ENABLED               1                                                                           ///< \brief Enable or disable the trace logger using SEGGER RTT


/* **************************************************************************************************************** */
/*                                                                                                                  */
/* BLE Service related                                                                                              */
/*                                                                                                                  */
/* **************************************************************************************************************** */
// ------------------------------------------------------------------------------------------------------------------
// Device Information 
#if defined(HEI)
    #define MANUFACTURER_NAME               "HEI Sion"
    #define COMPANY_IDENTIFIER              0x025A
//    #define COMPANY_IDENTIFIER              0
#elif defined(SOPROD) 
    #define MANUFACTURER_NAME               "Soprod"
    #define COMPANY_IDENTIFIER              0x0512
#else
    #define MANUFACTURER_NAME               "Nordic"                                                                    ///< \brief Manufacturer. Will be passed to Device Information Service.
    #define COMPANY_IDENTIFIER              0x0059                                                                      ///< \brief Company Identifier assigned by the Bluetooth SIG
#endif

// ------------------------------------------------------------------------------------------------------------------
// Device Name, Model Number, ...
#define DEVICE_NAME_MAX_SIZE                18
#define ADD_BT_ADDR_TO_DEVICE_NAME          1                                                                           ///< \brief To add both last bytes of the BT address to the device name
#define USE_INFORMATION_FROM_UICR           0                                                                           ///< \brief To use some information from the UICR register (serial number, hw version, etc)


/* **************************************************************************************************************** */
/*                                                                                                                  */
/* COMPONENTS, DRIVERS & LIBRARIES defintion                                                                        */
/*                                                                                                                  */
/* **************************************************************************************************************** */
// ------------------------------------------------------------------------------------------------------------------
// XF
#define USE_XF                              1

// ------------------------------------------------------------------------------------------------------------------
// UpTime related
#define USE_UPTIME                          0

// ------------------------------------------------------------------------------------------------------------------
// LEDs related
#define USE_LEDS                            1

// ------------------------------------------------------------------------------------------------------------------
// GPIO related
#define USE_GPIOS                           0

// ------------------------------------------------------------------------------------------------------------------
// Button's component related
#define USE_BUTTONS                         0

// ------------------------------------------------------------------------------------------------------------------
// SAADC related
#define USE_SAADC                           1

// ------------------------------------------------------------------------------------------------------------------
// Calendar's component related
#define USE_CALENDAR                        0

// ------------------------------------------------------------------------------------------------------------------
// Notification's component related
#define USE_NOTIFICATION                    0

// ------------------------------------------------------------------------------------------------------------------
// Motor's component related
#define USE_MOTORS                          0
#define TICK_AT_NOON                        1
#define NB_STEPS_FOR_MOTOR_INIT             10

// ------------------------------------------------------------------------------------------------------------------
// DFU's component related
#define USE_DFU                             0

// ------------------------------------------------------------------------------------------------------------------
// Battery/Charger component related
#define USE_BATTERY_MANAGER                 1
#define USE_CHARGER                         0
#define USE_STBC02                          0


// ------------------------------------------------------------------------------------------------------------------
// Vibrating motor's component related
#define USE_VIBRATING                       0

// ------------------------------------------------------------------------------------------------------------------
// Buzzer related
#define USE_BUZZER                          0
#define USE_MELODY_MANAGER                  0
#if (USE_MELODY_MANAGER != 0)
    #undef USE_BUZZER
    #define USE_BUZZER                      0
#endif

// ------------------------------------------------------------------------------------------------------------------
#if (USE_VIBRATING != 0) || ((USE_BUZZER != 0) && (USE_MELODY_MANAGER == 0))
    #define USE_TONE_MANAGER                1
#else
    #define USE_TONE_MANAGER                0
#endif

// ------------------------------------------------------------------------------------------------------------------
// PWM's component related
#define USE_PWM                             1
#if (((USE_TONE_MANAGER != 0) && ((USE_VIBRATING != 0) || (USE_BUZZER != 0))) || (USE_MELODY_MANAGER != 0)) && (USE_PWM == 0)
    #undef  USE_PWM
    #define USE_PWM                         1
#endif

// ------------------------------------------------------------------------------------------------------------------
// Flash component related
#define USE_FLASH       					1

// ------------------------------------------------------------------------------------------------------------------
// SPI's component related
#define USE_SPI                             0
#define USE_SPIM                            0
#define SPI_SPIM_ASYNC_MODE                 0

// ------------------------------------------------------------------------------------------------------------------
// SPI's component related
#define USE_I2C								0
#define I2C_ASYNC_MODE						0

// ------------------------------------------------------------------------------------------------------------------
// LIS2DW12's component related
#define USE_LIS2DW12                        0
#define USE_LIS2DW12_I2C                    0       // Set to `0` means USE_LIS2DW12_SPI_SPIM

// ------------------------------------------------------------------------------------------------------------------
// ADXL362's component related
#define USE_ADXL362                         0

// ------------------------------------------------------------------------------------------------------------------
// ADS1118 component related
#define USE_ADS111X                         2
#define USE_SPI_DRV                         1
#if (USE_ADS111X != 0)
    #undef  USE_SPI
    #undef  USE_SPIM
    #define USE_SPI                         1
    #define USE_SPIM                        0
#endif

// ------------------------------------------------------------------------------------------------------------------
// Watchdog's component related
#define USE_WATCHDOG                        0
#define WATCHDOG_DELAY                      30000

// ------------------------------------------------------------------------------------------------------------------
// System OFF related
#define ENABLE_SYSTEM_OFF                   1

// ------------------------------------------------------------------------------------------------------------------
// Security component related USE_PASSKEY_BONDING
#define USE_JUST_WORKS_BONDING              0
#define USE_MITM_BONDING                    0
#define USE_FIXED_PASSKEY                   false
#define PASS_KEY_SIZE                       6
#define ALLOW_REPAIRING                     true
#define CLEAR_BONDED_INFO                   true
