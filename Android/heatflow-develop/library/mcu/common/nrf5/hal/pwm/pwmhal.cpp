/************************************************************************************************************
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 ************************************************************************************************************/

/************************************************************************************************************/
/* Header files                                                                                             */
/************************************************************************************************************/
#include "pwmhal.h"

#include <string.h>
#include <nordic_common.h>
#include <nrf_gpio.h>

#include "tools/sdktools.h"
#include <factory.h>


/************************************************************************************************************/
/* DEBUG COMPILATION OPTION & MACRO                                                                         */
/************************************************************************************************************/
#define DEBUG_PWMHAL_ENABLE                 0
#if (DEBUG_PWMHAL_ENABLE == 1)
    #include <tools/logtrace.h>
    #define PWMH_DEBUG                      LOG_TRACE_DEBUG
#else
    #define PWMH_DEBUG(...)                 {(void) (0);}
#endif

#define DEBUG_PWMHAL_DEEP_ENABLE            0
#if (DEBUG_PWMHAL_DEEP_ENABLE != 0)
    #include <tools/logtrace.h>
    #define PWMHD_DEBUG                     LOG_TRACE_DEBUG
#else
    #define PWMHD_DEBUG(...)                {(void) (0);}
#endif


/************************************************************************************************************/
/* Namespace's Declaration                                                                                  */
/************************************************************************************************************/
using hal::PwmHal;


/************************************************************************************************************/
/* CONSTRUCTOR                                                                                              */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
PwmHal::PwmHal() : 
    _pwmId(PwmHal::UNKNOWN_PWM), _frequency(250), _pwmEvtHandler(NULL), _flags(0), _isInitialized(false)
{
    // Some intialization...
    memset(&_pwmInstance, 0, sizeof(nrfx_pwm_t));
    memset(&_pwmConfig,   0, sizeof(nrfx_pwm_config_t));
    memset( _outputs, NRFX_PWM_PIN_NOT_USED, NRF_PWM_CHANNEL_COUNT * sizeof(uint8_t));
    memset( _sequences,   0, 2 * sizeof(nrf_pwm_sequence_t));
}

//------------------------------------------------------------------------------------------------------------
PwmHal::~PwmHal() {}


/************************************************************************************************************/
/* PUBLIC SECTION                                                                                           */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
void PwmHal::initilialize(ePwmId pwmId, nrf_pwm_dec_load_t loadMode)
{
    switch(pwmId)
    {
        case PwmHal::PWM0:
        {
        #if (NRFX_PWM0_ENABLED == 1)
            _pwmInstance.p_registers    = NRF_PWM0;
            _pwmInstance.drv_inst_idx   = NRFX_PWM0_INST_IDX;
            break;
        #else
            PWMH_DEBUG("[PwmHal] initilialize() > NRFX_PWM0_ENABLED must be set in the SDK configuration's file !\r\n");
            ASSERT(false);
            return;
        #endif // #if (NRFX_PWM0_ENABLED == 1)
        }
        case PwmHal::PWM1:
        {
        #if (NRFX_PWM1_ENABLED == 1)
            _pwmInstance.p_registers    = NRF_PWM1;
            _pwmInstance.drv_inst_idx   = NRFX_PWM1_INST_IDX;
            break;
        #else
            PWMH_DEBUG("[PwmHal] initilialize() > NRFX_PWM1_ENABLED must be set in the SDK configuration's file !\r\n");
            ASSERT(false);
            return;
        #endif // #if (NRFX_PWM1_ENABLED == 1)
        }
        case PwmHal::PWM2:
        {
        #if (NRFX_PWM2_ENABLED == 1)
            _pwmInstance.p_registers    = NRF_PWM2;
            _pwmInstance.drv_inst_idx   = NRFX_PWM2_INST_IDX;
            break;
        #else
            PWMH_DEBUG("[PwmHal] initilialize() > NRFX_PWM2_ENABLED must be set in the SDK configuration's file !\r\n");
            ASSERT(false);
            return;
        #endif // #if (NRFX_PWM2_ENABLED == 1)
        }
        default:
            PWMH_DEBUG("[PwmHal] initilialize() > A valid PWM id MUST be provided here !\r\n");
            ASSERT(false);
            return;
    }
    
    // Set up some basic configuration
    _pwmId                              = pwmId;
    _pwmConfig.irq_priority             = APP_IRQ_PRIORITY_LOWEST;
    _pwmConfig.count_mode               = NRF_PWM_MODE_UP;
    _pwmConfig.load_mode                = loadMode;
    _pwmConfig.step_mode                = NRF_PWM_STEP_AUTO;
    _pwmConfig.base_clock               = NRF_PWM_CLK_125kHz;
    _pwmConfig.top_value                = _baseFrequency() / _frequency; // default frequency: 250 Hz
}

//------------------------------------------------------------------------------------------------------------
void PwmHal::registerEventHandler(nrfx_pwm_handler_t pwmEvtHandler, uint32_t flags)
{
    _pwmEvtHandler  = pwmEvtHandler;
    _flags          = flags;
}
        
//------------------------------------------------------------------------------------------------------------
bool PwmHal::setFrequency(uint32_t frequency, bool updateBaseClock)
{
    if(frequency < 1 || frequency > _frequencyMax)
    {
        PWMH_DEBUG("[PwmHal] setFrequency() > Frequency is out of range !\r\n", frequency);
        return false;
    }
    
    _uninitPwm();
    if(updateBaseClock)
    {
        _setBaseClk(frequency);
    }
    _frequency                  = frequency;
    _pwmConfig.top_value        = tools::SDKTools::roundedDiv((uint64_t) _baseFrequency(), (uint64_t) _frequency); 
    return true;
}

// -----------------------------------------------------------------------------------------------------------
void PwmHal::setOutputPin(uint8_t* output, size_t size, bool inverted)
{
    uint8_t len = MIN(NRF_PWM_CHANNEL_COUNT, size);
    
    _uninitPwm();
    memset(_pwmConfig.output_pins, NRFX_PWM_PIN_NOT_USED, NRF_PWM_CHANNEL_COUNT * sizeof(uint8_t));
    for(uint8_t i = 0; i < len; i++)
    {
        if(output[i] < NUMBER_OF_PINS)
        {
            _pwmConfig.output_pins[i] = output[i] | (inverted ? NRFX_PWM_PIN_INVERTED:0);
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
void PwmHal::addOutputPin(uint8_t  pinNbr, uint8_t idx, bool inverted)
{
    if(idx < NRF_PWM_CHANNEL_COUNT) 
    {
        _pwmConfig.output_pins[idx] = pinNbr | (inverted ? NRFX_PWM_PIN_INVERTED:0);
    }
}

// -----------------------------------------------------------------------------------------------------------
bool PwmHal::setSequenceValuesAndDurations(uint8_t    index, 
                                           uint16_t*  values, 
                                           size_t     size,
                                           uint32_t   duration,
                                           uint32_t   endDelay)
{
    return setSequenceValues(index, values, size) && setSequenceDurations(index, duration, endDelay);
}

// -----------------------------------------------------------------------------------------------------------
bool PwmHal::setSequenceValues(uint8_t    index, 
                               uint16_t*  values, 
                               size_t     size)
{
    if(index < APP_PWM_MAX_SEQUENCES_NUMBER)
    {
        switch(_pwmConfig.load_mode)
        {
            case NRF_PWM_LOAD_INDIVIDUAL:
                _sequences[index].values.p_individual   = (nrf_pwm_values_individual_t*) values;
                break;
            
            case NRF_PWM_LOAD_COMMON:
                _sequences[index].values.p_common       = (nrf_pwm_values_common_t*) values;
                break;
            
            case NRF_PWM_LOAD_GROUPED:
                _sequences[index].values.p_grouped      = (nrf_pwm_values_grouped_t*) values;
                break;
            
            case NRF_PWM_LOAD_WAVE_FORM:
                _sequences[index].values.p_wave_form    = (nrf_pwm_values_wave_form_t*) values;
                break;
            
            default:
                _sequences[index].values.p_raw          = values;
                break;
        }
        _sequences[index].length                        = size;
        PWMHD_DEBUG("[PwmHal] setSequenceValues() > Sequence #%d set: length = %2d\r\n", index, _sequences[index].length, _sequences[index].repeats);
        return true;
    }
    return false;
}

// -----------------------------------------------------------------------------------------------------------
bool PwmHal::setSequenceDurations(uint8_t index, uint32_t duration, uint32_t endDelay)
{
    if(index < APP_PWM_MAX_SEQUENCES_NUMBER)
    {
        _sequences[index].repeats                       = tools::SDKTools::roundedDiv(duration * _frequency, 1000);
        _sequences[index].end_delay                     = tools::SDKTools::roundedDiv(endDelay * _frequency, 1000);
        PWMHD_DEBUG("[PwmHal] setSequenceDurations() > Sequence #%d set: repeats = %3d, end_delay = %4d.\r\n", index, _sequences[index].repeats, _sequences[index].end_delay);
        return true;
    }
    return false;
}

        
// -----------------------------------------------------------------------------------------------------------
void PwmHal::clearSequenceValues(uint8_t index)
{
    if(index < APP_PWM_MAX_SEQUENCES_NUMBER)
    {
        _sequences[index].end_delay = 0;
        _sequences[index].length    = 0;
        _sequences[index].repeats   = 0;
        switch(_pwmConfig.load_mode)
        {
            case NRF_PWM_LOAD_INDIVIDUAL:
                _sequences[index].values.p_individual   = NULL;
                break;
            
            case NRF_PWM_LOAD_COMMON:
                _sequences[index].values.p_common       = NULL;
                break;
            
            case NRF_PWM_LOAD_GROUPED:
                _sequences[index].values.p_grouped      = NULL;
                break;
            
            case NRF_PWM_LOAD_WAVE_FORM:
                _sequences[index].values.p_wave_form    = NULL;
                break;
            
            default:
                _sequences[index].values.p_raw          = NULL;
                break;
        }
        PWMHD_DEBUG("[PwmHal] clearSequenceValues() > Sequence #%d cleared.\r\n", index);
    }
}

// -----------------------------------------------------------------------------------------------------------
bool PwmHal::start(uint16_t playbackCount, bool loop)
{
    _uninitPwm();
    uint32_t errCode = nrfx_pwm_init(&_pwmInstance, &_pwmConfig, _pwmEvtHandler);
    if(errCode != NRF_SUCCESS)
    {
        PWMH_DEBUG("[PwmHal] start() > nrfx_pwm_init() failed ! (errCode: 0x%04x)\r\n", errCode);
        return false;
    }
    _isInitialized  = true;
    
    uint32_t flags  = (loop ? NRFX_PWM_FLAG_LOOP:NRFX_PWM_FLAG_STOP) + _flags;
    if(_sequences[1].length > 0)
    {
        errCode = nrfx_pwm_complex_playback(&_pwmInstance, &_sequences[0], &_sequences[1], playbackCount, flags);
        PWMHD_DEBUG("[PwmHal] start() > Start nrfx_pwm_complex_playback!\r\n");
    }
    else
    {
        errCode = nrfx_pwm_simple_playback(&_pwmInstance, &_sequences[0], playbackCount, flags);
        PWMHD_DEBUG("[PwmHal] start() > Start nrfx_pwm_simple_playback!\r\n");
    }
    
    if(errCode != NRF_SUCCESS)
    {
        PWMH_DEBUG("[PwmHal] start() > nrfx_pwm_%s_playback() failed ! (errCode: 0x%04x)\r\n", (_sequences[1].length > 0) ? "complex":"simple", errCode);
        return false;
    }
    return true;
}


/************************************************************************************************************/
/* PRIVATE SECTION                                                                                          */
/************************************************************************************************************/
// -----------------------------------------------------------------------------------------------------------
void PwmHal::_uninitPwm()
{
    if(_isInitialized)
    {
        // Function for uninitializing the PWM driver. 
        // If any sequence playback is in progress, it is stopped immediately.
        nrfx_pwm_uninit(&_pwmInstance);
        _isInitialized  = false;
        PWMHD_DEBUG("[PwmHal] _uninitPwm() > PWM Uninitialized!\r\n");
    }
}

// -----------------------------------------------------------------------------------------------------------
void PwmHal::_setBaseClk(uint32_t frequency)
{
    if(frequency >= 32000)
    {
        PWMH_DEBUG("[PwmHal] _setBaseClk() > BaseClock set to 16 MHz.\r\n");
        _pwmConfig.base_clock   = NRF_PWM_CLK_16MHz;
    }
    else if(frequency >= 16000)
    {
        PWMH_DEBUG("[PwmHal] _setBaseClk() > BaseClock set to 8 MHz.\r\n");
        _pwmConfig.base_clock   = NRF_PWM_CLK_8MHz;
    }
    else if(frequency >= 8000)
    {
        PWMH_DEBUG("[PwmHal] _setBaseClk() > BaseClock set to 4 MHz.\r\n");
        _pwmConfig.base_clock   = NRF_PWM_CLK_4MHz;
    }
    else if(frequency >= 4000)
    {
        PWMH_DEBUG("[PwmHal] _setBaseClk() > BaseClock set to 2 MHz.\r\n");
        _pwmConfig.base_clock   = NRF_PWM_CLK_2MHz;
    }
    else if(frequency >= 2000)
    {
        PWMH_DEBUG("[PwmHal] _setBaseClk() > BaseClock set to 1 MHz.\r\n");
        _pwmConfig.base_clock   = NRF_PWM_CLK_1MHz;
    }
    else if(frequency >= 1000)
    {
        PWMH_DEBUG("[PwmHal] _setBaseClk() > BaseClock set to 500 kHz.\r\n");
        _pwmConfig.base_clock   = NRF_PWM_CLK_500kHz;
    }
    else if(frequency >= 500)
    {
        PWMH_DEBUG("[PwmHal] _setBaseClk() > BaseClock set to 250 kHz.\r\n");
        _pwmConfig.base_clock   = NRF_PWM_CLK_250kHz;
    }
    else
    {
        PWMH_DEBUG("[PwmHal] _setBaseClk() > BaseClock set to 125 kHz.\r\n");
        _pwmConfig.base_clock   = NRF_PWM_CLK_125kHz;
    }
}

// -----------------------------------------------------------------------------------------------------------
uint8_t PwmHal::_outputPin(uint8_t* index)
{
    *index = MIN(NRF_PWM_CHANNEL_COUNT-1, *index);
    return _outputs[*index];
}

