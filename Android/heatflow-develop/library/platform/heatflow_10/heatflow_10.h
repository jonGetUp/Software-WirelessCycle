/****************************************************************************************************************//**
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 *
 * \file    heatflow_10.h
 *
 * \author  Patrice Rudaz (patrice.rudaz@hevs.ch)
 * \date    August 2018
 ********************************************************************************************************************/
#ifndef HEATFLOW_10_H
#define HEATFLOW_10_H

#include <nrf_gpio.h>
#include <ble_types.h>

// ------------------------------------------------------------------------------------------------------------------
// Device Name, Model Number, ...
#define DEVICE_NAME                         "HeatFlow"                                                                  ///< \brief Name of device used when using the target hardware.
#define MODEL_NUMBER                        "1.0"                                                                       ///< \brief Model number. Will be passed to Device Information Service.
#define SERIAL_NUMBER                       10001                                                                       ///< \brief If set to 0, the serial Number of the device is read from the UICR
#define MODEL_IDENTIFIER                    0x0100
#define RESERVED1                           0xFECA
#define RESERVED2                           0xEFBE

// ------------------------------------------------------------------------------------------------------------------
// LEDs definitions for PCA10040
#define LEDS_NUMBER                         3
#define LED_ACTIVE_LOW                      1

#define LED_1                               18  // LEDg
#define LED_2                               17  // LEDb
#define LED_3                               16  // LEDr
//#define LED_4                               20

#define LEDS_LIST                           { LED_1, LED_2, LED_3/*, LED_4*/ }

// ------------------------------------------------------------------------------------------------------------------
// Button's Component
#define BUTTONS_NUMBER                      0
#define BUTTON_PULL_SETUP                   NRF_GPIO_PIN_PULLUP                                                         ///< \brief General configuration for the GPIO's pullup. Here only Pullup or pulldown config MUST be used !!!
#define BTN_IS_ACTIVE_HIGH	            	0

#define BUTTON_1                            13
#define BUTTON_2                            14
#define BUTTON_3                            15
#define BUTTON_4                            16

// ------------------------------------------------------------------------------------------------------------------
// UART's Component related
#define RX_PIN_NUMBER                       8
#define TX_PIN_NUMBER                       6
#define CTS_PIN_NUMBER                      7
#define RTS_PIN_NUMBER                      5
#define HWFC                                false
#define IRQ_USR2_PIN                        14                                                                           ///< \brief In low power mode, we need the master to wake up the UART... This GPIO is used to notify the UART master device to change the CTS line

// -----------------------------------------------------------------------------------------------------------
// Motor's Component related
/*
#define MOTORS_CONN_SERIES                  0
#define COMMON_COIL_C                       1

#if (MOTORS_CONN_SERIES != 0)
    #define NB_MAX_MOTORS                   5

    #define M1_CA_PIO                       5
    #define M1_CB_PIO                       7
    #define M1_CC_PIO                       6
    #define M1_STEPS                        60
    #define M1_NBS                          1
    #define M1_CCW                          0

    #define M2_CA_PIO                       7
    #define M2_CB_PIO                       11
    #define M2_CC_PIO                       8
    #define M2_STEPS                        180
    #define M2_NBS                          1
    #define M2_CCW                          1

    #define M3_CA_PIO                       11
    #define M3_CB_PIO                       22
    #define M3_CC_PIO                       12
    #define M3_STEPS                        120
    #define M3_NBS                          1
    #define M3_CCW                          0

    #define M4_CA_PIO                       22
    #define M4_CB_PIO                       24
    #define M4_CC_PIO                       23
    #define M4_STEPS                        180
    #define M4_NBS                          18
    #define M4_CCW                          1

    #define M5_CA_PIO                       24
    #define M5_CB_PIO                       10
    #define M5_CC_PIO                       9
    #define M5_STEPS                        180
    #define M5_NBS                          18
    #define M5_CCW                          1
#else
    #define NB_MAX_MOTORS                   5

    #define M1_CA_PIO                       5
    #define M1_CB_PIO                       7
    #define M1_CC_PIO                       6
    #define M1_STEPS                        60
    #define M1_NBS                          1
    #define M1_CCW                          0

    #define M2_CA_PIO                       8
    #define M2_CB_PIO                       10
    #define M2_CC_PIO                       9
    #define M2_STEPS                        180
    #define M2_NBS                          1
    #define M2_CCW                          1

    #define M3_CA_PIO                       11
    #define M3_CB_PIO                       22
    #define M3_CC_PIO                       12
    #define M3_STEPS                        120
    #define M3_NBS                          1
    #define M3_CCW                          0

    #define M4_CA_PIO                       23
    #define M4_CB_PIO                       28
    #define M4_CC_PIO                       24
    #define M4_STEPS                        180
    #define M4_NBS                          18
    #define M4_CCW                          1

    #define M5_CA_PIO                       29
    #define M5_CB_PIO                       31
    #define M5_CC_PIO                       30
    #define M5_STEPS                        180
    #define M5_NBS                          18
    #define M5_CCW                          1
#endif
*/

// ------------------------------------------------------------------------------------------------------------------
// Device Information
#define BLE_APPEARANCE                      BLE_APPEARANCE_GENERIC_COMPUTER                                             ///< could be : BLE_APPEARANCE_GENERIC_THERMOMETER or BLE_APPEARANCE_UNKNOWN \see ble_types.h

// ------------------------------------------------------------------------------------------------------------------
// Battery Measurement's Component related
#define GPIO_CHG_MES                        NRF_SAADC_INPUT_AIN3

// ------------------------------------------------------------------------------------------------------------------
// VCC related
#define GPIO_VCC_ON                         27

// ------------------------------------------------------------------------------------------------------------------
// SPI's Component related
#define SPI0_SCK_PIN                        24
#define SPI0_MOSI_PIN                       23
#define SPI0_MISO_PIN                       22
//#define SPI0_SS_PIN                         11

// ------------------------------------------------------------------------------------------------------------------
// ADS Chip Select related
#define ADS_CS1_PIN                         11
#define ADS_CS2_PIN                         12

// ------------------------------------------------------------------------------------------------------------------
// Hardware related
#define SENSOR_VDD_ENABLE                   20
#define SENSOR_VDD_ACTIF_LOW                1

#endif // HEATFLOW_10_H
