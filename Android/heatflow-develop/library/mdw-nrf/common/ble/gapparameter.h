/****************************************************************************************************************//**
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 *
 * \file    gapparameter.h
 * \class   GapParameter
 * \brief   Class defining the GAP Connection's parameters 
 *
 * This class is defining the connection's parameters at GAP level. It defines the min and max interval timings, 
 * the slave latency and finally the supervision timeout.
 * 
 * Initial author: Patrice Rudaz
 * Creation date: 2018-09-25
 *
 * \author  Patrice Rudaz (patrice.rudaz@hevs.ch)
 * \date    September 2018
 ********************************************************************************************************************/
#pragma once

/* **************************************************************************************************************** */
/*                                                                                                                  */
/* Header files                                                                                                     */
/*                                                                                                                  */
/* **************************************************************************************************************** */
#include <ble-gap-config.h>

/* **************************************************************************************************************** */
/*                                                                                                                  */
/* NAMESPACE Declaration                                                                                            */
/*                                                                                                                  */
/* **************************************************************************************************************** */
namespace ble 
{
    
    /* ************************************************************************************************************ */
    /*                                                                                                              */
    /* Class Declaration                                                                                            */
    /*                                                                                                              */
    /* ************************************************************************************************************ */
    class GapParameter
    {
    public:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* CONSTRUCTOR SECTION                                                                                      */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        /********************************************************************************************************//**
         * \brief   Constructor
         *
         * \note    If both supervisionTmeout and maxInterval are specified, then the following constraint applies:
         *                        supervisionTimeout * 4 > (1 + slaveLatency) * maxInterval
         *
         *          That corresponds to the following Bluetooth Spec requirement:
         *          - The SupervisionTimeout in milliseconds shall be larger than
         *            (1 + slaveLatency) * maxInterval_Max * 2, where maxInterval is given in milliseconds.
         ************************************************************************************************************/
        inline GapParameter(uint16_t minInterval        = 0x0006,       // Minimum allowed by the Bluetooth Spec
                            uint16_t maxInterval        = 0x0c80,       // Maximum allowed by the Bluetooth Spec
                            uint16_t slaveLatency       = SLAVE_LATENCY, 
                            uint16_t supervisionTimeout = APP_CONN_SUP_TIMEOUT) 
            : _minInterval(minInterval), 
              _maxInterval(maxInterval), 
              _slaveLatency(slaveLatency), 
              _supervisionTimeout(supervisionTimeout) {};


        /* ******************************************************************************************************** */
        /*																										    */
        /* PUBLIC DECLARATION SECTION                                                                               */
        /*																										    */
        /* ******************************************************************************************************** */
        /********************************************************************************************************//**
         * \brief   Serialisation of the class
          ***********************************************************************************************************/
        inline uint8_t* parameter()                     { return (uint8_t*) this; }

        /********************************************************************************************************//**
         * \brief   Returns the Minimum Interval, in counter's ticks.
         ************************************************************************************************************/
        inline uint16_t minInterval() const             { return _minInterval; }

        /********************************************************************************************************//**
         * \brief   Returns the Maximum Interval, in counter's ticks.
         ************************************************************************************************************/
        inline uint16_t maxInterval() const             { return _maxInterval; }
        

        /********************************************************************************************************//**
         * \brief   Returns the Minimum Interval, in milliseconds [ms].
         ************************************************************************************************************/
        inline float minIntervalMs() const              { return ((float) _minInterval) * 1.25; }

        /********************************************************************************************************//**
         * \brief   Returns the Maximum Interval, in milliseconds [ms].
         ************************************************************************************************************/
        inline float maxIntervalMs() const              { return ((float) _maxInterval) * 1.25; }


        /********************************************************************************************************//**
         * \brief   Returns the Slave Latency.
         ************************************************************************************************************/
        inline uint16_t slaveLatency() const            { return _slaveLatency; }

        /********************************************************************************************************//**
         * \brief   Returns the Supervision Timeout in counter's ticks.
         ************************************************************************************************************/
        inline uint16_t supervisionTimeout() const      { return _supervisionTimeout; }
        

        /********************************************************************************************************//**
         * \brief   Returns the Supervision Timeout in Second [s].
         ************************************************************************************************************/
        inline uint16_t supervisionTimeoutSec() const   { return _supervisionTimeout * 10 / 1000; }


    private:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PRIVATE DECLARATION SECTION                                                                              */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        // Attributes
        uint16_t _minInterval;                          ///< \brief Minimum Connection Interval in 1.25 ms units
        uint16_t _maxInterval;                          ///< \brief Maximum Connection Interval in 1.25 ms units
        uint16_t _slaveLatency;                         ///< \brief Slave Latency in number of connection events
        uint16_t _supervisionTimeout;                   ///< \brief Connection Supervision Timeout in 10 ms 
    };
    
}  // namespace ble
