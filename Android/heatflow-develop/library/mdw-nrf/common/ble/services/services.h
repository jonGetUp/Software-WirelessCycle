/****************************************************************************************************************//**
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 *
 * \file    services.h
 *
 * \addtogroup BLE
 * \{
 *
 * \defgroup bleSrv BLE Adopted Services
 * \{
 *
 * \class   ble::Services
 * \brief   Class handling BLE advertisement feature provided by the Nordic SoftDevice v15
 *
 * Initial author: Patrice Rudaz
 * Creation date: 2017-01-26
 *
 * \author  Patrice Rudaz (patrice.rudaz@hevs.ch)
 * \date    July 2018
 ********************************************************************************************************************/
#pragma once

/* **************************************************************************************************************** */
/*                                                                                                                  */
/* Header files                                                                                                     */
/*                                                                                                                  */
/* **************************************************************************************************************** */
#if defined(SOFTDEVICE_PRESENT)
#include <ble-services-config.h>
#include <ble_types.h>

#include "ble/interface/bleobserver.h"
#include "ble/interface/bleservicesobserver.h"
#include "ble/blecontroller.h"

#include "core/isubject.hpp"


#if defined(__cplusplus)
extern "C" 
{
#endif
    namespace ble 
    {
    #if (BAS_SUPPORT != 0)
        /********************************************************************************************************//**
         * \brief   Battery Service event handler.
         *
         * This function will be called for all Battery Service events, which are passed to the application.
         *
         * \param   pBas    Device Information Service stucture.
         * \param   pEvt    Event received from the Immedaite Alert Service.
         ************************************************************************************************************/
        void onBasEvent(ble_bas_t* pBas, ble_bas_evt_t* pEvt);
    #endif

    #if (IAS_SUPPORT != 0)
        /********************************************************************************************************//**
         * \brief   Immedaite Alert Service event handler.
         *
         * This function will be called for all Immedaite Alert Service events, which are passed to the application.
         *
         * \param   pIas    Immedaite Alert Service stucture.
         * \param   pEvt    Event received from the Immedaite Alert Service.
         ************************************************************************************************************/
        void onIasEvent(ble_ias_t* pIas, ble_ias_evt_t* pEvt);
    #endif    // Immediate Alert Service
};  // namespace ble

#if defined(__cplusplus)
}
#endif


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
    class Services : public ISubject<BleServicesObserver>
    {
    public:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* CONSTRUCTOR SECTION                                                                                      */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        Services();
        virtual ~Services();

        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PUBLIC DECLARATION SECTION                                                                               */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        /********************************************************************************************************//**
         * \brief   Initialization of the BLE Adopted Services in use in the current peripheral.
         *
         * This method calls the initialization's methods of the BLE adopted services needed. Thoses methods are
         * provided by Nordic in the libraries...
         * The needed services are defined in `ble_services-config.h`
         ************************************************************************************************************/
        void initializeAdoptedServices();

        #if (BAS_SUPPORT != 0)
        /********************************************************************************************************//**
         * \brief   Returns a pointer on the Battery Service object
         ************************************************************************************************************/
        inline ble_bas_t*   basService()    { return &_bas; }
        #endif

        #if (IAS_SUPPORT != 0)
        /********************************************************************************************************//**
         * \brief   Returns a pointer on the Immediate Alert Service object
         ************************************************************************************************************/
        inline ble_ias_t*   iasService()    { return &_ias; }

        /********************************************************************************************************//**
         * \brief   Notify all registered observers that the alert level has changed.
         *
         * \param   alaertLevel The new level of alert to be notified.
         ************************************************************************************************************/
        void notifyAlertLevel(uint8_t alertLevel);
        #endif


    private:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PRIVATE DECLARATION SECTION                                                                              */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        #if (BAS_SUPPORT != 0)
        /********************************************************************************************************//**
        * \brief    Initialize Battery Service.
         ************************************************************************************************************/
        void            _basInit();
        ble_bas_t       _bas;
        #endif

        #if (DIS_SUPPORT != 0)
        /********************************************************************************************************//**
         * \brief   Initialize Device Information Service.
         ************************************************************************************************************/
        void            _disInit();
        ble_dis_init_t  _dis;
        #endif

        #if (IAS_SUPPORT != 0)
        /********************************************************************************************************//**
         * \brief   Initialize Device Information Service.
         ************************************************************************************************************/
        void            _iasInit();
        ble_ias_t       _ias;
        #endif
    };  // class Services

}   // namespace ble

#endif  // #if defined(SOFTDEVICE_PRESENT)

/** \} */   // Group: Private Services

/** \} */   // Group: BLE
