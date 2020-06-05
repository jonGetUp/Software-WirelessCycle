/*************************************************************************************************************
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * All rights reserved.
 *
 * \file    ledhal.cpp
 *
 * Initial author: Patrice Rudaz
 * Creation date: 2016-06-16
 *
 * \author  Patrice Rudaz (patrice.rudaz@hevs.ch)
 * \date    February 2018
 ************************************************************************************************************/

/************************************************************************************************************/
/* Header files                                                                                             */
/************************************************************************************************************/
#include "ledhal.h"
#include "gpio/gpiohal.h"

#include <nrf.h>
#include <string.h>
#include <platform-config.h>
#include <boards.h>


/************************************************************************************************************/
/* DEBUG COMPILATION OPTION & MACRO                                                                         */
/************************************************************************************************************/
#define DEBUG_LEDHAL_ENABLE                 0
#if (DEBUG_LEDHAL_ENABLE == 1) && defined(DEBUG_NRF_USER)
    #include <tools/logtrace.h>
    #define LH_DEBUG                        LOG_TRACE_DEBUG
#else
    #define LH_DEBUG(...)                   {(void) (0);}
#endif



/************************************************************************************************************/
/* Namespace's Declaration                                                                                  */
/************************************************************************************************************/
using hal::LedHal;

/************************************************************************************************************/
/* PUBLIC SECTION                                                                                           */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
// static
bool LedHal::initLeds()
{
    #if (USE_LEDS != 0)
    // DEBUG
    LH_DEBUG("[LedHal] initLeds() > Set up GPIOs as output ");

    // Configure LED-pins as outputs
    uint8_t ledsList[] = LEDS_LIST;
    for(uint8_t i = 0; i < sizeof(ledsList); i++)
    {
        LH_DEBUG((char*)".");
        hal::GpioHal::cfgPinOutput(ledsList[i],
                                   GPIO_PIN_CNF_DRIVE_S0S1,         // GPIO_PIN_CNF_DRIVE_H0H1
                                   GPIO_PIN_CNF_PULL_Disabled,
                                   GPIO_PIN_CNF_INPUT_Disconnect);
    }
    LH_DEBUG((char*)" Done!\r\n");
    return true;
    #else
    // DEBUG
    LH_DEBUG("[LedHal] initLeds() > LEDs are desactivated (`platform-config.h`)\r\n");
    return false;
    #endif
}

//------------------------------------------------------------------------------------------------------------
// static
bool LedHal::initLed(uint8_t index)
{
    #if (USE_LEDS != 0)
    // DEBUG
    LH_DEBUG("[LedHal] initLED() > Set up GPIO for LED %d as output\r\n", index);

    // Configure LED-pins as outputs
    uint8_t ledsList[] = LEDS_LIST;
    if(index < sizeof(ledsList))
    {
        hal::GpioHal::cfgPinOutput(ledsList[index],
                                   GPIO_PIN_CNF_DRIVE_S0S1,         // GPIO_PIN_CNF_DRIVE_H0H1,
                                   GPIO_PIN_CNF_PULL_Disabled,
                                   GPIO_PIN_CNF_INPUT_Disconnect);
        return true;
    }
    else
    {
        // DEBUG
        LH_DEBUG("[LedHal] initLed() > `index` %d is out of bound!\r\n", index);
    }
    #else
    // DEBUG
    LH_DEBUG("[LedHal] initLed() > LEDs are desactivated (`platform-config.h`)\r\n");
    #endif
    return false;
}

//------------------------------------------------------------------------------------------------------------
// static
bool LedHal::setLedOff(uint8_t index)
{
    // DEBUG
    LH_DEBUG("[LedHal] setLEDxOff() > LED %d OFF\r\n", index);

    // Switch OFF the LED
    return _switchOnOff(index, false);
}

//------------------------------------------------------------------------------------------------------------
// static
bool LedHal::setLedOn(uint8_t index)
{
    // DEBUG
    LH_DEBUG("[LedHal] setLEDxOn()  > LED %d ON\r\n", index);

    // Switch OFF the LED
    return _switchOnOff(index, true);
}

//------------------------------------------------------------------------------------------------------------
// static
bool LedHal::toggleLed(uint8_t index)
{
    #if (USE_LEDS != 0)
    // DEBUG
    LH_DEBUG("[LedHal] toggleLed() > LED %d\r\n", index);

    // Configure LED-pins as outputs
    uint8_t ledsList[] = LEDS_LIST;
    if(index < sizeof(ledsList))
    {
        hal::GpioHal::pinToggle(ledsList[index]);
        return true;
    }
    else
    {
        // DEBUG
        LH_DEBUG("[LedHal] toggleLed() > `index` %d is out of bound!\r\n", index);
    }
    #else
    // DEBUG
    LH_DEBUG("[LedHal] toggleLed() > LEDs are desactivated (`platform-config.h`)\r\n");
    #endif
    return false;
}


/************************************************************************************************************/
/* PRIVATE SECTION                                                                                          */
/************************************************************************************************************/
// -----------------------------------------------------------------------------------------------------------
// static
bool LedHal::_switchOnOff(uint8_t index, bool enable)
{
    #if (USE_LEDS != 0)
    // Configure LED-pins as outputs
    uint8_t ledsList[] = LEDS_LIST;
    if(index < sizeof(ledsList))
    {
        if(enable)
        {
            _switchOn(ledsList[index]);
        }
        else
        {
            _switchOff(ledsList[index]);
        }
        return true;
    }
    else
    {
        // DEBUG
        LH_DEBUG("[LedHal] _switchOnOff() > `index` %02d is out of bound!\r\n", index);
    }
    #else
    // DEBUG
    LH_DEBUG("[LedHal] _switchOnOff() > LEDs are desactivated (`platform-config.h`)\r\n");
    #endif
    return false;
}

//------------------------------------------------------------------------------------------------------------
// static
void LedHal::_switchOn(uint8_t pinNbr)
{
    #if (LED_ACTIVE_LOW != 0)
    hal::GpioHal::pinClear(pinNbr);
    #else
    hal::GpioHal::pinSet(pinNbr);
    #endif
}

//------------------------------------------------------------------------------------------------------------
// static
void LedHal::_switchOff(uint8_t pinNbr)
{
    #if (LED_ACTIVE_LOW != 0)
    hal::GpioHal::pinSet(pinNbr);
    #else
    hal::GpioHal::pinClear(pinNbr);
    #endif
}
