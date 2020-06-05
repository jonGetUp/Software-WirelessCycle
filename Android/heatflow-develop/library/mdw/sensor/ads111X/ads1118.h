/****************************************************************************************************************//**
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 *
 * \file        ads1118.h
 *
 * \addtogroup  Sensor
 * \{
 *
 * \addtogroup  ADS
 * \{
 *
 * \brief   Library for Texas Instruments ADS1118 - 16-Bit Analog-to-Digital Converter
 *
 * Library for Texas Instruments ADS1118 - 16-Bit Analog-to-Digital Converter with internal Reference and 
 * Temperature Sensor
 *
 * Initial author: Alvaro Salazar <alvaro@denkitronik.com>
 *
 * \author  Patrice Rudaz
 * \date    February 2019
 ********************************************************************************************************************/
#pragma once

/* **************************************************************************************************************** */
/*                                                                                                                  */
/* Header files                                                                                                     */
/*                                                                                                                  */
/* **************************************************************************************************************** */
#include <platform-config.h>

#if (USE_ADS111X != 0)
    #include <stdint.h>

    #include <ble-private-service-config.h>
    #if defined(SOFTDEVICE_PRESENT) && (MHFS_SUPPORT != 0)
        #include "mhfs/mhfsdefinition.h"
        #include "mhfs/mhfsservice.h"
    #endif  // #if defined(SOFTDEVICE_PRESENT) && (MHFS_SUPPORT != 0


/* **************************************************************************************************************** */
/*                                                                                                                  */
/* NAMESPACE Declaration                                                                                            */
/*                                                                                                                  */
/* **************************************************************************************************************** */
namespace sensor 
{

    /* ************************************************************************************************************ */
    /*                                                                                                              */
    /* Class Declaration                                                                                            */
    /*                                                                                                              */
    /* ************************************************************************************************************ */
    class ADS1118
    {
    public:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* CONSTRUCTOR SECTION                                                                                      */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        ADS1118();
        virtual ~ADS1118();


        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PUBLIC DECLARATION SECTION                                                                               */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        /********************************************************************************************************//**
         * Enumeration of ADS118 inputs configurations
         * - Input multiplexer configuration selection for bits "MUX"
         ************************************************************************************************************/
        typedef enum
        {
            DIFF_0_1        = 0x00,         ///< \brief Differential input: Vin=A0-A1 (0b000)
            DIFF_0_3        = 0x01,         ///< \brief Differential input: Vin=A0-A3 (0b001) 
            DIFF_1_3        = 0x02,         ///< \brief Differential input: Vin=A1-A3 (0b010)
            DIFF_2_3        = 0x03,         ///< \brief Differential input: Vin=A2-A3 (0b011)

            AIN_0           = 0x04,         ///< \brief Single ended input: Vin=A0 (0b100)
            AIN_1           = 0x05,         ///< \brief Single ended input: Vin=A1 (0b101)
            AIN_2           = 0x06,         ///< \brief Single ended input: Vin=A2 (0b110)
            AIN_3           = 0x07          ///< \brief Single ended input: Vin=A3 (0b111)
        } eAdsInputs;

        /********************************************************************************************************//**
         * Enumeration of ADS118 modes
         * - ADC: External (inputs) voltage reading mode
         * - Temperature:  Internal temperature sensor reading mode
         ************************************************************************************************************/
        typedef enum
        {
            ADC_MODE        = 0,            ///< \brief External (inputs) voltage reading mode
            TEMP_MODE       = 1             ///< \brief Internal temperature sensor reading mode
        } eAdsTsMode;
        
        // Used by "MODE" bit
        typedef enum
        {
            CONTINUOUS      = 0,            ///< \brief Continuous conversion mode
            SINGLE_SHOT     = 1             ///< \brief Single-shot conversion and power down mode
        } eAdsMode;
        
        // Used by "PULL_UP_EN" bit
        typedef enum
        {
            PULLUP          = 1,            ///< \brief Internal pull-up resistor enabled for DOUT ***DEFAULT
            NO_PULL         = 0             ///< \brief Internal pull-up resistor disabled
        } eAdsPullUp;
        
        // Used by "NOP" bits
        typedef enum
        {
            VALID_CFG       = 0x01,         ///< \brief Data will be written to Config register
            NO_VALID_CFG    = 0x00          ///< \brief Data won't be written to Config register
        } eAdsNop;
        
        // Full scale range (FSR) selection by "PGA" bits. 
        // [Warning: this could increase the noise and the effective number of bits (ENOB). See tables above]
        typedef enum
        {
            FSR_6144       = 0x00,         ///< \brief Range: ±6.144 v. LSB SIZE = 187.5µV
            FSR_4096       = 0x01,         ///< \brief Range: ±4.096 v. LSB SIZE = 125µV
            FSR_2048       = 0x02,         ///< \brief Range: ±2.048 v. LSB SIZE = 62.5µV ***DEFAULT
            FSR_1024       = 0x03,         ///< \brief Range: ±1.024 v. LSB SIZE = 31.25µV
            FSR_0512       = 0x04,         ///< \brief Range: ±0.512 v. LSB SIZE = 15.625µV
            FSR_0256       = 0x05          ///< \brief Range: ±0.256 v. LSB SIZE = 7.8125µV
        } eAdsFsr;

        // Sampling rate selection by "DR" bits. 
        // [Warning: this could increase the noise and the effective number of bits (ENOB). See tables above]
        typedef enum
        {
            RATE_8SPS      = 0x00,         ///< \brief 8 samples/s, Tconv=125ms
            RATE_16SPS     = 0x01,         ///< \brief 16 samples/s, Tconv=62.5ms
            RATE_32SPS     = 0x02,         ///< \brief 32 samples/s, Tconv=31.25ms
            RATE_64SPS     = 0x03,         ///< \brief 64 samples/s, Tconv=15.625ms
            RATE_128SPS    = 0x04,         ///< \brief 128 samples/s, Tconv=7.8125ms
            RATE_250SPS    = 0x05,         ///< \brief 250 samples/s, Tconv=4ms
            RATE_475SPS    = 0x06,         ///< \brief 475 samples/s, Tconv=2.105ms
            RATE_860SPS    = 0x07          ///< \brief 860 samples/s, Tconv=1.163ms    
        } eAdsDr;
        
        /********************************************************************************************************//**
         * \brief   Union representing the "config register"
         *
         * Union representing the "config register" in 3 ways: 
         *  - bits, 
         *  - word (16 bits) 
         *  - byte (8 bits)
         *
         * \see See the datasheet [1] for more information...
         ************************************************************************************************************/
        typedef union
        {
            ///< \brief Structure of the config register of the ADS1118.
            struct adsConfigBits_s
            {
                uint8_t reserved:1;         ///< \brief "Reserved" bit
                uint8_t noOperation:2;      ///< \brief "NOP" bits
                uint8_t pullUp:1;           ///< \brief "PULL_UP_EN" bit    
                uint8_t sensorMode:1;       ///< \brief "TS_MODE" bit    
                uint8_t rate:3;             ///< \brief "DR" bits
                uint8_t operatingMode:1;    ///< \brief "MODE" bit        
                uint8_t pga:3;              ///< \brief "PGA" bits
                uint8_t mux:3;              ///< \brief "MUX" bits
                uint8_t singleStart:1;      ///< \brief "SS" bit
            } bits;
            uint16_t word;                  ///< \brief Representation in word (16-bits) format
            struct 
            {
                uint8_t lsb;			    ///< \brief Byte LSB
                uint8_t msb;			    ///< \brief Byte MSB
            } byte;						    ///< \brief Representation in bytes (8-bits) format
        } adsConfig_t;
        
        /********************************************************************************************************//**
         * \brief   Union representing the read value
         *
         * Union representing the ADC value in 2 ways: 
         *  - word (16 bits) 
         *  - byte (2 bits)
         ************************************************************************************************************/
        typedef union
        {
            uint16_t word;                  ///< \brief Representation in word (16-bits) format
            struct 
            {
                uint8_t lsb;			    ///< \brief Byte LSB
                uint8_t msb;			    ///< \brief Byte MSB
            } byte;						    ///< \brief Representation in bytes (8-bits) format
        } adsValue_t;


        /********************************************************************************************************//**
         * \brief    This method initialize the SPI port and the config register
         ************************************************************************************************************/
        void initialize(uint8_t adsId, uint8_t pinCS);
        
        #if defined(SOFTDEVICE_PRESENT) && (MHFS_SUPPORT != 0)
        /********************************************************************************************************//**
         * \brief    Binds the needed objects
         ************************************************************************************************************/
        void bind(ble::MhfsService* bleSrv);
        #endif  // #if defined(SOFTDEVICE_PRESENT) && (MHFS_SUPPORT != 0)
        
        /********************************************************************************************************//**
         * \brief   Getting the temperature in degrees celsius from the internal sensor of the ADS1118
         ************************************************************************************************************/
        float getTemperature(uint16_t adcValue)             { return _toTemperature(adcValue); }
        
        /********************************************************************************************************//**
         * \brief   Getting the ADC value in milliVolts
         ************************************************************************************************************/
        float getVolts(uint16_t adcValue);

        /********************************************************************************************************//**
         * \brief   Setting to continuous adquisition mode
         ************************************************************************************************************/
        bool setTemperatureMode()                           { return _setSensorMode(TEMP_MODE); } 

        /********************************************************************************************************//**
         * \brief   Setting to continuous adquisition mode
         ************************************************************************************************************/
        bool setAdcMode()                                   { return _setSensorMode(ADC_MODE); }

        /********************************************************************************************************//**
         * \brief   Setting the sampling rate specified in the config register
         *
         * \param   samplingRate    The value to be set in the config register.
         ************************************************************************************************************/
        bool setSampligRate(uint8_t samplingRate);

        /********************************************************************************************************//**
         * \brief   Setting the full scale range in the config register
         *
         * \param   fsr             The value to be set in the config register.
         ************************************************************************************************************/
        bool setFullScaleRange(uint8_t fsr);

        /********************************************************************************************************//**
         * \brief   Setting to continuous adquisition mode
         ************************************************************************************************************/
        bool setContinuousMode();

        /********************************************************************************************************//**
         * \brief   Setting to single shot adquisition and power down mode
         ************************************************************************************************************/
        bool setSingleShotMode();

        /********************************************************************************************************//**
         * \brief   Returns the delay in [ms] to wiat befaore having the first value ready.
         *
         * Returns the delay in [ms] to wiat befaore having the first value ready according to the current 
         * configuration.
         *
         * \return  The delay to wait in millisecond.
         ************************************************************************************************************/
        uint32_t delayBeforeFirstResult();

        /********************************************************************************************************//**
         * \brief   Disabling the internal pull-up resistor of the DOUT pin
         ************************************************************************************************************/
        bool disablePullup();

        /********************************************************************************************************//**
         * \brief   Enabling the internal pull-up resistor of the DOUT pin
         ************************************************************************************************************/
        bool enablePullup();
        
        inline uint16_t getConfig()                         { return _configRegister.word; }
        inline uint8_t  getMsbConfig()                      { return _configRegister.byte.msb; }
        inline uint8_t  getLsbConfig()                      { return _configRegister.byte.lsb; }
        inline uint16_t frequencyClock() const              { return _ADSCLK; }
        inline uint8_t  chipSelect() const                  { return _cs; }

        /********************************************************************************************************//**
         * \brief   Setting the inputs to be acquired in the config register.
         *
         * \param   input               The input to be acquired.
         ************************************************************************************************************/
        bool setInputSelected(uint8_t input);
        
        void storeRaw(uint16_t value, bool diffId); 
        void computeMeanRawValue(bool diffId);
        inline uint32_t meanRawValue(bool diffId)           { return _meanValue(diffId); }
        inline uint8_t rawAcumulatedSize() const            { return _rawAcumulatedSize; }
           
    protected:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PROTECTED DECLARATION SECTION                                                                            */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        
    private:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PRIVATE DECLARATION SECTION                                                                              */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        // Attributes
        adsConfig_t             _configRegister;        // Config register
        uint8_t                 _lastSensorMode;        // Last sensor mode selected (ADC_MODE or TEMP_MODE or none)
        uint8_t                 _cs;                    // Chip select pin (choose one)
        uint8_t                 _adsIndex;              // Identifier of the ADS (if more than one ADS is present on the hardware
        
        //Bit constants
        static const uint16_t   _ADSCLK = (uint16_t) 4000000;    // ADS1118 SCLK frequency: 2000000 Hz Maximum for ADS1118
    
        static const uint8_t    _rawAcumulatedSize = 0x10;
        static const uint8_t    _rawCurrentIdxMask = 0x0f;
        uint16_t                _rawAccumulated[2][_rawAcumulatedSize]; // Circular buffer of measurements of each differential input 0-1 & 2-3
        uint8_t                 _rawCurrentIdx[2];      // index
        
        #if defined(SOFTDEVICE_PRESENT) && (MHFS_SUPPORT != 0)
        ble::MhfsService*       _bleSrv;
        #endif  // #if defined(SOFTDEVICE_PRESENT) && (MHFS_SUPPORT != 0)
        
        // Methods
        bool                    _setSensorMode(eAdsTsMode mode);
        float                   _toTemperature(uint16_t adcValue);
        void                    _debugDecodeConfigRegister(adsConfig_t* configRegister);
        uint32_t                _meanValue(bool inputs);
        
    };  // class ADS1118
    
}   // namespace sensor

#endif  // #if (USE_ADS111X != 0)

/** \} */   // Group: ADS

/** \} */   // Group: Sensor
