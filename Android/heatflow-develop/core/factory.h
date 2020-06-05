/****************************************************************************************************************//**
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 *
 * \file    factory.h
 * \class   Factory
 * \brief   Where everything starts from...
 *
 * # Factory
 *
 * Here are instantiated and initialized all objects used in the current application.
 * The eXecution Framework's mainloop is also started from this class.
 *
 * \author  Patrice Rudaz
 * \date    June 2018
 ********************************************************************************************************************/
#pragma once

/* **************************************************************************************************************** */
/*                                                                                                                  */
/* Header files                                                                                                     */
/*                                                                                                                  */
/* **************************************************************************************************************** */
#include <platform-config.h>
#include <boards.h>

// ------------------------------------------------------------------------------------------------------------------
// Some checks
#if !defined(USE_LEDS)
    #error "USE_LEDS not defined !"
#elif (USE_LEDS != 0) && defined(LED_1)
    #include "gpio/ledcontroller.h"
#endif

#if !defined(USE_WATCHDOG)
    #error "USE_WATCHDOG not defined !"
#endif

#if defined(SOFTDEVICE_PRESENT)
    #include <ble/blecontroller.h>
    #include "ble/peripheralcontroller.h"
    #include "ble/services/blevendor.h"

    // --------------------------------------------------------------------------------------------------------------
    // Some checks
    #if !defined(AMS_SUPPORT)
        #error "AMS_SUPPORT not definedè"
    #elif (AMS_SUPPORT != 0)
        #include "ios/amshandler.h"
    #endif  // #if (AMS_SUPPORT != 0)
    
    #if !defined(ANCS_SUPPORT)
        #error "ANCS_SUPPORT not defined !"
    #elif (ANCS_SUPPORT != 0)
        #include "ios/ancshandler.h"
    #endif  // #if (ANCS_SUPPORT != 0)
    
    #if !defined(TBS_SUPPORT)
        #error "TBS_SUPPORT not defined !"
    #elif (TBS_SUPPORT != 0)
        #include "tbs/tbsservice.h"
    #endif  // #if (TBS_SUPPORT != 0)
    
    #if !defined(MHFS_SUPPORT)
        #error "MHFS_SUPPORT not defined !"
    #elif (MHFS_SUPPORT != 0)
        #include "mhfs/mhfsservice.h"
    #endif  // #if (MHFS_SUPPORT != 0)
#endif  // #if defined(SOFTDEVICE_PRESENT)

#if !defined(USE_FLASH)
    #error "USE_FLASH not defined !"
#elif (USE_FLASH != 0)
    #include "flash/flashcontroller.h"
    #include "flash/interface/persister.h"
#endif  // #if !defined(USE_FLASH)

#if !defined(USE_GPIOS)
    #error "USE_GPIOS not defined !"
#elif (USE_GPIOS != 0)
    #include "gpio/gpio.h"
    #include "gpio/gpiocontroller.h"
#endif  // #if !defined(USE_GPIOS)

#if !defined(USE_BUTTONS)
    #error "USE_BUTTONS not defined !"
#elif (USE_BUTTONS != 0)
    #include "button/button.h"
    #include "button/buttoncontroller.h"
#endif  // #if !defined(USE_BUTTONS)

#if !defined(USE_SAADC)
    #error "USE_SAADC not defined !"
#elif (USE_SAADC != 0)
    #include "saadc/saadchal.h"
#endif  // #if !defined(USE_SAADC)

#if !defined(USE_BATTERY_MANAGER)
    #error "USE_BATTERY_MANAGER not defined !"
#elif (USE_BATTERY_MANAGER != 0)
    #include "battery/batterymanager.h"
#endif  // #if !defined(USE_BATTERY_MANAGER)

//#if !defined(USE_SPI) && !defined(USE_SPIM)
//    #error "USE_SPI && USE_SPIM not defined !"
//#elif (USE_SPI != 0) || (USE_SPIM != 0)
//    #include "spi/spiinstance.h"
//    #include "spi/spimasterdriver.h"
//#endif  // #if !defined(USE_SPI) && !defined(USE_SPIM)

#if !defined(USE_PWM)
    #error  "USE_PWM not defined"
#elif (USE_PWM != 0)
    #include "pwm/pwmhal.h"
#endif  //  #if (USE_PWM != 0)

#if !defined(USE_DFU)
    #error "USE_DFU not defined !"
#elif (USE_DFU != 0)
    #include "controller/dfucontroller.h"
#endif  // #if !defined(USE_DFU)

#if !defined(USE_ADS111X)
    #error "USE_ADS111X not defined !"
#elif (USE_ADS111X != 0)
    #include "sensor/sensorcontroller.h"
    #include "sensor/ads111X/ads1118.h"
#endif

// The application
#include "application/heatflow.h"



/* **************************************************************************************************************** */
/*                                                                                                                  */
/* Global Enum Declaration                                                                                          */
/*                                                                                                                  */
/* **************************************************************************************************************** */
/****************************************************************************************************************//**
 * \brief   Enumeration used to have a unique identifier for every product's state
 ********************************************************************************************************************/
typedef enum
{
    PROD_UNKNOWN_MODE = 0,
    PROD_PCB_TEST_MODE,
    PROD_MVT_TEST_MODE,
    PROD_DEFAULT_MODE
} eProductMode;


/* **************************************************************************************************************** */
/*                                                                                                                  */
/* Class Declaration                                                                                                */
/*                                                                                                                  */
/* **************************************************************************************************************** */
class Factory
#if (USE_FLASH != 0)
    #if (USE_MOTORS != 0)
    , public flash::FlashObserver
    #else
    : public flash::FlashObserver
    #endif // #if (USE_MOTORS != 0)
#endif
{
public:

    /* ************************************************************************************************************ */
    /*                                                                                                              */
    /* PUBLIC DECLARATION SECTION                                                                                   */
    /*                                                                                                              */
    /* ************************************************************************************************************ */
#if defined(SOFTDEVICE_PRESENT)
    static ble::BleController                       bleController;
    static ble::PeripheralController                peripheralController;

    #if (NUMBER_OF_SOPROD_PRIVATE_SERVICES != 0) 
    static ble::BleVendor                           soprodVendor;
    #endif  // #if (NUMBER_OF_SOPROD_PRIVATE_SERVICES != 0)

    #if (NUMBER_OF_HEI_PRIVATE_SERVICES != 0)
    static ble::BleVendor                           heiVendor;
    #endif  // #if (NUMBER_OF_HEI_PRIVATE_SERVICES != 0)

    #if (AMS_SUPPORT != 0)
    static media::AMSHandler                        amsHandler;
    #endif  // #if (AMS_SUPPORT != 0)

    #if (ANCS_SUPPORT != 0)
    static notification::ANCSHandler                ancsHandler;
    #endif  // #if (ANCS_SUPPORT != 0)
    
    #if (TBS_SUPPORT != 0)
    static ble::TbsService                          tbsSrv;
    #endif  // #if (TBS_SUPPORT != 0)
    
    #if (MHFS_SUPPORT != 0)
    static ble::MhfsService                         mhfsSrv;
    #endif  // #if (MHFS_SUPPORT != 0)
#endif  // #if defined(SOFTDEVICE_PRESENT)

#if (USE_LEDS != 0) && defined(LED_1) && defined(LED_2) && defined(LED_3)
    static gpio::LedController                      ledA;
    static gpio::LedController                      ledB;
    static gpio::LedController                      ledC;
    #if defined(BOARD_PCA10040) || defined(BOARD_PCA10056)
    static gpio::LedController                      ledD;
    #endif  // #if defined(BOARD_PCA10040) || defined(BOARD_PCA10056)
#endif  // #if (USE_LEDS != 0) && defined(LED_1)

#if (USE_GPIOS != 0)
    static gpio::Gpio                               gpioA;
    static gpio::Gpio                               gpioB;
    static gpio::Gpio                               gpioC;
    static gpio::GpioController                     gpioController;
#endif  // #if (USE_GPIOS != 0)

#if (USE_BUTTONS != 0)
    static gpio::Button                             buttonA;
    static gpio::Button                             buttonB;
    static gpio::Button                             buttonC;
    #if defined(BOARD_PCA10040) || defined(BOARD_PCA10056)
    static gpio::Button                             buttonD;
    #endif  //#if defined(BOARD_PCA10040) || defined(BOARD_PCA10056)
    static gpio::ButtonController                   buttonController;
#endif  // #if (USE_BUTTONS != 0)

#if (USE_FLASH != 0)
    static flash::FlashController                   flashController;
#endif  // #if (USE_FLASH != 0)

#if (USE_SAADC != 0)
    static hal::SaadcHal                            saadcHal;
#endif  // #if (USE_SAADC != 0)

#if (USE_BATTERY_MANAGER != 0)
    static battery::BatteryManager                  batteryManager;
#endif  // #if (USE_BATTERY_MANAGER != 0)
    
#if (USE_DFU != 0)
    static dfu::Dfu                                 dfu;
#endif  // #if (USE_DFU != 0)

//#if (USE_SPI != 0) || (USE_SPIM != 0)
//    static comm::SpiMasterDriver                    spi0MasterDriver;
//#endif  // #if (USE_SPI != 0) || (USE_SPIM != 0)

#if (USE_PWM != 0)
    static hal::PwmHal                              pwmHAL;
#endif  //  #if (USE_PWM != 0)

#if (USE_ADS111X != 0)
    static sensor::ADS1118                          adsA;
    #if (USE_ADS111X > 1)
    static sensor::ADS1118                          adsB;
    #endif
    static sensor::SensorController                 sensorController;
#endif

    static application::HeatFlow                    heatFlow;


    /* ************************************************************************************************************ */
    /*                                                                                                              */
    /* CONSTRUCTOR SECTION                                                                                          */
    /*                                                                                                              */
    /* ************************************************************************************************************ */
    Factory();

    /* ************************************************************************************************************ */
    /*                                                                                                              */
    /* PUBLIC DECLARATION SECTION                                                                                   */
    /*                                                                                                              */
    /* ************************************************************************************************************ */
    /************************************************************************************************************//**
    * \brief   Execute the procedure to start the application in the corresponding mode
    *****************************************************************************************************************/
    void launch(eProductMode mode);

    /************************************************************************************************************//**
     * \brief   Observe when the flash is initialized
    *****************************************************************************************************************/
#if (USE_FLASH != 0)
    virtual void onDataSaved(flash::FlashController* flashController) {}
    virtual void onDataRestored(flash::FlashController* flashController) {}
    virtual void onDataCleared(flash::FlashController* flashController) {}
    virtual void onFlashInitialized(flash::FlashController* flashController);
    virtual void onFlashError(flash::FlashController* flashController) {}
#endif


    /* ************************************************************************************************************ */
    /*                                                                                                              */
    /* APPLICATION SECTION                                                                                          */
    /*                                                                                                              */
    /* ************************************************************************************************************ */
    void appInit();
    void appBuild();
    void appBind();
#if (USE_FLASH != 0)    
    void appEnableFlash();
    void appRestore();
#endif  // #if (USE_FLASH != 0)
    void appCheck();
    void appPrepare();
    void appStart();

private:
#if defined(DEBUG_NRF_USER)
    static Factory*                         _pInstance;
#endif
    static eProductMode                     _mode;
};
