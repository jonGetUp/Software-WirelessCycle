/****************************************************************************************************************//**
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 *
 * \file    flashfileid.h
 *
 * \addtogroup FLASH
 * \{
 *
 * \class   FlashFileId
 * \brief   Class to specify the available ID for the pages used to store informations in the flash.
 *
 * This class defines the identifiers of all pages to be stored in the FLaSH usingby the FlashController.
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
#include <stdint.h>


/* **************************************************************************************************************** */
/*                                                                                                                  */
/* Namespace Declaration                                                                                            */
/*                                                                                                                  */
/* **************************************************************************************************************** */
namespace flash 
{

    /* ************************************************************************************************************ */
    /*                                                                                                              */
    /* Class Declaration                                                                                            */
    /*                                                                                                              */
    /* ************************************************************************************************************ */
    class FlashFileId
    {
    public:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PUBLIC DECLARATION SECTION                                                                               */
        /*                                                                                                          */
        /* ******************************************************************************************************** */

        /********************************************************************************************************//**
         * \brief    Enumeration of page's identifier available for the current application
         ************************************************************************************************************/
        typedef enum
        {
            UnknownFileId = 0,                  ///< Unknown file id (not initialized).
            
            BatteryManagerFileId,               ///< Battery Manager file id
            ButtonFileId,                       ///< Button file id
            MotorFileId,                        ///< Motor file id
            PeripheralControllerFileId,         ///< PeripheralController file id
            SavFileId,                          ///< SAV file id
            SensorFileId,                       ///< Sensor file id

            TopEnumFileId                       ///< Last entry in the enumeration
        } eFlashFileId;

        /********************************************************************************************************//**
         * \brief    Checks of the given fileId is valid.
         *
         * \param    fileId    The file's identifier to check.
         *
         * \return   `true` if the file ID is a valid one and `false` otherwise
         ************************************************************************************************************/
        static bool isValid(uint8_t fileId)
        {
            return ((fileId == ButtonFileId)    || (fileId == BatteryManagerFileId)         ||
                    (fileId == MotorFileId)     || (fileId == PeripheralControllerFileId)   || 
                    (fileId == SavFileId)       || (fileId == SensorFileId));
        }
        
        
        /********************************************************************************************************//**
         * \brief   Returns the identifier according to the given `uint8_t` id.
         *
         * Returns the identifier according to the given `uint8_t` id. If the given id do not correspond to any entry 
         * of this enumeration, the method will returns `UnknownFileId`.
         *
         * \param   fileId    The file's id to be translated in `eFlashFileId`.
         ************************************************************************************************************/
        static eFlashFileId fromInt(uint8_t fileId) 
        {
            switch(fileId)
            {
                case 1:     return BatteryManagerFileId;
                case 2:     return ButtonFileId;
                case 3:     return MotorFileId;
                case 4:     return PeripheralControllerFileId;
                case 5:     return SavFileId;
                case 6:     return SensorFileId;
                default:    return UnknownFileId;
            }
        }
        

        /********************************************************************************************************//**
         * \brief   Returns the string which describes the given file identifier.
         *
         * \param   fileId    The identifier of the file to translate in string
         ************************************************************************************************************/
        static const char* toString(eFlashFileId fileId)
        {
            switch(fileId)
            {
                case BatteryManagerFileId:          return "BatteryManagerFileId";
                case ButtonFileId:                  return "ButtonFileId";
                case MotorFileId:                   return "MotorFileId";
                case PeripheralControllerFileId:    return "PeripheralControllerFileId";
                case SavFileId:                     return "SavFileId";
                case SensorFileId:                  return "SensorFileId";
                case TopEnumFileId:                 return "TopEnumFileId";
                default:                            return "UnknownFileId";
            }
        }
    };

}   // namespace flash

/** \} */   // Group: FLASH
