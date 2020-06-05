/************************************************************************************************************
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 ************************************************************************************************************/

/************************************************************************************************************/
/* Header files                                                                                             */
/************************************************************************************************************/
#include "gpiohal.h"
#include <gpio-config.h>

#include <nrf.h>
#if (GPIO_INIT_OUT_AND_IN != 0)
    #include <nrf_delay.h>
#endif


/************************************************************************************************************/
/* DEBUG COMPILATION OPTION & MACRO                                                                         */
/************************************************************************************************************/
#define DEBUG_GPIOHAL_ENABLE                0
#if (DEBUG_GPIOHAL_ENABLE == 1) && defined(DEBUG_NRF_USER)
    #include <tools/logtrace.h>
    #define GH_DEBUG                        LOG_TRACE_DEBUG
#else
    #define GH_DEBUG(...)                   {(void) (0);}
#endif


/************************************************************************************************************/
/* Namespace's Declaration                                                                                  */
/************************************************************************************************************/
using hal::GpioHal;


/************************************************************************************************************/
/* PUBLIC SECTION                                                                                           */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
// static
void GpioHal::init()
{
    uint32_t i = 0;
    #if (GPIO_INIT_OUT_AND_IN != 0)
    for(i = 0; i < NUMBER_OF_PINS; i++)
    {
        GH_DEBUG((char*) ".");
        nrf_gpio_cfg(i,
                     NRF_GPIO_PIN_DIR_OUTPUT,
                     NRF_GPIO_PIN_INPUT_DISCONNECT,
                     NRF_GPIO_PIN_NOPULL,
                     NRF_GPIO_PIN_H0H1,
                     NRF_GPIO_PIN_NOSENSE);
    }
    GH_DEBUG((char*) "-");
    NRF_P0->OUTCLR = 0xffffffff;
    #if (GPIO_COUNT == 2)
    NRF_P1->OUTCLR = 0xffffffff;
    #endif
    nrf_delay_ms(GPIO_INIT_OUT_AND_IN_DELAY);
    #endif  // #if (GPIO_INIT_OUT_AND_IN != 0)
    
    for(i = 0; i < NUMBER_OF_PINS; i++)
    {
        GH_DEBUG((char*) ".");
        nrf_gpio_cfg(i,
                     NRF_GPIO_PIN_DIR_INPUT,
                     NRF_GPIO_PIN_INPUT_DISCONNECT,
                     NRF_GPIO_PIN_NOPULL,
                     NRF_GPIO_PIN_S0S1,
                     NRF_GPIO_PIN_NOSENSE);
    }

    // DEBUG
    GH_DEBUG("[GpioHal] init() > Initialization of all GPIO ... Done !\r\n");
}

//------------------------------------------------------------------------------------------------------------
// static
void GpioHal::cfgPinInput(uint32_t pinNbr, uint32_t drive, uint32_t pullSetup, uint32_t connect)
{
    nrf_gpio_cfg(                                   pinNbr, 
                                                    NRF_GPIO_PIN_DIR_INPUT, 
                 static_cast<nrf_gpio_pin_input_t>  (connect), 
                 static_cast<nrf_gpio_pin_pull_t>   (pullSetup), 
                 static_cast<nrf_gpio_pin_drive_t>  (drive), 
                                                    GPIO_SENSE_SETUP);

    // DEBUG
    #if (APP_DEBUG_GPIO_PIN_XX != 0)
    if(pinNbr == APP_DEBUG_PIN_NUMBER)
    #endif
    {
        GH_DEBUG("[GpioHal] cfgPinInput() > Set pin %02u as INPUT, %s, drive %u, connect %u\r\n", 
                 pinNbr, (pullSetup == NRF_GPIO_PIN_NOPULL ? "No Pull":(pullSetup == NRF_GPIO_PIN_PULLUP ? "PullUp":"PullDown")), drive, connect);
    }
}

//------------------------------------------------------------------------------------------------------------
// static
void GpioHal::cfgPinInputNoPull(uint32_t pinNbr)
{
    cfgPinInput(pinNbr, GPIO_PIN_CNF_DRIVE_S0S1, GPIO_PIN_CNF_PULL_Disabled, GPIO_PIN_CNF_INPUT_Connect);
}

//------------------------------------------------------------------------------------------------------------
// static
void GpioHal::cfgPinInputPullup(uint32_t pinNbr)
{
    cfgPinInput(pinNbr, GPIO_PIN_CNF_DRIVE_S0S1, GPIO_PIN_CNF_PULL_Pullup, GPIO_PIN_CNF_INPUT_Connect);
}

//------------------------------------------------------------------------------------------------------------
// static
void GpioHal::cfgPinInputPullDown(uint32_t pinNbr)
{
    cfgPinInput(pinNbr, GPIO_PIN_CNF_DRIVE_S0S1, GPIO_PIN_CNF_PULL_Pulldown, GPIO_PIN_CNF_INPUT_Connect);
}

//------------------------------------------------------------------------------------------------------------
// static
void GpioHal::cfgPinOutput(uint32_t pinNbr, uint32_t drive, uint32_t pullSetup, uint32_t connect)
{
    nrf_gpio_cfg(                                   pinNbr, 
                                                    NRF_GPIO_PIN_DIR_OUTPUT, 
                 static_cast<nrf_gpio_pin_input_t>  (connect), 
                 static_cast<nrf_gpio_pin_pull_t>   (pullSetup), 
                 static_cast<nrf_gpio_pin_drive_t>  (drive), 
                                                    GPIO_SENSE_SETUP);

    // DEBUG
    #if (APP_DEBUG_GPIO_PIN_XX != 0)
    if(pinNbr == APP_DEBUG_PIN_NUMBER)
    #endif
    {
        GH_DEBUG("[GpioHal] cfgPinOutput() > Set pin %i as OUTPUT, %s, drive %u, connect %u\r\n", 
                 pinNbr, (pullSetup == NRF_GPIO_PIN_NOPULL ? "No Pull":(pullSetup == NRF_GPIO_PIN_PULLUP ? "PullUp":"PullDown")), drive, connect);
    }
}

//------------------------------------------------------------------------------------------------------------
// static
void GpioHal::cfgQuickOutput(uint32_t pinNbr, uint32_t drive)
{
    cfgPinOutput(pinNbr, drive, GPIO_PIN_CNF_PULL_Pulldown, GPIO_PIN_CNF_INPUT_Disconnect);
}

//------------------------------------------------------------------------------------------------------------
// static
void GpioHal::pinMaskClear(uint64_t pinMask)
{
    NRF_P0->OUTCLR      = (uint32_t) ( pinMask              & 0x00000000ffffffff);
    #if (GPIO_COUNT == 2)
    NRF_P1->OUTCLR      = (uint32_t) ((pinMask >> 32)       & 0x00000000ffffffff);
    #endif

    // DEBUG
    GH_DEBUG("[GpioHal] pinMaskClear() > Mask 0x%016x\r\n", pinMask);
}

//------------------------------------------------------------------------------------------------------------
// static
void GpioHal::pinMaskSet(uint64_t pinMask)
{
    NRF_P0->OUTSET      = (uint32_t) ( pinMask              & 0x00000000ffffffff);
    #if (GPIO_COUNT == 2)
    NRF_P1->OUTSET      = (uint32_t) ((pinMask >> 32)       & 0x00000000ffffffff);
    #endif

    // DEBUG
    GH_DEBUG("[GpioHal] pinMaskSet() > Mask 0x%016x\r\n", pinMask);
}

//------------------------------------------------------------------------------------------------------------
// static
void GpioHal::pinMaskToggle(uint64_t pinMask)
{
    uint64_t pinState   =  ((uint64_t) NRF_P0->OUT)         & 0x00000000ffffffff;
    #if (GPIO_COUNT == 2)
    pinState           |= (((uint64_t) NRF_P1->OUT) << 32)  & 0xffffffff00000000;
    #endif
    
    pinMaskSet(  (~pinState & 0xffffffffffffffff) & pinMask);
    pinMaskClear(( pinState & 0xffffffffffffffff) & pinMask);
    
    // DEBUG
    GH_DEBUG("[GpioHal] pinMaskToggle() > Mask 0x%016x\r\n", pinMask);
}

//------------------------------------------------------------------------------------------------------------
// static
void GpioHal::pinClear(uint32_t pinNbr)
{
    NRF_GPIO_Type* reg  = nrf_gpio_pin_port_decode(&pinNbr);
    reg->OUTCLR         = (1ULL << pinNbr);

    // DEBUG
    #if (APP_DEBUG_GPIO_PIN_XX != 0)
    if(pinNbr == APP_DEBUG_PIN_NUMBER)
    #endif
    {
        GH_DEBUG("[GpioHal] pinClear() > pin %i\r\n", pinNbr);
    }
}

//------------------------------------------------------------------------------------------------------------
// static
void GpioHal::pinSet(uint32_t pinNbr)
{
    NRF_GPIO_Type* reg  = nrf_gpio_pin_port_decode(&pinNbr);
    reg->OUTSET         = (1ULL << pinNbr);

    // DEBUG
    #if (APP_DEBUG_GPIO_PIN_XX != 0)
    if(pinNbr == APP_DEBUG_PIN_NUMBER)
    #endif
    {
        GH_DEBUG("[GpioHal] pinSet() > pin %i\r\n", pinNbr);
    }
}

//------------------------------------------------------------------------------------------------------------
// static
void GpioHal::pinToggle(uint32_t pinNbr)
{
    NRF_GPIO_Type* reg  = nrf_gpio_pin_port_decode(&pinNbr);
    uint32_t pinMask    = (1ULL << pinNbr);
    uint32_t pinState   = reg->OUT;
    reg->OUTSET         = (~pinState & 0xffffffff) & pinMask;
    reg->OUTCLR         = ( pinState & 0xffffffff) & pinMask;

    // DEBUG
    #if (APP_DEBUG_GPIO_PIN_XX != 0)
    if(pinNbr == APP_DEBUG_PIN_NUMBER)
    #endif
    {
        GH_DEBUG("[GpioHal] pinToggle() > Toggle pin %i\r\n", pinNbr);
    }
}

//------------------------------------------------------------------------------------------------------------
// static
void GpioHal::pinWrite(uint32_t pinNbr, uint8_t value)
{
    NRF_GPIO_Type* reg  = nrf_gpio_pin_port_decode(&pinNbr);
    if(value == 0)
    {
        reg->OUTCLR     = (1ULL << pinNbr);
    }
    else
    {
        reg->OUTSET     = (1ULL << pinNbr);
    }
    
    // DEBUG
    #if (APP_DEBUG_GPIO_PIN_XX != 0)
    if(pinNbr == APP_DEBUG_PIN_NUMBER)
    #endif
    {
        GH_DEBUG("[GpioHal] gpioPinWrite() > Set %i on pin %i\r\n", value, pinNbr);
    }
}

//------------------------------------------------------------------------------------------------------------
// static
bool GpioHal::pinRead(uint32_t pinNbr)
{
    NRF_GPIO_Type* reg  = nrf_gpio_pin_port_decode(&pinNbr);
    GH_DEBUG("[GpioHal] pinRead() > pin %02u is %d\r\n", pinNbr, ((reg->IN >> pinNbr) & 1ULL));
    return (((reg->IN >> pinNbr) & 1ULL) == 1ULL);
}

//------------------------------------------------------------------------------------------------------------
// static
uint64_t GpioHal::pinMaskRead(uint64_t pinMask)
{
    uint64_t pinState   =  ((uint64_t) NRF_P0->IN)          & 0x00000000ffffffff;
    #if (GPIO_COUNT == 2)
    pinState           |= (((uint64_t) NRF_P1->IN) << 32)   & 0xffffffff00000000;
    #endif
    
    GH_DEBUG("[GpioHal] pinMaskRead() > pinMask 0x%016x is 0x%016x\r\n", pinMask, pinState & pinMask);
    return (pinState & pinMask);
}

#if defined(DEBUG)
//------------------------------------------------------------------------------------------------------------
// static
void GpioHal::initDebugGPIO(uint8_t pinNbr)
{
    cfgQuickOutput(pinNbr, GPIO_PIN_CNF_DRIVE_S0S1);
}

//------------------------------------------------------------------------------------------------------------
// static
void GpioHal::toggleDebugGPIO(uint8_t pinNbr)
{
    pinToggle(pinNbr);
}
#endif
