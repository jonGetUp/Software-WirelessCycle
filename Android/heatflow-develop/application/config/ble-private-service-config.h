/* Copyright (c) 2018, HES-SO Valais
 * All rights reserved.
 */
#ifndef BLE_PRIVATE_SERVICE_CONFIG_H
#define BLE_PRIVATE_SERVICE_CONFIG_H

#include <app_util.h>
#include <ble_srv_common.h>
// <<< Use Configuration Wizard in Context Menu >>>\n
#ifdef USE_APP_CONFIG
#include "app_config.h"
#endif

/********************************************************************************************************************
 * \brief    Specific UUID for HEI Sion.
 *
 * This UUID is build on the acronyme "UoASVS" of the registered name of HEI Sion in the 
 * Bluetooth SIG, which is "University of Appled Science Valais/Haute Ecole Valaisanne".
 * The last part is build with the word CAFE and the company ID 0x025A.
 *
 * 0x53 0x56 0x53 0x41 0x6F 0x55 0xFF 0xFF 0x5A 0x02 0xFE 0xCA 0x00 0x00 0x00 0x00
 *  S    V    S    A    o    U      ...      5A   02   FE   CA         ...
 ********************************************************************************************************************/
#define BLE_HEI_UUID_PRIVATE_BASE           { 0x53, 0x56, 0x53, 0x41, 0x6F, 0x55, 0xFF, 0xFF, \
                                              0x5A, 0x02, 0xFE, 0xCA, 0x00, 0x00, 0x00, 0x00 }


/********************************************************************************************************************
 * \brief    Specific UUID for Soprod Sion.
 * 
 * This UUID is build on the company name "Soprod", its location (Sion) and the company ID assigned by 
 * the Bluetooth SIG (0xXXXX).
 *
 * 0x64 0x6F 0x72 0x70 0x6F 0x53 0xFF 0xFF 0x6E 0x6F 0x69 0x53 0x00 0x00 0x00 0x00
 *  d    o    r    p    o    S      ...      n    o    i    S           ...
 ********************************************************************************************************************/
#define BLE_SOPROD_UUID_PRIVATE_BASE        { 0x64, 0x6F, 0x72, 0x70, 0x6F, 0x53, 0xFF, 0xFF, \
                                              0x6E, 0x6F, 0x69, 0x53, 0x00, 0x00, 0x00, 0x00 }


/********************************************************************************************************************
 * \brief    Private Service's UUID given by Nordic...
 ********************************************************************************************************************/
#define BLE_UUID_PRIVATE_NS_BASE            { 0x23, 0xD1, 0xBC, 0xEA, 0x5F, 0x78, 0xFF, 0xFF, \
                                              0xDE, 0xEF, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00 }

// -----------------------------------------------------------------------------------------------------------
// Service Owner
#define HEI_OWNER_MASK_UUID                 0x1700                                                                      ///< \brief BLE Service Owner UUID: 0x1700 -> HEI Sion, 
#define SOPROD_OWNER_MASK_UUID              0x0512                                                                      ///<                                0x0512 -> Soprod (Company identifier from Bluetooth SIG)
#define OWNER_MASK_LOWER_IDX                6
#define OWNER_MASK_HIGHER_IDX               7


//==========================================================
// <h> Soprod Private Services
                                              
//==========================================================
// <e> ATS - Soprod Activity Tracker Service
// <i> Enable the usage of ATS Service (Soprod Activity Tracker Service)
#ifndef ATS_SUPPORT
#define ATS_SUPPORT                         0
#endif

//==========================================================
// <o> Add service's UUID in advertisement

// <0=> Do not advertise this service's UUID
// <1=> Add the service's UUID in advertisement Data
// <2=> Add the service's UUID in Scan response

#ifndef ADVERTISE_ATS_UUID
#define ADVERTISE_ATS_UUID                  0
#endif

//==========================================================
// </e>


//==========================================================
// <e> SAVS - Soprod After Sales Service
// <i> Enable the usage of SAVS Service (Soprod After Sales Service)
#ifndef SAVS_SUPPORT
#define SAVS_SUPPORT                         0
#endif

//==========================================================
// <o> Add service's UUID in advertisement

// <0=> Do not advertise this service's UUID
// <1=> Add the service's UUID in advertisement Data
// <2=> Add the service's UUID in Scan response

#ifndef ADVERTISE_SAVS_UUID
#define ADVERTISE_SAVS_UUID                  0
#endif

//==========================================================
// </e>


//==========================================================
// <e> SBS - Soprod Button Service
// <i> Enable the usage of SBS Service (Soprod Button Service)
#ifndef SBS_SUPPORT
#define SBS_SUPPORT                         0
#endif

//==========================================================
// <o> Add service's UUID in advertisement

// <0=> Do not advertise this service's UUID
// <1=> Add the service's UUID in advertisement Data
// <2=> Add the service's UUID in Scan response

#ifndef ADVERTISE_SBS_UUID
#define ADVERTISE_SBS_UUID                  0
#endif

//==========================================================
// </e>


//==========================================================
// <e> SNS - Soprod Notification Service
// <i> Enable the usage of SNS Service (Soprod Notification Service)
#ifndef SNS_SUPPORT
#define SNS_SUPPORT                         0
#endif

//==========================================================
// <o> Add service's UUID in advertisement

// <0=> Do not advertise this service's UUID
// <1=> Add the service's UUID in advertisement Data
// <2=> Add the service's UUID in Scan response

#ifndef ADVERTISE_SNS_UUID
#define ADVERTISE_SNS_UUID                  0
#endif

//==========================================================
// </e>


//==========================================================
// <e> SSWS - Soprod SmartWatch Service
// <i> Enable the usage of SSWS Service (Soprod SmartWatch Service)
#ifndef SSWS_SUPPORT
#define SSWS_SUPPORT                        0
#endif

//==========================================================
// <o> Add service's UUID in advertisement

// <0=> Do not advertise this service's UUID
// <1=> Add the service's UUID in advertisement Data
// <2=> Add the service's UUID in Scan response

#ifndef ADVERTISE_SSWS_UUID
#define ADVERTISE_SSWS_UUID                 0
#endif

//==========================================================
// <q> Update the hands positions through BLE notification
// <i> Enable/Disable the update of the motor position into the BLE charachteristic

#ifndef SSWS_MOTOR_POSITION_UPDATE
#define SSWS_MOTOR_POSITION_UPDATE          0
#endif

//==========================================================
// </e>


//==========================================================
// <e> TBS - Soprod Test Bench Service
// <i> Enable the usage of TBS Service (Soprod Test Bench Service)
#ifndef TBS_SUPPORT
#define TBS_SUPPORT                         0
#endif

//==========================================================
// <o> Add service's UUID in advertisement

// <0=> Do not advertise this service's UUID
// <1=> Add the service's UUID in advertisement Data
// <2=> Add the service's UUID in Scan response

#ifndef ADVERTISE_TBS_UUID
#define ADVERTISE_TBS_UUID                  0
#endif

//==========================================================
// </e>


// -----------------------------------------------------------------------------------------------------------
// Number of BLE adopted services activated
#define NUMBER_OF_SOPROD_PRIVATE_SERVICES   (ATS_SUPPORT+SBS_SUPPORT+SNS_SUPPORT+SSWS_SUPPORT+SAVS_SUPPORT+TBS_SUPPORT)

//==========================================================
// </h>

//==========================================================
// <h> HEI Private Services


//==========================================================
// <e> RCTS - Remote Controlled Toy Service
// <i> Enable the usage of RCTS Service (Remote Controlled Toy Service)
#ifndef RCTS_SUPPORT
#define RCTS_SUPPORT                        0
#endif


//==========================================================
// <o> Add service's UUID in advertisement

// <0=> Do not advertise this service's UUID
// <1=> Add the service's UUID in advertisement Data
// <2=> Add the service's UUID in Scan response

#ifndef ADVERTISE_RCTS_UUID
#define ADVERTISE_RCTS_UUID                 2
#endif


//==========================================================
// </e>


//==========================================================
// <e> MHFS - Monitor HeatFlow Service
// <i> Enable the usage of MHFS Service (Monitor HeatFlow Toy Service)
#ifndef MHFS_SUPPORT
#define MHFS_SUPPORT                        1
#endif


//==========================================================
// <o> Add service's UUID in advertisement

// <0=> Do not advertise this service's UUID
// <1=> Add the service's UUID in advertisement Data
// <2=> Add the service's UUID in Scan response

#ifndef ADVERTISE_MHFS_UUID
#define ADVERTISE_MHFS_UUID                 2
#endif


//==========================================================
// </e>




// -----------------------------------------------------------------------------------------------------------
// Number of BLE adopted services activated
#define NUMBER_OF_HEI_PRIVATE_SERVICES      (RCTS_SUPPORT+MHFS_SUPPORT)

//==========================================================
// </h>


//==========================================================
// <q> Enable User's description on characteristics
// ---------------------------------------------------------
// <i> Enable the usage of the user's descritpion on characteristics of BLE private service


#ifndef USE_USER_DESCRIPTION_FOR_CHAR
#define USE_USER_DESCRIPTION_FOR_CHAR       1
#endif

#endif // BLE_PRIVATE_SERVICE_CONFIG_H
