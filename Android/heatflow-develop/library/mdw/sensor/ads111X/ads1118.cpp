/************************************************************************************************************
*  Arduino Library for Texas Instruments ADS1118 - 16-Bit Analog-to-Digital Converter with 
*  Internal Reference and Temperature Sensor
*  
*  @author Alvaro Salazar <alvaro@denkitronik.com>
*  http://www.denkitronik.com
*
*************************************************************************************************************/


/************************************************************************************************************
 * The MIT License
 *
 * Copyright 2018 Alvaro Salazar <alvaro@denkitronik.com>.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
*************************************************************************************************************/

/************************************************************************************************************/
/* Header files                                                                                             */
/************************************************************************************************************/
#include "ads1118.h"

#if (USE_ADS111X != 0)

#include "gpio/gpiohal.h"
#include "tools/sdktools.h"

#include <boards.h>
#include <string.h>
#include <nrf_delay.h>


/************************************************************************************************************/
/* DEBUG COMPILATION OPTION & MACRO                                                                         */
/************************************************************************************************************/
#define DEBUG_ADS118_ENABLE                 1
#if (DEBUG_ADS118_ENABLE != 0) && defined(DEBUG_NRF_USER)
#include <tools/logtrace.h>
    #define ADS_DEBUG                       LOG_TRACE_DEBUG
#else
    #define ADS_DEBUG(...)                  {(void) (0);}
#endif

#define DEBUG_ADS118_DEEP_ENABLE            1
#if (DEBUG_ADS118_DEEP_ENABLE != 0) && defined(DEBUG_NRF_USER)
#include <tools/logtrace.h>
    #define ADSD_DEBUG                      LOG_TRACE_DEBUG
#else
    #define ADSD_DEBUG(...)                 {(void) (0);}
#endif


/************************************************************************************************************/
/* Namespace's Declaration                                                                                  */
/************************************************************************************************************/
using sensor::ADS1118;

    
/************************************************************************************************************/
/* Declaration section                                                                                      */
/************************************************************************************************************/

/************************************************************************************************************/
/* CONSTRUCTOR                                                                                              */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
ADS1118::ADS1118() : _lastSensorMode(3)
{
//    _spi->initialize();
}

//------------------------------------------------------------------------------------------------------------
ADS1118::~ADS1118()   {}

    
/************************************************************************************************************/
/* PUBLIC SECTION                                                                                           */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
void ADS1118::initialize(uint8_t adsId, uint8_t pinCS) 
{
    // Setup the CS line
    _cs                                 = pinCS;
    _adsIndex                           = adsId;

    // Setup the ADS with default values
    _configRegister.bits.reserved       = 1;            // 0b0000 0000 0000 0001 = 0x0001
    _configRegister.bits.noOperation    = VALID_CFG;    // 0b0000 0000 0000 0010 = 0x0003
    _configRegister.bits.pullUp         = PULLUP;       // 0b0000 0000 0000 1000 = 0x000B
    _configRegister.bits.sensorMode     = ADC_MODE;     // 0b0000 0000 0000 0000 = 0x000B
    _configRegister.bits.rate           = RATE_128SPS;  // 0b0000 0000 1000 0000 = 0x008B
    _configRegister.bits.operatingMode  = SINGLE_SHOT;  // 0b0000 0001 0000 0000 = 0x018B
    _configRegister.bits.pga            = FSR_0256;     // 0b0000 1010 0000 0000 = 0x0B8B
    _configRegister.bits.mux            = DIFF_0_1;     // 0b0000 0000 0000 0000 = 0x0B8B
    _configRegister.bits.singleStart    = 1;            // 0b1000 0000 0000 0000 = 0x8B8B
                                                        // ---------------------
    // Print the config register as DEBUG log           // 0b1000 1011 1000 1011
    _debugDecodeConfigRegister(&_configRegister);       // 0b0000 0100 1001 1011
}

        
#if defined(SOFTDEVICE_PRESENT) && (MHFS_SUPPORT != 0)
//------------------------------------------------------------------------------------------------------------
void ADS1118::bind(ble::MhfsService* bleSrv)
{
    if(bleSrv == NULL)
    {
        ADS_DEBUG("[ADS118] bind() > NUll pointer error on bleSrv !\r\n");
        ASSERT(false);
    }
    _bleSrv                             = bleSrv;
    _bleSrv->adsConfigurationUpdated(_adsIndex, _configRegister.word);
}
#endif  // #if defined(SOFTDEVICE_PRESENT) && (MHFS_SUPPORT != 0)

//------------------------------------------------------------------------------------------------------------
float ADS1118::getVolts(uint16_t adcValue)
{
    float        volts  = tools::SDKTools::unsignedInt16ToFloat(adcValue);
    static float FSR[8] = {6.144, 4.096, 2.048, 1.024, 0.512, 0.256, 0.256, 0.256};
    float        fsr    = FSR[_configRegister.bits.pga];
    uint32_t     value  = 0;
    
    ADS_DEBUG("[ADS118] getVolts() > ads Value: 0x%04x\r\n", adcValue);
    volts = volts * fsr / 32768;

    #if (DEBUG_ADS118_ENABLE != 0) && defined(DEBUG_NRF_USER)
    {
        char debVolt[10] = { 0 };
        sprintf(debVolt, "%8.2f", volts * 1000);
        ADS_DEBUG("[ADS118] getMilliVolts() > Volts: %s [mV]\r\n", debVolt);
    }
    #endif

    #if defined(SOFTDEVICE_PRESENT) && (MHFS_SUPPORT != 0)
    ADSD_DEBUG("[ADS118] getVolts() > --> pushing to characteristic\r\n");
    memcpy(&value, &volts, sizeof(float));
    uint32_t errCode = _bleSrv->adsValuesUpdated(_adsIndex, _configRegister.bits.mux, _configRegister.bits.pga, true, value);
    if(errCode != NRF_SUCCESS)
    {
        ADS_DEBUG("[ADS118] getVolts() > _bleSrv->adsValuesUpdated() failed ! (errCode: 0x%04x)\r\n", errCode);
    }
    else
    {
        ADSD_DEBUG("[ADS118] getVolts() > _bleSrv->adsValuesUpdated() succeeded ! (errCode: 0x%04x)\r\n", errCode);
    }
    #endif // #if defined(SOFTDEVICE_PRESENT) && (MHFS_SUPPORT != 0)
    return volts;
}

//------------------------------------------------------------------------------------------------------------
bool ADS1118::setInputSelected(uint8_t input)
{
    if(input != _configRegister.bits.mux)
    {
        _configRegister.bits.mux = input;
        #if defined(SOFTDEVICE_PRESENT) && (MHFS_SUPPORT != 0)
        _bleSrv->adsConfigurationUpdated(_adsIndex, _configRegister.word);
        #endif // #if defined(SOFTDEVICE_PRESENT) && (MHFS_SUPPORT != 0)
        _debugDecodeConfigRegister(&_configRegister);
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------------------------------------
bool ADS1118::setSampligRate(uint8_t samplingRate)
{
    if(samplingRate != _configRegister.bits.rate)
    {
        _configRegister.bits.rate=samplingRate;
        #if defined(SOFTDEVICE_PRESENT) && (MHFS_SUPPORT != 0)
        _bleSrv->adsConfigurationUpdated(_adsIndex, _configRegister.word);
        #endif // #if defined(SOFTDEVICE_PRESENT) && (MHFS_SUPPORT != 0)
        _debugDecodeConfigRegister(&_configRegister);
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------------------------------------
bool ADS1118::setFullScaleRange(uint8_t fsr)
{
    if(fsr != _configRegister.bits.pga)
    {
        _configRegister.bits.pga = fsr;
        #if defined(SOFTDEVICE_PRESENT) && (MHFS_SUPPORT != 0)
        _bleSrv->adsConfigurationUpdated(_adsIndex, _configRegister.word);
        #endif // #if defined(SOFTDEVICE_PRESENT) && (MHFS_SUPPORT != 0)
        _debugDecodeConfigRegister(&_configRegister);
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------------------------------------
bool ADS1118::setContinuousMode()
{
    if(_configRegister.bits.operatingMode != CONTINUOUS)
    {
        _configRegister.bits.operatingMode = CONTINUOUS;
        #if defined(SOFTDEVICE_PRESENT) && (MHFS_SUPPORT != 0)
        _bleSrv->adsConfigurationUpdated(_adsIndex, _configRegister.word);
        #endif // #if defined(SOFTDEVICE_PRESENT) && (MHFS_SUPPORT != 0)
        _debugDecodeConfigRegister(&_configRegister);
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------------------------------------
bool ADS1118::setSingleShotMode()
{
    if(_configRegister.bits.operatingMode != SINGLE_SHOT)
    {
        _configRegister.bits.operatingMode = SINGLE_SHOT;
        #if defined(SOFTDEVICE_PRESENT) && (MHFS_SUPPORT != 0)
        _bleSrv->adsConfigurationUpdated(_adsIndex, _configRegister.word);
        #endif // #if defined(SOFTDEVICE_PRESENT) && (MHFS_SUPPORT != 0)
        _debugDecodeConfigRegister(&_configRegister);
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------------------------------------
uint32_t ADS1118::delayBeforeFirstResult()
{
    switch(_configRegister.bits.rate)
    {
        case RATE_8SPS:
            return 130;
        case RATE_16SPS:
            return 65;
        case RATE_32SPS:
            return 40;
        case RATE_64SPS:
            return 20;
        case RATE_128SPS:
            return 10;
        case RATE_250SPS:
            return 5;
        case RATE_475SPS:
            return 3;
        default:
            return 2;
    }
}

//------------------------------------------------------------------------------------------------------------
bool ADS1118::disablePullup()
{
    if(_configRegister.bits.pullUp != NO_PULL)
    {
        _configRegister.bits.operatingMode = NO_PULL;
        #if defined(SOFTDEVICE_PRESENT) && (MHFS_SUPPORT != 0)
        _bleSrv->adsConfigurationUpdated(_adsIndex, _configRegister.word);
        #endif // #if defined(SOFTDEVICE_PRESENT) && (MHFS_SUPPORT != 0)
        _debugDecodeConfigRegister(&_configRegister);
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------------------------------------
bool ADS1118::enablePullup()
{
    if(_configRegister.bits.pullUp != PULLUP)
    {
        _configRegister.bits.pullUp = PULLUP;
        #if defined(SOFTDEVICE_PRESENT) && (MHFS_SUPPORT != 0)
        _bleSrv->adsConfigurationUpdated(_adsIndex, _configRegister.word);
        #endif // #if defined(SOFTDEVICE_PRESENT) && (MHFS_SUPPORT != 0)
        _debugDecodeConfigRegister(&_configRegister);
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------------------------------------
void ADS1118::storeRaw(uint16_t value, bool diffId)
{
    uint8_t index                   = (_rawCurrentIdx[diffId]++) & _rawCurrentIdxMask;
    ADSD_DEBUG("[ADS118] storeRaw() > index: %d, diffId: %d, value: 0x%04x\r\n", index, diffId, value);
    _rawAccumulated[diffId][index]  = value;
}

//------------------------------------------------------------------------------------------------------------
void ADS1118::computeMeanRawValue(bool diffId)
{
    #if defined(SOFTDEVICE_PRESENT) && (MHFS_SUPPORT != 0)
    uint32_t value = _meanValue(diffId);
    ADS_DEBUG("[ADS118] computeMeanRawValue() > mean value: 0x%04x\r\n", value);
    uint32_t errCode = _bleSrv->adsValuesUpdated(_adsIndex, _configRegister.bits.mux, _configRegister.bits.pga, false, value);
    if(errCode != NRF_SUCCESS)
    {
        ADS_DEBUG("[ADS118] computeMeanRawValue() > _bleSrv->adsValuesUpdated() failed ! (errCode: 0x%04x)\r\n", errCode);
    }
    else
    {
        ADSD_DEBUG("[ADS118] computeMeanRawValue() > _bleSrv->adsValuesUpdated() succeeded ! (errCode: 0x%04x)\r\n", errCode);
    }
    #endif //     #if defined(SOFTDEVICE_PRESENT) && (MHFS_SUPPORT != 0)
}


/************************************************************************************************************/
/* PRIVATE SECTION                                                                                          */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
bool ADS1118::_setSensorMode(eAdsTsMode mode)
{
    if(mode != _lastSensorMode)
    {
        _configRegister.bits.sensorMode = mode; 
        _lastSensorMode                 = mode;
        #if defined(SOFTDEVICE_PRESENT) && (MHFS_SUPPORT != 0)
        _bleSrv->adsConfigurationUpdated(_adsIndex, _configRegister.word);
        #endif // #if defined(SOFTDEVICE_PRESENT) && (MHFS_SUPPORT != 0)
        _debugDecodeConfigRegister(&_configRegister);
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------------------------------------
float ADS1118::_toTemperature(uint16_t adcValue)
{
    // The ADS1118 offers an integrated precision temperature sensor. The temperature sensor mode is enabled 
    // by setting bit TS_MODE = 1 in the Config Register. Temperature data are represented as a 14-bit result 
    // that is leftjustified within the 16-bit conversion result. Data are output starting with the most 
    // significant byte (MSB). When reading the two data bytes, the first 14 bits are used to indicate the 
    // temperature measurement result. One 14-bit LSB equals 0.03125°C. Negative numbers are represented in 
    // binary twos complement format.
    double dblTemp = 0.0;
    adsValue_t temp;
    
    temp.word = (adcValue >> 2);
    ADS_DEBUG("[ADS118] _toTemperature() > adcValue: 0x%04x, aligned: 0x%04x\r\n", adcValue, temp.word);

    if((adcValue & 0x8000) == 0)
    {
        dblTemp = ((double) temp.word) * 0.03125;
    }
    else 
    {
        // Converting to right-justified and applying binary two's complement format
        temp.word = ~(temp.word-1); 
        dblTemp = ((double) temp.word) * 0.03125 * - 1;
    }

    #if (DEBUG_ADS118_ENABLE != 0) && defined(DEBUG_NRF_USER)
    {
        char debTemp[10] = { 0 };
        sprintf(debTemp, "%6.2f", dblTemp);
        ADS_DEBUG("[ADS118] getTemperature() > Temperature:  %s [C]\r\n", debTemp);
    }
    #endif
    return (float) dblTemp;
}

//------------------------------------------------------------------------------------------------------------
uint32_t ADS1118::_meanValue(bool inputs)
{
    uint64_t result             = 0;
    for(uint8_t i = 0; i < _rawAcumulatedSize; i++)
    {
        result                 += (uint64_t) _rawAccumulated[inputs][i] & 0x000000000000ffff;
    }
    return tools::SDKTools::roundedDiv(result, (uint64_t) _rawAcumulatedSize);
}

//------------------------------------------------------------------------------------------------------------
void ADS1118::_debugDecodeConfigRegister(adsConfig_t* configRegister)
{
    #if (DEBUG_ADS118_ENABLE != 0) && defined(DEBUG_NRF_USER)
    char    strDeb[55] = { 0 };
    uint8_t idxDeb     = 0;

    switch(configRegister->bits.singleStart)
    {
        case 0: sprintf(&strDeb[idxDeb], "%s ", "NOINI"); break;
        case 1: sprintf(&strDeb[idxDeb], "%s ", "START"); break;
    }
    idxDeb +=6;
    
    switch(configRegister->bits.mux)
    {
        case 0: sprintf(&strDeb[idxDeb], "%s ", "A0-A1"); break;
        case 1: sprintf(&strDeb[idxDeb], "%s ", "A0-A3"); break;
        case 2: sprintf(&strDeb[idxDeb], "%s ", "A1-A3"); break;
        case 3: sprintf(&strDeb[idxDeb], "%s ", "A2-A3"); break;
        case 4: sprintf(&strDeb[idxDeb], "%s ", "A0-GD"); break;
        case 5: sprintf(&strDeb[idxDeb], "%s ", "A1-GD"); break;
        case 6: sprintf(&strDeb[idxDeb], "%s ", "A2-GD"); break;
        case 7: sprintf(&strDeb[idxDeb], "%s ", "A3-GD"); break;
    }
    idxDeb +=6;
    
    switch(configRegister->bits.pga)
    {
        case 0: sprintf(&strDeb[idxDeb], "%s ", "6.144"); break;
        case 1: sprintf(&strDeb[idxDeb], "%s ", "4.096"); break;
        case 2: sprintf(&strDeb[idxDeb], "%s ", "2.048"); break;
        case 3: sprintf(&strDeb[idxDeb], "%s ", "1.024"); break;
        case 4: sprintf(&strDeb[idxDeb], "%s ", "0.512"); break;
        case 5: sprintf(&strDeb[idxDeb], "%s ", "0.256"); break;
        case 6: sprintf(&strDeb[idxDeb], "%s ", "0.256"); break;
        case 7: sprintf(&strDeb[idxDeb], "%s ", "0.256"); break;
        }
    idxDeb +=6;        
    
    switch(configRegister->bits.operatingMode)
    {
        case 0: sprintf(&strDeb[idxDeb], "%s ", "CONT."); break;
        case 1: sprintf(&strDeb[idxDeb], "%s ", "SSHOT"); break;
    }
    idxDeb +=6;        
    
    switch(configRegister->bits.rate)
    {
        case 0: sprintf(&strDeb[idxDeb], "%s ", " 8 SPS"); break;
        case 1: sprintf(&strDeb[idxDeb], "%s ", " 16SPS"); break;
        case 2: sprintf(&strDeb[idxDeb], "%s ", " 32SPS"); break;
        case 3: sprintf(&strDeb[idxDeb], "%s ", " 64SPS"); break;
        case 4: sprintf(&strDeb[idxDeb], "%s ", "128SPS"); break;
        case 5: sprintf(&strDeb[idxDeb], "%s ", "250SPS"); break;
        case 6: sprintf(&strDeb[idxDeb], "%s ", "475SPS"); break;
        case 7: sprintf(&strDeb[idxDeb], "%s ", "860SPS"); break;
    }
    idxDeb +=7;        

    switch(configRegister->bits.sensorMode)
    {
        case 0: sprintf(&strDeb[idxDeb], "%s ", "ADC_M"); break;
        case 1: sprintf(&strDeb[idxDeb], "%s ", "TMP_M"); break;
    }
    idxDeb +=6;        

    switch(configRegister->bits.pullUp)
    {
        case 0: sprintf(&strDeb[idxDeb], "%s ", "DISAB"); break;
        case 1: sprintf(&strDeb[idxDeb], "%s ", "ENABL"); break;
    }    
    idxDeb +=6;        

    switch(configRegister->bits.noOperation)
    {
        case 0: sprintf(&strDeb[idxDeb], "%s ", "INVAL"); break;
        case 1: sprintf(&strDeb[idxDeb], "%s ", "VALID"); break;
        case 2: sprintf(&strDeb[idxDeb], "%s ", "INVAL"); break;
        case 3: sprintf(&strDeb[idxDeb], "%s ", "INVAL"); break;
    }
    idxDeb +=6;        

    switch(configRegister->bits.reserved)
    {
        case 0: sprintf(&strDeb[idxDeb], "%s ", "RSRV0"); break;
        case 1: sprintf(&strDeb[idxDeb], "%s ", "RSRV1"); break;
    }

    ADS_DEBUG("[ADS118] _debugDecodeConfigRegister() > Config: %s (0x%04x)\r\n", strDeb, configRegister->word);
    #endif // #if (DEBUG_ADS118_ENABLE != 0) && defined(DEBUG_NRF_USER)
}

#endif  // #if (USE_ADS111X != 0)
