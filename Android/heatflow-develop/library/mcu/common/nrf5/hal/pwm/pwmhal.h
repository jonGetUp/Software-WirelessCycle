/****************************************************************************************************************//**
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 *
 * \file    pwmhal.h
 *
 * \addtogroup LowHAL
 * @{
 *
 * \class   hal::PwmHal
 * \brief   Class of Hardware Abstraction Layer to handle the PWM Module of the Nordic nRF5 series chips.
 *
 * Class of Hardware Abstraction Layer to handle the PWM Module of of the nRf5 series chips based on Nordic SDK v15
 * and later. The Pulse Width Modulation (PWM) module driver includes two layers: the hardware access layer (HAL) 
 * and the driver layer (DRV). This class uses only the driver layer.
 *
 * Key features include:
 *  - Multi-instance support
 *  - Two kinds of playback:
 *  - Simple: a single sequence of duty cycle values
 *  - Complex: two concatenated sequences
 *  - Sequences are played back a specified number of times
 *  - Optionally, the whole playback can be repeated in a loop
 *  - A user-defined event handler can be used to perform additional actions when a sequence or the whole playback 
 *    is finished
 *  - If no event handler is used, playbacks can be carried out without involving the CPU
 *
 * The configurable parameters include:
 *  - Pins to be used as outputs for PWM channels (up to 4 per instance)
 *  - Base clock frequency and top counter value (these parameters jointly determine the PWM period and resolution)
 *  - Mode of loading the sequence values from RAM and distributing them to compare registers:
 *    - Common:     One value is used in all channels.
 *    - Grouped:    One value is used in channels 0 and 1, and another one in channels 2 and 3.
 *    - Individual: Separate values are used in each channel.
 *    - Waveform:   Similar to Individual mode, but only three channels are used. The fourth value is loaded into the 
 *                  pulse generator counter as its top value.
 *  - Mode of advancing the active sequence (automatic or triggered via a task)
 *
 * For more details, look at the Nordic's documentation:
 * \link http://infocenter.nordicsemi.com/index.jsp?topic=/com.nordic.infocenter.nrf52/dita/nrf52/development/nrf52_dev_kit.html&cp=1_1
 *
 * Initial author: Patrice Rudaz
 * Creation date: 2018-08-02
 *
 * \author  Patrice Rudaz
 * \date    August 2018
 ********************************************************************************************************************/
#pragma once


/* **************************************************************************************************************** */
/*                                                                                                                  */
/* Header files                                                                                                     */
/*                                                                                                                  */
/* **************************************************************************************************************** */
#include <nrfx_pwm.h>
#include <pwm-config.h>


/* **************************************************************************************************************** */
/*                                                                                                                  */
/* NAMESPACE Declaration                                                                                            */
/*                                                                                                                  */
/* **************************************************************************************************************** */
namespace hal 
{

    /* ************************************************************************************************************ */
    /*                                                                                                              */
    /* Class Declaration                                                                                            */
    /*                                                                                                              */
    /* ************************************************************************************************************ */
    class PwmHal
    {
    public:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* CONSTRUCTOR DECLARATION SECTION                                                                          */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        PwmHal();
        virtual ~PwmHal();
        
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PUBLIC DECLARATION SECTION                                                                               */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        /********************************************************************************************************//**
         * \brief   Enumeration of PWM instances
         ************************************************************************************************************/
        typedef enum
        {
            UNKNOWN_PWM = 0xFF,
            PWM0        = 0x00,
            PWM1        = 0x01,
            PWM2        = 0x02
        } ePwmId;

        /********************************************************************************************************//**
         * \brief   Initialize and configure the driver for a given instance.
         *
         * \param   pwmId       The identifier of the instance of PWM. MUST be member of \ref ePwmID enumeration.
         * \param   loadMode    Mode of loading sequence data from RAM.
         ************************************************************************************************************/
        void initilialize(ePwmId pwmId, nrf_pwm_dec_load_t loadMode = NRF_PWM_LOAD_INDIVIDUAL);
        
        /********************************************************************************************************//**
         * \brief   register a specific event handler for the current PWM.
         *
         * \param   pwmEvtHandler   Event handler provided by the user. If NULL is passed instead, event 
         *                          notifications are not done and PWM interrupts are disabled.
         * \param   flag            Enable/Disable specific interrupts when using an external PWM Event handler.
         ************************************************************************************************************/
        void registerEventHandler(nrfx_pwm_handler_t pwmEvtHandler, uint32_t flags = 0);
        
        /********************************************************************************************************//**
         * \brief   Set the frequency for the current PWM (in Hz).
         *
         * The frequency value is epxressed in Hz and must be more than 0 un less than 16 MHz
         *
         * \param   frequency           The frequency in Hz.
         * \param   updateBaseClock     Flag to let the base clock been updated, or not.
         *
         * \return  `true` if the new parameter could be set successfully and `false` otherwise.
         ************************************************************************************************************/
        bool setFrequency(uint32_t frequency, bool updateBaseClock = true);
       
        /********************************************************************************************************//**
         * \brief   Set the gpios used as output.
         *
         * \param   outputs     Pointer to an array of PIN numbers. The size of the array can not overload the 
         *                      \ref NRF_PWM_CHANNEL_COUNT
         * \param   size        Amount of given PIN numbers.
         * \param   inverted    A flag to let the PWM know if the GPIO is active low or high.
         ************************************************************************************************************/
        void setOutputPin(uint8_t* output, size_t size, bool inverted);
       
        /********************************************************************************************************//**
         * \brief   Adds the GPIO defined by its PIN number as output of the current PWM instance.
         *
         * \param   pinNbr      The PIN number to use as output.
         * \param   channelIdx  Identifier of the channel to assign the given PIN number
         * \param   inverted    A flag to let the PWM know if the GPIO is active low or high.
         ************************************************************************************************************/
        void addOutputPin(uint8_t  pinNbr, uint8_t channelIdx, bool inverted);
        
        /********************************************************************************************************//**
         * \brief   Set the values and the durations for the sequence identified by its index.
         *
         * The pWM module works with sequences and is able to play 2 sequences (one after the other). This method 
         * MUST be used to set the values of one sequence. The sequence is identified by its index, which should 
         * only be `0` or `1`.
         *
         * \param   index       The sequence's identifier
         * \param   values      Pointer to the sequence's values (for indivirual load mode)
         * \param   size        Size of the sequence.
         * \param   duration    Time, in [ms], of each value.
         * \param   endDelay    Additional time, in [ms], to be applied after the last value of the sequence.
         ************************************************************************************************************/
        bool setSequenceValuesAndDurations(uint8_t    index, 
                                           uint16_t*  values, 
                                           size_t     size,
                                           uint32_t   duration,
                                           uint32_t   endDelay);
        
        /********************************************************************************************************//**
         * \brief   Set the values for the sequence identified by its index.
         *
         * \param   index       The sequence's identifier
         * \param   values      Pointer to the sequence's values (for indivirual load mode)
         * \param   size        Size of the sequence.
         ************************************************************************************************************/
        bool setSequenceValues(uint8_t    index, 
                               uint16_t*  values, 
                               size_t     size);

        /********************************************************************************************************//**
         * \brief   Set the durations for the sequence identified by its index.
         *
         * \param   index       The sequence's identifier
         * \param   duration    Time, in [ms], of each value.
         * \param   endDelay    Additional time, in [ms], to be applied after the last value of the sequence.
         ************************************************************************************************************/
        bool setSequenceDurations(uint8_t    index, 
                                  uint32_t   duration,
                                  uint32_t   endDelay);
        
        /********************************************************************************************************//**
         * \brief   Clear the values of the sequence identified by the given index.
         ************************************************************************************************************/
        void clearSequenceValues(uint8_t index);

        /********************************************************************************************************//**
         * \brief   Starts pplaying back the defined sequences for n times.
         *
         * \param   playbackCount   The number of time to play back the seuqences...
         * \param   loop            If `true` the PWM will be played again and again. And if `false` the pwm will
         *                          repeat the sequence only the `playbackCount` times. 
         ************************************************************************************************************/
        bool start(uint16_t playbackCount, bool loop);
        
        /********************************************************************************************************//**
         * \brief   Returns the BaseClock configuration of the current PWM.
         *
         * Returns the BaseClock configuration of the current PWM. IN combination with the `topCounterValue`, we can
         * compute the frequency of the PWM: frequency = baseClk / topCounterValue
         ************************************************************************************************************/
        inline nrf_pwm_clk_t baseClk() const        { return _pwmConfig.base_clock; }
        
        /********************************************************************************************************//**
         * \brief   Returns the loading mode of the values of the current PWM.
         ************************************************************************************************************/
        inline nrf_pwm_dec_load_t loadMode() const  { return _pwmConfig.load_mode; }
        
        /********************************************************************************************************//**
         * \brief   Returns the first PIN used as output HGPIO.
         *
         * \param   index   Pointer tot he index of the PIN to look for. If the index is greater as the number of
         *                  pin in used, the method retrieves the the last PIN... 
         ************************************************************************************************************/
        inline uint8_t outputPin(uint8_t* index)    { return _outputPin(index); }

        /********************************************************************************************************//**
         * \brief   Stop immediately the current PWM.
         ************************************************************************************************************/
        inline void stop()                          { _uninitPwm(); }
        
        /********************************************************************************************************//**
         * \brief   Returns the counter value of the current PWM. 
         *
         * Returns the counter value of the current PWM. IN combination with the Base Clock, we can compute the
         * frequency of the PWM: frequency = baseClk / topCounterValue
         ************************************************************************************************************/
        inline uint16_t topCounterValue() const     { return _pwmConfig.top_value; }
        
        /********************************************************************************************************//**
         * \brief   Unset the output PIN for the given PWM channel
         *
         * \param   channelId   The channel number (0..3)
         ************************************************************************************************************/
        inline void delOutputPin(uint8_t channelId) { _pwmConfig.output_pins[channelId] = NRFX_PWM_PIN_NOT_USED; }
        
        /********************************************************************************************************//**
         * \brief   Unregister the event handler for the current PWM.
         ************************************************************************************************************/
        inline void unregisterEventHandler()        { _pwmEvtHandler = NULL; _flags = 0; }
        
        /********************************************************************************************************//**
         * \brief   Returns the PWM Identifier as a String. (DEBUG purpose)
         *
         * \param   id   The given PWM id to translate in string.
         *************************************************************************************************************/
        inline const char* toString()
        {
            switch(_pwmId)
            {
                case PWM0: return "PWM0";
                case PWM1: return "PWM1";
                case PWM2: return "PWM2";
                default:   return "Unknown PWM";
            }
        }
        
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
        // Constnts
        static const uint32_t               _frequencyMax = APP_PWM_MAX_FREQUENCY;

        // Attributes
        ePwmId                              _pwmId;
        uint8_t                             _outputs[NRF_PWM_CHANNEL_COUNT];
        nrf_pwm_dec_load_t                  _loadMode;

        uint32_t                            _frequency;
        nrfx_pwm_t                          _pwmInstance;
        nrfx_pwm_config_t                   _pwmConfig;
        nrfx_pwm_handler_t                  _pwmEvtHandler;
        uint32_t                            _flags;
    
        nrf_pwm_sequence_t                  _sequences[2];

        bool                                _isInitialized;

        // Methods
        void                                _uninitPwm();
        void                                _setBaseClk(uint32_t frequency);
        uint8_t                             _outputPin(uint8_t* index);


        /************************************************************************************************************
         * \brief   Returns the frequency related to the given base clock.
         *
         * Returns the frequency related to the given base clock. If the base clock is unknowm, it reutrns `0`.
         *
         * \param   baseClk  The given base clock.
         *
         * \return  The frequency value  in Hz related to the given base clock. `0` Hz, if the base clock is unkonwn.
         *************************************************************************************************************/
        inline uint32_t _baseFrequency()
        {
            switch(_pwmConfig.base_clock)
            {
                case NRF_PWM_CLK_16MHz:     return 16000000;
                case NRF_PWM_CLK_8MHz:      return  8000000;
                case NRF_PWM_CLK_4MHz:      return  4000000;
                case NRF_PWM_CLK_2MHz:      return  2000000;
                case NRF_PWM_CLK_1MHz:      return  1000000;
                case NRF_PWM_CLK_500kHz:    return   500000;
                case NRF_PWM_CLK_250kHz:    return   250000;
                case NRF_PWM_CLK_125kHz:    return   125000;
                default:                    return        0;
            }
        }
    };
    
}   // namespace hal

/** @} */
