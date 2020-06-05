/*************************************************************************************************************
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 ************************************************************************************************************/

/************************************************************************************************************/
/* Header files                                                                                             */
/************************************************************************************************************/
#include "batterymanager.h"

#if (USE_BATTERY_MANAGER != 0)
#include <boards.h>
#include "app_timer.h"

#include "tools/sdktools.h"
#include "nrf_gpio.h"
#include "nrf_saadc.h"


/************************************************************************************************************/
/* DEBUG COMPILATION OPTION & MACRO                                                                         */
/************************************************************************************************************/
#define DEBUG_BATT_ENABLE                   0
#if (DEBUG_BATT_ENABLE != 0) && defined(DEBUG_NRF_USER)
    #include <tools/logtrace.h>
    #define BATT_DEBUG                      LOG_TRACE_DEBUG
#else
    #define BATT_DEBUG(...)                 {(void) (0);}
#endif

#define DEBUG_BATT_ST_ENABLE                0
#if (DEBUG_BATT_ST_ENABLE != 0) && defined(DEBUG_NRF_USER)
    #include <tools/logtrace.h>
    #define BATTS_DEBUG                     LOG_TRACE_DEBUG
#else
    #define BATTS_DEBUG(...)                {(void) (0);}
#endif

#define DEBUG_BATT_DEEP_ENABLE              0
#if (DEBUG_BATT_DEEP_ENABLE != 0) && defined(DEBUG_NRF_USER)
    #include <tools/logtrace.h>
    #define BADE_DEBUG                      LOG_TRACE_DEBUG
#else
    #define BADE_DEBUG(...)                 {(void) (0);}
#endif

/************************************************************************************************************/
/* Namespace Declaration                                                                                    */
/************************************************************************************************************/
using battery::BatteryManager;

/************************************************************************************************************/
/* Declaration Section                                                                                      */
/************************************************************************************************************/
#if (APP_BATT_USE_XF_TIMER == 0)
    APP_TIMER_DEF(_batteryManagerTimerId);

    /********************************************************************************************************/
    /* --- Extern "C" Section ---                                                                           */
    /*     Callback's section                                                                               */
    /********************************************************************************************************/
    // -------------------------------------------------------------------------------------------------------
    void battery::battManagerTimeoutHandler(void* pContext)
    {
        if(pContext != NULL)
        {
            BADE_DEBUG("[BatteryManager] battManagerTimeoutHandler() > start a measure with the SAADC \r\n");
            static_cast<BatteryManager*>(pContext)->onBatteryManagerTimeout();
        }
        else
        {
            BADE_DEBUG("[BatteryManager] battManagerTimeoutHandler() > Context is NULL !\r\n");
            ASSERT(false);
        }
    }
#endif


/************************************************************************************************************/
/* Constructor | Destructor                                                                                 */
/************************************************************************************************************/
// -----------------------------------------------------------------------------------------------------------
BatteryManager::BatteryManager() : _currentState(BatteryManager::ST_INIT),
                                   _batteryState(BatteryState::BATT_STATE_UNKNOWN),
                                   _batteryLevel(0xff),
                                   _isTimerInitialized(false),
                                   _isTimerEnabled(false)
{
#if (APP_BATT_USE_XF_TIMER != 0)
    #if (DEBUG_BATT_ENABLE != 0) || (DEBUG_BATT_DEEP_ENABLE != 0) || (DEBUG_BATT_ST_ENABLE != 0)
    _measuringWaitingDelay          = 15000;
    #else
    _measuringWaitingDelay          = (BATT_MEASURING_TIMEOUT_DELAY);
    _measuringShippingDelay         = (BATT_MEASURING_SHIPPING_DELAY);
    #endif
    _measuringDelayAfterCharging    = (BATT_AFTER_CHARGE_MEAS_DELAY);
    _inBetweenSamplesDelay          = (APP_BATT_SAMPLE_DELAY);
    _prepareGPIODelay               = (APP_BATT_PREPARE_GPIO_DELAY);
    _standardDelay                  = (APP_BATT_MANAGER_STD_DELAY);
#else
    #if (DEBUG_BATT_DEEP_ENABLE != 0)
    _measuringWaitingDelay          = APP_TIMER_TICKS(15000);
    #else
    _measuringWaitingDelay          = APP_TIMER_TICKS(BATT_MEASURING_TIMEOUT_DELAY);
    _measuringShippingDelay         = APP_TIMER_TICKS(BATT_MEASURING_SHIPPING_DELAY);
    #endif
    _measuringDelayAfterCharging    = APP_TIMER_TICKS(BATT_AFTER_CHARGE_MEAS_DELAY);
    _inBetweenSamplesDelay          = APP_TIMER_TICKS(APP_BATT_SAMPLE_DELAY);
    _prepareGPIODelay               = APP_TIMER_TICKS(APP_BATT_PREPARE_GPIO_DELAY);
    _standardDelay                  = APP_TIMER_TICKS(APP_BATT_MANAGER_STD_DELAY);
#endif

    for(uint8_t i = 0; i < APP_BATT_SAMPLES; i++)
    {
        _batteryLevelBuffer[i]  = 0;
    }

#if (USE_SAADC != 0)
    _isSaadcInitialized         = false;
    #if (APP_BATT_USE_BATT_CARACTERISATION == 0)
    _a                          = (BATT_SAADC_INTERNAL_VMAX * 100) / (((float)BATT_SAADC_VALMAX * BATT_SAADC_VMAX) - ((float)BATT_SAADC_VALMIN * BATT_SAADC_INTERNAL_VMAX));
    _b                          = (uint64_t) (_a * (float)BATT_SAADC_VALMIN);
    #endif  // #if (APP_BATT_USE_BATT_CARACTERISATION == 0)
#endif  // #if (USE_SAADC != 0)

#if (USE_CHARGER != 0)
    _chargeTime                 = 0;
    _charging                   = false;
    _charger                    = NULL;
#endif  // #if (USE_CHARGER != 0)
}

// -----------------------------------------------------------------------------------------------------------
BatteryManager::~BatteryManager()   {}

/************************************************************************************************************/
/*                                              PUBLIC SECTION                                              */
/************************************************************************************************************/
// -----------------------------------------------------------------------------------------------------------
void BatteryManager::initialize()
{
    // Initialize the different app timer we need
    if(!_isTimerInitialized)
    {
        #if (APP_BATT_USE_XF_TIMER == 0)
        uint32_t errCode = tools::SDKTools::appTimerCreate(&_batteryManagerTimerId,
                                                           APP_TIMER_MODE_SINGLE_SHOT,
                                                           (void*) battery::battManagerTimeoutHandler);
        if(errCode != NRF_SUCCESS)
        {
            BATT_DEBUG("[BatteryManager] initialize() > battMeasTimer: createTimer() FAILED (error: 0x%04x)\r\n", errCode);
            ASSERT(false);
            return;
        }
        #else
        this->startBehaviour();
        #endif  // #if (APP_BATT_USE_XF_TIMER == 0)
        _isTimerInitialized = true;
    }
    BATT_DEBUG("[BatteryManager] initialize() > ... done !\r\n");
}

// -----------------------------------------------------------------------------------------------------------
void BatteryManager::bind(
    #if (USE_SAADC != 0)
    hal::SaadcHal* saadcHal
    #endif
    #if (USE_CHARGER != 0)
  , Charger* charger
        #if (USE_CALENDAR != 0)
  , cal::Calendar* calendar
        #endif
    #endif
)
{
    #if (USE_SAADC != 0)
    if(saadcHal == NULL)
    {
        BATT_DEBUG("[BatteryManager] bind() > SAADC reference is NULL !\r\n");
        ASSERT(false);
    }
    _saadcHal   = saadcHal;
    _saadcHal->addObserver(this);
    #endif  // #if (USE_SAADC != 0)

    #if (USE_CHARGER != 0)
    if(charger == NULL)
    {
        BATT_DEBUG("[BatteryManager] bind() > Charger reference is NULL !\r\n");
        ASSERT(false);
    }
    _charger    = charger;
    charger->addObserver(this);
    #if (USE_CALENDAR != 0)
    if(calendar == NULL)
    {
        BATT_DEBUG("[BatteryManager] bind() > Calendar reference is NULL !\r\n");
        ASSERT(false);
    }
    _calendar   = calendar;
    #endif  // #if (USE_CALENDAR != 0)
    #endif  // #if (USE_CHARGER != 0)

    #if (USE_SAADC != 0)
    if(!_isSaadcInitialized)
    {
        if(!_saadcHal->addObserver(this))
        {
            BATT_DEBUG("[BatteryManager] initialize() > _saadcHal->addObserver() FAILED !\r\n");
            ASSERT(false);
        }
        BATT_DEBUG("[BatteryManager] initialize() > _saadcHal->addObserver() SUCCESS !\r\n");
    }
    #endif  // #if (USE_SAADC != 0)
}

// -----------------------------------------------------------------------------------------------------------
void BatteryManager::disable()
{
    if(_stopTimer())
    {
        _currentState = ST_IDLE;
    }
    else
    {
        BATT_DEBUG("[BatteryManager] disable() > _stopTimer FAILED !\r\n");
        ASSERT(false);
    }
}

// -----------------------------------------------------------------------------------------------------------
void BatteryManager::enable()
{
    _startBatteryMeasurement();
}

// -----------------------------------------------------------------------------------------------------------
void BatteryManager::performShippingMode(bool enter)
{
    // stop the measurement
    disable();

   // Update state
    _updateBatteryState(enter);

    // restart in shipping or normal mode
    enable();
}

#if (USE_FLASH != 0)
// -----------------------------------------------------------------------------------------------------------
uint32_t* BatteryManager::save(uint16_t* size)
{
    *size = sizeof(BatteryManagerPersistor) / 4;
    BATT_DEBUG("[BatteryManager] save() > saving battery manager's parameters: charging cycles = %d\r\n", _persistentParams.chargingCyclesCounter());
    return _persistentParams.data();
}

// -----------------------------------------------------------------------------------------------------------
void BatteryManager::restore(uint32_t* data)
{
    memcpy(_persistentParams.data(), data, sizeof(BatteryManagerPersistor));
    BATT_DEBUG("[BatteryManager] restore() > restoring battery manager's parameters: charging cycles = %d\r\n", _persistentParams.chargingCyclesCounter());
}
#endif  // #if (USE_FLASH != 0)


/************************************************************************************************************/
/*                                            PROTECTED SECTION                                             */
/************************************************************************************************************/
// -----------------------------------------------------------------------------------------------------------
void BatteryManager::onBatteryManagerTimeout()
{
    _isTimerEnabled = false;

    switch(_currentState)
    {
        case ST_WAITING:
            BATTS_DEBUG("[BatteryManager] onBatteryManagerTimeout() > ST_WAITING...\r\n");
            if(!_saadcHal->isInitialized())
            {
                _isSaadcInitialized = _initMeasurer();
                BATTS_DEBUG("[BatteryManager] onBatteryManagerTimeout() > Initialize SAADC...\r\n");
                if(_startTimer(_inBetweenSamplesDelay))
                {
                    #if (USE_CHARGER != 0) && (USE_STBC02 != 0)
                    _currentState = ST_PREPARE_GPIO;
                    #else
                    _currentState = ST_MEASURING;
                    #endif
                }
            }
            break;
            
        case ST_PREPARE_GPIO:
            BATTS_DEBUG("[BatteryManager] onBatteryManagerTimeout() > ST_PREPARE_GPIO...\r\n");
            #if (USE_CHARGER != 0) && (USE_STBC02 != 0)
            _charger->sendSWIRECommand(battery::Charger::BATMS_ON);
            #endif
            BATTS_DEBUG("[BatteryManager] onBatteryManagerTimeout() > Enabling STBC (BATMS_ON)...\r\n");
            if(_startTimer(_prepareGPIODelay))
            {
                _currentState = ST_MEASURING;
            }
            break;
            
        case ST_MEASURING:
            BATTS_DEBUG("[BatteryManager] onBatteryManagerTimeout() > ST_MEASURING...\r\n");
            if(_measure())
            {
                _startTimer(_inBetweenSamplesDelay);
            }
            break;
            
        case ST_MEASURE_DONE:
        {
            BATTS_DEBUG("[BatteryManager] onBatteryManagerTimeout() > ST_MEASURE_DONE...\r\n");
            uint32_t delay = ((_batteryState == BatteryState::BATT_STATE_SHIPPING_MODE_CHARGED)    ||
                              (_batteryState == BatteryState::BATT_STATE_SHIPPING_MODE_DISCHARGED)) ? _measuringShippingDelay : _measuringWaitingDelay;
            if(_saadcHal->uninit() && _startTimer(delay))
            {
                #if (USE_CHARGER != 0) && (USE_STBC02 != 0)
                BATTS_DEBUG("[BatteryManager] onBatteryManagerTimeout() > Disabling STBC (BATMS_OFF)...\r\n");
                _charger->sendSWIRECommand(battery::Charger::BATMS_OFF);
                #endif
                _isSaadcInitialized = false;
                _currentState = ST_WAITING;
            }
        }
            break;
            
        default:
            BATTS_DEBUG("[BatteryManager] onBatteryManagerTimeout() > State not handled ! state = %d\r\n", _currentState);
            break;
    }
}

// -----------------------------------------------------------------------------------------------------------
#if (USE_SAADC != 0)
void BatteryManager::onMeasureDone(hal::SaadcHal* saadcHal, nrf_saadc_value_t* buff, uint16_t buffSize)
{
    if(_currentState == ST_MEASURING)
    {
        #if (APP_USE_FAKE_BATT_LEVEL != 0)
        _batteryLevel           = APP_FAKE_BATT_LEVEL;
        _currentState           = ST_MEASURE_DONE;
        #else
        static int16_t sum      = 0;
        // Checks the size of the received buffer
        if(buffSize > APP_BATT_SAMPLES)
        {
            BATT_DEBUG("[BatteryManager] onMeasureDone() > Too many samples in the buffer: %d \r\n", buffSize);
            buffSize            = APP_BATT_SAMPLES;
        }

        // Copying the samples in the Battery Manager record
        for(uint8_t i = 0; i < buffSize; i++)
        {
            _batteryLevelBuffer[_measurementCounter++] = buff[i];
            sum                                       += buff[i];
            BATT_DEBUG("[BatteryManager] onMeasureDone() > Copying sample #%d: %d\r\n", _measurementCounter, buff[i]);
            if(_measurementCounter == APP_BATT_SAMPLES)
            {
                BATT_DEBUG("[BatteryManager] onMeasureDone() > Local buffer is full !\r\n");
                break;
            }
        }

        // Still measurements to do ?
        if(_measurementCounter >= APP_BATT_SAMPLES)
        {
            BATT_DEBUG("[BatteryManager] onMeasureDone() > Processing Battery level... (%d)\r\n", _measurementCounter);
            _currentState       = ST_MEASURE_DONE;
            int16_t mean        = sum / _measurementCounter;
            if(mean < BATT_SAADC_VALMIN)
            {
                mean            = BATT_SAADC_VALMIN;
            }
            else if(mean > BATT_SAADC_VALMAX)
            {
                mean            = BATT_SAADC_VALMAX;
            }
            BATT_DEBUG("[BatteryManager] onMeasureDone() > sum: %d, mean: %d\r\n", sum, mean);
            sum                 = 0;
            _measurementCounter = 0;

            // Process battery level and notify observer and update battery's State if needed !
            _getBattLevelInPercent(mean);
        }
        #endif  // #if (USE_FAKE_BATT_LEVEL != 0)
        if(!_startTimer(_inBetweenSamplesDelay))
        {
            BATT_DEBUG("[BatteryManager] onMeasureDone() > Error wile restarting BatteryManager's timer !\r\n");
        }
    }
}
#endif  // #if (USE_SAADC != 0)

#if (USE_CHARGER != 0)
// -----------------------------------------------------------------------------------------------------------
void BatteryManager::onChargerStateChanged(Charger* charger, ChargerState::eState state)
{
    switch(state)
    {
        case ChargerState::ST_CHARGER_CHARGE:
        {
            BATT_DEBUG("[BatteryManager] onChargerStateChanged() > start charging the battery ...\r\n");

            // Stops any measurement
            disable();

            // convert percentage in charge time
            _chargeTime = _percentToChargeTime();
            BATT_DEBUG("[BatteryManager] onChargerStateChanged() > charge time updated to %ds which will be converted to %d [percent] \r\n", _chargeTime, _chargeTimeToPercent());
            _charging = true;

            #if (APP_BATT_USE_XF_TIMER != 0)
            if(!_isChargerTimerEnabled)
            {
                getThread()->scheduleTimeout(TimeoutChargerId, 1000, this);
                _isChargerTimerEnabled = true;
            }
            #elif (USE_CALENDAR != 0)
            _calendar->addObserver(this);
            #endif
            break;
        }
        case ChargerState::ST_CHARGER_CHARGE_END:
        {
            BATT_DEBUG("[BatteryManager] onChargerStateChanged() > stop charging the battery \r\n");
            #if (APP_BATT_USE_XF_TIMER != 0)
            if(_isChargerTimerEnabled)
            {
                _isChargerTimerEnabled = false;
                getThread()->unscheduleTimeout(TimeoutChargerId, this);
            }
            #elif (USE_CALENDAR != 0)
            _calendar->delObserver(this);
            #endif
            _persistentParams.incrChargingCyclesCounter();

            BATT_DEBUG("[BatteryManager] onChargerStateChanged() > the battery finished to charge \r\n");
            uint8_t level = _chargeTimeToPercent();
            if(level > _batteryLevel)
            {
                // convert charge time in percent
                _batteryLevel = _chargeTimeToPercent();
            }

            // notify new battery value
            BATT_DEBUG("[BatteryManager] onChargerStateChanged() > the battery is charged at: %d% \r\n", _batteryLevel);
            _notifyBattLevelChanged();

            // update battery state
            _updateBatteryState();

            // clear charging flag
            _charging = false;

            // start after charge timer
            if(_startTimer(_measuringDelayAfterCharging))
            {
                _currentState = ST_WAITING;
            }
            break;
        }
        default:
            BATT_DEBUG("[BatteryManager] onChargerStateChanged() > New state: %s \r\n", ChargerState::toString(state));
            break;
    }
}

// -----------------------------------------------------------------------------------------------------------
void BatteryManager::onChargerAvailable(battery::Charger* charger, bool available)
{
    if(available)
    {
        BATT_DEBUG("[BatteryManager] onChargerAvailable() > a charger is detected\r\n");
    }
    else
    {
        BATT_DEBUG("[BatteryManager] onChargerAvailable() > the charger is no more detected !\r\n");
    }
}

// -----------------------------------------------------------------------------------------------------------
#if (USE_CALENDAR != 0)
void BatteryManager::BatteryManager::onSecond(cal::Calendar* calendar)
{
    _chargerTimerHandler();
}
#endif  // #if (USE_CALENDAR != 0)
#endif  // #if (USE_CHARGER != 0)

#if (APP_BATT_USE_XF_TIMER != 0)
//------------------------------------------------------------------------------------------------------------
EventStatus BatteryManager::processEvent()
{
    switch(getCurrentEvent()->getEventType())
    {
        /****************************************************************************************************/
        /* Timeout event                                                                                    */
        /****************************************************************************************************/
        case IXFEvent::Timeout:
        {
            eTimeoutId timeoutId = static_cast<eTimeoutId>(getCurrentTimeout()->getId());
            BADE_DEBUG("[BatteryManager] processEvent() > Timeout Event received (id: %s)...\r\n", 
                        timeoutId == TimeoutBatteryId ? "TimeoutBatteryId":"TimeoutChargerId");
            if(timeoutId == TimeoutBatteryId && _isTimerEnabled)
            {
                onBatteryManagerTimeout();
            }
            #if (USE_CHARGER != 0)
            else if(timeoutId == TimeoutChargerId && _isChargerTimerEnabled)
            {
                _isChargerTimerEnabled = false;
                getThread()->scheduleTimeout(Timeout_Charger_id, 1000, this);
                _isChargerTimerEnabled = true;
                _chargerTimerHandler();
            }
            #endif // #if (USE_CHARGER != 0)
            break;
        }

        /****************************************************************************************************/
        /* Initial event                                                                                    */
        /****************************************************************************************************/
        case IXFEvent::Initial:
        {
            BADE_DEBUG("[BatteryManager] processEvent() > Initiale Event received...\r\n");
            if(_currentState == ST_INIT)
            {
                // Update State
                _currentState = ST_IDLE;
            }
            break;
        }
        default:
            break;
    }

    return EventStatus::Consumed;
}
#endif  // #if (APP_BATT_USE_XF_TIMER != 0)


/************************************************************************************************************/
/*                                             PRIVATE SECTION                                              */
/************************************************************************************************************/
// -----------------------------------------------------------------------------------------------------------
bool BatteryManager::_initMeasurer()
{
    #if (USE_SAADC != 0)
    nrfx_saadc_config_t         config;
    nrf_saadc_channel_config_t  channelConf;

    //Configure SAADC
    config.low_power_mode       = true;
    config.resolution           = (nrf_saadc_resolution_t) APP_BATT_SAADC_RESOLUTION;
    config.oversample           = NRF_SAADC_OVERSAMPLE_DISABLED;
    config.interrupt_priority   = NRFX_SAADC_CONFIG_IRQ_PRIORITY;

    //Configure SAADC channel
    channelConf.reference       = NRF_SAADC_REFERENCE_INTERNAL;
    channelConf.gain            = (nrf_saadc_gain_t) BATT_LEVEL_SAADC_GAIN;
    channelConf.acq_time        = NRF_SAADC_ACQTIME_10US;
    channelConf.mode            = NRF_SAADC_MODE_SINGLE_ENDED;
    channelConf.burst           = NRF_SAADC_BURST_DISABLED;
    channelConf.pin_p           = (nrf_saadc_input_t) BATT_LEVEL_MEASURE_GPIO;
    channelConf.pin_n           = NRF_SAADC_INPUT_DISABLED;
    channelConf.resistor_p      = NRF_SAADC_RESISTOR_DISABLED;
    channelConf.resistor_n      = NRF_SAADC_RESISTOR_DISABLED;
    #endif

    return _saadcHal->init(&config, &channelConf);
}

// -----------------------------------------------------------------------------------------------------------
void BatteryManager::_getBattLevelInPercent(int16_t battLevel)
{
    uint64_t level          = (uint64_t) (battLevel & 0xffff);
    #if (APP_BATT_USE_BATT_CARACTERISATION != 0)
    uint64_t levelSquare    = level * level;
    uint64_t a              = (levelSquare * level) / (BATT_C0_INT * BATT_SCALE_C0_I);
    uint32_t b              = (levelSquare * (uint64_t) BATT_SCALE_C1) / BATT_C1_INT;
    uint32_t c              = (level * (uint64_t) BATT_C2_INT) / BATT_SCALE_C2;
    uint64_t result         = BATT_C3_INT / BATT_SCALE_C3;
    BATT_DEBUG("[BatteryManager] _getBattLevelInPercent() > result: %llu, a: %u, b: %du, c: %du\r\n", (uint8_t) result, a, b, c);

    result                 -= a;
    result                 += b;
    result                 -= c;
    #else
    int64_t result          = 0;
    if(level > BATT_SAADC_VALMIN)
    {
        result              = (uint64_t) (_a * (float) level) - _b;
    }
    #endif
    if(result > 100)
    {
        result              = 100;
    }

    #if (APP_BATT_USE_BATT_CARACTERISATION != 0)
    BATT_DEBUG("[BatteryManager] _getBattLevelInPercent() > Battery level measured in percent: %d, _batteryLevel: %d (a: %llu b: %du c: %du)\r\n", (uint8_t) result, _batteryLevel, a, b, c);
    #else
    BATT_DEBUG("[BatteryManager] _getBattLevelInPercent() > Battery level measured in percent: %d, _batteryLevel: %d\r\n", (uint8_t) result, _batteryLevel);
    #endif
    if(result < _batteryLevel)
    {
        _batteryLevel = result;
        _notifyBattLevelChanged();
        _updateBatteryState();
    }
}

// -----------------------------------------------------------------------------------------------------------
void BatteryManager::_startBatteryMeasurement()
{
    BATT_DEBUG("[BatteryManager] _startBatteryMeasurement() > Start Battery Manager...\r\n");
    disable();
    if(_currentState == ST_IDLE)
    {
        _currentState = ST_WAITING;
        _startTimer(_inBetweenSamplesDelay);
    }
}

// -----------------------------------------------------------------------------------------------------------
bool BatteryManager::_measure()
{
    if(!_isSaadcInitialized)
    {
        BATT_DEBUG("[BatteryManager] _measure() > Initialize first the SAADC Hardware\r\n");
        _isSaadcInitialized = _initMeasurer();
    }
    if(_isSaadcInitialized && _saadcHal->sample())
    {
        BATT_DEBUG("[BatteryManager] _measure() > Sampling done...\r\n");
        return true;
    }
    BATT_DEBUG("[BatteryManager] _measure() > Error while starting a measuremenet...\r\n");
    return false;
}

// -----------------------------------------------------------------------------------------------------------
void BatteryManager::_updateBatteryState()
{
    _updateBatteryState(_batteryState == BatteryState::BATT_STATE_SHIPPING_MODE_CHARGED ||
                        _batteryState == BatteryState::BATT_STATE_SHIPPING_MODE_DISCHARGED);
}

// -----------------------------------------------------------------------------------------------------------
void BatteryManager::_updateBatteryState(bool isInShippingMode)
{
    BatteryState::eBatteryState state;

    if(isInShippingMode)
    {
        if(_batteryLevel <= APP_BATT_SHIPPING_LOW_THRESHOLD)
        {
            state = BatteryState::BATT_STATE_SHIPPING_MODE_DISCHARGED;
        }
        else
        {
            state = BatteryState::BATT_STATE_SHIPPING_MODE_CHARGED;
        }
    }
    else
    {
        #if (APP_BATT_USE_EOL_MODE != 0)
        if(     _batteryLevel <= APP_BATT_EOL1 && _batteryLevel > APP_BATT_EOL2)
        {
            state = BatteryState::BATT_STATE_EOL_FIRST;
        }
        else if(_batteryLevel <= APP_BATT_EOL2 && _batteryLevel > APP_BATT_EOL3)
        {
            state = BatteryState::BATT_STATE_EOL_SECOND;
        }
        else if(_batteryLevel <= APP_BATT_EOL3)
        {
            state = BatteryState::BATT_STATE_EOL_THIRD;
        }
        #else
        if(_batteryLevel <= APP_BATT_LOW_BATT_THRESHOLD)
        {
            state = BatteryState::BATT_STATE_LOW_BAT;
        }
        #endif
        else
        {
            state = BatteryState::BATT_STATE_OK;
        }
    }

    if(_batteryState != state)
    {
        BATT_DEBUG("[BatteryManager] _updateBatteryState() > notify battery state: %s (state=%d)\r\n", BatteryState::toString(state), state);
        _batteryState = state;
        _notifyBattStateChanged();
    }
}

// -----------------------------------------------------------------------------------------------------------
bool BatteryManager::_startTimer(uint32_t delay)
{
    if(!_isTimerEnabled)
    {
        #if (APP_BATT_USE_XF_TIMER != 0)
        getThread()->scheduleTimeout(TimeoutBatteryId, delay, this);
        #else
        uint32_t errCode = tools::SDKTools::appTimerStart(_batteryManagerTimerId, delay, this);
        if(errCode != NRF_SUCCESS)
        {
            BATT_DEBUG("[BatteryManager] _startTimer() > SDKTools::appTimerStart() failed ! (errCode: 0x%04x)\r\n", errCode);
            return false;
        }
        #endif  // #if (APP_BATT_USE_XF_TIMER != 0)
        _isTimerEnabled = true;
    }
    return true;
}

// -----------------------------------------------------------------------------------------------------------
bool BatteryManager::_stopTimer()
{
    if(_isTimerEnabled)
    {
        #if (APP_BATT_USE_XF_TIMER != 0)
        getThread()->unscheduleTimeout(TimeoutBatteryId, this);
        #else
        uint32_t errCode = tools::SDKTools::appTimerStop(_batteryManagerTimerId);
        if(errCode != NRF_SUCCESS)
        {
            BATT_DEBUG("[BatteryManager] _stopTimer() > SDKTools::appTimerStop() failed ! (errCode: 0x%04x)\r\n", errCode);
            return false;
        }
        #endif  // #if (APP_BATT_USE_XF_TIMER != 0)
        _isTimerEnabled = false;
    }
    return true;
}

#if (USE_CHARGER != 0)
// -----------------------------------------------------------------------------------------------------------
uint16_t BatteryManager::_percentToChargeTime()
{
    uint16_t time;
    if(     _batteryLevel <= 85)
    {
        time =            (uint16_t)((uint32_t)( BATT_PC1             *  _batteryLevel)       / 85);
    }
    else if(_batteryLevel <= 95)
    {
        time = BATT_PC1 + (uint16_t)((uint32_t)((BATT_PC2 - BATT_PC1) * (_batteryLevel - 85)) / 10);
    }
    else if(_batteryLevel < 100)
    {
        time = BATT_PC2 + (uint16_t)((uint32_t)((BATT_PC3 - BATT_PC2) * (_batteryLevel - 95)) /  4);
    }
    else
    {
        time = BATT_PC4;
    }
    return time;
}

// -----------------------------------------------------------------------------------------------------------
uint8_t BatteryManager::_chargeTimeToPercent()
{
    uint8_t level;
    if(_chargeTime <= BATT_PC1)
    {
        level =      (uint8_t)((uint32_t)( _chargeTime             * 85) / BATT_PC1);
    }
    else if(_chargeTime <= BATT_PC2)
    {
        level = 85 + (uint8_t)((uint32_t)((_chargeTime - BATT_PC1) * 10) / (BATT_PC2 - BATT_PC1));
    }
    else if(_chargeTime <= BATT_PC3)
    {
        level = 95 + (uint8_t)((uint32_t)((_chargeTime - BATT_PC2) *  4) / (BATT_PC3 - BATT_PC2));
    }
    else
    {
        level = 100;
    }
    BATT_DEBUG("[BatteryManager] _chargeTimeToPercent() > level: %d\r\n", level);
    return level;
}

// -----------------------------------------------------------------------------------------------------------
void BatteryManager::_chargerTimerHandler()
{
    if(_charging)
    {
        if(_chargeTime < BATT_PC4)
        {
            _chargeTime++;
            uint8_t level = _chargeTimeToPercent();
            if(level > _batteryLevel)
            {
                // convert charge time in percent
                _batteryLevel = level;
            }
        }
        else if(_chargeTime == BATT_PC4 && _batteryLevel != 100)
        {
            _batteryLevel = 100;
        }
        else
        {
            return;
        }

        BATT_DEBUG("[BatteryStateController] onSecond() > the battery is charged at: %d% \r\n", _batteryLevel);

        // notify new battery value
        _notifyBattLevelChanged();

        // update battery state
        _updateBatteryState();
    }
}
#endif  // #if (USE_CHARGER != 0)

// -----------------------------------------------------------------------------------------------------------
void BatteryManager::_notifyBattStateChanged()
{
    BATT_DEBUG("[BatteryManager] _notifyBattStateChanged() > New battery state: %s (state=%d)\r\n", BatteryState::toString(_batteryState), _batteryState);
    for(uint8_t i = 0; i < observersCount(); i++)
    {
        if(observer(i) != NULL)
        {
            observer(i)->onBatteryStateChanged(this, _batteryState);
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
void BatteryManager::_notifyBattLevelChanged()
{
    BATT_DEBUG("[BatteryManager] _notifyBattLevelChanged() > New battery level in percent: %d\r\n", _batteryLevel);
    for(uint8_t i = 0; i < observersCount(); i++)
    {
        if(observer(i) != NULL)
        {
            observer(i)->onBatteryLevelChanged(this, _batteryLevel);
        }
    }
}

#endif // #if (USE_BATTERY_MANAGER != 0)
