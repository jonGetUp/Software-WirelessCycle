/*************************************************************************************************************
 * Copyright (C) Soprod SA, Sion. 2018
 * Created by Thierry Hischier (thischier@soprod.ch)
 *
 * \file    spimasterdriver.cpp
 * \brief   Contains all necessary functions and event handlers to communication via a SPI peripheral
 *
 * Initial author: Thierry Hischier
 * Creation date: 2018-03-13
 ************************************************************************************************************/

/************************************************************************************************************/
/*                                                                                                          */
/* Header files                                                                                             */
/*                                                                                                          */
/************************************************************************************************************/
#include "spimasterdriver.h"

#if (USE_SPI != 0) || (USE_SPIM != 0)
#include "factory.h"
#include "gpio/gpiohal.h"

#include <sdk_config.h>

#include <nrf52_bitfields.h>
    
#if (USE_SPI != 0)
    #if (NRFX_SPI0_ENABLED == 0) && (NRFX_SPI1_ENABLED == 0) && (NRFX_SPI2_ENABLED == 0) && (NRFX_SPI3_ENABLED == 0) 
        #error "Enable at least one of the NRFX_SPIx_ENABLED in sdk_config.h !"
    #endif
#else
    #if (NRFX_SPIM0_ENABLED == 0) && (NRFX_SPIM1_ENABLED == 0) && (NRFX_SPIM2_ENABLED == 0) && (NRFX_SPIM3_ENABLED == 0) 
        #error "Enable at least one of the NRFX_SPIMx_ENABLED in sdk_config.h !"
    #endif
#endif


/************************************************************************************************************/
/* DEBUG COMPILATION OPTION & MACRO                                                                         */
/************************************************************************************************************/
#define DEBUG_SPI_MASTER_DRIVER_ENABLE      0
#if (DEBUG_SPI_MASTER_DRIVER_ENABLE != 0) && defined(DEBUG_NRF_USER)
    #include <tools/logtrace.h>
    #define SMD_DEBUG                       LOG_TRACE_DEBUG
#else
    #define SMD_DEBUG(...)                  {(void) (0);}
#endif


/************************************************************************************************************/
/* Namespace                                                                                                */
/************************************************************************************************************/
using comm::SpiMasterDriver;

/************************************************************************************************************/
/* --- Extern "C" Section ---                                                                               */
/*     Callback's section                                                                                   */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
#if (USE_SPI != 0)
void comm::spiEventHandler(nrfx_spi_evt_t const*  pEvent, void* pContext)
#else
void comm::spiEventHandler(nrfx_spim_evt_t const* pEvent, void* pContext)
#endif
{
    if(pContext != NULL)
    {
        static_cast<SpiMasterDriver*>(pContext)->onSpiDoneEvent((void*) pEvent);
    }
    else
    {
        SMD_DEBUG("[SpiMasterDriver] spiEventHandler() > Context is NULL !\r\n");
    }
}

    
/************************************************************************************************************/
/* Declaration section                                                                                      */
/************************************************************************************************************/

/************************************************************************************************************/
/* Constructor | Destructor                                                                                 */
/************************************************************************************************************/
// -----------------------------------------------------------------------------------------------------------
SpiMasterDriver::SpiMasterDriver() {}

// -----------------------------------------------------------------------------------------------------------
SpiMasterDriver::~SpiMasterDriver() {}


/************************************************************************************************************/
/*                                              PUBLIC SECTION                                              */
/************************************************************************************************************/
// -----------------------------------------------------------------------------------------------------------
void SpiMasterDriver::initialize()
{
    _spiConfig.sck_pin          = 0xFF;
    _spiConfig.mosi_pin         = 0xFF;
    _spiConfig.miso_pin         = 0xFF;
    _spiConfig.ss_pin           = 0xFF;
    _spiConfig.irq_priority     = NRFX_SPI_DEFAULT_CONFIG_IRQ_PRIORITY;
    _spiConfig.orc              = 0xFF;

    #if (USE_SPIM != 0)
    _spiConfig.ss_active_high   = false;
    #endif
    
    setClock(0x40000000UL);
    setMode(0);
    setBitOrder(SPIInstance::SPI_BIT_ORDER_MSB_FIRST);
    
    _selectedSlave              = 0xFF;
    _initialized                = false;
    _spiXferDone                = false;

    _fullDuplex                 = false;
    _isChipSelectHandled        = false;
    _txValueOnFullDuplex        = 0x00;
    SMD_DEBUG("[SpiMasterDriver] initialize() > Initialise Done...\r\n");
}

// -----------------------------------------------------------------------------------------------------------
void SpiMasterDriver::bind(SPIInstance::eSPIInstance instance)
{
    switch(instance)
    {
        #if (NRFX_SPI0_ENABLED == 1) || (NRFX_SPIM0_ENABLED == 1)
        case SPIInstance::SPI0:
            _spi.p_reg                  = NRF_SPI0;
            _spi.drv_inst_idx           = SPIInstance::SPI0_INST_IDX;
            _spiConfig.sck_pin          = SPI0_SCK_PIN;
            _spiConfig.mosi_pin         = SPI0_MOSI_PIN;
            _spiConfig.miso_pin         = SPI0_MISO_PIN;
            break;
        #endif

        #if (NRFX_SPI1_ENABLED == 1) || (NRFX_SPIM1_ENABLED == 1)
        case SPIInstance::SPI1:
            _spi.p_reg                  = NRF_SPI1;
            _spi.drv_inst_idx           = SPIInstance::SPI1_INST_IDX;
            _spiConfig.sck_pin          = SPI1_SCK_PIN;
            _spiConfig.mosi_pin         = SPI1_MOSI_PIN;
            _spiConfig.miso_pin         = SPI1_MISO_PIN;
            break;
        #endif

        #if (NRFX_SPI2_ENABLED == 1) || (NRFX_SPIM2_ENABLED == 1)
        case SPIInstance::SPI2:
            _spi.p_reg                  = NRF_SPI2;
            _spi.drv_inst_idx           = SPIInstance::SPI2_INST_IDX;
            _spiConfig.sck_pin          = SPI2_SCK_PIN;
            _spiConfig.mosi_pin         = SPI2_MOSI_PIN;
            _spiConfig.miso_pin         = SPI2_MISO_PIN;
            break;
        #endif
        
        #if (NRFX_SPI3_ENABLED == 1) || (NRFX_SPIM3_ENABLED == 1)
        case SPIInstance::SPI3:
            _spi.p_reg                  = NRF_SPI3;
            _spi.drv_inst_idx           = SPIInstance::SPI3_INST_IDX;
            _spiConfig.sck_pin          = SPI3_SCK_PIN;
            _spiConfig.mosi_pin         = SPI3_MOSI_PIN;
            _spiConfig.miso_pin         = SPI3_MISO_PIN;
            break;
        #endif
        default:
        {
            SMD_DEBUG("[SpiMasterDriver] bind() > Impossible to bind SPI instance to the SPI Master Driver !!! Check your configuration ...\n");
            ASSERT(false);
            break;
        }
    }
    
    SMD_DEBUG("[SpiMasterDriver] bind() > Bind Done...\r\n");
}

// -----------------------------------------------------------------------------------------------------------
bool SpiMasterDriver::setConfig(uint32_t frequencyClk, uint8_t mode)
{
    return setClock(frequencyClk) && setMode(mode);
}

// -----------------------------------------------------------------------------------------------------------
bool SpiMasterDriver::setClock(uint32_t frequencyClk)
{
    if(// SPI
       frequencyClk == SPI_FREQUENCY_FREQUENCY_K125     ||
       frequencyClk == SPI_FREQUENCY_FREQUENCY_K250     ||
       frequencyClk == SPI_FREQUENCY_FREQUENCY_K500     ||
       frequencyClk == SPI_FREQUENCY_FREQUENCY_M1       ||
       frequencyClk == SPI_FREQUENCY_FREQUENCY_M2       ||
       frequencyClk == SPI_FREQUENCY_FREQUENCY_M4       ||
       frequencyClk == SPI_FREQUENCY_FREQUENCY_M8       ||
    
       // SPIM
       frequencyClk == SPIM_FREQUENCY_FREQUENCY_K125    ||
       frequencyClk == SPIM_FREQUENCY_FREQUENCY_K250    ||
       frequencyClk == SPIM_FREQUENCY_FREQUENCY_K500    ||
       frequencyClk == SPIM_FREQUENCY_FREQUENCY_M1      ||
       frequencyClk == SPIM_FREQUENCY_FREQUENCY_M2      ||
       frequencyClk == SPIM_FREQUENCY_FREQUENCY_M4      ||
       frequencyClk == SPIM_FREQUENCY_FREQUENCY_M8)
    {
        #if (USE_SPI != 0)
        _spiConfig.frequency = static_cast<nrf_spi_frequency_t>(frequencyClk);
        #else
        _spiConfig.frequency = static_cast<nrf_spim_frequency_t>(frequencyClk);
        #endif
        return true;
    }
    else
    {
        SMD_DEBUG("[SpiMasterDriver] setClock() > Invalid frequency: %d\r\n", frequencyClk);
        return false;
    }
}

// -----------------------------------------------------------------------------------------------------------
bool SpiMasterDriver::setMode(uint8_t mode)
{
    if(mode < 4)
    {
        #if (USE_SPI != 0)
        _spiConfig.mode = static_cast<nrf_spi_mode_t>(mode);
        #else
        _spiConfig.mode = static_cast<nrf_spim_mode_t>(mode);
        #endif
        return true;
    }
    else
    {
        SMD_DEBUG("[SpiMasterDriver] setMode() > Invalid mode: %d\r\n", mode);
        return false;
    }
}

// -----------------------------------------------------------------------------------------------------------
void SpiMasterDriver::setBitOrder(SPIInstance::eSPIBitOrder bitOrder)
{
    #if (USE_SPI != 0)
    _spiConfig.bit_order = static_cast<nrf_spi_bit_order_t>(bitOrder);
    #else
    _spiConfig.bit_order = static_cast<nrf_spim_bit_order_t>(bitOrder);
    #endif
}

// -----------------------------------------------------------------------------------------------------------
void SpiMasterDriver::initialiseDriver()
{
    _initSpi();
}

// -----------------------------------------------------------------------------------------------------------
void SpiMasterDriver::unInitialiseDriver()
{
    _unInitSpi();
}

// -----------------------------------------------------------------------------------------------------------
bool SpiMasterDriver::readByte(uint8_t* txData, uint32_t txLength, uint8_t* rxData)
{
    uint8_t _rxData[SPI_BUFFER_SIZE] = {0};

    if(_spiTransfer(txData, txLength, _rxData, 1, _fullDuplex))
    {
        *rxData = _rxData[txLength];
        return true;
    }
    else
    {
        *rxData = 0;
        SMD_DEBUG("[SpiMasterDriver] readByte() > Read Byte not successful...\r\n");
        return false;
    }

}

// -----------------------------------------------------------------------------------------------------------
bool SpiMasterDriver::readBytes(uint8_t* txData, uint32_t txLength, uint8_t* rxData, uint32_t rxLength)
{
    uint8_t _rxData[SPI_BUFFER_SIZE] = {0};

    if(_spiTransfer(txData, txLength, _rxData, rxLength, _fullDuplex))
    {
        memcpy(rxData, &_rxData[txLength], rxLength);
        return true;
    }
    else
    {
        memset(rxData, 0, rxLength);
        SMD_DEBUG("[SpiMasterDriver] readBytes() > Read Byte not successful...\r\n");
        return false;
    }
}

// -----------------------------------------------------------------------------------------------------------
bool SpiMasterDriver::writeByte(uint8_t txData)
{
    uint8_t _rxData[SPI_BUFFER_SIZE] = {0};
    
    if(_spiTransfer(&txData, 1, _rxData, 0, _fullDuplex))
    {
        return true;
    }
    else
    {
        SMD_DEBUG("[SpiMasterDriver] writeByte() > Write Byte not successful...\r\n");
        return false;
    }
}

// -----------------------------------------------------------------------------------------------------------
bool SpiMasterDriver::writeBytes(uint8_t* txData, uint32_t txLength)
{
    uint8_t _rxData[SPI_BUFFER_SIZE] = {0};
    
    if(_spiTransfer(txData, txLength, _rxData, 0, _fullDuplex))
    {
        return true;
    }
    else
    {
        SMD_DEBUG("[SpiMasterDriver] writeBytes() > Write Bytes not successful...\r\n");
        return false;
    }
}

// -----------------------------------------------------------------------------------------------------------
void SpiMasterDriver::selectSlave(uint8_t slave)
{
	_selectedSlave = slave;
    
    _unInitSpi();

    if(_isChipSelectHandled)
    {
        _spiConfig.ss_pin = slave;
    }
    else
    {
        hal::GpioHal::cfgQuickOutput(slave);
        hal::GpioHal::pinSet(slave);
    }

    _initSpi();
}

// -----------------------------------------------------------------------------------------------------------
void SpiMasterDriver::setFullDuplex(bool fullDuplex)
{
    _fullDuplex = fullDuplex;
}

// -----------------------------------------------------------------------------------------------------------
#if (USE_SPI != 0)
nrfx_spi_config_t SpiMasterDriver::getConfig()
#else
nrfx_spim_config_t SpiMasterDriver::getConfig()
#endif
{
    return _spiConfig;
}

// -----------------------------------------------------------------------------------------------------------
#if (USE_SPI != 0)
void SpiMasterDriver::setConfig(nrfx_spi_config_t config)
#else
void SpiMasterDriver::setConfig(nrfx_spim_config_t config)
#endif
{
    _unInitSpi();
    _spiConfig = config;
    _initSpi();
}

// -----------------------------------------------------------------------------------------------------------
void SpiMasterDriver::onSpiDoneEvent(void const* pEvent)
{
#if (USE_SPI != 0)
    nrfx_spi_evt_t const* event  = static_cast<nrfx_spi_evt_t const*>(pEvent);
    if(event->type == NRFX_SPI_EVENT_DONE)
#else
    nrfx_spim_evt_t const* event = static_cast<nrfx_spim_evt_t const*>(pEvent);
    if(event->type == NRFX_SPIM_EVENT_DONE)
#endif
    {
        _spiXferDone = true;
        SMD_DEBUG("[SpiMasterDriver] onSpiDoneEvent() > %d Bytes sent, %d Bytes received\r\n", pEvent->xfer_desc.tx_length, pEvent->xfer_desc.rx_length);
    }
}

/************************************************************************************************************/
/*                                              PRIVATE SECTION                                             */
/************************************************************************************************************/
// -----------------------------------------------------------------------------------------------------------
void SpiMasterDriver::_initSpi()
{
    if(!_initialized)
    {
        #if (USE_SPI != 0)
        uint32_t  errCode = nrfx_spi_init(&_spi, &_spiConfig, NULL, NULL);
        #else
        uint32_t  errCode = nrfx_spim_init(&_spi, &_spimConfig, NULL, NULL);
        #endif
        if(errCode != NRF_SUCCESS)
        {
            SMD_DEBUG("[SpiMasterDriver] _initSpi() > nrfx SPI driver failed to init ! (errCode: 0x%04x)\r\n", errCode);
            return; 
        }
        _initialized = true;
    }
}

// -----------------------------------------------------------------------------------------------------------
void SpiMasterDriver::_unInitSpi()
{
    if(_initialized)
    {
        #if (USE_SPI != 0)
        nrfx_spi_uninit(&_spi);
        #elif (USE_SPIM != 0)
        nrfx_spim_uninit(&_spi);
        #endif
        _initialized = false;
    }
}

// -----------------------------------------------------------------------------------------------------------
bool SpiMasterDriver::_spiTransfer(uint8_t* txData, uint32_t txLength, uint8_t* rxData, uint32_t rxLength, bool fullDuplex)
{
    #if (USE_SPI != 0)
    nrfx_spi_xfer_desc_t spiXferDescriptor;
    #else
    nrfx_spim_xfer_desc_t spiXferDescriptor;
    #endif
    
    _spiXferDone     = false;
    if(_isChipSelectHandled)
    {
        hal::GpioHal::pinClear(_selectedSlave);
    }
    _initSpi();
    
    // start the transaction
    spiXferDescriptor.p_tx_buffer   = txData;
    spiXferDescriptor.tx_length     = txLength;
    spiXferDescriptor.p_rx_buffer   = rxData;
    spiXferDescriptor.rx_length     = fullDuplex ? (txLength + rxLength):rxLength;

    #if (USE_SPI != 0)
    uint32_t errCode                = nrfx_spi_xfer( &_spi, &spiXferDescriptor, 0);
    #else
    uint32_t errCode                = nrfx_spim_xfer(&_spi, &spiXferDescriptor, 0);
    #endif
    if(errCode != NRF_SUCCESS)
    {
        SMD_DEBUG("[SpiMasterDriver] _spiTransfer() > Error: 0x%04x\r\n", errCode);
    }
        
    if(_isChipSelectHandled)
    {
        hal::GpioHal::pinSet(_selectedSlave);
    }
    _unInitSpi();
	
    return (errCode == NRF_SUCCESS);
}

#endif  // #if (USE_SPI != 0) || (USE_SPIM != 0)
