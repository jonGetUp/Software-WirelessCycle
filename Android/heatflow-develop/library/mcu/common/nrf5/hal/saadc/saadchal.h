/****************************************************************************************************************//**
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 *
 * \file    saadchal.h
 *
 * \addtogroup LowHAL
 * @{
 *
 * \class   hal::SaadcHal
 * \brief   Class of Hardware Abstraction Layer to handle the SAADC hardware of the nRf5 series chips.
 *
 * This file contains all functions necessary for the use of the SAADC. It starts with the function saadcInit()` to 
 * initialize the SAADC.
 *
 * \author  Patrice Rudaz (patrice.rudaz@hevs.ch)
 * \date    June 2018
 ********************************************************************************************************************/
#pragma once

/* **************************************************************************************************************** */
/*                                                                                                                  */
/* Header files                                                                                                     */
/*                                                                                                                  */
/* **************************************************************************************************************** */
#include <platform-config.h>

#if (USE_SAADC != 0)
#include <nrfx_config.h>
#include <nrfx_saadc.h>
#include <saadc-config.h>

#include <stdint.h>

#include "saadc/interface/saadcobserver.h"
#include "isubject.hpp"


/* **************************************************************************************************************** */
/*                                                                                                                  */
/* APPLICATION defintion                                                                                            */
/*                                                                                                                  */
/* **************************************************************************************************************** */

/* **************************************************************************************************************** */
/* --- Extern "C" Section ---                                                                                       */
/*     Callback's section                                                                                           */
/* **************************************************************************************************************** */
#if defined( __cplusplus )
extern "C"
{
#endif
    namespace hal
    {
        /********************************************************************************************************//**
         * \brief   Function for handling the SAADC peripheral events
         *
         * \details This function will be called each time the SAADC peripheral creates an event.
         *
         * \param   pEvent  Pointer used for passing the current SAADC Event to the event handler.
         ************************************************************************************************************/
        void saadcEventHandler(nrfx_saadc_evt_t const* pEvent);

    } // namespace hal

#if defined( __cplusplus )
}
#endif


/* **************************************************************************************************************** */
/*                                                                                                                  */
/* NAMESPACE Declaration                                                                                            */
/*                                                                                                                  */
/* **************************************************************************************************************** */
namespace hal
{
    /* ************************************************************************************************************ */
    /*                                                                                                              */
    /* Class Declaration                                                                                            */
    /*                                                                                                              */
    /* ************************************************************************************************************ */
    class SaadcHal : public ISubject<SaadcObserver>
    {
        friend void saadcEventHandler(nrfx_saadc_evt_t const* pEvent);

    public:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* CONSTRUCTOR SECTION                                                                                      */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        SaadcHal();
        ~SaadcHal();

        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PUBLIC DECLARATION SECTION                                                                               */
        /*                                                                                                          */
        /* ******************************************************************************************************** */

        /********************************************************************************************************//**
         * \brief   Initialize the SAADC hardware according to the given parameters.
         *
         * Initialize the SAADC hardware according to the given parameters. If one of them or both of them are `NULL`
         * the default configuration is used.
         *
         * \param   config          Configutation to be used for the SAADC hardware
         * \param   channelConfig   Configuration of the first channel of the SAADC (channel: 0)
         *
         * \return  `true` if the initialization of the SAADC succeeded and `false` otherwise.
         ************************************************************************************************************/
        bool init(nrfx_saadc_config_t* config = NULL, nrf_saadc_channel_config_t* channelConfig = NULL);

        /********************************************************************************************************//**
         * \brief   Returns initialization's state of the SAADC.
         ************************************************************************************************************/
        inline bool isInitialized() const           { return _isInitialized; }

        /********************************************************************************************************//**
         * \brief   Uninitialize the SAADC.
         ************************************************************************************************************/
        inline bool uninit()                        { return _uninitSaadc(); }

        /********************************************************************************************************//**
         * \brief   Trigger the SAADC SAMPLE task
         ************************************************************************************************************/
        bool sample();

        /********************************************************************************************************//**
         * \brief   Handles the SAADC Event coming from the driver
         ************************************************************************************************************/
        void onSaadcEvent(nrfx_saadc_evt_t const* pEvent);

    private:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PRIVATE DECLARATION SECTION                                                                              */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        /************************************************************************************************************
         * \brief   Set the default config for the SAADC and one channel
         *
         * Setup the default configuration of the SAADC. The default parameters are:
         *  - 12 bit resolution
         *  - Over sample 4x
         *  - interrupt to low priority
         *  - ...
         ************************************************************************************************************/
        void _setDefaultConfig();

        /************************************************************************************************************
         * \brief   Set up the SAADC configuration according to the given one.
         *
         * Setup of the SAADC configuration. This configuration is defined by the given parameters `saadcConfig`.
         *
         * \param   saadcConfig     Configuration of the SAADC hardware
         ************************************************************************************************************/
        void _setSaadcConfig(nrfx_saadc_config_t* saadcConfig);

        /************************************************************************************************************
         * \brief   Set up the configuration of the SAADC's channel
         *
         * Setup the SAADC's channel. This configuration is defined by the given parameter `channelConfig`.
         *
         * \param   channelConfig  Configuration for the channel.
         ************************************************************************************************************/
        void _setChannelConfig(nrf_saadc_channel_config_t* channelConfig);

        /************************************************************************************************************
         * \brief   Initialize the SAADC hardware.
         *
         * \return  `true` or `false` according to the success of the process...
         ************************************************************************************************************/
        bool _initSaadc();

        /************************************************************************************************************
         * \brief   Uninitialize the SAADC hardware.
         *
         * \return  `true` or `false` according to the success of the process...
         ************************************************************************************************************/
        bool _uninitSaadc();

        /************************************************************************************************************
         * \brief   Initiliaze the SAADC's channel.
         *
         * Initialize the SAADC's channel defined by the given identifier.
         *
         * \param   channelId   Identifier of the channel to initiliaze.
         *
         * \return  `true` if the initialization of the channel succeeded and `false` otherwise.
         ************************************************************************************************************/
        bool _initChannel(uint8_t channelId);

        /************************************************************************************************************
         * \brief   unregister the SAADC's channel
         *
         * \param   channelId       The channel's identifier to uninit.
         ************************************************************************************************************/
        void _uninitChannel(uint8_t channelId);

        /************************************************************************************************************
         * \brief   Convert all buffers used by the SAADC
         ************************************************************************************************************/
        bool _setupSampleBuffers();
        
        /************************************************************************************************************
         * \brief   Notify all registered observers about the sampled values...
         ************************************************************************************************************/
        void _notifyObserverNewMeasure(nrf_saadc_value_t* buffer, uint16_t bufferSize);

        uint8_t                     _channelIds[NRF_SAADC_CHANNEL_COUNT];
        nrfx_saadc_config_t         _saadcConfig;
        nrf_saadc_channel_config_t  _channelConfig;
        nrf_saadc_value_t           _poolBuffer[2][APP_SAADC_SAMPLES_IN_BUFFER];
        bool                        _isInitialized;
        bool                        _isCalibrating;
        bool                        _isConfigSet;
    };  // Class saadcHAL

}  // Namepsace hal

#endif

/** @} */
