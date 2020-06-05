/*************************************************************************************************************
 * Copyright (C) Soprod SA, Sion. 2018
 * Created by Thierry Hischier (thischier@soprod.ch)
 *
 * \file    pwmdriver.h
 * \brief   Class that manages a Software-PWM (TIMER peripheral)
 *
 * Initial author: Thierry Hischier
 * Creation date: 2018-03-16
 ************************************************************************************************************/
#pragma once


/************************************************************************************************************/
/*                                                                                                          */
/* Header files                                                                                             */
/*                                                                                                          */
/************************************************************************************************************/
#include "platform-config.h"
#include "timerinstance.h"

#include <nrfx_timer.h>
#include <nrf_gpio.h>

#if defined(__cplusplus)
extern "C" {
#endif
namespace pwm 
{
    /*********************************************************************************************************
     * \brief         TIMER's event handler.
     *
     * This function will be called each time the used TIMER throws an event.
     *
     * \param   eventType   The event type of the used RTC.
     * \param   pContext    Pointer used for passing some arbitrary information (context) to the event handler.
     ********************************************************************************************************/
    void pwmTimerHandler(nrf_timer_event_t timerEvent, void* pContext);
} // namespace pwm
#if defined(__cplusplus)
}
#endif

/************************************************************************************************************/
/*                                                                                                          */
/* TYPEDEF DECLARATION                                                                                      */
/*                                                                                                          */
/************************************************************************************************************/

/************************************************************************************************************/
/*                                                                                                          */
/* Namespace Declaration                                                                                    */
/*                                                                                                          */
/************************************************************************************************************/
namespace pwm {

/************************************************************************************************************/
/*                                                                                                          */
/* Class Declaration                                                                                        */
/*                                                                                                          */
/************************************************************************************************************/
class PwmDriver
{
    friend void pwmTimerHandler(nrf_timer_event_t timerEvent, void* pContext);
    
public:
    /********************************************************************************************************/
    /*                                            PUBLIC SECTION                                            */
    /********************************************************************************************************/
    PwmDriver();
    virtual ~PwmDriver();

    void initialize();
    void bind(TIMERInstance::eTIMERInstance instance);

    void setFrequencyDutyCycle(uint32_t frequency, uint8_t dutyCycle);
   
    /*********************************************************************************************************
     * \brief   Set the frequency for the PWM peripheral (in Hz).
     ********************************************************************************************************/
    void setFrequency(uint32_t frequency);
    inline uint32_t frequency()     {return _frequency;}

    /*********************************************************************************************************
     * \brief   Set the duty cycle for the PWM peripheral (in %).
     ********************************************************************************************************/
    void setDutyCycle(uint8_t dutyCycle);
    inline uint8_t dutyCycle()      {return _dutyCycle;}
    
    /*********************************************************************************************************
     * \brief   Set the duration for the PWM peripheral (in seconds). If the duration is set to 0, 
     *          the PWM peripheral is running in an infinite while loop.
     ********************************************************************************************************/
    void setDuration(uint32_t duration);
    inline uint32_t duration()      {return _duration;}
    
    /*********************************************************************************************************
     * \brief   Set the output pin for the PWM peripheral.
     ********************************************************************************************************/
    void setOutputPin(uint8_t outputPin);
    inline uint8_t outputPin()      {return _outputPin;}
    
    void start();
    void stop();

protected:
    /********************************************************************************************************/
    /*                                        PROTECTED SECTION                                             */
    /********************************************************************************************************/

private:
    /********************************************************************************************************/
    /*                                         PRIVATE SECTION                                              */
    /********************************************************************************************************/
    // Attributes
    uint32_t                _frequency;
    uint32_t                _frequencyTicks;
    uint8_t                 _dutyCycle;
    uint32_t                _dutyCycleTicks;
    uint32_t                _outputPin;
    uint32_t                _duration;
    uint16_t                _durationCounter;
    uint16_t                _durationCycles;

    nrfx_timer_t            _timerInstance;
    nrfx_timer_config_t     _timerConfig;

    bool                    _isInitialised;
    bool                    _isPWMRunning;

    // Methods

};  // class PwmDriver

};  // namespace pwm
