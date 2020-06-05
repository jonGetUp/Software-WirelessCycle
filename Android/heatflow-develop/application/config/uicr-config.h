/* Copyright (c) 2018, HES-SO Valais
 * All rights reserved.
 */
#ifndef UICR_CONFIG_H
#define UICR_CONFIG_H


/****************************************************************************************************************//**
 * Define the address in the flash for the UICR[0] register. 
 * This register is reserved for DFU feature
 ********************************************************************************************************************/
#define UICR_DFU_RESERVED_OFFSET                        (0x080)

/****************************************************************************************************************//**
 * Define the address in the flash for the UICR[1] register. 
 * This register is reserved for the serial number
 ********************************************************************************************************************/
#define UICR_SERIAL_NUMBER_OFFSET                       (0x084)
#define UICR_GET_SERIAL_NUMBER                          (*((uint32_t *) (NRF_UICR_BASE + UICR_SERIAL_NUMBER_OFFSET)))

/****************************************************************************************************************//**
 * Define the address in the flash for the UICR[2] register. 
 * This register stores the date and time of the production of the product as a timestamp
 ********************************************************************************************************************/
#define UICR_PRODUCTION_TIMESTAMP_OFFSET                (0x088)
#define UICR_GET_PRODUCTION_TIMESTAMP                   (*((uint32_t *) (NRF_UICR_BASE + UICR_PRODUCTION_TIMESTAMP_OFFSET)))

/****************************************************************************************************************//**
 * Define the address in the flash for the UICR[3] register. 
 * This register stores the date and time of the final control of the product as a timestamp
 ********************************************************************************************************************/
#define UICR_CONTROL_TIMESTAMP_OFFSET                   (0x08C)
#define UICR_GET_CONTROL_TIMESTAMP                      (*((uint32_t *) (NRF_UICR_BASE + UICR_CONTROL_TIMESTAMP_OFFSET)))

/****************************************************************************************************************//**
 * Define the address in the flash for the UICR[4] register. 
 * This register stores the value to be inhibited, according to the LFCK's measurements at 8 degree celsius
 ********************************************************************************************************************/
#define UICR_INHIBITION_VALUE_8C_OFFSET                 (0x090)
#define UICR_GET_INHIBITION_VALUE_8C                    (*((uint32_t *) (NRF_UICR_BASE + UICR_INHIBITION_VALUE_8C_OFFSET)))

/****************************************************************************************************************//**
 * Define the address in the flash for the UICR[5] register. 
 * This register stores the value to be inhibited, according to the LFCK's measurements at 23 degree celsius
 ********************************************************************************************************************/
#define UICR_INHIBITION_VALUE_23C_OFFSET                (0x094)
#define UICR_GET_INHIBITION_VALUE_23C                   (*((uint32_t *) (NRF_UICR_BASE + UICR_INHIBITION_VALUE_23C_OFFSET)))

/****************************************************************************************************************//**
 * Define the address in the flash for the UICR[6] register. 
 * This register stores the value to be inhibited, according to the LFCK's measurements at 38 degree celsius
 ********************************************************************************************************************/
#define UICR_INHIBITION_VALUE_38C_OFFSET                (0x098)
#define UICR_GET_INHIBITION_VALUE_38C                   (*((uint32_t *) (NRF_UICR_BASE + UICR_INHIBITION_VALUE_38C_OFFSET)))

/****************************************************************************************************************//**
 * Define the address in the flash for the UICR[7] register. 
 * This register stores the interval which nust be used to apply the ihnibition's value .
 ********************************************************************************************************************/
#define UICR_INTERVAL_INHIBITION_OFFSET                 (0x09C)
#define UICR_GET_INTERVAL_INHIBITION                    (*((uint32_t *) (NRF_UICR_BASE + UICR_INTERVAL_INHIBITION_OFFSET)))

/****************************************************************************************************************//**
 * Define the address in the flash for the UICR[8] register. 
 * This register stores the calibration value for the x-axis of the accelerometer .
 ********************************************************************************************************************/
#define UICR_ACCELEROMETER_X_AXIS_CALIBRATION_OFFSET    (0x0A0)
#define UICR_GET_ACCELEROMETER_X_AXIS_CALIBRATION       (*((uint32_t *) (NRF_UICR_BASE + UICR_ACCELEROMETER_X_AXIS_CALIBRATION_OFFSET)))

/****************************************************************************************************************//**
 * Define the address in the flash for the UICR[9] register. 
 * This register stores the calibration value for the y-axis of the accelerometer .
 ********************************************************************************************************************/
#define UICR_ACCELEROMETER_Y_AXIS_CALIBRATION_OFFSET    (0x0A4)
#define UICR_GET_ACCELEROMETER_Y_AXIS_CALIBRATION       (*((uint32_t *) (NRF_UICR_BASE + UICR_ACCELEROMETER_Y_AXIS_CALIBRATION_OFFSET)))

/****************************************************************************************************************//**
 * Define the address in the flash for the UICR[10] register. 
 * This register stores the calibration value for the z-axis of the accelerometer .
 ********************************************************************************************************************/
#define UICR_ACCELEROMETER_Z_AXIS_CALIBRATION_OFFSET    (0x0A8)
#define UICR_GET_ACCELEROMETER_Z_AXIS_CALIBRATION       (*((uint32_t *) (NRF_UICR_BASE + UICR_ACCELEROMETER_Z_AXIS_CALIBRATION_OFFSET)))

/****************************************************************************************************************//**
 * Define the address in the flash for the UICR[11] register. 
 * This register stores the calibration value for the battery .
 ********************************************************************************************************************/
#define UICR_BATTERY_OFFSET                             (0x0AC)
#define UICR_GET_BATTERY_OFFSET                         (*((uint32_t *) (NRF_UICR_BASE + UICR_BATTERY_OFFSET)))


#endif // UICR_CONFIG_H
