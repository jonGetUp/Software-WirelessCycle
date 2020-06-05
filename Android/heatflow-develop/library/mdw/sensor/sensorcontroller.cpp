/************************************************************************************************************
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 ************************************************************************************************************/

/************************************************************************************************************/
/* Header files                                                                                             */
/************************************************************************************************************/
#include "sensorcontroller.h"
#include <boards.h>

#if defined(SENSOR_VDD_ENABLE) || (USE_SPI_DRV != 0) 
    #include "gpio/gpiohal.h"
    #include <nrf52_bitfields.h>
    #if (USE_SPI_DRV != 0)
        #include <nrf_delay.h>
    #endif
#endif




/************************************************************************************************************/
/* DEBUG COMPILATION OPTION & MACRO                                                                         */
/************************************************************************************************************/
#define DEBUG_SENSOR_CONTROLLER_ENABLE      0
#if (DEBUG_SENSOR_CONTROLLER_ENABLE != 0)  && defined(DEBUG_NRF_USER)
    #include <tools/logtrace.h>
    #define SC_DEBUG                        LOG_TRACE_DEBUG
#else
    #define SC_DEBUG(...)                   {(void) (0);}
#endif

#define DEBUG_SENSOR_CONTROLLER_ST_ENABLE   0
#if (DEBUG_SENSOR_CONTROLLER_ST_ENABLE != 0)  && defined(DEBUG_NRF_USER)
    #include <tools/logtrace.h>
    #define SCS_DEBUG                       LOG_TRACE_DEBUG
#else
    #define SCS_DEBUG(...)                  {(void) (0);}
#endif

#define DEBUG_SENSOR_CONTROLLER_DEEP_ENABLE 0
#if (DEBUG_SENSOR_CONTROLLER_DEEP_ENABLE != 0)  && defined(DEBUG_NRF_USER)
    #include <tools/logtrace.h>
    #define SCD_DEBUG                       LOG_TRACE_DEBUG
#else
    #define SCD_DEBUG(...)                  {(void) (0);}
#endif

#if (USE_SPI_DRV != 0)
/************************************************************************************************************/
/* --- Extern "C" Section ---                                                                               */
/*     Callback's section                                                                                   */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
#if (USE_SPI != 0)
void sensor::spiSensorEventHandler(nrfx_spi_evt_t const*  pEvent, void* pContext)
#else
void sensor::spiEventHandler(nrfx_spim_evt_t const* pEvent, void* pContext)
#endif
{
    if(pContext != NULL)
    {
        static_cast<SensorController*>(pContext)->onSpiDoneEvent((void*) pEvent);
    }
    else
    {
        SC_DEBUG("[SpiMasterDriver] spiEventHandler() > Context is NULL !\r\n");
    }
}
#endif // #if (USE_SPI_DRV != 0)


/************************************************************************************************************/
/* Namespace's Declaration                                                                                  */
/************************************************************************************************************/
using sensor::SensorController;


/************************************************************************************************************/
/* Declaration section                                                                                      */
/************************************************************************************************************/
const int LONG_DELAY        = 2500;
const int STD_DELAY         = 1000;
const int SHORT_DELAY       =   50;


/************************************************************************************************************/
/* Constructor|Destructor section                                                                           */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
SensorController::SensorController()
{
    // State's initialization
    _currentState           = ST_INIT;
    _stateToReach           = ST_IDLE;

    _isSensorTimeoutActive  = false;
    _isMeasureActive        = false;

    _spi.p_reg              = NRF_SPI0;
    _spiConfig.sck_pin      = SPI0_SCK_PIN;
    _spiConfig.mosi_pin     = SPI0_MOSI_PIN;
    _spiConfig.miso_pin     = SPI0_MISO_PIN;
    _spiConfig.irq_priority = NRFX_SPI_DEFAULT_CONFIG_IRQ_PRIORITY;
    _spiConfig.orc          = 0xFF;
    _spiConfig.ss_pin       = 0xFF;
    #if (USE_SPI != 0)
    _spi.drv_inst_idx       = NRFX_SPI0_INST_IDX;
    _spiConfig.frequency    = NRF_SPI_FREQ_4M;
    _spiConfig.mode         = NRF_SPI_MODE_1;
    _spiConfig.bit_order    = NRF_SPI_BIT_ORDER_MSB_FIRST;  // NRF_SPI_BIT_ORDER_LSB_FIRST
    #else
    _spim.drv_inst_idx      = NRFX_SPIM0_INST_IDX;
    _spiConfig.ss_active_high = false;
    _spiConfig.frequency    = NRF_SPIM_FREQ_8M;
    _spiConfig.mode         = NRF_SPIM_MODE_1;
    _spiConfig.bit_order    = NRF_SPIM_BIT_ORDER_MSB_FIRST;
    #endif
    
    #if defined(SENSOR_VDD_ENABLE)
    hal::GpioHal::cfgPinOutput(SENSOR_VDD_ENABLE, GPIO_PIN_CNF_DRIVE_H0H1, GPIO_PIN_CNF_PULL_Disabled, GPIO_PIN_CNF_INPUT_Disconnect);
    _handleVddSensorChain(false);
    #endif  // #if defined(SENSOR_VDD_ENABLE)
}

//------------------------------------------------------------------------------------------------------------
SensorController::~SensorController()
{
}


/************************************************************************************************************/
/* Public section                                                                                           */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
void SensorController::initialize()
{
    SC_DEBUG("[SensorController] initialize() > Set up the GPIOs and initialize the app_timers used for the measurements.\r\n");
    
    // App_timer 
    //_adcMeasureTimerTick = 0;
}

//------------------------------------------------------------------------------------------------------------
void SensorController::bind(
#if (USE_ADS111X != 0)
                            ADS1118*              ads1118_a
    #if (USE_ADS111X > 1)
                          , ADS1118*              ads1118_b
    #endif
#endif // #if (USE_ADS111X != 0)
#if (SENSOR_CONTROLLER_HANDLES_LED != 0) && (USE_LEDS != 0) && (LED_2 != 0)
    #if (USE_ADS111X != 0)
                          ,
    #endif // (USE_ADS111X != 0)
                            gpio::LedController*  led
#endif // #if (USE_LEDS != 0) && (LED_2 != 0)
)
{
#if (USE_ADS111X != 0)
    if(ads1118_a == NULL)
    {
        SC_DEBUG("[SensorController] bind() > NULL pointer error on ads1118_a !\r\n");
        ASSERT(false);
    }
    _ads1118_a     = ads1118_a;
    
    // Configuration of the ADS
    _ads1118_a->enablePullup();
    _ads1118_a->setFullScaleRange(ADS1118::FSR_0512);
    _ads1118_a->setSingleShotMode();
    
    _setSpiFrequency(_ads1118_a->frequencyClock());
    #if (USE_SPI != 0)
    _spiConfig.mode = NRF_SPI_MODE_1;
    #elif (USE_SPIM != 0)
    _spiConfig.mode = NRF_SPIM_MODE_1;
    #endif
    
    #if (USE_ADS111X > 1)
    if(ads1118_b == NULL)
    {
        SC_DEBUG("[SensorController] bind() > NULL pointer error on ads1118_b !\r\n");
        ASSERT(false);
    }
    _ads1118_b     = ads1118_b;
    
    // Configuration of the ADS
    _ads1118_b->enablePullup();
    _ads1118_b->setFullScaleRange(ADS1118::FSR_0256);
    _ads1118_b->setSingleShotMode();
    #endif
#endif // #if (USE_ADS111X != 0)
#if (SENSOR_CONTROLLER_HANDLES_LED != 0) && (USE_LEDS != 0) && (LED_2 != 0)
    if(led == 0)
    {
        SC_DEBUG("[SensorController] bind() > NULL pointer error on LedController !\r\n");
        ASSERT(false);
    }
    _led = led;
    _led->setTimeforQuickStandardAndSlowBlinkingState(50,  450, 
                                                      50, 1950, 
                                                      50, 3950);
#endif // #if (USE_LEDS != 0) && (LED_2 != 0)
}

//------------------------------------------------------------------------------------------------------------
void SensorController::setState(eSensorControllerState newState)
{
    SC_DEBUG("[SensorController] setState() > Update `stateToReach` to 0x%02x.\r\n", newState);
    _stateToReach = newState;                                                                        // Set up the state to reach
    this->pushEvent(&_xfSensorEvt);
}
        
//------------------------------------------------------------------------------------------------------------
void SensorController::onSpiDoneEvent(void const* pEvent)
{
    #if (USE_SPI != 0)
    nrfx_spi_evt_t const* event  = static_cast<nrfx_spi_evt_t const*>(pEvent);
    if(event->type == NRFX_SPI_EVENT_DONE)
    #else
    nrfx_spim_evt_t const* event = static_cast<nrfx_spim_evt_t const*>(pEvent);
    if(event->type == NRFX_SPIM_EVENT_DONE)
    #endif
    {
        SC_DEBUG("[SensorController] onSpiDoneEvent() > %d Bytes sent, %d Bytes received\r\n", event->xfer_desc.tx_length, event->xfer_desc.rx_length);
    }
}

/************************************************************************************************************/
/* Protected section                                                                                        */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
EventStatus SensorController::processEvent()
{
    switch(getCurrentEvent()->getEventType())                                                                   // Checks first the type of Event
    {
        /* ************************************************************************************************ */
        /* Event event                                                                                      */
        /* ************************************************************************************************ */
        case IXFEvent::Event:
        {
            if(_currentState != _stateToReach)
            {
                switch(_currentState)
                {
                /* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
                case ST_IDLE:
                {
                    if(_stateToReach == ST_MEASUREMENT) 
                    {
                        SCS_DEBUG("[SensorController] processEvent() > SC goes on ST_DELAY from ST_IDLE (IXFEvent::Event).\r\n");
                        _currentState = ST_DELAY;
    
                        #if defined(SENSOR_VDD_ENABLE)
                        _handleVddSensorChain(true);
                        #endif // #if defined(SENSOR_VDD_ENABLE)

                        _ST_DELAY_Action();
                    }
                    break;
                }
                /* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
                case ST_MEASUREMENT:
                {
                    if(_stateToReach == ST_SHUTDOWN)
                    {
                        SCS_DEBUG("[SensorController] processEvent() > SC goes on ST_SHUTDOWN from ST_MEASUREMENT (IXFEvent::Event).\r\n");
                        _currentState = ST_SHUTDOWN;
                        _ST_SHUTDOWN_Action();
                    }
                    #if (SENSOR_CONTROLLER_ASYNC_MODE != 0)
                    else if(_stateToReach == ST_MEASUREMENT) 
                    {
                        SCS_DEBUG("[SensorController] processEvent() > SC stays in ST_MEASUREMENT ...\r\n");
                        _ST_MEASUREMENT_Action();
                    }
                    #endif  // #if (SENSOR_CONTROLLER_ASYNC_MODE != 0)
                    break;
                }
                /* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
                case ST_DELAY:
                {
                    if(_stateToReach == ST_SHUTDOWN){
                        SCS_DEBUG("[SensorController] processEvent() > SC goes on ST_SHUTDOWN from ST_DELAY (IXFEvent::Event).\r\n");
                        _currentState = ST_SHUTDOWN;
                        _ST_SHUTDOWN_Action();
                    }
                    else
                    {
                        SCS_DEBUG("[SensorController] processEvent() > SC goes on ST_MEASUREMENT from ST_DELAY.\r\n");
                        _currentState = ST_MEASUREMENT;
                        _ST_MEASUREMENT_Action();
                    }
                    break;
                }
                default:
                    break;
                }
            }
            else
            {
                SCS_DEBUG("[SensorController] processEvent() > The sensor is already in the desired state: %#04x\r\n", _stateToReach);
            }
            break;
        }
        /* ************************************************************************************************* */
        /* Timeout event                                                                                     */
        /* ************************************************************************************************* */
        case IXFEvent::Timeout:
        {
            if(getCurrentTimeout()->getId() == TimeoutSensorId)
            {
                SCS_DEBUG("[SensorController] processEvent() > Timeout_Sensor_id received ...\r\n");
                _isSensorTimeoutActive = false;
                switch(_currentState)
                {
                    /* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
                    case ST_DELAY:
                    {
                        SCS_DEBUG("[SensorController] processEvent() > SC goes on ST_MEASUREMENT from ST_DELAY.\r\n");
                        _currentState = ST_MEASUREMENT;
                        _ST_MEASUREMENT_Action();
                        break;
                    }
                    /* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
                    case ST_SHUTDOWN:
                    {
                        SCS_DEBUG("[SensorController] processEvent() > SC goes on ST_IDLE from ST_SHUTDOWN.\r\n");
                        _currentState = ST_IDLE;
                        _ST_IDLE_Action();
                        break;
                    }
                    default:
                        break;
                }       
            }
            break;
        }
        /* ************************************************************************************************ */
        /* Initial event                                                                                    */
        /* ************************************************************************************************ */
        case IXFEvent::Initial:
        {
            if(_currentState == ST_INIT)
            {
                // Update State
                SCS_DEBUG("[SensorController] processEvent() > -> ST_IDLE from ST_INIT ((IXFEvent::Initial).\r\n");
                _currentState = ST_IDLE;
                _ST_IDLE_Action();
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
/* Protected section                                                                                        */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------


/************************************************************************************************************/
/* Private section                                                                                          */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
void SensorController::_handleActivityTimeout(bool start, int delay)
{
    if(start)
    {
        if(!_isSensorTimeoutActive)
        {
            SC_DEBUG("[SensorController] _handleActivityTimeout() > State Machine Timer is started.\r\n");
            _isSensorTimeoutActive = true;
            _currentState = ST_DELAY;
            getThread()->scheduleTimeout(TimeoutSensorId, delay, this);
        }
    }
    else
    {
        if(_isSensorTimeoutActive)
        {
            SC_DEBUG("[SensorController] _handleActivityTimeout() > Stopping State Machine Timer ...\r\n");
            _isSensorTimeoutActive = false;
            getThread()->unscheduleTimeout(TimeoutSensorId, this);
        }
    }
}

//------------------------------------------------------------------------------------------------------------
void SensorController::_stopMeasurement()
{
    if(_isMeasureActive)
    {
        SC_DEBUG("[SensorController] _stopMeasurement() > Stopping appTimers for measurements ...\r\n");
        _isMeasureActive = false;
    }
    else
    {
        SC_DEBUG("[SensorController] _stopMeasurement() > Nothing to do here, no Measurement active !\r\n");
    }
}

#if (USE_SPI_DRV != 0)
//------------------------------------------------------------------------------------------------------------
void SensorController::_spiInit(nrfx_spi_evt_handler_t handler, void* context)
{
    #if (USE_SPI != 0)
    uint32_t  errCode = nrfx_spi_init(&_spi, &_spiConfig, handler, context);
    #else
    uint32_t  errCode = nrfx_spim_init(&_spi, &_spimConfig, NULL, NULL);
    #endif
    if(errCode != NRF_SUCCESS)
    {
        SC_DEBUG("[SensorController] _spiInit() > nrfx SPI driver failed to init ! (errCode: 0x%04x)\r\n", errCode);
        return; 
    }
}

//------------------------------------------------------------------------------------------------------------
bool SensorController::_spiTransfer(uint8_t* tx, size_t txSize, uint8_t* rx, size_t rxSize)
{
    // setup the transfer
    _spiXferDescriptor.p_tx_buffer   = tx;
    _spiXferDescriptor.tx_length     = txSize;
    _spiXferDescriptor.p_rx_buffer   = rx;
    _spiXferDescriptor.rx_length     = rxSize;

    // start the transfer
    #if (USE_SPI != 0)
    uint32_t errCode                = nrfx_spi_xfer( &_spi, &_spiXferDescriptor, 0);
    #else
    uint32_t errCode                = nrfx_spim_xfer(&_spi, &spiXferDescriptor, 0);
    #endif
    if(errCode != NRF_SUCCESS)
    {
        SC_DEBUG("[SensorController] _spiTransfer() > Error: 0x%04x\r\n", errCode);
        return false;
    }
    return true;
}

//------------------------------------------------------------------------------------------------------------
void SensorController::_spiUnit()
{
    #if (USE_SPI != 0)
    nrfx_spi_uninit(&_spi);
    #elif (USE_SPIM != 0)
    nrfx_spim_uninit(&_spi);
    #endif
}

//------------------------------------------------------------------------------------------------------------
void SensorController::_setSpiFrequency(uint16_t frequency)
{
    if(frequency == (uint16_t) 2000000)
    {
        #if (USE_SPI != 0)
        _spiConfig.frequency = static_cast<nrf_spi_frequency_t>(SPI_FREQUENCY_FREQUENCY_M2);
        #else
        _spiConfig.frequency = static_cast<nrf_spim_frequency_t>(SPIM_FREQUENCY_FREQUENCY_M2);
        #endif
    }
    else if (frequency == (uint16_t) 4000000)
    {
        #if (USE_SPI != 0)
        _spiConfig.frequency = static_cast<nrf_spi_frequency_t>(SPI_FREQUENCY_FREQUENCY_M4);
        #else
        _spiConfig.frequency = static_cast<nrf_spim_frequency_t>(SPIM_FREQUENCY_FREQUENCY_M4);
        #endif
    }
    else
    {
        SC_DEBUG("[SensorController] _setSpiFrequency() > Invalid frequency: %d\r\n", frequency);
    }
}

//------------------------------------------------------------------------------------------------------------
uint16_t SensorController::_adsTransfer(ADS1118* ads)
{
    static uint8_t adsTxBuffer[4 * sizeof(uint8_t)] = { 0 };
    static uint8_t adsRxBuffer[4 * sizeof(uint8_t)] = { 0 };
    ADS1118::adsValue_t  adsValue;

    nrf_delay_ms(ads->delayBeforeFirstResult());

    // prepare buffers...
    adsTxBuffer[0] = ads->getMsbConfig();
    adsTxBuffer[1] = ads->getLsbConfig();
    adsTxBuffer[2] = ads->getMsbConfig();
    adsTxBuffer[3] = ads->getLsbConfig();
    
    if(_spiTransfer(adsTxBuffer, sizeof(adsTxBuffer), adsRxBuffer, sizeof(adsRxBuffer)))
    {
        adsValue.byte.msb  = adsRxBuffer[0];
        adsValue.byte.lsb  = adsRxBuffer[1];
        
        #if (DEBUG_SENSOR_CONTROLLER_DEEP_ENABLE != 0)  && defined(DEBUG_NRF_USER)
        adsConfig.byte.msb = adsRxBuffer[2];
        adsConfig.byte.lsb = adsRxBuffer[3];
        
        SCD_DEBUG("[SensorController] _readFromSensor() > adcValue: 0x%04x\r\n", adsValue.word);
        SCD_DEBUG("[SensorController] _readFromSensor() > adConfig: 0x%04x\r\n", adsConfig.word);
        #endif // #if (DEBUG_SENSOR_CONTROLLER_DEEP_ENABLE != 0)  && defined(DEBUG_NRF_USER)
    }
    
    return adsValue.word;
}

//------------------------------------------------------------------------------------------------------------
uint16_t SensorController::_readFromSensor(ADS1118* ads, bool configHasChanged)
{
    hal::GpioHal::pinClear(ads->chipSelect());

    if(configHasChanged) 
    {
        _adsTransfer(ads);
        _adsTransfer(ads);
    }
    uint16_t result = _adsTransfer(ads);

    hal::GpioHal::pinSet(ads->chipSelect());

    return result;
}
#endif  // #if (USE_SPI_DRV != 0)


/************************************************************************************************************/
/* State Machine Actions                                                                                    */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
void SensorController::_ST_IDLE_Action()
{
    SC_DEBUG("[SensorController] _ST_IDLE_Action() > ...\r\n");
    #if (SENSOR_CONTROLLER_HANDLES_LED != 0) && (USE_LEDS != 0) && (LED_2 != 0)
    _led->LEDoFF();
    #endif
}
//------------------------------------------------------------------------------------------------------------
void SensorController::_ST_SHUTDOWN_Action()
{
    SC_DEBUG("[SensorController] _ST_SHUTDOWN_Action() > ...\r\n");
    
    #if defined(SENSOR_VDD_ENABLE)
    _handleVddSensorChain(false);
    #endif // #if defined(SENSOR_VDD_ENABLE)
    
    _handleActivityTimeout(true, STD_DELAY);
}

//------------------------------------------------------------------------------------------------------------
void SensorController::_ST_DELAY_Action()
{
    SC_DEBUG("[SensorController] _ST_DELAY_Action() > ...\r\n");
    #if (SENSOR_CONTROLLER_HANDLES_LED != 0) && (USE_LEDS != 0) && (LED_2 != 0)
    _led->ledOn();
    #endif
    
    _handleActivityTimeout(true, LONG_DELAY);
}

//------------------------------------------------------------------------------------------------------------
void SensorController::_ST_MEASUREMENT_Action()
{
    SC_DEBUG("[SensorController] _ST_MEASUREMENT_Action() > ...\r\n");
    #if (SENSOR_CONTROLLER_HANDLES_LED != 0) && (USE_LEDS != 0) && (LED_2 != 0)
    _led->ledOn();
    #endif
    
    //Config ADC
    #if (USE_ADS111X != 0)
    ADS1118* ads = _ads1118_a;
    float volts  = 0.0f;
    _spiInit(NULL, NULL);
    
    // Read ADC from ADS #1
        #if (SENSOR_CONTROLLER_ASYNC_MODE != 0)
    static uint8_t cMeasA1 = 0; 
    static uint8_t cMeasA2 = 0;
    
    if(cMeasA1 < ads->rawAcumulatedSize())
    {
        _readAdcValue(ads, ADS1118::DIFF_0_1, "Raw A.1");
        cMeasA1++;
    }
    else if(cMeasA1 == ads->rawAcumulatedSize()) 
    {
        volts = ads->getVolts(ads->meanRawValue((bool) ADS1118::DIFF_0_1));
        debugPrintFloat(volts, "ADS A.1", "V");
        cMeasA1++;
    }
    else if(cMeasA2 < ads->rawAcumulatedSize())
    {
        _readAdcValue(ads, ADS1118::DIFF_2_3, "Raw A.2");
        cMeasA2++;
    }
    else if(cMeasA2 == ads->rawAcumulatedSize())
    {
        volts = ads->getVolts(ads->meanRawValue((bool) ADS1118::DIFF_2_3));
        debugPrintFloat(volts, "ADS A.2", "V");
        cMeasA2++;
    }
    #else
    for(uint8_t i = 0; i < ads->rawAcumulatedSize(); i++)
    {
        _readAdcValue(ads, ADS1118::DIFF_0_1, "Raw A.1");
    }
    volts = ads->getVolts(ads->meanRawValue((bool) ADS1118::DIFF_0_1));
    debugPrintFloat(volts, "ADS A.1", "V");
    
    for(uint8_t i = 0; i < ads->rawAcumulatedSize(); i++)
    {
        _readAdcValue(ads, ADS1118::DIFF_2_3, "Raw A.2");
    }
    volts = ads->getVolts(ads->meanRawValue((bool) ADS1118::DIFF_2_3));
    debugPrintFloat(volts, "ADS A.2", "V");
        #endif  // #if (SENSOR_CONTROLLER_ASYNC_MODE != 0)

        #if (USE_ADS111X > 1)
    // Read Volts from ADS #2
    ads = _ads1118_b;
    
    // Read ADC from ADS #2
            #if (SENSOR_CONTROLLER_ASYNC_MODE != 0)
    static uint8_t cMeasB1 = 0;
    
    if(cMeasA1 > _ads1118_a->rawAcumulatedSize() && cMeasA2 > _ads1118_a->rawAcumulatedSize())
    {
        if(cMeasB1 < ads->rawAcumulatedSize())
        {
            _readAdcValue(ads, ADS1118::DIFF_0_1, "Raw B.1");
            cMeasB1++;
        }
        else
        {
            ads->computeMeanRawValue(ADS1118::DIFF_0_1);
            cMeasA1 = 0;
            cMeasA2 = 0;
            cMeasB1 = 0;
        }
    }
            #else
    for(uint8_t i = 0; i < ads->rawAcumulatedSize(); i++)
    {
        _readAdcValue(ads, ADS1118::DIFF_0_1, "Raw B.1");
    }
    volts = ads->getVolts(ads->meanRawValue(ADS1118::DIFF_0_1));
    debugPrintFloat(volts, "ADS B.1", "V");
            #endif  // #if (SENSOR_CONTROLLER_ASYNC_MODE != 0)
        #else   // #if (USE_ADS111X > 1)
    if(cMeasA1 > _ads1118_a->rawAcumulatedSize() && cMeasA2 > _ads1118_a->rawAcumulatedSize())
    {
        cMeasA1 = 0;
        cMeasA2 = 0;
    }
        #endif  // #if (USE_ADS111X > 1)

    _spiUnit();
    #endif // #if (USE_ADS111X != 0)
    
    SC_DEBUG("[SensorController] _ST_DELAY_Action() > Starting a timer for temporisation ...\r\n");
    #if (SENSOR_CONTROLLER_ASYNC_MODE != 0)
        #if defined(DEBUG_NRF_USER)
            _handleActivityTimeout(true, SHORT_DELAY);
        #else
            _handleActivityTimeout(true, 2* SHORT_DELAY);
        #endif
    #else
    _handleActivityTimeout(true, STD_DELAY);
    #endif  // #if (SENSOR_CONTROLLER_ASYNC_MODE != 0)
}



#if (USE_ADS111X != 0)
//------------------------------------------------------------------------------------------------------------
void SensorController::_readTemperature(ADS1118* ads, const char* dbgMsg)
{
    uint8_t count = 1;
    if(ads->setTemperatureMode())
    {
        count = 2;
    }
    
    uint16_t word = _readFromSensor(ads, count);
    if(word != 0xffff)
    {
        SC_DEBUG("[SensorController] _readTemperature() > RAW: 0x%04x\r\n", word);
        float temp = _ads1118_a->getTemperature(word);
        debugPrintFloat(temp, dbgMsg, "C");
    }
    else
    {
        SC_DEBUG("[SensorController] _readTemperature() > FAILED !!!\r\n", word);
    }
}

//------------------------------------------------------------------------------------------------------------
void SensorController::_readAdcValue(ADS1118* ads, uint8_t inputs, const char* dbgMsg)
{
    bool configChanged  = ads->setAdcMode() || ads->setInputSelected(inputs);
    uint8_t count       = configChanged ? 2:1;
    uint16_t word       = _readFromSensor(ads, count);
    if(word != 0xffff)
    {
        SC_DEBUG("[SensorController] _readAdcValue() > RAW: 0x%04x\r\n", word);
        ads->storeRaw(word, inputs);
    }
    else
    {
        SC_DEBUG("[SensorController] _readAdcValue() > FAILED !!!\r\n", word);
    }
}

//------------------------------------------------------------------------------------------------------------
void SensorController::_readVolts(ADS1118* ads, uint8_t inputs, const char* dbgMsg)
{
    uint8_t count = (ads->setAdcMode() || ads->setInputSelected(inputs)) ? 2:1;
    uint16_t word = _readFromSensor(ads, count);
    if(word != 0xffff)
    {
        SC_DEBUG("[SensorController] _readVolts() > RAW: 0x%04x\r\n", word);
        float v = _ads1118_a->getVolts(word);
        debugPrintFloat(v, dbgMsg, "V");
    }
    else
    {
        SC_DEBUG("[SensorController] _readVolts() > FAILED !!!\r\n", word);
    }
}
#endif  // #if (USE_ADS111X != 0)

#if defined(SENSOR_VDD_ENABLE)
//------------------------------------------------------------------------------------------------------------
void SensorController::_handleVddSensorChain(bool enable)
{
    if(enable)
    {
        #if (SENSOR_VDD_ACTIF_LOW != 0)
        hal::GpioHal::pinClear(SENSOR_VDD_ENABLE);
        #else
        hal::GpioHal::pinSet(SENSOR_VDD_ENABLE);
        #endif
    }
    else
    {
        #if (SENSOR_VDD_ACTIF_LOW != 0)
        hal::GpioHal::pinSet(SENSOR_VDD_ENABLE);
        #else
        hal::GpioHal::pinClear(SENSOR_VDD_ENABLE);
        #endif
    }
}
#endif  // #if defined(SENSOR_VDD_ENABLE)

//------------------------------------------------------------------------------------------------------------
void SensorController::debugPrintFloat(float value, const char* msg, const char* unit)
{
    #if (DEBUG_SENSOR_CONTROLLER_ENABLE != 0) && defined(DEBUG_NRF_USER)
    {
        char tmp[10] = { 0 };
        sprintf(tmp, "%6.2f", value);
        SC_DEBUG("[SensorController] _ST_MEASUREMENT_Action() > %s: %s [%s]\r\n", msg, tmp, unit);
    }
    #endif
}
