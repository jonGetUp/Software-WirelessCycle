/****************************************************************************************************************//**
 * Copyright (C) Soprod SA, Sion. 2018
 * Created by Thierry Hischier (thischier@soprod.ch)
 *
 * \file        icommunication.h
 *
 * \defgroup    Communication
 * \{
 *
 * \brief       Interface for the different communication protocols (I2C, SPI, ...)
 *
 * Initial author: Thierry Hischier
 * Creation date: 2018-03-13
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

/* **************************************************************************************************************** */
/*                                                                                                                  */
/* Interface Declaration                                                                                            */
/*                                                                                                                  */
/* **************************************************************************************************************** */
class ICommunication
{
public:
    /* ************************************************************************************************************ */
    /*                                                                                                              */
    /* CONSTRUCTOR/DESTRUCTOR DECLARATION SECTION                                                                   */
    /*                                                                                                              */
    /* ************************************************************************************************************ */
    ICommunication()            {}
    virtual ~ICommunication()   {}

    /* ************************************************************************************************************ */
    /*                                                                                                              */
    /* PUBLIC DECLARATION SECTION                                                                                   */
    /*                                                                                                              */
    /* ************************************************************************************************************ */
    /************************************************************************************************************//**
     * \name    ICommunicationsAbstractMethods 
     * \brief   Abstract methods to be implemented by any instance of Communication's Interface
     * \{
     ****************************************************************************************************************/
    virtual void initialiseDriver() = 0;
    virtual void unInitialiseDriver() = 0;
    virtual bool readByte(uint8_t* txData, uint32_t txLength, uint8_t* rxData) = 0;
    virtual bool readBytes(uint8_t* txData, uint32_t txLength, uint8_t* rxData, uint32_t rxLength) = 0;
    virtual bool writeByte(uint8_t txData) = 0;
    virtual bool writeBytes(uint8_t* txData, uint32_t txLength) = 0;
    virtual void selectSlave(uint8_t slave) = 0;
    virtual void setFullDuplex(bool fullDuplex = false) = 0;
    /** \} */
};  // class ICommunication

/** \} */   // Group: Communication
