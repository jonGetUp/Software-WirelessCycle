/************************************************************************************************************
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 ************************************************************************************************************/

/************************************************************************************************************/
/* Header files                                                                                             */
/************************************************************************************************************/
#include "factory.h"

#include <tools/sdktools.h>
#include <core/xf.h>

#include <ble_types.h>

#include <ble-services-config.h>
#include <ble-private-service-config.h>

#include <gpio/gpiohal.h>

#if (USE_WATCHDOG != 0)
    #include "wdt/watchdoghal.h"
#endif

#if (USE_FLASH != 0)
    #include "flashfileid.h"
#endif


/************************************************************************************************************/
/* DEBUG COMPILATION OPTION & MACRO                                                                         */
/************************************************************************************************************/
#define DEBUG_FACTORY_ENABLE                0
#if (DEBUG_FACTORY_ENABLE == 1) && defined(DEBUG_NRF_USER)
    #include <tools/logtrace.h>
    #define F_DEBUG                         LOG_TRACE_DEBUG
#else
    #define F_DEBUG(...)                    {(void) (0);}
#endif

    
/************************************************************************************************************/
/* OBJECT'S DECLARATION                                                                                     */
/************************************************************************************************************/
// Main class of the application
application::HeatFlow                       Factory::heatFlow;


/************************************************************************************************************/
/* PUBLIC SECTION                                                                                           */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
void Factory::appInit()
{
    // Initialize the SDK
    tools::SDKTools::init();

#if defined(SOFTDEVICE_PRESENT)
    bleController.initialize();
    F_DEBUG("[Factory] appInit() > Initialize BleController ... Done! \r\n");

    #if (NUMBER_OF_SOPROD_PRIVATE_SERVICES != 0)
    ble_uuid128_t soprodBaseUuid = BLE_SOPROD_UUID_PRIVATE_BASE;
    soprodVendor.registerVendor(&soprodBaseUuid, SOPROD_OWNER_MASK_UUID);
    F_DEBUG("[Factory] appInit() > Register Soprod as BLE Service's Vendor ... Done! \r\n");
    #endif

    #if (NUMBER_OF_HEI_PRIVATE_SERVICES != 0)
    ble_uuid128_t heiBaseUuid = BLE_HEI_UUID_PRIVATE_BASE;
    heiVendor.registerVendor(&heiBaseUuid, HEI_OWNER_MASK_UUID);
    F_DEBUG("[Factory] appInit() > Register HEI as BLE Service's Vendor ... Done! \r\n");
    #endif
#endif    // SOFTDEVICE_PRESENT
}

//------------------------------------------------------------------------------------------------------------
void Factory::appBuild()
{
    F_DEBUG((char *) "[Factory] appBuild() ...\r\n");

#if defined(SOFTDEVICE_PRESENT)
    peripheralController.initialize();

    #if (AMS_SUPPORT != 0)
        amsHandler.initialize();
    #endif

    #if (ANCS_SUPPORT != 0)
        ancsHandler.initialize();
    #endif

    #if (TBS_SUPPORT != 0)
    peripheralController.registerService(&tbsSrv);
    #endif

    #if (MHFS_SUPPORT != 0)
    peripheralController.registerService(&mhfsSrv);
    #endif
#endif

#if (USE_WATCHDOG != 0)
    hal::WatchdogHal::initialize();
#endif

#if (USE_LEDS != 0) && defined(LED_1) && defined(LED_2) && defined(LED_3)
    ledA.initialize(0); // LEDg
    ledB.initialize(1); // LEDb
    ledC.initialize(2); // LEDr
    #if defined(BOARD_PCA10040) || defined(BOARD_PCA10056)
    ledD.initialize(3, 250, 250);
    #endif  // #if defined(BOARD_PCA10040) || defined(BOARD_PCA10056)
#endif

#if (USE_GPIOS != 0)
    gpioA.initialize(PIN_TEST_RX,       gpio::GpioId::Gpio1, NRF_GPIO_PIN_PULLUP);
    gpioB.initialize(PIN_TEST_TX,       gpio::GpioId::Gpio2, NRF_GPIO_PIN_PULLUP);
    gpioC.initialize(LIS2DW12_INT1_PIN, gpio::GpioId::Gpio3, NRF_GPIO_PIN_PULLUP);
    gpioController.initialize();
#endif

#if (USE_BUTTONS != 0)
    buttonA.initialize(BUTTON_1, gpio::ButtonId::Button1);
    buttonB.initialize(BUTTON_2, gpio::ButtonId::Button2);
    buttonC.initialize(BUTTON_3, gpio::ButtonId::Button3);
    #if defined(BOARD_PCA10040) || defined(BOARD_PCA10056)
    buttonD.initialize(BUTTON_4, gpio::ButtonId::Button4);
    #endif // #if defined(BOARD_PCA10040) || defined(BOARD_PCA10056)
    buttonController.initialize();
#endif

#if (USE_BATTERY_MANAGER != 0)
    batteryManager.initialize();
#endif

#if (USE_FLASH != 0)
    flashController.initialize();
#endif

#if ((USE_SPI != 0) || (USE_SPIM != 0)) && (USE_SPI_DRV == 0)
    spi0MasterDriver.initialize();
    spi0MasterDriver.bind(comm::SPIInstance::SPI0);
    spi0MasterDriver.setFullDuplex(false);
#endif

#if (USE_DFU !=0)
    dfu.initialize();
#endif

#if (USE_ADS111X != 0)
    adsA.initialize(1, ADS_CS1_PIN);
    adsB.initialize(2, ADS_CS2_PIN);
    sensorController.initialize();
#endif

    heatFlow.initialize();
}

//------------------------------------------------------------------------------------------------------------
void Factory::appBind()
{
    F_DEBUG((char *) "[Factory] appBind() ...\r\n");
    
    // Check library dependency
#if defined(SOFTDEVICE_PRESENT)

    peripheralController.bind(
        &bleController
    #if (USE_CALENDAR != 0)
      , &calendar
    #endif
    #if (AMS_SUPPORT != 0)
      , &amsHandler
    #endif
    #if (ANCS_SUPPORT != 0)
      , &ancsHandler
    #endif
    );

    #if (USE_FLASH != 0)
    flashController.addPersister(&peripheralController, flash::FlashFileId::PeripheralControllerFileId);
    #endif

    #if (TBS_SUPPORT != 0)
    tbsSrv.bind(&bleController);
    #endif

    #if (MHFS_SUPPORT != 0)
    mhfsSrv.bind(&bleController);
    #endif
#endif

#if (USE_SPI_DRV == 0)
    #if (USE_SPI != 0)
    spi0MasterDriver.bind(comm::SPIInstance::SPI0);
    #elif (USE_SPIM != 0)
    spi0MasterDriver.bind(comm::SPIInstance::SPIM0);
    #endif
#endif

#if (USE_BATTERY_MANAGER != 0)
    batteryManager.bind(
    #if (USE_SAADC != 0)
        &saadcHal
    #endif
    #if (USE_CHARGER != 0) 
      , &charger
        #if (USE_CALENDAR != 0)
      , &calendar
        #endif
    #endif
    );
    
    #if (USE_FLASH != 0)
    flashController.addPersister(&batteryManager, flash::FlashFileId::BatteryManagerFileId);
    #endif
#endif

#if (USE_DFU != 0)
    dfu.bind(&peripheralController
#if (USE_FLASH != 0)
         , &flashController
#endif  // #if (USE_FLASH != 0)
#if (USE_MOTORS != 0)
         , &motorController
#endif  // #if (USE_MOTORS != 0)  
     );
#endif  // #if (USE_DFU != 0)

    heatFlow.bind(
#if defined(SOFTDEVICE_PRESENT)
              &peripheralController
#endif  // #if defined(SOFTDEVICE_PRESENT)
#if (USE_FLASH != 0)
    #if defined(SOFTDEVICE_PRESENT)
            ,
    #endif  // #if defined(SOFTDEVICE_PRESENT)
              &flashController
#endif  // #if (USE_FLASH != 0)
#if (USE_BATTERY_MANAGER != 0)
    #if defined(SOFTDEVICE_PRESENT) || (USE_FLASH != 0)
            ,
    #endif  // #if defined(SOFTDEVICE_PRESENT)
             &batteryManager
#endif  // #if (USE_BATTERY_MANAGER != 0)
#if (USE_GPIOS != 0)
    #if defined(SOFTDEVICE_PRESENT) || (USE_FLASH != 0) || (USE_BATTERY_MANAGER != 0)
            ,
    #endif  // #if defined(SOFTDEVICE_PRESENT)
             &gpioA, &gpioB
#endif  // #if (USE_GPIOS != 0)
#if (USE_LEDS != 0) && defined(LED_1) && defined(LED_2) && defined(LED_3)
    #if defined(SOFTDEVICE_PRESENT) || (USE_FLASH != 0) || (USE_BATTERY_MANAGER != 0) || (USE_GPIOS != 0)
            ,
    #endif  // #if defined(SOFTDEVICE_PRESENT)
              &ledA, &ledB, &ledC
    #if defined(BOARD_PCA10040) || defined(BOARD_PCA10056)
            , &ledD
    #endif  // #if defined(BOARD_PCA10040) || defined(BOARD_PCA10056)
#endif  // #if (USE_LEDS != 0) && defined(LED_1)
#if (USE_BUTTONS != 0)
    #if defined(SOFTDEVICE_PRESENT) || (USE_FLASH != 0) || (USE_BATTERY_MANAGER != 0) || (USE_GPIOS != 0) || ((USE_LEDS != 0) && defined(LED_1) && defined(LED_2) && defined(LED_3))
//            ,
    #endif  // #if defined(SOFTDEVICE_PRESENT)
              &buttonA, &buttonB, &buttonC
    #if defined(BOARD_PCA10040) || defined(BOARD_PCA10056)
            , &buttonD
    #endif
#endif  // #if (USE_BUTTONS != 0)
#if (USE_PWM != 0)
    #if defined(SOFTDEVICE_PRESENT) || (USE_FLASH != 0) || (USE_BATTERY_MANAGER != 0) || (USE_GPIOS != 0) || ((USE_LEDS != 0) && defined(LED_1) && defined(LED_2) && defined(LED_3)) || (USE_BUTTONS != 0)
            ,
    #endif  // #if defined(SOFTDEVICE_PRESENT)
             &pwmHAL
#endif
#if (USE_DFU != 0)
    #if defined(SOFTDEVICE_PRESENT) || (USE_FLASH != 0) || (USE_BATTERY_MANAGER != 0) || (USE_GPIOS != 0) || ((USE_LEDS != 0) && defined(LED_1) && defined(LED_2) && defined(LED_3)) || (USE_BUTTONS != 0) || (USE_PWM != 0)
            ,
    #endif  // #if defined(SOFTDEVICE_PRESENT)
             &dfu
#endif
#if (USE_ADS111X != 0)
    #if defined(SOFTDEVICE_PRESENT) || (USE_FLASH != 0) || (USE_BATTERY_MANAGER != 0) || (USE_GPIOS != 0) || ((USE_LEDS != 0) && defined(LED_1) && defined(LED_2) && defined(LED_3)) || (USE_BUTTONS != 0) || (USE_PWM != 0) || (USE_DFU != 0)
            ,
    #endif
              &sensorController
#endif
    );

#if (USE_ADS111X != 0)
    sensorController.bind(
        #if (USE_ADS111X != 0)
                  &adsA
            #if (USE_ADS111X > 1)
                , &adsB
            #endif
        #endif // #if (USE_ADS111X != 0)
        #if (SENSOR_CONTROLLER_HANDLES_LED != 0) && (USE_LEDS != 0) && (LED_2 != 0)
            #if (USE_ADS111X != 0)
                ,
            #endif // (USE_ADS111X != 0)
                  &ledA
        #endif // #if (USE_LEDS != 0) && (LED_2 != 0)
    );
#endif

    // internal libraries link    
#if (USE_GPIOS != 0)
    gpioController.registerGpio(&gpioA, true);
    gpioController.registerGpio(&gpioB, true);
    gpioController.registerGpio(&gpioC, true);
#endif

#if (USE_BUTTONS != 0)
    buttonController.registerButton(&buttonA);
    buttonController.registerButton(&buttonB);
    buttonController.registerButton(&buttonC);
    #if defined(BOARD_PCA10040) || defined(BOARD_PCA10056)
    buttonController.registerButton(&buttonD);
    #endif

    #if (USE_FLASH != 0)
    flashController.addPersister(&buttonA, flash::FlashFileId::ButtonFileId);
    flashController.addPersister(&buttonB, flash::FlashFileId::ButtonFileId);
    flashController.addPersister(&buttonC, flash::FlashFileId::ButtonFileId);
        #if defined(BOARD_PCA10040) || defined(BOARD_PCA10056)
    flashController.addPersister(&buttonD, flash::FlashFileId::ButtonFileId);
        #endif
    #endif
#endif    

    // link application vith library
#if (USE_FLASH != 0)
    if(!flashController.addObserver(this))                      {F_DEBUG("[Factory] appBind() > Impossible to attach appFactory to flashController !\r\n");}
#endif
    
/* TEST */
#if (USE_NOTIFICATION != 0)
    #if (ANCS_SUPPORT != 0)
    if(!ancsHandler.addObserver(&notificationController))       {F_DEBUG("[Factory] appBind() > Impossible to attach ancsHandler to notificationController !\r\n");}
    #endif
    if(!snsHandler.addObserver(&notificationController))        {F_DEBUG("[Factory] appBind() > Impossible to attach snsHandler to notificationController !\r\n");}
#endif
/* TEST */

#if defined(SOFTDEVICE_PRESENT)
    if(!peripheralController.addObserver(&heatFlow))            {F_DEBUG("[Factory] appBind() > Impossible to attach peripheralController to sdkUpdater !\r\n");}
#endif
}

#if (USE_FLASH != 0)
//------------------------------------------------------------------------------------------------------------
void Factory::appEnableFlash() 
{
    F_DEBUG((char *) "[Factory] appEnableFlash() > ...\r\n");
    flashController.enable();
    F_DEBUG((char *) "[Factory] appEnableFlash() > done!\r\n");
}

//------------------------------------------------------------------------------------------------------------
void Factory::appRestore()
{   
   F_DEBUG((char *) "[Factory] appRestore() > ...\r\n");

   if(flashController.restoreAll())
   {
       F_DEBUG((char *) "[Factory] appRestore() > done!\r\n");
   } 
   else 
   {
       F_DEBUG((char *) "[Factory] appRestore() > FAILED !\r\n");
       ASSERT(false);
   }
   
}
#endif  // #if (USE_FLASH != 0)

//------------------------------------------------------------------------------------------------------------
void Factory::appCheck()
{
   F_DEBUG((char *) "[Factory] appCheck() > ...\r\n");
   F_DEBUG((char *) "[Factory] appCheck() > done!\r\n");
}

//------------------------------------------------------------------------------------------------------------
void Factory::appPrepare()
{
    F_DEBUG((char *) "[Factory] appPrepare() > ...\r\n");
    
    // SoftDevice related
#if defined(SOFTDEVICE_PRESENT)

    peripheralController.initSecurityAndGap();
    
    #if (TBS_SUPPORT !=0)
    tbsSrv.addObserver(&heatFlow);
    #endif
    
    #if (MHFS_SUPPORT !=0)
    mhfsSrv.addObserver(&heatFlow);
    adsA.bind(&mhfsSrv);
        #if (USE_ADS111X > 1)
    adsB.bind(&mhfsSrv);
        #endif
    #endif  // #if defined(SOFTDEVICE_PRESENT) && (MHFS_SUPPORT != 0)

#endif

#if (USE_LEDS != 0) && defined(LED_1) && defined(LED_2) && defined(LED_3)
    ledA.startBehaviour();
    ledB.startBehaviour();
    ledC.startBehaviour();
    #if defined(BOARD_PCA10040) || defined(BOARD_PCA10056)
    ledD.startBehaviour();
    #endif  // #if defined(BOARD_PCA10040) || defined(BOARD_PCA10056)
#endif

    F_DEBUG((char *) "[Factory] appPrepare() > done!\r\n");

#if (USE_GPIOS != 0)
    gpioController.enableController();
#endif
#if (USE_BUTTONS != 0)
    buttonController.enableController();
#endif

#if (USE_ADS111X != 0)
    sensorController.startBehaviour();
#endif

    // Start the application
    appStart();
}

//------------------------------------------------------------------------------------------------------------
void Factory::appStart()
{
    F_DEBUG((char *)  "[Factory] appStart() > starting application\r\n");

#if (USE_WATCHDOG != 0)
    F_DEBUG((char *)  "[Factory] appStart() > starting watchdog\r\n");
    hal::WatchdogHal::start();
#endif

    heatFlow.startBehaviour();
}
