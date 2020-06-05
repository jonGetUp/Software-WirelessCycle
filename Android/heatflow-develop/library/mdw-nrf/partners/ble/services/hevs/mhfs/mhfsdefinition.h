/****************************************************************************************************************//**
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 *
 * \file        mhfsdefinition.h
 *
 * \addtogroup BLE
 * \{
 *
 * \addtogroup prSrv
 * \{
 *
 * \defgroup mhfs Hevs HeatFlow Service
 * \{
 *
 * \brief   File handling the definition of the private BLE services `Monitor HeatFlow Service (MHFS)`.
 *
 * This module defines the UUIDs used by the Monitor HeatFlow Service, which is a BLE private service.
 *
 * Initial author: Patrice Rudaz
 * Creation date: 2019-01-31
 *
 * \author  Patrice Rudaz (patrice.rudaz@hevs.ch)
 * \date    January 2019
 ********************************************************************************************************************/
#pragma once


/* **************************************************************************************************************** */
/*                                                                                                                  */
/* Header files                                                                                                     */
/*                                                                                                                  */
/* **************************************************************************************************************** */
#include <ble_types.h>

#include <stdint.h>
#include <stdbool.h>


/* **************************************************************************************************************** */
/*                                                                                                                  */
/* DEFINITION                                                                                                       */
/*                                                                                                                  */
/* **************************************************************************************************************** */
#define BLE_UUID_TYPE_MHFS                  BLE_UUID_TYPE_VENDOR_BEGIN

/* **************************************************************************************************************** */
/*                                                                                                                  */
/* PRIVATE BLE SERVICE DEFINITION                                                                                   */
/*                                                                                                                  */
/* **************************************************************************************************************** */
#define MHFS_SERVICE_UUID                   0x1011
#define MHFS_SERVICE_DESC                   "Monitor HeatFlow Service"


/* **************************************************************************************************************** */
/*                                                                                                                  */
/* SERVICE'S CHARACTERISTICS DEFINITION                                                                             */
/*                                                                                                                  */
/* **************************************************************************************************************** */
// ------------------------------------------------------------------------------------------------------------------
// MHFS ADS values
#define MHFS_ADS_VAL_CHAR                   0x1031
#define MHFS_ADS_VAL_CHAR_DESC              "ADS Values"

// ------------------------------------------------------------------------------------------------------------------
// MHFS First ADS Configuration
#define MHFS_ADS_CONFIGURATION_CHAR         0x1041
#define MHFS_ADS_CONFIGURATION_CHAR_DESC    "ADS Configuration"

// ------------------------------------------------------------------------------------------------------------------
// MHFS HeatFlow command
#define MHFS_CMD_CHAR                       0x1051
#define MHFS_CMD_CHAR_DESC                  "Command"

/* **************************************************************************************************************** */
/*                                                                                                                  */
/* TYPE DEFINITION                                                                                                  */
/*                                                                                                                  */
/* **************************************************************************************************************** */
#pragma pack(1)

// ------------------------------------------------------------------------------------------------------------------
/** \brief Monitor HeatFlow Service: data's structure for all ADS value characteristic. */
typedef struct
{
    uint8_t                                 index;                                                                      ///< Index of ADS
    uint16_t                                configuration;                                                              ///< Configuration of the ADS
} mhfsAdsConfiguration_t;

// ------------------------------------------------------------------------------------------------------------------
/** \brief Monitor HeatFlow Service: data's structure for ADS value characteristic. */
typedef union
{
    uint32_t                                rawValue;                                                                   ///< Value retrieved by the ADS comparator
    float                                   milliVolts;                                                                 ///< Value expressed in milli-volts (float 32 IEEE 754)
} mhfsAdsRaw_t;

typedef struct
{
    struct
    {
        uint8_t         adsId:1;
        uint8_t         mux:3;
        uint8_t         pga:3;
        bool            isFloat:1;
    } params;
    mhfsAdsRaw_t        value;
} mhfsAdsRecord_t;

// ------------------------------------------------------------------------------------------------------------------
/** \brief Monitor HeatFlow Service:: data's structure for the measurement command characteristic. */
typedef enum
{
    MhfsStop            = 0,
    MhfsStart,
    MhfsSingleAdc,
    MhfsContinuousAdc,
    MhfsTemperature
} eMhfsCommandId;

typedef struct
{
    eMhfsCommandId                          commandId;
} mhfsCommand_t;

#pragma pack()

/** \} */

/** \} */   // Group: Private Services

/** \} */   // Group: BLE
