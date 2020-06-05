/************************************************************************************************************
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 ************************************************************************************************************/

/************************************************************************************************************/
/* Header files                                                                                             */
/************************************************************************************************************/
#include "sdktools.h"
#include <sdk-tools-config.h>
#include <tools/logtrace.h>

#include <app_timer.h>
#include <nrf.h>
#include <string.h>

#include <math.h>


#if !defined(APP_SCHEDULER_ENABLED)
    #error "APP_SCHEDULER not defined for sdkTools"
#elif (APP_SCHEDULER_ENABLED != 0)
    #include "ble/scheduler.h"
#endif

#if !defined(USE_APP_TIMER)
    #error "USE_APP_TIMER not defined for sdkTools"
#elif (USE_APP_TIMER != 0)
    #include <app_timer.h>
#else
    #warning "USE_APP_TIMER set to 0 for sdkTools"
#endif


/************************************************************************************************************/
/* DEBUG COMPILATION OPTION & MACRO                                                                         */
/************************************************************************************************************/
#define DEBUG_SDKTOOLS_ENABLE               1
#if (DEBUG_SDKTOOLS_ENABLE != 0) && defined(DEBUG_NRF_USER)
    #include <tools/logtrace.h>
    #define SDKTOOLS_DEBUG                  LOG_TRACE_DEBUG
#else
    #define SDKTOOLS_DEBUG(...)             {(void) (0);}
#endif


/************************************************************************************************************/
/* Namespace Declaration                                                                                    */
/************************************************************************************************************/
using tools::SDKTools;


/************************************************************************************************************/
/* Global Variable Declaration                                                                              */
/************************************************************************************************************/
static int8_t _advertiserTxPower          = -128;
static int8_t _connectionTxPower          = -128;
static int8_t _scanInitTxPower            = -128;


/************************************************************************************************************/
/* PUBLIC SECTION                                                                                           */
/************************************************************************************************************/
// -----------------------------------------------------------------------------------------------------------
// static
void SDKTools::init()
{
    uint32_t errCode = NRF_SUCCESS;

#if (APP_SCHEDULER_ENABLED != 0)
    ble::Scheduler::init();
#endif
    
#if (USE_APP_TIMER != 0)
    SDKTOOLS_DEBUG("[SDKTools] init() > `app_timer` initializing ...\r\n");
    errCode = _appTimerInit();
#endif

    if(errCode == NRF_SUCCESS)
    {
        SDKTOOLS_DEBUG("[SDKTools] init() > Initialization of SDK modules done!\r\n");
    }
    else
    {
        SDKTOOLS_DEBUG("[SDKTools] init() > Initialization of SDK modules FAILED ! (errCode: 0x%04x)\r\n", errCode);
    }
}

// -----------------------------------------------------------------------------------------------------------
// static
int8_t SDKTools::getTxPower(uint8_t role)
{
    switch(role)
    {
        case BLE_GAP_TX_POWER_ROLE_ADV:
            SDKTOOLS_DEBUG("[SDKTools] getTxPower() > Radio's TX Power for BLE_GAP_TX_POWER_ROLE_ADV: %3d [dBm].\r\n", _advertiserTxPower);
            return _advertiserTxPower;
        
        case BLE_GAP_TX_POWER_ROLE_CONN:
            SDKTOOLS_DEBUG("[SDKTools] getTxPower() > Radio's TX Power for BLE_GAP_TX_POWER_ROLE_CONN: %3d [dBm].\r\n", _connectionTxPower);
            return _connectionTxPower;
        
        case BLE_GAP_TX_POWER_ROLE_SCAN_INIT:
            SDKTOOLS_DEBUG("[SDKTools] getTxPower() > Radio's TX Power for BLE_GAP_TX_POWER_ROLE_SCAN_INIT: %3d [dBm].\r\n", _scanInitTxPower);
            return _scanInitTxPower;
        
        default:
            SDKTOOLS_DEBUG("[SDKTools] getTxPower() > Unknown role : 0x%02x.\r\n", role);
            ASSERT(false);
            return -128;
    }
}

// -----------------------------------------------------------------------------------------------------------
// static
bool SDKTools::setTxPower(uint8_t role, uint16_t handle, int8_t txPower)
{
    // Validation of Radio transmit power in dBm.
    // Supported txPower values: -40dBm, -20dBm, -16dBm, -12dBm, -8dBm, -4dBm, 0dBm, +3dBm and +4dBm.
    switch(txPower)
    {
        case -40:
            txPower = -20;
        case -20:
        case -16:
        case -12:
        case -8:
        case -4:
        case  0:
        case  3:
        case  4:
        {
            uint32_t errCode = sd_ble_gap_tx_power_set(role, handle, txPower);
            if(errCode == NRF_SUCCESS)
            {
                switch(role)
                {
                    case BLE_GAP_TX_POWER_ROLE_CONN:
                        _connectionTxPower = txPower;
                        break;
                    
                    case BLE_GAP_TX_POWER_ROLE_ADV:
                        _advertiserTxPower = txPower;
                        break;
                    
                    case BLE_GAP_TX_POWER_ROLE_SCAN_INIT:
                        _scanInitTxPower = txPower;
                        break;
                }
                
                SDKTOOLS_DEBUG("[SDKTools] setTxPower() > TX Power Level set to %d dBm.\r\n", txPower);
                return true;
            }
            SDKTOOLS_DEBUG("[SDKTools] setTxPower() > sd_ble_gap_tx_power_set() FAILED! errCode: 0x%04x\r\n", errCode);
            break;
        }
        default:
        {
            SDKTOOLS_DEBUG("[SDKTools] setTxPower()> Wrong value for TX power level ! (%3d)\r\n", txPower);
        }
    }
    return false;
}

// -----------------------------------------------------------------------------------------------------------
// static
bool SDKTools::setDeviceName(uint8_t* name, size_t nameLength)
{
    // TODO: implement this feature
    return true;
}

#if (USE_IEEE11073_16 != 0)
// -----------------------------------------------------------------------------------------------------------
// static
uint8_t SDKTools::floatToIEEE11073Float(float value, uint8_t precision, uint8_t* encodedValue)
{
    uint16_t    tmpResult;
    int8_t      exponent;   ///< Base 10 exponent
    int16_t     mantissa;   ///< Mantissa, should be using only 24 bits

    // Bluetooth FLOAT-TYPE is defined in ISO/IEEE Std. 11073
    // SFLOATs are 16 bits
    // Format [4bit exponent][12bit mantissa]
    if(precision > IEEE11073_16_EXPONENT_MAX)
    {
        precision = IEEE11073_16_EXPONENT_MAX;
    }

    exponent = -precision & IEEE11073_16_EXPONENT_MASK;
    mantissa = (int16_t) floorf(value * powf(10.0, (float) precision));

    if(mantissa > IEEE11073_16_MANTISSA_MAX)
    {
        mantissa = IEEE11073_16_MANTISSA_MAX;
    }
    else if(mantissa < -(IEEE11073_16_MANTISSA_MASK))
    {
        mantissa = -(IEEE11073_16_MANTISSA_MASK);
    }

    tmpResult = (uint16_t)(((int16_t) exponent) << 4) | (mantissa & IEEE11073_16_MANTISSA_MASK);

    encodedValue[0] = (uint8_t)((tmpResult) & 0x00ff);
    encodedValue[1] = (uint8_t)((tmpResult >>  8) & 0x00ff);

    return sizeof(uint16_t);
}

// -----------------------------------------------------------------------------------------------------------
// static
float SDKTools::IEEE11073ToFloat(uint8_t* ieeeFloat)
{
    int8_t  exponent;
    int32_t mantissa;

    exponent = (int8_t) ((ieeeFloat[1] >> 4) & IEEE11073_16_EXPONENT_MASK);
    mantissa = (int16_t)  ieeeFloat[0] + (((int16_t)(ieeeFloat[1] & IEEE11073_16_EXPONENT_MASK) << 8));

    if(exponent > IEEE11073_16_EXPONENT_MAX)
    {
        exponent -= IEEE11073_16_EXPONENT_MASK;
    }

    if(mantissa > IEEE11073_16_MANTISSA_MAX)
    {
        mantissa -= IEEE11073_16_MANTISSA_MASK;
    }
    return ((float) mantissa) * ((float) powf(10.0, exponent));
}
#else
// -----------------------------------------------------------------------------------------------------------
// static
uint8_t SDKTools::floatToIEEE11073Float(float value, uint8_t precision, uint8_t* encodedValue)
{
    uint32_t    tmpResult;
    int8_t      exponent;   ///< Base 10 exponent
    int32_t     mantissa;   ///< Mantissa, should be using only 24 bits

    // Bluetooth FLOAT-TYPE is defined in ISO/IEEE Std. 11073
    // FLOATs are 32 bits
    // Format [8bit exponent][24bit mantissa]
    exponent = -precision;
    mantissa = (int32_t) floorf(value * powf(10.0, (float) precision));

    if(mantissa > IEEE11073_MANTISSA_MAX)
    {
        mantissa = IEEE11073_MANTISSA_MAX;
    }
    else if(mantissa < -(IEEE11073_MANTISSA_MASK))
    {
        mantissa = -(IEEE11073_MANTISSA_MASK);
    }

    tmpResult = (uint32_t)(((int32_t) exponent) << 24) | (mantissa & IEEE11073_MANTISSA_MASK);

    encodedValue[0] = (uint8_t)((tmpResult) & 0x000000ff);
    encodedValue[1] = (uint8_t)((tmpResult >>  8) & 0x000000ff);
    encodedValue[2] = (uint8_t)((tmpResult >> 16) & 0x000000ff);
    encodedValue[3] = (uint8_t)((tmpResult >> 24) & 0x000000ff);

    return sizeof(uint32_t);
}

// -----------------------------------------------------------------------------------------------------------
// static
float SDKTools::IEEE11073ToFloat(uint8_t* ieeeFloat)
{
    int8_t  exponent;
    int32_t mantissa;

    exponent = (int8_t)  ieeeFloat[3];
    mantissa = (int32_t) ieeeFloat[0] + (((int32_t) ieeeFloat[1]) << 8) + (((int32_t) ieeeFloat[2]) << 16);

    if(mantissa > IEEE11073_MANTISSA_MAX)
    {
        mantissa -= IEEE11073_MANTISSA_MASK;
    }
    return ((float) mantissa) * ((float) powf(10.0, exponent));
}
#endif // #if (USE_IEEE11073_16 != 0)

// -----------------------------------------------------------------------------------------------------------
// static
uint8_t SDKTools::floatToUnsignedByteArray(float value, uint8_t* encodedValue)
{
    intFloatUnion_t u;

    u.f = value;
    encodedValue[0] = (uint8_t)((u.i) & 0x000000ff);
    encodedValue[1] = (uint8_t)((u.i >>  8) & 0x000000ff);
    encodedValue[2] = (uint8_t)((u.i >> 16) & 0x000000ff);
    encodedValue[3] = (uint8_t)((u.i >> 24) & 0x000000ff);

    return sizeof(uint32_t);
}

// -----------------------------------------------------------------------------------------------------------
// static
float SDKTools::unsignedByteArrayToFloat(uint8_t* encodedValue)
{
    intFloatUnion_t u;

    u.i  = ((uint32_t) encodedValue[0]);
    u.i |= ((uint32_t) encodedValue[1]) <<  8;
    u.i |= ((uint32_t) encodedValue[2]) << 16;
    u.i |= ((uint32_t) encodedValue[3]) << 24;

    return u.f;
}

// -----------------------------------------------------------------------------------------------------------
// static
float SDKTools::unsignedInt16ToFloat(uint16_t value)
{
    float f;
	if(value >= 0x8000)
    {
        // Applying binary twos complement format
		value = (~value) + 1; 
		f     = ((float) value) * -1;
	} else {
		f     =  (float) value;
	}
    return f;
}

// -----------------------------------------------------------------------------------------------------------
// static
uint16_t SDKTools::GCD(uint16_t a, uint16_t b)
{
    uint8_t c;
    while(a != 0)
    {
        c = a;
        a = b % a;
        b = c;
    }
    return b;
}

// -----------------------------------------------------------------------------------------------------------
// static
uint32_t SDKTools::roundedDiv(uint64_t a, uint64_t b)
{
    uint64_t result = roundedDiv64(a, b);
    return (uint32_t)(result & 0xffffffff);
}

// -----------------------------------------------------------------------------------------------------------
// static
uint64_t SDKTools::roundedDiv64(uint64_t a, uint64_t b)
{
    if(a == 0)
    {
        return 0;
    }
       
    if(b != 0)
    {
        return ((a + (b / 2)) / b);
    }
    else
    {
        ASSERT(false);
        APP_ERROR_HANDLER(NRF_ERROR_INVALID_DATA);
        return 0xffffffffffffffff;
    }
}

// -----------------------------------------------------------------------------------------------------------
// static
uint32_t SDKTools::ceiledDiv(uint64_t a, uint64_t b)
{
    uint64_t result = ceiledDiv64(a, b);
    return (uint32_t)(result & 0xffffffff);
}

// -----------------------------------------------------------------------------------------------------------
// static
uint64_t SDKTools::ceiledDiv64(uint64_t a, uint64_t b)
{
    if(a == 0)
    {
        return 0;
    }

    if(b != 0)
    {
        return 1 + ((a - 1) / b);
    }
    else
    {
        ASSERT(false); 
        APP_ERROR_HANDLER(NRF_ERROR_INVALID_DATA);
        return 0xffffffffffffffff;
    }
}

// -----------------------------------------------------------------------------------------------------------
// static
bool SDKTools::isObjectNULL(void* object)
{
    if(object == NULL)
    {
        LOG_TRACE_DEBUG("[SDKTools] chekcNullObject() > Object is NULL !\r\n");
        return true;
    }
    return false;
}

// -----------------------------------------------------------------------------------------------------------
// static
uint32_t SDKTools::appTimerCreate(void const* timerId, uint8_t timerMode, void* callback)
{
    if(timerMode != APP_TIMER_MODE_SINGLE_SHOT && timerMode != APP_TIMER_MODE_REPEATED)
    {
        LOG_TRACE_DEBUG("[SDKTools] createTimer() > No valid value for timerMode !\r\n");
        return NRF_ERROR_INVALID_PARAM;
    }
    return app_timer_create((app_timer_id_t*) timerId, (app_timer_mode_t) timerMode, (app_timer_timeout_handler_t) callback);
}

// -----------------------------------------------------------------------------------------------------------
// static
uint32_t SDKTools::appTimerStart(void* timerId, uint32_t timeout, void* context)
{
    return app_timer_start((app_timer_id_t) timerId, timeout, context);
}

// -----------------------------------------------------------------------------------------------------------
// static
uint32_t SDKTools::appTimerStop(void* timerId)
{
    return app_timer_stop((app_timer_id_t) timerId);
}

// -----------------------------------------------------------------------------------------------------------
// static
uint32_t SDKTools::appTimerGetCounter()
{ 
    return app_timer_cnt_get(); 
}



/************************************************************************************************************/
/* PRIVATE SECTION                                                                                          */
/************************************************************************************************************/
// -----------------------------------------------------------------------------------------------------------
// static
uint32_t SDKTools::_appTimerInit()
{
#if (USE_APP_TIMER != 0)
    return app_timer_init();
#endif
}
