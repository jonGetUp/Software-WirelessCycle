/*************************************************************************************************************
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 *
 * \file    saadchal.cpp
 ************************************************************************************************************/

/************************************************************************************************************/
/* Header files                                                                                             */
/************************************************************************************************************/
#include "saadchal.h"

#include <factory.h>
#include <string.h>

#if (USE_SAADC != 0)

/************************************************************************************************************/
/* DEBUG COMPILATION OPTION & MACRO                                                                         */
/************************************************************************************************************/
#define DEBUG_SAADCHAL_ENABLE               0
#if (DEBUG_SAADCHAL_ENABLE != 0) && defined(DEBUG_NRF_USER)
    #include <tools/logtrace.h>
    #define SAH_DEBUG                       LOG_TRACE_DEBUG
#else
    #define SAH_DEBUG(...)                  {(void) (0);}
#endif

#define DEBUG_SAADCHAL_CB_ENABLE            0
#if (DEBUG_SAADCHAL_CB_ENABLE != 0) && defined(DEBUG_NRF_USER)
    #include <nrf_log.h>
    #define SAHC_DEBUG                      LOG_TRACE_DEBUG
#else
    #define SAHC_DEBUG(...)                 {(void) (0);}
#endif


/************************************************************************************************************/
/* Namespace's Declaration                                                                                  */
/************************************************************************************************************/
using hal::SaadcHal;


/************************************************************************************************************/
/* --- Extern "C" Section ---                                                                               */
/*     Callback's section                                                                                   */
/************************************************************************************************************/
// ----------------------------------------------------------------------------------------------------------
void hal::saadcEventHandler(nrfx_saadc_evt_t const* pEvent)
{
    #if (USE_SAADC != 0)
    Factory::saadcHal.onSaadcEvent(pEvent);
    #endif  // #if (USE_SAADC != 0)    
}

/************************************************************************************************************/
/* CONSTRUCTOR                                                                                              */
/************************************************************************************************************/
// ----------------------------------------------------------------------------------------------------------
SaadcHal::SaadcHal() : _isInitialized(false), _isCalibrating(false), _isConfigSet(false) {}

// ----------------------------------------------------------------------------------------------------------
SaadcHal::~SaadcHal() {}


/************************************************************************************************************/
/* PUBLIC SECTION                                                                                           */
/************************************************************************************************************/
// ----------------------------------------------------------------------------------------------------------
bool SaadcHal::init(nrfx_saadc_config_t* config, nrf_saadc_channel_config_t* channelConfig)
{
    if(((config == NULL) || (channelConfig == NULL)) && !_isConfigSet)
    {
        _setDefaultConfig();
    }
    else 
    {
        _setSaadcConfig(config);
        _setChannelConfig(channelConfig);
    }

    if(!_initSaadc())
    {
        SAH_DEBUG("[SaadcHal] init() > Error while initializing the SAADC !\r\n");
        return false;
    }
    
    if(!_initChannel(0))
    {
        SAH_DEBUG("[SaadcHal] init() > Error while initializing the SAADC's channel 0 !\r\n");
        return false;
    }

    _isInitialized = _setupSampleBuffers();
    return _isInitialized;
}

// ----------------------------------------------------------------------------------------------------------
bool SaadcHal::sample()
{
    if(_isInitialized)
    {
        SAHC_DEBUG("[SaadcHal] sample() > execute nrfx_saadc_sample() \r\n");
        nrfx_saadc_sample();
        return true;
    }

    SAHC_DEBUG("[SaadcHal] sample() > SAADC is not inialized !\r\n");
    return false;
}

// ----------------------------------------------------------------------------------------------------------
void SaadcHal::onSaadcEvent(nrfx_saadc_evt_t const* pEvent)
{
    static uint8_t eventCounter = 1;
    ret_code_t     errorCode    = NRF_SUCCESS;

    if(pEvent->type == NRFX_SAADC_EVT_DONE)
    {
        // Capture offset calibration complete event
        if(eventCounter > APP_SAADC_CALIBRATION_INTERVAL)
        {
            // Evaluate if offset calibration should be performed. 
            // Configure the SAADC_CALIBRATION_INTERVAL constant to change the calibration frequency
            SAH_DEBUG("[SaadcHal] saadcEventHandler() > SAADC calibration starting...\r\n");
            _isCalibrating = true;
            eventCounter   = 1;
            nrfx_saadc_abort();
            while(nrfx_saadc_calibrate_offset() != NRF_SUCCESS);
        }

        if(!_isCalibrating)
        {
            errorCode = nrfx_saadc_buffer_convert(pEvent->data.done.p_buffer, pEvent->data.done.size);
            if(errorCode == NRF_SUCCESS)
            {
                _notifyObserverNewMeasure(pEvent->data.done.p_buffer, pEvent->data.done.size);
                #if (DEBUG_SAADCHAL_ENABLE != 0)
                SAH_DEBUG("[SaadcHal] saadcEventHandler() > SAADC event number: %d\r\n", (int) eventCounter);
                for(uint8_t i = 0; i < pEvent->data.done.size; i++)
                {
                    SAH_DEBUG("[SaadcHal] saadcEventHandler() > SAADC value %d: %d\r\n", i, pEvent->data.done.p_buffer[i]);
                }
                #endif  // #if (DEBUG_SAADCHAL_ENABLE != 0)
            }
            else
            {
                SAH_DEBUG("[SaadcHal] saadcEventHandler() > nrfx_saadc_buffer_convert error: 0x%04x\r\n", errorCode);
            }
            eventCounter++;
            SAHC_DEBUG("[SaadcHal] saadcEventHandler() > Leaving `saadcEventHandler`\r\n");
        }
    }
    else if (pEvent->type == NRFX_SAADC_EVT_CALIBRATEDONE)
    {
        SAHC_DEBUG("[SaadcHal] saadcEventHandler() > `NRF_DRV_SAADC_EVT_CALIBRATEDONE` event's type\r\n");
        (void) _setupSampleBuffers();
        _isCalibrating = false;
    }
    else
    {
        SAHC_DEBUG("[SaadcHal] saadcEventHandler() > Event #%d NOT handled !\r\n", pEvent->type);
    }
}

/************************************************************************************************************/
/* PRIVATE SECTION                                                                                          */
/************************************************************************************************************/
// -----------------------------------------------------------------------------------------------------------
void SaadcHal::_setDefaultConfig()
{
    //Configure SAADC
    _saadcConfig.low_power_mode     = (bool) APP_SAADC_CONFIG_LP_MODE;                                                  // Enable or disable low power mode.
    _saadcConfig.resolution         = (nrf_saadc_resolution_t) APP_SAADC_CONFIG_RESOLUTION;                             // Set up the SAADC resolution
    _saadcConfig.oversample         = (nrf_saadc_oversample_t) APP_SAADC_CONFIG_OVERSAMPLE;                             // Set up the oversample. If SAADC_CONFIG_OVERSAMPLE is 4, this will make the SAADC output a single averaged value when the SAMPLE task is triggered 4 times.
    _saadcConfig.interrupt_priority = NRFX_SAADC_CONFIG_IRQ_PRIORITY;                                                   // Set SAADC interrupt to low priority.

    //Configure SAADC channel
    _channelConfig.reference        = NRF_SAADC_REFERENCE_INTERNAL;                                                     // Set internal reference of fixed 0.6 volts
    _channelConfig.gain             = NRF_SAADC_GAIN1_6;                                                                // Set input gain to 1/6. The maximum SAADC input voltage is then 0.6V/(1/6)=3.6V. The single ended input range is then 0V-3.6V
    _channelConfig.acq_time         = NRF_SAADC_ACQTIME_10US;                                                           // Set acquisition time. Set low acquisition time to enable maximum sampling frequency of 200kHz. Set high acquisition time to allow maximum source resistance up to 800 kohm, see the SAADC electrical specification in the PS.
    _channelConfig.mode             = NRF_SAADC_MODE_SINGLE_ENDED;                                                      // Set SAADC as single ended. This means it will only have the positive pin as input, and the negative pin is shorted to ground (0V) internally.
    _channelConfig.burst            = (APP_SAADC_BURST_MODE == 0) ? NRF_SAADC_BURST_DISABLED : NRF_SAADC_BURST_ENABLED; // Enable or disable Burst mode (disabled: normal operation). 
    _channelConfig.pin_p            = NRF_SAADC_INPUT_AIN0;                                                             // Select the input pin for the channel. AIN0 pin maps to physical pin P0.02.
    _channelConfig.pin_n            = NRF_SAADC_INPUT_DISABLED;                                                         // Since the SAADC is single ended, the negative pin is disabled. The negative pin is shorted to ground internally.
    _channelConfig.resistor_p       = NRF_SAADC_RESISTOR_DISABLED;                                                      // Disable pullup resistor on the input pin
    _channelConfig.resistor_n       = NRF_SAADC_RESISTOR_DISABLED;                                                      // Disable pulldown resistor on the input pin
    
    _isConfigSet = true;
}

// ----------------------------------------------------------------------------------------------------------
void SaadcHal::_setSaadcConfig(nrfx_saadc_config_t* saadcConfig)
{
    SAH_DEBUG("[SaadcHal] _setSaadcConfig() > low power mode: %d, resolution: %d, oversampling: %d, priority: %d\r\n", 
              saadcConfig->low_power_mode, saadcConfig->resolution, saadcConfig->oversample, saadcConfig->interrupt_priority);
    _saadcConfig.low_power_mode     = saadcConfig->low_power_mode;
    _saadcConfig.resolution         = saadcConfig->resolution;
    _saadcConfig.oversample         = saadcConfig->oversample;
    _saadcConfig.interrupt_priority = saadcConfig->interrupt_priority;
    SAH_DEBUG("[SaadcHal] _setSaadcConfig() > New config: low power mode: %d, resolution: %d, oversampling: %d, priority: %d\r\n", 
              _saadcConfig.low_power_mode, _saadcConfig.resolution, _saadcConfig.oversample, _saadcConfig.interrupt_priority);
    
}

// ----------------------------------------------------------------------------------------------------------
void SaadcHal::_setChannelConfig(nrf_saadc_channel_config_t* channelConfig)
{
    _channelConfig.reference    = channelConfig->reference;
    _channelConfig.gain         = channelConfig->gain;
    _channelConfig.acq_time     = channelConfig->acq_time;
    _channelConfig.mode         = channelConfig->mode;
    _channelConfig.burst        = channelConfig->burst;
    _channelConfig.pin_p        = channelConfig->pin_p;
    _channelConfig.pin_n        = channelConfig->pin_n;
    _channelConfig.resistor_p   = channelConfig->resistor_p;
    _channelConfig.resistor_n   = channelConfig->resistor_n;
}

// ----------------------------------------------------------------------------------------------------------
bool SaadcHal::_initSaadc()
{    
    ret_code_t errCode = nrfx_saadc_init(&_saadcConfig, hal::saadcEventHandler);
    if(errCode != NRF_SUCCESS)
    {
        SAH_DEBUG("[SaadcHal] _init() > nrfx_saadc_init error: 0x%04x \r\n", errCode);
        return false;
    }
    SAH_DEBUG("[SaadcHal] _init() > nrfx_saadc_init() succeeded ! \r\n");
    return true;
}

// ----------------------------------------------------------------------------------------------------------
bool SaadcHal::_uninitSaadc()
{
    if(_isInitialized)
    {
        nrfx_saadc_uninit();
        SAH_DEBUG("[SaadcHal] _uninitSaadc() > nrfx_saadc_uninit() succeeded ! \r\n");
        _isInitialized = false;
        return true;
    }
    return false;
}

// ----------------------------------------------------------------------------------------------------------
bool SaadcHal::_initChannel(uint8_t channelId)
{
    //Initialize SAADC channel
    ret_code_t errorCode = nrfx_saadc_channel_init(channelId, &_channelConfig);
    if(errorCode != NRF_SUCCESS)
    {
        SAH_DEBUG("[SaadcHal] _initChannel() > nrfx_saadc_channel_init error: 0x%04x \r\n", errorCode);
        return false;
    }
    SAH_DEBUG("[SaadcHal] _initChannel() > succeeded ! \r\n");
    return true;
}

// ----------------------------------------------------------------------------------------------------------
void SaadcHal::_uninitChannel(uint8_t channelId)
{
    ret_code_t errCode = nrfx_saadc_channel_uninit(channelId);
    if(errCode != NRF_SUCCESS)
    {
        SAH_DEBUG("[SaadcHal] _uninitChannel() > nrfx_saadc_channel_uninit error: 0x%04x \r\n", errCode);
        
    }
}

// ----------------------------------------------------------------------------------------------------------
bool SaadcHal::_setupSampleBuffers()
{
    ret_code_t errCode = nrfx_saadc_buffer_convert(_poolBuffer[0], APP_SAADC_SAMPLES_IN_BUFFER);
    if(errCode != NRF_SUCCESS)
    {
        SAH_DEBUG("[SaadcHal] _bufferConvert() > buffer 0 nrfx_saadc_buffer_convert error: 0x%04x \r\n", errCode);
        return false;
    }

    errCode = nrfx_saadc_buffer_convert(_poolBuffer[1], APP_SAADC_SAMPLES_IN_BUFFER);
    if(errCode != NRF_SUCCESS)
    {
        SAH_DEBUG("[SaadcHal] _bufferConvert() > buffer 1 nrfx_saadc_buffer_convert error: 0x%04x \r\n", errCode);
        return false;
    }
    return true;
}

// ----------------------------------------------------------------------------------------------------------
void SaadcHal::_notifyObserverNewMeasure(nrf_saadc_value_t* buffer, uint16_t bufferSize)
{
    for(uint8_t i = 0; i < observersCount(); i++)
    {
        if(observer(i) != NULL)
        {
            observer(i)->onMeasureDone(this, buffer, bufferSize);
        }
    }
}

#endif
