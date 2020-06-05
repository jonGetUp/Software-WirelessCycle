/****************************************************************************************************************//**
 * Copyright (C) Soprod SA, Sion. 2018
 * Created by Thierry Hischier (thischier@soprod.ch)
 *
 * \file    i2cmasterdriver.h
 *
 * \addtogroup  Communication
 * \{
 *
 * \addtogroup  I2C
 * \{
 *
 * \brief   Class that manages an I2C peripheral as Master
 *
 * Initial author: Thierry Hischier
 * Creation date: 2018-03-20
 ********************************************************************************************************************/
#pragma once

/* **************************************************************************************************************** */
/*                                                                                                                  */
/* Header files                                                                                                     */
/*                                                                                                                  */
/* **************************************************************************************************************** */
#include <platform-config.h>

#include <nrfx_twim.h>

#include <icommunication.h>
#include <core/isubject.hpp>

#include "i2cinstance.h"
#include "interface/i2cmasterobserver.h"


#if defined(__cplusplus)
extern "C" {
#endif
    namespace comm
    {
        /********************************************************************************************************//**
         * \brief   Event handler for the TWIM driver.
         *
         * This function will be called each time the TWIM driver throws an event.
         *
         * \param   pEvent      Pointer used for passing the TWIM event.
         * \param   pContext    Pointer used for passing some arbitrary information (context).
         ************************************************************************************************************/
        void i2cEventHandler(nrfx_twim_evt_t const* pEvent, void* pContext);
    }
#if defined(__cplusplus)
}
#endif

/* **************************************************************************************************************** */
/*                                                                                                                  */
/* TYPEDEF DECLARATION                                                                                              */
/*                                                                                                                  */
/* **************************************************************************************************************** */
#define I2C_BUFFER_SIZE 256

/* **************************************************************************************************************** */
/*                                                                                                                  */
/* Namespace Declaration                                                                                            */
/*                                                                                                                  */
/* **************************************************************************************************************** */
namespace comm
{

    /* ************************************************************************************************************ */
    /*                                                                                                              */
    /* Class Declaration                                                                                            */
    /*                                                                                                              */
    /* ************************************************************************************************************ */
    class I2cMasterDriver : public ICommunication, public ISubject<I2cMasterObserver>
    {
        friend void i2cEventHandler(nrfx_twim_evt_t const* pEvent, void* pContext);
        
    public:
        /* ******************************************************************************************************** */
        /*                                            PUBLIC SECTION                                            */
        /* ******************************************************************************************************** */
        I2cMasterDriver();
        virtual ~I2cMasterDriver();

        void initialize();
        void bind(I2CInstance::eI2CInstance instance);
       
        /********************************************************************************************************//**
         * \brief   Manual Initialisation of the I2C Driver Instance.
         ************************************************************************************************************/
        virtual void initialiseDriver();
        
        /********************************************************************************************************//**
         * \brief   Manual Uninitialisation of the I2C Driver Instance.
         ************************************************************************************************************/
        virtual void unInitialiseDriver();

        /********************************************************************************************************//**
         * \brief   Read a byte from a register address via I2C.
         * \param[in]   txData     Data like the register to transmit.
         * \param[in]   txLength   Length of the transmission data
         * \param[out]  rxdata     The received data byte
         *
         * \return  False if the transaction failed
         ************************************************************************************************************/
        virtual bool readByte(uint8_t* txData, uint32_t txLength, uint8_t* rxData);

        /********************************************************************************************************//**
         * \brief   Read a series of bytes from a starting-register via I2C.
         * \param[in]   txData     Data like the register to transmit.
         * \param[in]   txLength   Length of the transmission data
         * \param[out]  rxData     The received data bytes
         * \param[in]   rxLength   The length of data to read
         *
         * \return  False if the transaction failed
         ************************************************************************************************************/
        virtual bool readBytes(uint8_t* txData, uint32_t txLength, uint8_t* rxData, uint32_t rxLength);

        /********************************************************************************************************//**
         * \brief   Write a byte to a register via I2C.
         * \param[in]   txData     Data like to transmit.
         *
         * \return  False if the transaction failed
         ************************************************************************************************************/
        virtual bool writeByte(uint8_t txData);

        /********************************************************************************************************//**
         * \brief   Write several bytes out of I2C to a register in the device.
         * \param[in]   txData     Data to transmit.
         * \param[in]   txLength   Length of the transmission data
         *
         * \return  False if the transaction failed
         ************************************************************************************************************/
        virtual bool writeBytes(uint8_t* txData, uint32_t txLength);

        /********************************************************************************************************//**
         * \brief    Select the desired slave address
         ************************************************************************************************************/
        virtual void selectSlave(uint8_t slave);
        
        /********************************************************************************************************//**
         * \brief    Select the desired slave address
         ************************************************************************************************************/
        virtual void setFullDuplex(bool fullDuplex = false) {}

    protected:
        /* ******************************************************************************************************** */
        /*                                        PROTECTED SECTION                                             */
        /* ******************************************************************************************************** */

    private:
        /* ******************************************************************************************************** */
        /*                                         PRIVATE SECTION                                              */
        /* ******************************************************************************************************** */
        // Attributes
        nrfx_twim_t _i2c;
        nrfx_twim_config_t _i2cConfig;
        uint8_t _slaveAddress;
        bool _initialized;

        // Methods
        void _initI2C();
        void _unInitI2C();
        bool _i2cTransfer(uint8_t* txData, uint32_t txLength, uint8_t* rxData, uint32_t rxLength);
        
        void _notifyTxDone();
        void _notifyDataReceived();
        void _notifyAddrNack();
    };  // class I2cMasterDriver

}   // namespace comm

/** \} */   // Group: I2C

/** \} */   // Group: Communication
