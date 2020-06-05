/*************************************************************************************************************
 * Copyright (C) Soprod SA, Sion. 2018
 * Created by Thierry Hischier (thischier@soprod.ch)
 *
 * \file    i2cmasterdriver.cpp
 * \brief   Contains all necessary functions and event handlers to communication via an I2C peripheral
 *
 * Initial author: Thierry Hischier
 * Creation date: 2018-03-20
 ************************************************************************************************************/

/************************************************************************************************************/
/*                                                                                                          */
/* Header files                                                                                             */
/*                                                                                                          */
/************************************************************************************************************/
#include "i2cmasterdriver.h"
#include "app_util_platform.h"
#include "factory.h"
#include "gpio/gpiohal.h"

//#if (USE_I2C != 0)
/************************************************************************************************************/
/* DEBUG COMPILATION OPTION & MACRO                                                                         */
/************************************************************************************************************/
#define DEBUG_I2C_MASTER_DRIVER_ENABLE      1
#if (DEBUG_I2C_MASTER_DRIVER_ENABLE != 0) && defined(DEBUG_NRF_USER)
    #include <tools/logtrace.h>
    #define I2CMD_DEBUG                     LOG_TRACE_DEBUG
#else
    #define I2CMD_DEBUG(...)                {(void) (0);}
#endif


/************************************************************************************************************/
/* Namespace                                                                                                */
/************************************************************************************************************/
using comm::I2cMasterDriver;

/************************************************************************************************************/
/* --- Extern "C" Section ---                                                                               */
/*     Callback's section                                                                                   */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
void comm::i2cEventHandler(nrfx_twim_evt_t const* pEvent, void* pContext)
{
    I2cMasterDriver* i2cMasterDriver = NULL;
    if(pContext != NULL)
    {
        i2cMasterDriver = (reinterpret_cast<I2cMasterDriver*>(pContext));
    }
    else
    {
        ASSERT(false);
    }

    switch(pEvent->type)
    {
        case NRFX_TWIM_EVT_DONE:
            I2CMD_DEBUG("[I2cMasterDriver] i2cEventHandler() > Transfer completed! Device Address: 0x%X, Bytes transferred: %d\r\n", 
                        pEvent->xfer_desc.address, pEvent->xfer_desc.primary_length);
            if(pEvent->xfer_desc.type == NRFX_TWIM_XFER_TX)
            {
                i2cMasterDriver->_unInitI2C();
                i2cMasterDriver->_notifyTxDone();
            }
            else
            {
                i2cMasterDriver->_unInitI2C();
                i2cMasterDriver->_notifyDataReceived();
            }
            break;
        case NRFX_TWIM_EVT_ADDRESS_NACK:
            I2CMD_DEBUG("[I2cMasterDriver] i2cEventHandler() > Error: NACK received after sending the address.\r\n");
            i2cMasterDriver->_unInitI2C();
            i2cMasterDriver->_notifyAddrNack();
            break;
        case NRFX_TWIM_EVT_DATA_NACK:
            i2cMasterDriver->_unInitI2C();
            I2CMD_DEBUG("[I2cMasterDriver] i2cEventHandler() > Error: NACK received after sending a data byte.\r\n");
            break;
        default:
            break;
    }
}

/************************************************************************************************************/
/* Declaration section                                                                                      */
/************************************************************************************************************/

/************************************************************************************************************/
/* Constructor | Destructor                                                                                 */
/************************************************************************************************************/
// -----------------------------------------------------------------------------------------------------------
I2cMasterDriver::I2cMasterDriver() {}

// -----------------------------------------------------------------------------------------------------------
I2cMasterDriver::~I2cMasterDriver() {}


/************************************************************************************************************/
/*                                              PUBLIC SECTION                                              */
/************************************************************************************************************/
// -----------------------------------------------------------------------------------------------------------
void I2cMasterDriver::initialize()
{
    _i2cConfig.frequency            = (nrf_twim_frequency_t)NRFX_TWIM_DEFAULT_CONFIG_FREQUENCY;
    _i2cConfig.scl                  = 0;
    _i2cConfig.sda                  = 0;
    _i2cConfig.interrupt_priority   = NRFX_TWIM_DEFAULT_CONFIG_IRQ_PRIORITY;
    _i2cConfig.hold_bus_uninit      = NRFX_TWIM_DEFAULT_CONFIG_HOLD_BUS_UNINIT;
    _initialized                    = false;
    _slaveAddress                   = 0;
    I2CMD_DEBUG("[I2cMasterDriver] initialize() > Initialise Done...\r\n");
}

// -----------------------------------------------------------------------------------------------------------
void I2cMasterDriver::bind(I2CInstance::eI2CInstance instance)
{
    switch(instance)
    {
        #if (NRFX_TWIM0_ENABLED == 1)
        case I2CInstance::I2C0:
            _i2c.p_twim = NRF_TWIM0;
            _i2c.drv_inst_idx = NRFX_TWIM0_INST_IDX;
            _i2cConfig.scl = I2C0_SCL_PIN;
            _i2cConfig.sda = I2C0_SDA_PIN;
            hal::GpioHal::cfgPinInput(I2C0_SCL_PIN, NRF_GPIO_PIN_S0D1, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_INPUT_CONNECT);
            hal::GpioHal::cfgPinInput(I2C0_SDA_PIN, NRF_GPIO_PIN_S0D1, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_INPUT_CONNECT);
            break;
        #endif

        #if (NRFX_TWIM1_ENABLED == 1)
        case I2CInstance::I2C1:
            _i2c.p_twim = NRF_TWIM1;
            _i2c.drv_inst_idx = NRFX_TWIM1_INST_IDX;
            _i2cConfig.scl = I2C1_SCL_PIN;
            _i2cConfig.sda = I2C1_SDA_PIN;
            #if (USE_GPIOS != 0)
            hal::GpioHal::cfgPinInput(I2C1_SCL_PIN, NRF_GPIO_PIN_S0D1, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_INPUT_CONNECT);
            hal::GpioHal::cfgPinInput(I2C1_SDA_PIN, NRF_GPIO_PIN_S0D1, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_INPUT_CONNECT);
            #endif
            break;
        #endif

        default:
            // do nothing
            break;
    }
    I2CMD_DEBUG("[I2cMasterDriver] bind() > Bind Done...\r\n");
}

// -----------------------------------------------------------------------------------------------------------
void I2cMasterDriver::initialiseDriver()
{
    if(!_initialized)
    {
    #if (USE_I2C != 0)
        uint32_t errCode = NRF_SUCCESS;
        
        #if (I2C_ASYNC_MODE != 0)
        errCode = nrfx_twim_init(&_i2c, &_i2cConfig, i2cEventHandler, this);
        #else
        errCode = nrfx_twim_init(&_i2c, &_i2cConfig, NULL, NULL);
        #endif
        
        if(errCode != NRF_SUCCESS)
        {
            I2CMD_DEBUG("[I2cMasterDriver] initialiseDriver() > nrfx_twim_init failed with ErrorCode: 0x%04x)\r\n", errCode);
            return; 
        }
        nrfx_twim_enable(&_i2c);
    #endif
        _initialized = true;
        I2CMD_DEBUG("[I2cMasterDriver] initialiseDriver() > I2C Driver Instance manually initialised\r\n");
    }
}

// -----------------------------------------------------------------------------------------------------------
void I2cMasterDriver::unInitialiseDriver()
{
    if(_initialized)
    {
    #if (USE_I2C != 0)
        nrfx_twim_disable(&_i2c);
        nrfx_twim_uninit(&_i2c);
    #endif
        _initialized = false;
        I2CMD_DEBUG("[I2cMasterDriver] unInitialiseDriver() > I2C Driver Instance manually uninitialised\r\n");
    }
}

// -----------------------------------------------------------------------------------------------------------
bool I2cMasterDriver::readByte(uint8_t* txData, uint32_t txLength, uint8_t* rxData)
{
    if(_i2cTransfer(txData, txLength, rxData, 1))
    {
        //I2CMD_DEBUG("[I2cMasterDriver] readByte() > Read Byte successful...\r\n");
        return true;
    }
    else
    {
        I2CMD_DEBUG("[I2cMasterDriver] readByte() > Read Byte not successful...\r\n");
        return false;
    }

}

// -----------------------------------------------------------------------------------------------------------
bool I2cMasterDriver::readBytes(uint8_t* txData, uint32_t txLength, uint8_t* rxData, uint32_t rxLength)
{
    if(_i2cTransfer(txData, txLength, rxData, rxLength))
    {
        //I2CMD_DEBUG("[I2cMasterDriver] readBytes() > Read Bytes successful...\r\n");
        return true;
    }
    else
    {
        I2CMD_DEBUG("[I2cMasterDriver] readBytes() > Read Byte not successful...\r\n");
        return false;
    }
}

// -----------------------------------------------------------------------------------------------------------
bool I2cMasterDriver::writeByte(uint8_t txData)
{    
    if(_i2cTransfer(&txData, 1, NULL, 0))
    {
        //I2CMD_DEBUG("[I2cMasterDriver] writeByte() > Write Byte successful...\r\n");
        return true;
    }
    else
    {
        I2CMD_DEBUG("[I2cMasterDriver] writeByte() > Write Byte not successful...\r\n");
        return false;
    }
}

// -----------------------------------------------------------------------------------------------------------
bool I2cMasterDriver::writeBytes(uint8_t* txData, uint32_t txLength)
{    
    if(_i2cTransfer(txData, txLength, NULL, 0))
    {
        //I2CMD_DEBUG("[I2cMasterDriver] writeBytes() > Write Bytes successful...\r\n");
        return true;
    }
    else
    {
        I2CMD_DEBUG("[I2cMasterDriver] writeBytes() > Write Bytes not successful...\r\n");
        return false;
    }
}

// -----------------------------------------------------------------------------------------------------------
void I2cMasterDriver::selectSlave(uint8_t slave)
{
	_slaveAddress = slave;
}

/************************************************************************************************************/
/*                                              PRIVATE SECTION                                             */
/************************************************************************************************************/
// -----------------------------------------------------------------------------------------------------------
void I2cMasterDriver::_initI2C()
{
    if(!_initialized)
    {
    #if (USE_I2C != 0)
        uint32_t errCode = NRF_SUCCESS;
        
        #if (I2C_ASYNC_MODE != 0)
        errCode = nrfx_twim_init(&_i2c, &_i2cConfig, i2cEventHandler, this);
        #else
        errCode = nrfx_twim_init(&_i2c, &_i2cConfig, NULL, NULL);
        #endif
        
        if(errCode != NRF_SUCCESS)
        {
            I2CMD_DEBUG("[I2cMasterDriver] _initI2C() > nrfx_twim_init failed with ErrorCode: 0x%04x)\r\n", errCode);
            return; 
        }
        nrfx_twim_enable(&_i2c);
    #endif
        _initialized = true;
        //I2CMD_DEBUG("[I2cMasterDriver] _initI2C() > I2C Driver Instance manually initialised\r\n");
    }
}

// -----------------------------------------------------------------------------------------------------------
void I2cMasterDriver::_unInitI2C()
{
    if(_initialized)
    {
    #if (USE_I2C != 0)
        nrfx_twim_disable(&_i2c);
        nrfx_twim_uninit(&_i2c);
    #endif
        _initialized = false;
    }
}

// -----------------------------------------------------------------------------------------------------------
bool I2cMasterDriver::_i2cTransfer(uint8_t* txData, uint32_t txLength, uint8_t* rxData, uint32_t rxLength)
{
    uint32_t errCode = NRF_SUCCESS;
    _initI2C();
    
    // start the transaction
    if(txLength != 0)
    {
    #if (USE_I2C != 0)
        nrfx_twim_xfer_desc_t twimXferDescriptor;
        twimXferDescriptor.address = _slaveAddress;
        twimXferDescriptor.type = NRFX_TWIM_XFER_TX;
        twimXferDescriptor.p_primary_buf = txData;
        twimXferDescriptor.primary_length = txLength;
        twimXferDescriptor.p_secondary_buf = NULL;
        twimXferDescriptor.secondary_length = 0;
        errCode = nrfx_twim_xfer(&_i2c, &twimXferDescriptor, 0);
        if(errCode != NRF_SUCCESS)
        {
            I2CMD_DEBUG("[I2cMasterDriver] _i2cTransfer() > nrfx_twim_tx failed with ErrorCode: 0x%04x)\r\n", errCode);
            _unInitI2C();
            return false;
        }
    #endif
    }
    
    
    // start the reception
    if(rxLength != 0)
    {
    #if (USE_I2C != 0)
        nrfx_twim_xfer_desc_t twimXferDescriptor;
        twimXferDescriptor.address = _slaveAddress;
        twimXferDescriptor.type = NRFX_TWIM_XFER_RX;
        twimXferDescriptor.p_primary_buf = rxData;
        twimXferDescriptor.primary_length = rxLength;
        twimXferDescriptor.p_secondary_buf = NULL;
        twimXferDescriptor.secondary_length = 0;
        errCode = nrfx_twim_xfer(&_i2c, &twimXferDescriptor, 0);
    #endif
    }
    
	#if (I2C_ASYNC_MODE == 0)
        _unInitI2C();
    #endif
    
    if(errCode == NRF_SUCCESS)
    {
        return true;
    }
    else
    {
        I2CMD_DEBUG("[I2cMasterDriver] _i2cTransfer() > nrfx_twim_rx failed with ErrorCode: 0x%04x)\r\n", errCode);
        return false;
    }
}

// ------------------------------------------------------------------------------------------------------------------
void I2cMasterDriver::_notifyTxDone()
{
    for(uint8_t i = 0; i < observersCount(); i++)
    {
        if(observer(i) != NULL)
        {
            observer(i)->onTransferCompleted(this);
        }
    }
}

// ------------------------------------------------------------------------------------------------------------------
void I2cMasterDriver::_notifyDataReceived()
{
    for(uint8_t i = 0; i < observersCount(); i++)
    {
        if(observer(i) != NULL)
        {
            observer(i)->onDataReceived(this);
        }
    }
}

// ------------------------------------------------------------------------------------------------------------------
void I2cMasterDriver::_notifyAddrNack()
{
    for(uint8_t i = 0; i < observersCount(); i++)
    {
        if(observer(i) != NULL)
        {
            observer(i)->onAddrNack(this);
        }
    }
}
//#endif
