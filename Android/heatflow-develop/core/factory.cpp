/************************************************************************************************************
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 ************************************************************************************************************/

/************************************************************************************************************/
/* Header files                                                                                             */
/************************************************************************************************************/
#include "factory.h"

/************************************************************************************************************/
/* Declaration section                                                                                      */
/************************************************************************************************************/
// Global objects
#if defined(DEBUG_NRF_USER)
    Factory*                                Factory::_pInstance(NULL);
#endif    
    eProductMode                            Factory::_mode;

#if defined(SOFTDEVICE_PRESENT)
    ble::BleController                      Factory::bleController;
    ble::PeripheralController               Factory::peripheralController;

    #if (NUMBER_OF_SOPROD_PRIVATE_SERVICES != 0) 
    ble::BleVendor                          Factory::soprodVendor;
    #endif  // #if (NUMBER_OF_SOPROD_PRIVATE_SERVICES != 0)
    
    #if (NUMBER_OF_HEI_PRIVATE_SERVICES != 0)
    ble::BleVendor                          Factory::heiVendor;
    #endif  // #if (NUMBER_OF_HEI_PRIVATE_SERVICES != 0)

    #if (TBS_SUPPORT != 0)
        ble::TbsService                     Factory::tbsSrv;
    #endif  // #if (TBS_SUPPORT != 0)

    #if (MHFS_SUPPORT != 0)
        ble::MhfsService                    Factory::mhfsSrv;
    #endif  // #if (MHFS_SUPPORT != 0)

#endif  // #if defined(SOFTDEVICE_PRESENT)

#if (USE_LEDS != 0) && defined(LED_1) && defined(LED_2) && defined(LED_3)
    gpio::LedController                     Factory::ledA;
    gpio::LedController                     Factory::ledB;
    gpio::LedController                     Factory::ledC;
    #if defined(BOARD_PCA10040) || defined(BOARD_PCA10056)
    gpio::LedController                     Factory::ledD;
    #endif  // #if defined(BOARD_PCA10040) || defined(BOARD_PCA10056)
#endif  // #if (USE_LEDS != 0) && defined(LED_1)

#if (USE_GPIOS != 0)
    gpio::Gpio                              Factory::gpioA;
    gpio::Gpio                              Factory::gpioB;
    gpio::Gpio                              Factory::gpioC;
    gpio::GpioController                    Factory::gpioController;
#endif  // #if !defined(USE_GPIOS)

#if (USE_BUTTONS != 0)
    gpio::Button                            Factory::buttonA;
    gpio::Button                            Factory::buttonB;
    gpio::Button                            Factory::buttonC;
    #if defined(BOARD_PCA10040) || defined(BOARD_PCA10056)
    gpio::Button                            Factory::buttonD;
    #endif  //#if defined(BOARD_PCA10040) || defined(BOARD_PCA10056)
    gpio::ButtonController                  Factory::buttonController;
#endif  // #if (USE_BUTTONS != 0)

#if (USE_FLASH != 0)
    flash::FlashController                  Factory::flashController;
#endif  // #if (USE_FLASH != 0)

#if (USE_SAADC != 0)
    hal::SaadcHal                           Factory::saadcHal;
#endif  // #if (USE_SAADC != 0)

#if (USE_BATTERY_MANAGER != 0)
    battery::BatteryManager                 Factory::batteryManager;
#endif  // #if (USE_BATTERY_MANAGER != 0)

//#if (USE_SPI != 0) || (USE_SPIM != 0)
//    comm::SpiMasterDriver                   Factory::spi0MasterDriver;
//#endif  // #if (USE_SPI != 0) || (USE_SPIM != 0)

#if (USE_PWM != 0)
    hal::PwmHal                             Factory::pwmHAL;
#endif  //  #if (USE_PWM != 0)
    
#if (USE_DFU != 0)
    dfu::Dfu                                Factory::dfu;
#endif  // #if (USE_DFU != 0)

#if (USE_ADS111X != 0)
    sensor::ADS1118                         Factory::adsA;
    #if (USE_ADS111X > 1)
    sensor::ADS1118                         Factory::adsB;
    #endif
    sensor::SensorController                Factory::sensorController;
#endif


/************************************************************************************************************/
/* CONSTRUCTOR                                                                                              */
/************************************************************************************************************/
Factory::Factory()
{
    #if defined(DEBUG_NRF_USER)
    ASSERT(_pInstance == NULL);
    _pInstance = this;
    #endif // DEBUG
    _mode = PROD_UNKNOWN_MODE;
}

/************************************************************************************************************/
/* PUBLIC SECTION                                                                                           */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
void Factory::launch(eProductMode mode)
{
    _mode = mode;
    switch(_mode)
    {
        case PROD_DEFAULT_MODE:
            appInit();
            appBuild();
            appBind();
#if (USE_FLASH != 0)
            appEnableFlash();
#else
            appCheck();
            appPrepare();
#endif  // #if (USE_FLASH != 0)
            break;

        default:
            break;
    }
}

//------------------------------------------------------------------------------------------------------------
#if (USE_FLASH != 0)
void Factory::onFlashInitialized(flash::FlashController* flashController)
{
    switch(_mode)
    {
        case PROD_DEFAULT_MODE:
            appRestore();
            appCheck();
            appPrepare();
            break;

        default:
            break;
    }
}
#endif
