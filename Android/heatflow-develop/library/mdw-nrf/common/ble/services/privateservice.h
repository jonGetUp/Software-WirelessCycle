/****************************************************************************************************************//**
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 *
 * \file    privateservice.h
 *
 * \addtogroup BLE
 * \{
 *
 * \defgroup prSrv Private Services
 * \{
 *
 * \class   ble::PrivateService
 * \brief   Class handling the definition of any private BLE services.
 *
 * This file is a mother class used to define and handle any private service. Each BLE private service MUST be an
 * instance of this class.
 *
 * Initial author: Patrice Rudaz
 * Creation date: 2016-08-24
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
#include <ble-private-service-config.h>
#include <ble_gatts.h>
#include <ble_gatt.h>

#include "core/isubject.hpp"

#include "ble/interface/bleObserver.h"
#include "ble/interface/blePrivateServiceObserver.h"
#include "ble/security.h"


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
    class PrivateService : public BleObserver
    {
    public:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* CONSTRUCTOR SECTION                                                                                      */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        PrivateService();
        virtual ~PrivateService();

        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PUBLIC DECLARATION SECTION                                                                               */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        /********************************************************************************************************//**
         * \brief   Initialize the Soprod SmartWatch Service.
         *
         * \param   uuidType        Reference to the UUID type of the private service, related to its owner (vendor 
         *                          specific).
         ************************************************************************************************************/
        void initializeService(uint8_t uuidType, Security::eSecurityLevelId securityLevel);

        /********************************************************************************************************//**
         * \brief   Returns the UUID of the BLE service.
         *
         * Returns the UUID of the BLE service. As it is a private service, this 16bits value is part of a 128bits 
         * UUID. This base 128bits UUID belongs to a specific Vendor (company)
         ************************************************************************************************************/
        inline uint16_t uuid() const                { return _serviceUuid.uuid; }

        /********************************************************************************************************//**
         * \brief   Returns the type of the UUID of the BLE service (private or adopted means 16 or 128bits)
         ************************************************************************************************************/
        inline uint8_t uuidType() const             { return _serviceUuid.type; }

        /********************************************************************************************************//**
         * \brief   Returns the index of the registered service in SD to advertise its UUID.
         ************************************************************************************************************/
        inline uint8_t advertiseUUID() const        { return _advertiseUUID; }

        /********************************************************************************************************//**
         * \brief   Change the security level according to the desired use case...
         *
         * \param   securityLevel   The new security level to reach ! \ref ble::Security::eSecurityLevelId
         ************************************************************************************************************/
        virtual void updateSecurityForCharacteristics(Security::eSecurityLevelId securityLevel) = 0;

    protected:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PROTECTED DECLARATION SECTION                                                                            */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        /********************************************************************************************************//**
         * \brief   This Method handles the authorization for ever Read and Write request of the service.
         ************************************************************************************************************/
        uint8_t handleReadWriteAuthorizeRequest(ble_evt_t const*                       pBleEvent,
                                                ble_gatts_rw_authorize_reply_params_t* pAuthReply);

        /********************************************************************************************************//**
         * \name    BleObserver
         * \brief   BLE Observer interface implementation
         * \{
         ************************************************************************************************************/
        virtual void onBleEvent(ble_evt_t const* pBleEvent, void* pContext) = 0;
        virtual void onSystemEvent(uint32_t systemEvent) = 0;
        /** \} */

        /********************************************************************************************************//**
         * \brief   Abstract method to initalize the service
         ************************************************************************************************************/
        virtual uint32_t _initService() = 0;

        Security::eSecurityLevelId _securityLevel;

        // Keeps the Vendor ID and the UUID of the BLE private service.
        ble_uuid_t  _serviceUuid;
        uint8_t     _advertiseUUID;

    private:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PRIVATE DECLARATION SECTION                                                                              */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        /************************************************************************************************************
         * \brief   Set up the GATT status for the reply to authentication read or write request
         *
         * \param   type        Type of the request: READ or WRITE.
         * \param   reply       Reference to the GATT's reply parameter.
         ************************************************************************************************************/
        void _setAuthorisationReply(uint8_t type, uint16_t gattStatus, ble_gatts_rw_authorize_reply_params_t* reply);
    };

}   // namespace ble

#endif    // #if defined(SOFTDEVICE_PRESENT)

/** \} */   // Group: Private Services

/** \} */   // Group: BLE
