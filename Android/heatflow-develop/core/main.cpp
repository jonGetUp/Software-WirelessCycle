/*************************************************************************************************************
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 *
 * \file        main.cpp
 * \brief       Main file for the project
 * \mainpage    The mainpage documentation for Soprod uBlueWatch
 *
 * Initial author: Patrice Rudaz
 * Creation date: 2018-01-11
 *
 * Entry point !
 ************************************************************************************************************/
#include <platform-config.h>
#include <sdk-tools-config.h>

#include <core/xf.h>
#include <factory.h>
#include <gpio/gpiohal.h>
#include <nrf_delay.h>

#if defined(NRF_LOG_BACKEND_RTT_ENABLED) && (NRF_LOG_BACKEND_RTT_ENABLED != 0)    
    #include <nrf_log.h>
    #include <nrf_log_ctrl.h>
    #include <nrf_log_default_backends.h>
#endif

#if defined(DEBUG_NRF_USER)
    #include <tools/logtrace.h>
    #include <mcu.h>
#endif


/************************************************************************************************************/
/* Declaration section                                                                                      */
/************************************************************************************************************/
// Global objects
static Factory factory;


/************************************************************************************************************/
/* PUBLIC SECTION                                                                                           */
/************************************************************************************************************/
/********************************************************************************************************//**
 * \brief   Callback function for asserts in the SoftDevice.
 *
 * \details This function will be called in case of an assert in the SoftDevice.
 *
 * \warning This handler is an example only and does not fit a final product. You need to analyze how your 
 *          product is supposed to react in case of Assert.
 * \warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * \param[in] line_num   Line number of the failing ASSERT call.
 * \param[in] file_name  File name of the failing ASSERT call.
 ************************************************************************************************************/
extern "C" void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    #if defined(DEBUG_NRF_USER)
    LOG_TRACE_INFO("[Main] assert_nrf_callback() > ASSERT failed ! file: %s (line: %d)\r\n", p_file_name, line_num);
    breakOnDebug();
    #endif
}

extern "C" void HardFault_Handler()
{
    #if defined(DEBUG_NRF_USER)
    LOG_TRACE_DEBUG("[MAIN] HardFault_Handler() !\r\n");
    #endif  // #if defined(DEBUG_NRF_USER)
    while(1)
        ;
}

/********************************************************************************************************//**
 * \brief   Application main function.
 ************************************************************************************************************/
int main()
{
    // Initialize the LogTrace utility class.
    #if (defined(NRF_LOG_ENABLED) && (NRF_LOG_ENABLED != 0)) && (defined(NRF_LOG_BACKEND_RTT_ENABLED) && (NRF_LOG_BACKEND_RTT_ENABLED != 0))
    uint32_t errCode = NRF_LOG_INIT(NULL);
    if(errCode != NRF_SUCCESS)
    {
        ASSERT(false);
    }
    NRF_LOG_DEFAULT_BACKENDS_INIT();
    #elif defined(LOG_TRACE_RTT_ENABLED) && (LOG_TRACE_RTT_ENABLED != 0) && defined(DEBUG_NRF_USER)
    tools::LogTrace::init();
    #endif
    
    #if defined(DEBUG_NRF_USER)
    LOG_TRACE_INFO("\r\n\r\n[MAIN] main() > Initializing application...\r\n\r\n");
    #endif  // #if defined(DEBUG_NRF_USER)

    // Initialize all gpios as disconnected inputs
    hal::GpioHal::init();

    // Configure the Chip Select's lines for both ADS
    hal::GpioHal::cfgPinOutput(ADS_CS1_PIN, GPIO_PIN_CNF_DRIVE_H0H1, GPIO_PIN_CNF_PULL_Disabled, GPIO_PIN_CNF_INPUT_Disconnect);
    hal::GpioHal::cfgPinOutput(ADS_CS2_PIN, GPIO_PIN_CNF_DRIVE_H0H1, GPIO_PIN_CNF_PULL_Disabled, GPIO_PIN_CNF_INPUT_Disconnect);
    hal::GpioHal::pinSet(ADS_CS1_PIN);
    hal::GpioHal::pinSet(ADS_CS2_PIN);
 
    #if defined(DEBUG_NRF_USER)
    LOG_TRACE_DEBUG("[MAIN] main() > launching application...\r\n");
    #endif  // #if defined(DEBUG_NRF_USER)
    factory.launch(PROD_DEFAULT_MODE);
    
    // Starts the XF
    #if defined(DEBUG_NRF_USER)
    LOG_TRACE_DEBUG("[MAIN] main() > starting eXecution Framework (XF)...\r\n");
    #endif  // #if defined(DEBUG_NRF_USER)
    XF::start();

    return 0;
}
