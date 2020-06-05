/*************************************************************************************************************
 * Copyright (C) Soprod SA, Sion. 2018
 * Created by Thierry Hischier (thischier@soprod.ch)
 *
 * \file    pwmdriver.cpp
 * \brief   Contains all necessary functions and event handlers to drive a PWM peripheral
 *
 * Initial author: Thierry Hischier
 * Creation date: 2018-03-16
 ************************************************************************************************************/

/************************************************************************************************************/
/*                                                                                                          */
/* Header files                                                                                             */
/*                                                                                                          */
/************************************************************************************************************/
#include "pwmdriver.h"
#include "app_util_platform.h"
#include "factory.h"

#include "hal/gpio/gpiohal.h"
#include "gpio/gpioid.h"

/************************************************************************************************************/
/* DEBUG COMPILATION OPTION & MACRO                                                                         */
/************************************************************************************************************/
#define DEBUG_PWM_DRIVER_ENABLE             1
#if (DEBUG_PWM_DRIVER_ENABLE != 0)
    #include <tools/logtrace.h>
    #define PWMD_DEBUG                      LOG_TRACE_DEBUG
#else
    #define PWMD_DEBUG(...)                 {(void) (0);}
#endif


/************************************************************************************************************/
/* Namespace                                                                                                */
/************************************************************************************************************/
using pwm::PwmDriver;
    
/************************************************************************************************************/
/* Declaration section                                                                                      */
/************************************************************************************************************/

/************************************************************************************************************/
/* --- Extern "C" Section ---                                                                               */
/*     Callback's section                                                                                   */
/************************************************************************************************************/
// -----------------------------------------------------------------------------------------------------------
void pwm::pwmTimerHandler(nrf_timer_event_t timerEvent, void* pContext)
{
    if(pContext != NULL)
    {
        PwmDriver* pwmDriver = (reinterpret_cast<PwmDriver*>(pContext));
        switch(timerEvent)
        {
            case NRF_TIMER_EVENT_COMPARE0:
                if(pwmDriver->_duration != 0)
                {
                    if(pwmDriver->_durationCounter < pwmDriver->_durationCycles)
                    {
                        hal::GpioHal::pinSet(pwmDriver->_outputPin);
                        pwmDriver->_durationCounter++;
                    }
                    else
                    {
                        nrfx_timer_disable(&pwmDriver->_timerInstance);
                    }
                }
                else
                {
                    hal::GpioHal::pinSet(pwmDriver->_outputPin);
                }
                break;
            
            case NRF_TIMER_EVENT_COMPARE1:
                hal::GpioHal::pinClear(pwmDriver->_outputPin);
                break;
            
            case NRF_TIMER_EVENT_COMPARE2:
                break;
            
            case NRF_TIMER_EVENT_COMPARE3:
                break;

            default:
                break;
        }
    }
    else
    {
        ASSERT(false);
    }
}
    
/************************************************************************************************************/
/* Constructor | Destructor                                                                                 */
/************************************************************************************************************/
// -----------------------------------------------------------------------------------------------------------
PwmDriver::PwmDriver() {}

// -----------------------------------------------------------------------------------------------------------
PwmDriver::~PwmDriver() {}


/************************************************************************************************************/
/*                                              PUBLIC SECTION                                              */
/************************************************************************************************************/
// -----------------------------------------------------------------------------------------------------------
void PwmDriver::initialize()
{
    _frequency = 0;
    _frequencyTicks = 0;
    _dutyCycle = 0;
    _dutyCycleTicks = 0;
    _outputPin = 0;
    _duration = 0;
    _durationCycles = 0;
    _durationCounter = 0;

    // Initialise TIMER structure
    _timerConfig.frequency = (nrf_timer_frequency_t)NRFX_TIMER_DEFAULT_CONFIG_FREQUENCY;
    _timerConfig.mode = (nrf_timer_mode_t)NRFX_TIMER_DEFAULT_CONFIG_MODE;
    _timerConfig.bit_width = (nrf_timer_bit_width_t)NRFX_TIMER_DEFAULT_CONFIG_BIT_WIDTH;
    _timerConfig.interrupt_priority = NRFX_TIMER_DEFAULT_CONFIG_IRQ_PRIORITY;
    _timerConfig.p_context = this;
    
    _isInitialised = false;
    _isPWMRunning = false;
    PWMD_DEBUG("[PwmDriver] initialize() > Initialise Done...\r\n");
}

// -----------------------------------------------------------------------------------------------------------
void PwmDriver::bind(TIMERInstance::eTIMERInstance instance)
{
    switch(instance)
    {
        #if (NRFX_TIMER0_ENABLED == 1)
        case TIMERInstance::TIMER0:
            _timerInstance.p_reg = NRF_TIMER0;
            _timerInstance.instance_id = NRFX_TIMER0_INST_IDX;
            _timerInstance.cc_channel_count = TIMER0_CC_NUM;
            break;
        #endif

        #if (NRFX_TIMER1_ENABLED == 1)
        case TIMERInstance::TIMER1:
            _timerInstance.p_reg = NRF_TIMER1;
            _timerInstance.instance_id = NRFX_TIMER1_INST_IDX;
            _timerInstance.cc_channel_count = TIMER1_CC_NUM;
            break;
        #endif

        #if (NRFX_TIMER2_ENABLED == 1)
        case TIMERInstance::TIMER2:
            _timerInstance.p_reg = NRF_TIMER2;
            _timerInstance.instance_id = NRFX_TIMER2_INST_IDX;
            _timerInstance.cc_channel_count = TIMER2_CC_NUM;
            break;
        #endif
        
        #if (NRFX_TIMER3_ENABLED == 1)
        case TIMERInstance::TIMER1:
            _timerInstance.p_reg = NRF_TIMER3;
            _timerInstance.instance_id = NRFX_TIMER3_INST_IDX;
            _timerInstance.cc_channel_count = TIMER3_CC_NUM;
            break;
        #endif

        #if (NRFX_TIMER4_ENABLED == 1)
        case TIMERInstance::TIMER2:
            _timerInstance.p_reg = NRF_TIMER4;
            _timerInstance.instance_id = NRFX_TIMER4_INST_IDX;
            _timerInstance.cc_channel_count = TIMER4_CC_NUM;
            break;
        #endif

        default:
            // do nothing
            break;
    }
    PWMD_DEBUG("[PwmDriver] bind() > Bind Done...\r\n");
}

// -----------------------------------------------------------------------------------------------------------
void PwmDriver::setFrequencyDutyCycle(uint32_t frequency, uint8_t dutyCycle)
{
    PWMD_DEBUG("[PwmDriver] setFrequencyDutyCycle() > pwmFrequency: %d and pwmDutyCycle: %d\r\n", frequency, dutyCycle);
    
    setFrequency(frequency);
    setDutyCycle(dutyCycle);
    
    PWMD_DEBUG("[PwmDriver] setFrequencyDutyCycle() > Frequency: %d, Duty Cycle: %d!\r\n", _frequency, _dutyCycle);

    PWMD_DEBUG("[PwmDriver] setFrequencyDutyCycle() > Done!\r\n");
}

// -----------------------------------------------------------------------------------------------------------
void PwmDriver::setFrequency(uint32_t frequency)
{
    uint32_t errCode = NRF_SUCCESS;
    
    if(_isPWMRunning)
    {
        stop();
    }
    if(frequency >= 1 || frequency <= 16000000)
    {
        if(frequency != _frequency)
        {
            if(_isInitialised)
            {
                nrfx_timer_uninit(&_timerInstance);
                _isInitialised = false;
            }
            
            if(frequency >= 250000)
            {
                _timerConfig.frequency = (nrf_timer_frequency_t)NRF_TIMER_FREQ_16MHz;
            }
            else
            {
                _timerConfig.frequency = (nrf_timer_frequency_t)NRF_TIMER_FREQ_250kHz;
            }
            errCode = nrfx_timer_init(&_timerInstance, &_timerConfig, pwm::pwmTimerHandler);
            if(errCode != NRF_SUCCESS)
            {
                PWMD_DEBUG("[PwmDriver] setFrequency() > nrfx_timer_init() FAILED (error: 0x%04x)\r\n", errCode);
                ASSERT(false);
            }
            _isInitialised = true;
            _frequency = frequency;
            PWMD_DEBUG("[PwmDriver] setFrequency() > done!\r\n");
        }
    }
    else
    {
        PWMD_DEBUG("[PwmDriver] setFrequency() > Frequency out of possible values. Nothing is done...!\r\n");
    }
}

// -----------------------------------------------------------------------------------------------------------
void PwmDriver::setDutyCycle(uint8_t dutyCycle)
{
    if(_isPWMRunning)
    {
        stop();
    }
    if(_dutyCycle != dutyCycle)
    {
        if(dutyCycle > 100)
        {
            _dutyCycle = 100;
        }
        else if(dutyCycle <= 0)
        {
            _dutyCycle = 0;
        }
        else
        {
            _dutyCycle = dutyCycle;
        }
        PWMD_DEBUG("[PwmDriver] setDutyCycle() > done!\r\n");
    }
    else
    {
        PWMD_DEBUG("[PwmDriver] setDutyCycle() > Duty Cycle the same. Nothing to change!\r\n");
    }
}

// -----------------------------------------------------------------------------------------------------------
void PwmDriver::setDuration(uint32_t duration)
{
    if(_isPWMRunning)
    {
        stop();
    }
    if(_duration != duration)
    {
        if(duration > 65535)
        {
            _duration = 65535;
        }
        else if(duration <= 0)
        {
            _duration = 0;
        }
        else
        {
            _duration = duration;
        }
        PWMD_DEBUG("[PwmDriver] setDuration() > done!\r\n");
    }
    else
    {
        PWMD_DEBUG("[PwmDriver] setDuration() > Duration the same. Nothing to change!\r\n");
    }
}

// -----------------------------------------------------------------------------------------------------------
void PwmDriver::setOutputPin(uint8_t outputPin)
{
    _outputPin = outputPin;
}

// -----------------------------------------------------------------------------------------------------------
void PwmDriver::start()
{
    if(_isPWMRunning)
    {
        stop();
    }
    
    // Calculation of needed ticks for the frequency
    if(_frequency < 250000)
    {
        float frequencyUS = (1000000.0 / (4 * _frequency));
        _frequencyTicks = (uint32_t)frequencyUS;
        nrfx_timer_us_to_ticks(&_timerInstance, _frequencyTicks);
        
        // Calculation of needed ticks for the duty cycle
        _dutyCycleTicks = (uint32_t)(_frequencyTicks * (_dutyCycle / 100.0));
        
        // Calculation of the repeats if a duration is given
        if(_duration != 0)
        {
            _durationCycles = _duration * _frequency;
            _durationCounter = 0;
        }
        else
        {
            _durationCycles = 0;
            _durationCounter = 0;
        }
        
        // Setup the different compare registers
        nrfx_timer_compare(&_timerInstance, NRF_TIMER_CC_CHANNEL0, _dutyCycleTicks, true);
        nrfx_timer_extended_compare(&_timerInstance, NRF_TIMER_CC_CHANNEL1, _frequencyTicks, NRF_TIMER_SHORT_COMPARE1_CLEAR_MASK, true);
        nrfx_timer_enable(&_timerInstance);
        PWMD_DEBUG("[PwmDriver] start() > PWM started on GPIO: %d, Frequency: %d (Ticks: %d) and duty cycle: %d (Ticks: %d)!\r\n", _outputPin, _frequency, _frequencyTicks, _dutyCycle, _dutyCycleTicks);
        _isPWMRunning = true;
    }
}

// -----------------------------------------------------------------------------------------------------------
void PwmDriver::stop()
{
    if(_isPWMRunning)
    {
        nrfx_timer_disable(&_timerInstance);
        _isPWMRunning = false;
    }
}
