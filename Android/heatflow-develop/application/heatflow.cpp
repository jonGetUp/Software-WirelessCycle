/*************************************************************************************************************
 * Copyright (c) 2016, HES-SO Valais
 * All rights reserved.
 *
 * Initial author: Patrice Rudaz
 * Creation date: 2018-01-22
 *
 * \file heatflow.cpp
 *
 * # HeatFlow
 *
 * This class has been developed to be able to test all compenents of the current library dedicated to the nRF5 
 * series SDK 14.X.YY.
 * It uses the eXecution Framework designed and developed by the HES-SO Valais.
 ************************************************************************************************************/

/************************************************************************************************************/
/*                                                                                                          */
/* Header files                                                                                             */
/*                                                                                                          */
/************************************************************************************************************/
#include "heatflow.h"

#include "core/initialevent.h"
#include "eventid.h"

#include "factory.h"

#if (USE_GPIOS != 0)
    using gpio::GpioId;
#endif

#if (USE_BUTTONS != 0)
    using gpio::ButtonId;
#endif

#if defined(DEBUG_NRF_USER)
    #include <ble-advertiser-config.h>
#endif // #if defined(DEBUG_NRF_USER)


/************************************************************************************************************/
/* DEBUG COMPILATION OPTION & MACRO                                                                         */
/************************************************************************************************************/
#define DEBUG_HEATFLOW_ENABLE               1
#if (DEBUG_HEATFLOW_ENABLE != 0) && defined(DEBUG_NRF_USER)
    #include <tools/logtrace.h>
    #define HF_DEBUG                        LOG_TRACE_DEBUG
#else
    #define HF_DEBUG(...)                   {(void) (0);}
#endif

// For State_machine debug
#define DEBUG_HEATFLOW_ST_ENABLE            0
#if (DEBUG_HEATFLOW_ST_ENABLE == 1)
    #define HFS_DEBUG                       LOG_TRACE_DEBUG
#else
    #define HFS_DEBUG(...)                  {(void) (0);}
#endif


/************************************************************************************************************/
/* Namespace Declaration                                                                                    */
/************************************************************************************************************/
using application::HeatFlow;

/************************************************************************************************************/
/* Constructor | Destructor                                                                                 */
/************************************************************************************************************/
// ----------------------------------------------------------------------------------------------------------
HeatFlow::HeatFlow() : _currentState(ST_UNKOWN)
{
    #if defined(SOFTDEVICE_PRESENT)
    _isBleConnected                     = false;
    _isBleAdvertising                   = false;
    _updateForHigherReactivity          = false;
    _isTimeoutGenericScheduled          = false;
    _isTimeoutNoActivityScheduled       = false;
    
    #if (USE_ADS111X != 0)
    _isTimeoutAdsScheduled              = false;
    #endif
    
    _noActivityCounter                  = 0;
    #endif  // #if defined(SOFTDEVICE_PRESENT)
}

/************************************************************************************************************/
/*                                              PUBLIC SECTION                                              */
/************************************************************************************************************/
// ----------------------------------------------------------------------------------------------------------
void HeatFlow::initialize()
{
    _currentState               = ST_INIT;
    _noActivityCounter          = NO_ACTIVITY_DELAY;
    
    #if defined(SOFTDEVICE_PRESENT)
    _peripheralController       = NULL;
    #endif  // #if defined(SOFTDEVICE_PRESENT)

    #if (USE_BUTTONS != 0)
    for(uint8_t i = 0; i < _nbrButtons; i++)
    {
        _buttons[i]             = NULL;
    }
    #endif  // #if (USE_BUTTONS != 0)

    #if (USE_LEDS != 0) && defined(LED_1) && defined(LED_2) && defined(LED_3)
    _greenLed                       = NULL;
    _blueLed                       = NULL;
    _redLed                       = NULL;
        #if defined(BOARD_PCA10040) || defined(BOARD_PCA10056)
        _ledD                   = NULL;
        #endif  // #if defined(BOARD_PCA10040) || defined(BOARD_PCA10056)
    #endif  // #if (USE_LEDS != 0) && defined(LED_1)

    #if (USE_FLASH != 0)
    _flashController            = NULL;
    #endif  // #if (USE_FLASH != 0)

    #if (USE_BATTERY_MANAGER != 0)
    _batteryManager             = NULL;
    #endif  // #if (USE_BATTERY_MANAGER != 0)

    #if (USE_PWM != 0)
    _pwmHAL                     = NULL;
    #endif  // #if (USE_PWM != 0)

    #if (USE_DFU != 0)
    _dfu                        = NULL;
    #endif  // #if (USE_DFU != 0)

    HF_DEBUG("[HeatFlow] initialize() > Initialise done...\r\n");
}

// ----------------------------------------------------------------------------------------------------------
void HeatFlow::bind(
        #if defined(SOFTDEVICE_PRESENT)
                      ble::PeripheralController* peripheralController
        #endif  // #if defined(SOFTDEVICE_PRESENT)
        #if (USE_FLASH != 0)
            #if defined(SOFTDEVICE_PRESENT)
                    ,
            #endif
                      flash::FlashController* flashController
        #endif  // #if (USE_FLASH != 0)
        #if (USE_BATTERY_MANAGER != 0)
            #if defined(SOFTDEVICE_PRESENT) || (USE_FLASH != 0)
                    ,
            #endif
                      battery::BatteryManager* batteryManager
        #endif  // #if (USE_BATTERY_MANAGER != 0)
        #if (USE_GPIOS != 0)
            #if defined(SOFTDEVICE_PRESENT) || (USE_FLASH != 0) || (USE_BATTERY_MANAGER != 0)
                    ,
            #endif
                      gpio::Gpio* gpioA, gpio::Gpio* gpioB
        #endif  // #if (USE_GPIOS != 0)
        #if (USE_LEDS != 0) && defined(LED_1) && defined(LED_2) && defined(LED_3)
            #if defined(SOFTDEVICE_PRESENT) || (USE_FLASH != 0) || (USE_BATTERY_MANAGER != 0) || (USE_GPIOS != 0)
                    ,
            #endif
                      gpio::LedController* ledA, gpio::LedController* ledB, gpio::LedController* ledC
            #if defined(BOARD_PCA10040) || defined(BOARD_PCA10056)
                    , gpio::LedController* ledD
            #endif  // #if defined(BOARD_PCA10040) || defined(BOARD_PCA10056)
        #endif  // #if (USE_LEDS != 0) && defined(LED_1)
        #if (USE_BUTTONS != 0)
            #if defined(SOFTDEVICE_PRESENT) || (USE_FLASH != 0) || (USE_BATTERY_MANAGER != 0) || (USE_GPIOS != 0) || ((USE_LEDS != 0) && defined(LED_1) && defined(LED_2) && defined(LED_3))
                    ,
            #endif
                      gpio::Button* buttonA, gpio::Button* buttonB, gpio::Button* buttonC
            #if defined(BOARD_PCA10040) || defined(BOARD_PCA10056)
                    , gpio::Button* buttonD
            #endif // #if defined(BOARD_PCA10040) || defined(BOARD_PCA10056)
        #endif  // #if (USE_BUTTONS != 0)
        #if (USE_PWM != 0)
            #if defined(SOFTDEVICE_PRESENT) || (USE_FLASH != 0) || (USE_BATTERY_MANAGER != 0) || (USE_GPIOS != 0) || ((USE_LEDS != 0) && defined(LED_1) && defined(LED_2) && defined(LED_3)) || (USE_BUTTONS != 0)
                    ,
            #endif
                      hal::PwmHal* pwmHAL
        #endif  // #if (USE_PWM != 0)
        #if (USE_DFU != 0)
            #if defined(SOFTDEVICE_PRESENT) || (USE_FLASH != 0) || (USE_BATTERY_MANAGER != 0) || (USE_GPIOS != 0) || ((USE_LEDS != 0) && defined(LED_1) && defined(LED_2) && defined(LED_3)) || (USE_BUTTONS != 0) || (USE_PWM != 0)
                    ,
            #endif
                      dfu::Dfu* dfu
        #endif  // #if (USE_DFU != 0)
        #if (USE_ADS111X != 0)
            #if defined(SOFTDEVICE_PRESENT) || (USE_FLASH != 0) || (USE_BATTERY_MANAGER != 0) || (USE_GPIOS != 0) || ((USE_LEDS != 0) && defined(LED_1) && defined(LED_2) && defined(LED_3)) || (USE_BUTTONS != 0) || (USE_PWM != 0) || (USE_DFU != 0)
                    ,
            #endif
                      sensor::SensorController* sensorCtrl
        #endif
)
{
    #if defined(SOFTDEVICE_PRESENT)
    if(peripheralController == NULL)
    {
        HF_DEBUG("[HeatFlow] bind() > peripheralController NULL error !\r\n");
        ASSERT(false);
    }
    _peripheralController       = peripheralController;
    #endif  // #if defined(SOFTDEVICE_PRESENT)
    #if (USE_LEDS != 0) && defined(LED_1) && defined(LED_2) && defined(LED_3)
    if(ledA == NULL || ledB == NULL || ledC == NULL)
    {
        HF_DEBUG("[HeatFlow] bind() > ledA NULL error !\r\n");
        ASSERT(false);
    }
    _greenLed                   = ledA;
    _blueLed                    = ledB;
    _redLed                     = ledC;
    #if defined(BOARD_PCA10040) || defined(BOARD_PCA10056)
    if(ledD == NULL)
    {
        HF_DEBUG("[HeatFlow] bind() > ledB, ledC or ledD is NULL !\r\n");
        ASSERT(false);
    }
    _ledD                       = ledD;
    #endif  // #if defined(BOARD_PCA10040) || defined(BOARD_PCA10056)
    #endif  // #if (USE_LEDS != 0) && defined(LED_1)

    #if (USE_BUTTONS != 0)
    if(buttonA == NULL || buttonB == NULL || buttonC == NULL)
    {
        HF_DEBUG("[HeatFlow] bind() > button, buttonB or buttonC NULL error !\r\n");
        ASSERT(false);
    }
    _buttons[0]                 = buttonA;
    _buttons[1]                 = buttonB;
    _buttons[2]                 = buttonC;
        #if defined(BOARD_PCA10040) || defined(BOARD_PCA10056)
    if(buttonD == NULL)
    {
        HF_DEBUG("[HeatFlow] bind() > buttonD NULL error !\r\n");
        ASSERT(false);
    }
    _buttons[3]             = buttonD;
        #endif // #if defined(BOARD_PCA10040) || defined(BOARD_PCA10056)
    #endif  // #if (USE_BUTTONS != 0)
    #if (USE_BATTERY_MANAGER != 0)
    if(batteryManager == NULL)
    {
        HF_DEBUG("[HeatFlow] bind() > batteryManager NULL error !\r\n");
        ASSERT(false);
    }
    _batteryManager             = batteryManager;
    #endif  // #if (USE_BATTERY_MANAGER != 0)
    #if (USE_PWM != 0)
    if(pwmHAL == NULL)
    {
        HF_DEBUG("[HeatFlow] bind() > pwmHAL NULL error !\r\n");
        ASSERT(false);
    }
    _pwmHAL = pwmHAL;
    _pwmHAL->initilialize(hal::PwmHal::PWM2);
        #if defined(GPIO_PWM)
    _pwmHAL->addOutputPin(GPIO_PWM, 0, false);
        #else
    _pwmHAL->addOutputPin(NRFX_PWM_DEFAULT_CONFIG_OUT2_PIN, 0, false);
        #endif
    _pwmHAL->setFrequency(250);
    #endif  // #if (USE_PWM != 0)
    #if (USE_DFU != 0)
    if(dfu == NULL)
    {
        HF_DEBUG("[HeatFlow] bind() > dfu NULL error !\r\n");
        ASSERT(false);
    }
    _dfu = dfu;
    #endif  // #if (USE_DFU != 0)
    #if (USE_ADS111X != 0)
    if(sensorCtrl == NULL)
    {
        HF_DEBUG("[HeatFlow] bind() > sensorCtrl NULL error !\r\n");
        ASSERT(false);
    }
    _sensorCtrl = sensorCtrl;
    #endif
}

#if (USE_GPIOS != 0)
// -----------------------------------------------------------------------------------------------------------
void HeatFlow::onFallingEdge(gpio::Gpio * gpio)
{
    HF_DEBUG("[HeatFlow] onFallingEdge() > on Gpio #%u\r\n", gpio->gpioId());
}

// -----------------------------------------------------------------------------------------------------------
void HeatFlow::onRisingEdge(gpio::Gpio * gpio)
{
    HF_DEBUG("[HeatFlow] onRisingEdge() > on Gpio #%u\r\n", gpio->gpioId());
}
#endif

#if (USE_BUTTONS != 0)
// -----------------------------------------------------------------------------------------------------------
const gpio::Button* HeatFlow::button(int index) const
{
    return _buttons[index];
}

// -----------------------------------------------------------------------------------------------------------
gpio::Button* HeatFlow::button(int index)
{
    return _buttons[index];
}

// -----------------------------------------------------------------------------------------------------------
void HeatFlow::onButtonSingleClick(gpio::Button * button)
{
    HF_DEBUG("[HeatFlow] onButtonSingleClick() > on Button #%u\r\n", button->buttonId());
}

// -----------------------------------------------------------------------------------------------------------
void HeatFlow::onButtonDoubleClick(gpio::Button * button)
{
    HF_DEBUG("[HeatFlow] onButtonDoubleClick() > on Button #%u\r\n", button->buttonId());
}

// -----------------------------------------------------------------------------------------------------------
void HeatFlow::onButtonTripleClick(gpio::Button * button)
{
    HF_DEBUG("[HeatFlow] onButtonTripleClick() > on Button #%u\r\n", button->buttonId());
}

// -----------------------------------------------------------------------------------------------------------
void HeatFlow::onButtonLongPress(gpio::Button * button)
{
    HF_DEBUG("[HeatFlow] onButtonLongPress() > on Button #%u\r\n", button->buttonId());
}

// -----------------------------------------------------------------------------------------------------------
void HeatFlow::onButtonVeryLongPress(gpio::Button * button)
{
    #if (USE_LEDS != 0) && (defined(BOARD_PCA10040) || defined(BOARD_PCA10056))
    switch(button->buttonId())
    {
        default:
            if(_redLed != NULL)
            {
                HF_DEBUG("[HeatFlow] onButtonVeryLongPress() > All leds ON !\r\n");
                _blueLed->ledOn();
                _redLed->ledOn();
                _ledD->ledOn();
            }
            break;
    }
    #endif
    HF_DEBUG("[HeatFlow] onButtonVeryLongPress() > on Button #%u\r\n", button->buttonId());
}

// -----------------------------------------------------------------------------------------------------------
void HeatFlow::onButtonReleased(gpio::Button * button)
{
    HF_DEBUG("[HeatFlow] onButtonReleased() > on Button #%u\r\n", button->buttonId());

    #if (USE_LEDS != 0) && (defined(BOARD_PCA10040) || defined(BOARD_PCA10056))
    if(button->buttonLastState() == gpio::Button::ButtonSingleClick)
    {
        switch(button->buttonId())
        {
            case ButtonId::Button1:
            {
                if(_blueLed != NULL)
                {
                    HF_DEBUG("[HeatFlow] onButtonSingleClick() > Starts blinking LED B\r\n");
                    _blueLed->startBlinking(true);
                }
                break;
            }
            case ButtonId::Button2:
            {
                if(_redLed != NULL)
                {
                    HF_DEBUG("[HeatFlow] onButtonSingleClick() > Starts blinking LED C\r\n");
                    _redLed->startBlinking(true);
                }
                break;
            }
            case ButtonId::Button3:
            {
                if(_ledD != NULL)
                {
                    HF_DEBUG("[HeatFlow] onButtonSingleClick() > Starts blinking LED D\r\n");
                    _ledD->startBlinking(true);
                }
                break;
            }
            default:
            {
                if(_blueLed != NULL && _redLed != NULL && _ledD != NULL)
                {
                    HF_DEBUG("[HeatFlow] onButtonSingleClick() > Synchronize all LEDs ...\r\n[HeatFlow] onButtonSingleClick() > _blueLed...\r\n");
                    _blueLed->startBlinking(true);
                    HF_DEBUG("[HeatFlow] onButtonSingleClick() > _redLed...\r\n");
                    _redLed->startBlinking(true);
                    HF_DEBUG("[HeatFlow] onButtonSingleClick() > _ledD...\r\n");
                    _ledD->startBlinking(true);
                }
                break;
            }
        }
    }    
    #endif
}
#endif

#if defined(SOFTDEVICE_PRESENT)
// -----------------------------------------------------------------------------------------------------------
void HeatFlow::onAdvertiseStopped(ble::PeripheralController * peripheral)
{
    HF_DEBUG("[HeatFlow] onAdvStoppped()\r\n");
    _currentState = ST_IDLE;
    _isBleAdvertising = false;
    
    #if (ADVERTISE_FOREVER != 0) && (START_ADVERTISE_AUTO == 0)
    _startAdvertise(_isCharging);
    #endif  // #if (ADVERTISE_FOREVER != 0) && (START_ADVERTISE_AUTO == 0)
}

// -----------------------------------------------------------------------------------------------------------
void HeatFlow:: onAdvertiseStateChanged(ble::PeripheralController * peripheral, ble_adv_evt_t bleAdvEvent)
{
    #if (DEBUG_HEATFLOW_ENABLE != 0)
    switch(bleAdvEvent)
    {
        case BLE_ADV_EVT_IDLE:                  HF_DEBUG("[HeatFlow] onAdvStateChanged() > BLE_ADV_EVT_IDLE\r\n");              break;
        case BLE_ADV_EVT_DIRECTED_HIGH_DUTY:    HF_DEBUG("[HeatFlow] onAdvStateChanged() > BLE_ADV_EVT_DIRECTED_HIGH_DUTY\r\n");break;
        case BLE_ADV_EVT_DIRECTED:              HF_DEBUG("[HeatFlow] onAdvStateChanged() > BLE_ADV_EVT_DIRECTED\r\n");          break;
        case BLE_ADV_EVT_FAST:                  HF_DEBUG("[HeatFlow] onAdvStateChanged() > BLE_ADV_EVT_FAST\r\n");              break;
        case BLE_ADV_EVT_FAST_WHITELIST:        HF_DEBUG("[HeatFlow] onAdvStateChanged() > BLE_ADV_EVT_FAST_WHITELIST\r\n");    break;
        case BLE_ADV_EVT_SLOW:                  HF_DEBUG("[HeatFlow] onAdvStateChanged() > BLE_ADV_EVT_SLOW\r\n");              break;
        case BLE_ADV_EVT_SLOW_WHITELIST:        HF_DEBUG("[HeatFlow] onAdvStateChanged() > BLE_ADV_EVT_SLOW_WHITELIST\r\n");    break;
        case BLE_ADV_EVT_WHITELIST_REQUEST:     HF_DEBUG("[HeatFlow] onAdvStateChanged() > BLE_ADV_EVT_WHITELIST_REQUEST\r\n"); break;
        case BLE_ADV_EVT_PEER_ADDR_REQUEST:     HF_DEBUG("[HeatFlow] onAdvStateChanged() > BLE_ADV_EVT_PEER_ADDR_REQUEST\r\n"); break;
        default: break;
    }
    #endif  // #if (DEBUG_HEATFLOW_ENABLE != 0)
    _currentState = ST_BLE_ADVERTISING;
    _isBleAdvertising = true;

    #if (USE_LEDS != 0) && defined(LED_1)    
    switch(bleAdvEvent)
    {
        case BLE_ADV_EVT_FAST:
        case BLE_ADV_EVT_FAST_WHITELIST:
        {
            _greenLed->startStandardBlinking();
            break;
        }
        case BLE_ADV_EVT_SLOW:
        case BLE_ADV_EVT_SLOW_WHITELIST:
        {
            _greenLed->startSlowBlinking();
            break;
        }
        case BLE_ADV_EVT_IDLE:
        {
            _greenLed->ledOff();
            break;
        }
        default:
            break;
    }
    #endif  // #if (USE_LEDS != 0) && defined(LED_1)
}

// -----------------------------------------------------------------------------------------------------------
void HeatFlow::onAdvertiseError(ble::PeripheralController * peripheral, uint32_t nrfError)
{
    HF_DEBUG("[HeatFlow] onAdvertiseError() > Error: 0x%04x\r\n", nrfError);
    #if (USE_LEDS != 0) && defined(LED_1)    
    _greenLed->ledOn();
    #endif  // #if (USE_LEDS != 0) && defined(LED_1)
}

// -----------------------------------------------------------------------------------------------------------
void HeatFlow::onPeripheralConnected(ble::PeripheralController * peripheral)
{
    HF_DEBUG("[HeatFlow] onBleConnected()\r\n");
    _currentState = ST_BLE_CONNECTED;
    _onBleConnected();
}

// -----------------------------------------------------------------------------------------------------------
void HeatFlow::onPeripheralDisconnected(ble::PeripheralController * peripheral)
{
    HF_DEBUG("[HeatFlow] onBleDisconencted()\r\n");
    _onBleDisconnected();
}

// -----------------------------------------------------------------------------------------------------------
void HeatFlow::onPeripheralStateChanged(ble::PeripheralController * peripheral, uint8_t state)
{
    HF_DEBUG("[HeatFlow] onPeripheralStateChanged() > new GAP's Peripheral state: %s\r\n", 
                ble::PeripheralController::state2String(static_cast<ble::PeripheralController::peripheralState_e>(state)));
}

// -----------------------------------------------------------------------------------------------------------
void HeatFlow::onPasskeyDisplay(ble::PeripheralController* peripheral, char* passkey)
{
    HF_DEBUG("[HeatFlow] onPasskeyDisplay() > Passkey: %s\r\n", passkey);
}

// -----------------------------------------------------------------------------------------------------------
void HeatFlow::onAuthStatusUpdated(ble::PeripheralController* peripheral, ble_gap_evt_auth_status_t status)
{
    HF_DEBUG("[HeatFlow] onAuthStatusUpdated() > Auth. Status: %d\r\n", status);
}


#if (MHFS_SUPPORT != 0)
// -----------------------------------------------------------------------------------------------------------
void HeatFlow::onConfigurationChange(ble::MhfsService* mhfsSrv, uint8_t adsId, uint16_t config)
{
    HF_DEBUG("[HeatFlow] onConfigurationChange() > ads#%d config: 0x%04x\r\n", adsId, config);
}

// -----------------------------------------------------------------------------------------------------------
void HeatFlow::onCommandChange(ble::MhfsService* mhfsSrv, eMhfsCommandId commandId)
{
    HF_DEBUG("[HeatFlow] onConfigurationChange() > ads command: %d\r\n", commandId);
    switch(commandId)
    {
        case MhfsStop:
        {
            _handleMeasurement(false);
            break;
        }
        case MhfsStart:
        {
            _handleMeasurement(true);
            break;
        }
        default:
        {
            break;
        }
    }
}
#endif  // #if (MHFS_SUPPORT != 0)
#endif  // #if defined(SOFTDEVICE_PRESENT)

#if (USE_FLASH != 0) && (FLASH_CONTROLLER_TEST_MODE != 0)
// -----------------------------------------------------------------------------------------------------------
void HeatFlow::onDataSaved(flash::FlashController* flashController)
{
    HF_DEBUG("[HeatFlow] onDataSaved() ...\r\n");
}

// -----------------------------------------------------------------------------------------------------------
void HeatFlow::onDataRestored(flash::FlashController* flashController)
{
    HF_DEBUG("[HeatFlow] onDataRestored() ...\r\n");
}

// -----------------------------------------------------------------------------------------------------------
void HeatFlow::onDataCleared(flash::FlashController* flashController)
{
    HF_DEBUG("[HeatFlow] onDataCleared() ...\r\n");
}

// -----------------------------------------------------------------------------------------------------------
void HeatFlow::onFlashError(flash::FlashController* flashController)
{
    HF_DEBUG("[HeatFlow] onFlashError() ...\r\n");
}
#endif  // #if (USE_FLASH != 0) && (FLASH_CONTROLLER_TEST_MODE != 0)

#if (USE_BATTERY_MANAGER != 0)
// -----------------------------------------------------------------------------------------------------------
void HeatFlow::onBatteryStateChanged(battery::BatteryManager* batteryManager, 
                                       battery::BatteryState::eBatteryState state)
{
    HF_DEBUG("[HeatFlow] onBatteryStateChanged() > new state: %s\r\n", battery::BatteryState::toString(state));
}

// -----------------------------------------------------------------------------------------------------------
void HeatFlow::onBatteryLevelChanged(battery::BatteryManager* batteryManager, uint8_t batteryLevel)
{
    HF_DEBUG("[HeatFlow] onBatteryLevelChanged() > New battery level in percent: %d\r\n", batteryLevel);
    uint32_t errCode = ble_bas_battery_level_update(_peripheralController->bleAdoptedServices()->basService(), batteryLevel, _peripheralController->connHandle());
    if(errCode != NRF_SUCCESS)
    {
        HF_DEBUG("[HeatFlow] onBatteryLevelChanged() > ble_bas_battery_level_update() ERROR: 0x%04x\r\n", errCode);
    }
}
#endif  // #if (USE_BATTERY_MANAGER != 0)



// -----------------------------------------------------------------------------------------------------------
EventStatus HeatFlow::processEvent()
{
    switch(getCurrentEvent()->getEventType())
    {
        /* ************************************************************************************************ */
        /* `Event` event                                                                                    */
        /* ************************************************************************************************ */
        case IXFEvent::Event:
        {
            break;
        }
        /* ************************************************************************************************ */
        /* `Timeout` event                                                                                  */
        /* ************************************************************************************************ */
        case IXFEvent::Timeout:
        {
            switch( getCurrentTimeout()->getId() )
            {
                case TimeoutNoActivityId:
                {
                    _isTimeoutNoActivityScheduled = false;
                    _noActivityCounter -= NO_ACTIVITY_DECREMENT;
                    HF_DEBUG("[HeatFlow] processEvent() > _noActivityCounter = %d\r\n", _noActivityCounter);
                    if(_noActivityCounter == 0)
                    {
                        HF_DEBUG("[HeatFlow] processEvent() > -> ST_SYSTEM_OFF ...\r\n");
                        _currentState = ST_SYSTEM_OFF;
                        _ST_SYSTEM_OFF_Action();
                    }
                    else
                    {
                        _handleNoActivityTimeout(true);
                    }
                    break;
                }
                #if (USE_ADS111X != 0)
                case TimeoutAdsId:
                {
                    static uint8_t adsCounter = 0;
                    _isTimeoutAdsScheduled = false;
                    if(_isMeasuring) 
                    {
                        HF_DEBUG("[HeatFlow] processEvent() > TimeoutAdsId -> ADS should stop measuring ...\r\n");
                        _isMeasuring = false;
                        adsCounter++;
                        _sensorCtrl->setState(sensor::SensorController::ST_SHUTDOWN);
                    }
                    else
                    {
                        HF_DEBUG("[HeatFlow] processEvent() > TimeoutAdsId -> ADS should start measuring ...\r\n");
                        _isMeasuring = true;
                        _sensorCtrl->setState(sensor::SensorController::ST_MEASUREMENT);
                    }
                    
                     if(adsCounter < 10) {
                         _handleAdsTimeout(true);
                     }
                    break;
                }
                #endif  // #if (USE_ADS111X != 0)
                case TimeoutGenericId:
                {
                    _isTimeoutGenericScheduled = false;
                    break;
                }
                default:
                {
                    break;
                }
            }
            break;
        }
        /* ************************************************************************************************ */
        /* `Initial` event                                                                                  */
        /* ************************************************************************************************ */
        case IXFEvent::Initial:
        {
            if(_currentState == ST_INIT)
            {
                //HFS_DEBUG("[HeatFlow] processEvent() > -> ST_IDLE from ST_INIT ((IXFEvent::Initial)\r\n");
                _currentState = ST_IDLE;
                
                #if defined(SOFTDEVICE_PRESENT)
                    #if (ADVERTISE_FOREVER != 0) || (START_ADVERTISE_AUTO != 0)
                _startAdvertise(_isCharging);
                    #else
                _stopAdvertise();
                _handleMeasurement(true);
                    #endif  // #if defined(ADVERTISE_FOREVER != 0)
                #endif  // #if defined(SOFTDEVICE_PRESENT)
                
                #if (USE_BUTTONS != 0)
                for(uint8_t i = 0; i < BUTTONS_NUMBER; i++)
                {
                    if(_buttons[i] != NULL)
                    {
                        _buttons[i]->addObserver(this);
                    }
                }
                #endif  // #if (USE_BUTTONS != 0)

                #if (USE_BATTERY_MANAGER != 0)
                _batteryManager->addObserver(this);
                _batteryManager->enable();
                #endif  // #if (USE_BATTERY_MANAGER != 0)

                #if (USE_UPTIME != 0)
                _upTime.start();
                #endif    // USE_UPTIME
                
                #if (ENABLE_SYSTEM_OFF != 0)
                _handleNoActivityTimeout(true);
                #endif
            }
            break;
        }
        default:
        {
            break;
        }
    }

    return EventStatus::Consumed;
}

/************************************************************************************************************/
/*                                             PRIVATE SECTION                                              */
/************************************************************************************************************/
#if defined(SOFTDEVICE_PRESENT)
// -----------------------------------------------------------------------------------------------------------
void HeatFlow::_startAdvertise(bool withoutWhitelist)
{
    if(!_isBleConnected)
    {        
        if((_currentState != ST_INIT) && (_currentState != ST_UNKOWN))
        { 
            HF_DEBUG("[HeatFlow] _startAdvertise() > start advertising\r\n");
            _peripheralController->startAdvertising(withoutWhitelist);
            #if (USE_LEDS != 0) && defined(LED_2)
            _blueLed->ledOff();
            #endif  // #if (USE_LEDS != 0) && defined(LED_2)
        }  
        else
        {
            HF_DEBUG("[HeatFlow] _startAdvertise() > the application is not ready to advertise\r\n");
        }
    }
    else
    {
        HF_DEBUG("[HeatFlow] _startAdvertise() > the watch is connected don't advertise\r\n");
    }
}

// -----------------------------------------------------------------------------------------------------------
void HeatFlow::_stopAdvertise()
{
    if(_isBleAdvertising)
    {  
        HF_DEBUG("[HeatFlow] _stopAdvertise() > stop advertising\r\n");
        _peripheralController->stopAdvertising();
        _isBleAdvertising = false;
        #if (USE_LEDS != 0) && defined(LED_2)
        _blueLed->startSlowBlinking();
        #endif  // #if (USE_LEDS != 0) && defined(LED_2)
    }
}

// -----------------------------------------------------------------------------------------------------------
void HeatFlow::_onBleConnected()
{
    HF_DEBUG("[HeatFlow] _onBleConnected() > ...\r\n");
    _isBleConnected = true;
    _stopAdvertise();
    #if (USE_LEDS != 0) && defined(LED_1)
    _greenLed->startQuickBlinking();
    #endif  // #if (USE_LEDS != 0) && defined(LED_1)
}

// -----------------------------------------------------------------------------------------------------------
void HeatFlow::_onBleDisconnected()
{
    HF_DEBUG("[HeatFlow] _onBleDisconnected() > ...\r\n");
  
    _currentState = ST_IDLE;
    _handleMeasurement(false);
    _isBleConnected = false;
}

// -----------------------------------------------------------------------------------------------------------
void HeatFlow::_updateSecurityLevelForServices(bool justWorks, bool mitm, bool whiteList)
{
    _peripheralController->setSecuritySettings(justWorks, mitm, whiteList);
    _peripheralController->updateSecurityForCharacteristics();
    _peripheralController->startAdvertising();
}

// -----------------------------------------------------------------------------------------------------------
void HeatFlow::_handleAdsTimeout(bool start)
{
	if(start)
	{
		if(!_isTimeoutAdsScheduled)
		{
			getThread()->scheduleTimeout(TimeoutAdsId, 10000, this);
			_isTimeoutAdsScheduled = true;
            _noActivityCounter = NO_ACTIVITY_DELAY;
		}
	}
	else
	{
		_isTimeoutAdsScheduled = false;
		getThread()->unscheduleTimeout(TimeoutAdsId, this);
        _sensorCtrl->setState(sensor::SensorController::ST_IDLE);
	}
}

// -----------------------------------------------------------------------------------------------------------
void HeatFlow::_handleNoActivityTimeout(bool start)
{
	#if (ENABLE_SYSTEM_OFF != 0)
	if(start)
	{
		if(!_isTimeoutNoActivityScheduled)
		{
			getThread()->scheduleTimeout(TimeoutNoActivityId, NO_ACTIVITY_TIMEMOUT_STEP, this);
			_isTimeoutNoActivityScheduled = true;
		}
	}
	else
	{
		_isTimeoutNoActivityScheduled = false;
		getThread()->unscheduleTimeout(TimeoutNoActivityId, this);
		_noActivityCounter = NO_ACTIVITY_DELAY;
	}
	#endif
}

// ------------------------------------------------------------------------------------------------
void HeatFlow::_ST_SYSTEM_OFF_Action()
{
	_greenLed->ledOff();
	_blueLed->ledOff();
	_redLed->ledOff();
	_isBleConnected = false;
    
    _peripheralController->stopAdvertising();
	_isBleAdvertising = false;
	
	HF_DEBUG("[HeatFlow] _ST_SYSTEM_OFF_Action() > Device enter system OFF mode...\r\n" );
	sd_power_system_off();
}

// ------------------------------------------------------------------------------------------------
void HeatFlow::_handleMeasurement(bool enable)
{
    if(enable)
    {
        if(!_isMeasuring)
        {
            HF_DEBUG("[HeatFlow] onCommandChange() > TimeoutAdsId -> ADS should start measuring ...\r\n");
            _isMeasuring = true;
            _sensorCtrl->setState(sensor::SensorController::ST_MEASUREMENT);
        }
    }
    else
    {
        if(_isMeasuring)
        {
            HF_DEBUG("[HeatFlow] onCommandChange() > TimeoutAdsId -> ADS should stop measuring ...\r\n");
            _isMeasuring = false;
            _sensorCtrl->setState(sensor::SensorController::ST_SHUTDOWN);
        }
    }
}


 

#endif  // #if defined(SOFTDEVICE_PRESENT)

