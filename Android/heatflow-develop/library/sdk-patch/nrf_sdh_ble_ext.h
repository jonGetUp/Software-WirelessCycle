/****************************************************************************************************************//**
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Thomas Sterren (thomas.sterren@hevs.ch) & Patrice Rudaz (patrice.rudaz@hevs.ch)
 * All rights reserved.
 *
 * \file    nrf_sdh_ble_ext.h
 *
 * \addtogroup PatchSDK
 * \{
 *
 * \brief   Re-defines the Nordic's MACROS for C++ code !
 *
 * Initial author: Patrice Rudaz
 * Creation date: 2017-11-26
 *
 * \author  Patrice Rudaz
 * \date    July 2018
 ********************************************************************************************************************/
#pragma once

/* **************************************************************************************************************** */
/*                                                                                                                  */
/* Header files                                                                                                     */
/*                                                                                                                  */
/* **************************************************************************************************************** */
#include <nrf_sdh_ble.h>

#ifdef __cplusplus
    #if !(defined(__LINT__))

        /********************************************************************************************************//**
         * \brief   Macro for registering \ref nrf_sdh_soc_evt_observer_t. Modules that want to be
         *          notified about SoC events must register the handler using this macro.
         *
         * \details This macro places the observer in a section named "sdh_soc_observers".
         *
         * \param[in]   _name       Observer name.
         * \param[in]   _prio       Priority of the observer event handler.
         *                          The smaller the number, the higher the priority.
         * \param[in]   _handler    BLE event handler.
         * \param[in]   _context    Parameter to the event handler.
         * \hideinitializer
         ************************************************************************************************************/
        #define NRF_SDH_BLE_OBSERVER_CPP(_name, _prio, _handler, _context)                                          \
        STATIC_ASSERT(NRF_SDH_BLE_ENABLED, "NRF_SDH_BLE_ENABLED not set!");                                         \
        STATIC_ASSERT(_prio < NRF_SDH_BLE_OBSERVER_PRIO_LEVELS, "Priority level unavailable.");                     \
        NRF_SECTION_SET_ITEM_REGISTER(sdh_ble_observers, _prio, static nrf_sdh_ble_evt_observer_t _name) =          \
        {                                                                                                           \
            _handler,                                                                                               \
            _context                                                                                                \
        }

        /********************************************************************************************************//**
         * \brief   Macro for registering an array of \ref nrf_sdh_ble_evt_observer_t.
         *          Modules that want to be notified about SoC events must register the handler using
         *          this macro.
         *
         * Each observer's handler will be dispatched an event with its relative context from \p _context.
         * This macro places the observer in a section named "sdh_ble_observers".
         *
         * \param[in]   _name       Observer name.
         * \param[in]   _prio       Priority of the observer event handler.
         *                          The smaller the number, the higher the priority.
         * \param[in]   _handler    BLE event handler.
         * \param[in]   _context    An array of parameters to the event handler.
         * \param[in]   _cnt        Number of observers to register.
         ************************************************************************************************************/
        #define NRF_SDH_BLE_OBSERVERS_CPP(_name, _prio, _handler, _context, _cnt)                                   \
        STATIC_ASSERT(NRF_SDH_BLE_ENABLED, "NRF_SDH_BLE_ENABLED not set!");                                         \
        STATIC_ASSERT(_prio < NRF_SDH_BLE_OBSERVER_PRIO_LEVELS, "Priority level unavailable.");                     \
        NRF_SECTION_SET_ITEM_REGISTER(sdh_ble_observers, _prio, static nrf_sdh_ble_evt_observer_t _name[_cnt]) =    \
        {                                                                                                           \
            MACRO_REPEAT_FOR(_cnt, HANDLER_SET_CPP, _handler, _context)                                             \
        }

        #if !(defined(DOXYGEN))
            #define HANDLER_SET_CPP(_idx, _handler, _context)                                                       \
            {                                                                                                       \
                _handler,                                                                                           \
                _context[_idx],                                                                                     \
            },
        #endif

    #else   // __LINT__

        /********************************************************************************************************//**
         * Swallow semicolons
         ************************************************************************************************************/
        /*lint -save -esym(528, *) -esym(529, *) : Symbol not referenced. */
        #define NRF_SDH_BLE_OBSERVER_CPP(A, B, C, D)     static int semicolon_swallow_##A
        #define NRF_SDH_BLE_OBSERVERS_CPP(A, B, C, D, E) static int semicolon_swallow_##A
        /*lint -restore */

    #endif  // #if !(defined(__LINT__))

#endif  // #ifdef __cplusplus

/** \} */   // Group: PatchSDK
