/****************************************************************************************************************//**
 * Copyright (C) Soprod SA, Sion. 2018
 * Created by Thierry Hischier (thischier@soprod.ch)
 *
 * \file        spiminstance.h
 *
 * \addtogroup  Communication
 * \{
 *
 * \addtogroup  SPI
 * \{
 *
 * \brief       Class that enumerates the different available SPIM instances
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
#include <platform-config.h>

#if (USE_SPI != 0)
    #include <nrfx_spi.h>
#else
    #include <nrfx_spim.h>
#endif


/* **************************************************************************************************************** */
/*                                                                                                                  */
/* TYPEDEF DECLARATION                                                                                              */
/*                                                                                                                  */
/* **************************************************************************************************************** */


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
    class SPIInstance
    {
    public:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PUBLIC DECLARATION SECTION                                                                               */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        /********************************************************************************************************//**
         * Enumeration of SPI instances
         ************************************************************************************************************/
        typedef enum
        {
            SPI0                = 0x00,
            SPI1                = 0x01,
            SPI2                = 0x02,
            SPI3                = 0x03,

            SPI_UNKNOWN         = 0xFF
        } eSPIInstance;
        
        
        typedef enum
        {
            #if (USE_SPI != 0) && (NRFX_SPI0_ENABLED == 1)
            SPI0_INST_IDX       = NRFX_SPI0_INST_IDX,
            #endif
            #if (USE_SPIM != 0) && (NRFX_SPIM0_ENABLED == 1)
            SPI0_INST_IDX       = NRFX_SPIM0_INST_IDX,
            #endif
            #if (USE_SPI != 0) && (NRFX_SPI1_ENABLED == 1)
            SPI1_INST_IDX       = NRFX_SPI1_INST_IDX,
            #endif
            #if (USE_SPIM != 0) && (NRFX_SPIM1_ENABLED == 1)
            SPI1_INST_IDX       = NRFX_SPIM1_INST_IDX,
            #endif
            #if (USE_SPI != 0) && (NRFX_SPI2_ENABLED == 1)
            SPI2_INST_IDX       = NRFX_SPI2_INST_IDX,
            #endif
            #if (USE_SPIM != 0) && (NRFX_SPIM3_ENABLED == 1)
            SPI2_INST_IDX       = NRFX_SPIM2_INST_IDX,
            #endif
            #if (USE_SPI != 0) && (NRFX_SPI3_ENABLED == 1)
            SPI3_INST_IDX       = NRFX_SPI3_INST_IDX,
            #endif
            #if (USE_SPIM != 0) && (NRFX_SPIM3_ENABLED == 1)
            SPI3_INST_IDX       = NRFX_SPIM3_INST_IDX,
            #endif
        } eSPIInstanceIndex;

        typedef enum
        {
            SPI_MODE_0          = 0x00,     ///< SCK active high, sample on leading edge of clock.
            SPI_MODE_1          = 0x01,     ///< SCK active high, sample on trailing edge of clock.
            SPI_MODE_2          = 0x02,     ///< SCK active low, sample on leading edge of clock.
            SPI_MODE_3          = 0x03,     ///< SCK active low, sample on trailing edge of clock.

            SPI_MODE_UNKNOWN    = 0xFF
        } eSPIMode;

        /**
         * @brief SPI bit orders.
         */
        typedef enum
        {
            SPI_BIT_ORDER_MSB_FIRST = 0,    ///< Most significant bit shifted out first.
            SPI_BIT_ORDER_LSB_FIRST = 1     ///< Least significant bit shifted out first.
        } eSPIBitOrder;

    };

}   // Namespace comm

/** \} */   // Group: SPI

/** \} */   // Group: Communication
