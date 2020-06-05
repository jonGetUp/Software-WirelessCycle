/****************************************************************************************************************//**
 * Copyright (C) Soprod SA, Sion. 2018
 * Created by Thierry Hischier (thischier@soprod.ch)
 *
 * \file    spimasterdriver.h
 *
 * \addtogroup  Communication
 * \{
 *
 * \addtogroup  SPI
 * \{
 *
 * \brief   Class that manages an SPI peripheral as Master
 *
 * Initial author: Thierry Hischier
 * Creation date: 2018-03-13
 *
 * \author  Patrice Rudaz
 * \date    February 2019
 ********************************************************************************************************************/
#pragma once

/* **************************************************************************************************************** */
/*                                                                                                                  */
/* Header files                                                                                                     */
/*                                                                                                                  */
/* **************************************************************************************************************** */
#include <platform-config.h>
#include <boards.h>

#if (USE_SPI != 0) || (USE_SPIM != 0)

#include "icommunication.h"
#include "spiinstance.h"

#if (USE_SPI != 0)
    #include <nrfx_spi.h>
#else
    #include <nrfx_spim.h>
#endif

#if defined(__cplusplus)
extern "C" 
{
#endif
    namespace comm 
    {
        /********************************************************************************************************//**
         * \brief   Event handler for the SPIM driver.
         *
         * This function will be called each time the SPIM driver throws an event.
         *
         * \param   pEvent      Pointer used for passing the SPIM event.
         * \param   pContext    Pointer used for passing some arbitrary information (context).
         ************************************************************************************************************/
        #if (USE_SPI != 0)
            void spiEventHandler(nrfx_spi_evt_t const* pEvent, void* pContext);
        #else
            void spiEventHandler(nrfx_spim_evt_t const* pEvent, void* pContext);
        #endif
    }
#if defined(__cplusplus)
}
#endif

/* **************************************************************************************************************** */
/*                                                                                                                  */
/* TYPEDEF DECLARATION                                                                                              */
/*                                                                                                                  */
/* **************************************************************************************************************** */
#define SPI_BUFFER_SIZE 256

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
    class SpiMasterDriver : public ICommunication
    {
    #if (USE_SPI != 0)
        friend void spiEventHandler(nrfx_spi_evt_t const* pEvent, void* pContext);
    #else
        friend void spiEventHandler(nrfx_spim_evt_t const* pEvent, void* pContext);
    #endif
        
    public:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PUBLIC DECLARATION SECTION                                                                               */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        SpiMasterDriver();
        virtual ~SpiMasterDriver();

        void initialize();
        void bind(SPIInstance::eSPIInstance instance);
        bool setConfig(uint32_t frequencyClk, uint8_t mode);
        bool setClock(uint32_t frequencyClk);
        bool setMode(uint8_t mode);
        void setBitOrder(SPIInstance::eSPIBitOrder bitOrder);
    
        inline void setChipSelectHandling(bool internally)          { _isChipSelectHandled = internally; }
        inline void setTxValueOnFullDuplex(uint8_t value)           { _txValueOnFullDuplex = value; }
        inline bool isChipSelectHandledInternally() const           { return _isChipSelectHandled; }
        inline uint8_t txValueOnFullDuplex() const                  { return _txValueOnFullDuplex; }
       
        /********************************************************************************************************//**
         * \brief   Manual Initialisation of the SPI Driver Instance.
         ************************************************************************************************************/
        virtual void initialiseDriver();
        
        /********************************************************************************************************//**
         * \brief   Manual Uninitialisation of the SPI Driver Instance.
         ************************************************************************************************************/
        virtual void unInitialiseDriver();

        /********************************************************************************************************//**
         * \brief   Read a byte from a register address via SPI.
         *
         * \param[in]   txData     Data like the register to transmit.
         * \param[in]   txLength   Length of the transmission data
         * \param[out]  rxdata     The received data byte
         *
         * \return  False if the transaction failed
         ************************************************************************************************************/
        virtual bool readByte(uint8_t* txData, uint32_t txLength, uint8_t* rxData);

        /********************************************************************************************************//**
         * \brief   Read a series of bytes from a starting-register via SPI.
         *
         * \param[in]   txData     Data like the register to transmit.
         * \param[in]   txLength   Length of the transmission data
         * \param[out]  rxData     The received data bytes
         * \param[in]   rxLength   The length of data to read
         *
         * \return  False if the transaction failed
         ************************************************************************************************************/
        virtual bool readBytes(uint8_t* txData, uint32_t txLength, uint8_t* rxData, uint32_t rxLength);

        /********************************************************************************************************//**
         * \brief   Write a byte to a register via SPI.
         *
         * \param[in]   txData     Data like to transmit.
         *
         * \return  False if the transaction failed
         ************************************************************************************************************/
        virtual bool writeByte(uint8_t txData);

        /********************************************************************************************************//**
         * \brief   Write several bytes out of SPI to a register in the device.
         *
         * \param[in]   txData     Data to transmit.
         * \param[in]   txLength   Length of the transmission data
         *
         * \return  False if the transaction failed
         ************************************************************************************************************/
        virtual bool writeBytes(uint8_t* txData, uint32_t txLength);

        /********************************************************************************************************//**
         * \brief    Select the desired slave
         ************************************************************************************************************/
        virtual void selectSlave(uint8_t slave);
        
        /********************************************************************************************************//**
         * \brief    Select the desired slave
         ************************************************************************************************************/
        virtual void setFullDuplex(bool fullDuplex = false);
        
        #if (USE_SPI != 0)
        /********************************************************************************************************//**
         * \brief    Get SPI Master configuration
         ************************************************************************************************************/
        nrfx_spi_config_t getConfig();
        
        /********************************************************************************************************//**
         * \brief    Set SPI Master new configuration
         ************************************************************************************************************/
        void setConfig(nrfx_spi_config_t config);
        
        #else
        /********************************************************************************************************//**
         * \brief    Get SPIM Master configuration
         ************************************************************************************************************/
        nrfx_spim_config_t getConfig();
        
        /********************************************************************************************************//**
         * \brief    Set SPIM Master new configuration
         ************************************************************************************************************/
        void setConfig(nrfx_spim_config_t config);
        #endif
        
        /********************************************************************************************************//**
         * \brief    Method handling the NRFX_SPI_EVENT_DONE or NRFX_SPIM_EVENT_DONE event...
         ************************************************************************************************************/
        void onSpiDoneEvent(void const* pEvent);

    private:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PRIVATE DECLARATION SECTION                                                                              */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        // Attributes
        #if (USE_SPI != 0)
        nrfx_spi_t                          _spi;
        nrfx_spi_config_t                   _spiConfig;
        #else
        nrfx_spim_t                         _spi;
        nrfx_spim_config_t                  _spiConfig;
        #endif
        uint8_t                             _selectedSlave;
        bool                                _initialized;
        bool                                _spiXferDone;

        bool                                _fullDuplex;
        bool                                _isChipSelectHandled;
        uint8_t                             _txValueOnFullDuplex;

        // Methods
        void                                _initSpi();
        void                                _unInitSpi();
        bool                                _spiTransfer(uint8_t* txData, uint32_t txLength, uint8_t* rxData, uint32_t rxLength, bool fullDuplex);
    };  // class SpiMasterDriver

}   // namespace spi

#endif  // #if (USE_SPI != 0) || (USE_SPIM != 0)

/** \} */   // Group: SPI

/** \} */   // Group: Communication
