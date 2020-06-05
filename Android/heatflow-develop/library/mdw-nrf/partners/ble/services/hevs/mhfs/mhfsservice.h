/****************************************************************************************************************//**
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 *
 * \file    mhfsservice.h
 *
 * \addtogroup BLE
 * \{
 *
 * \addtogroup prSrv
 * \{
 *
 * \addtogroup mhfs
 * \{
 *
 * \class   ble::MhfsService
 * \brief   Definition file for the mhfsservice.cpp
 *
 * Initial author: Patrice Rudaz
 * Creation date: 2019-01-31
 *
 * \author  Patrice Rudaz (patrice.rudaz@hevs.ch)
 * \date    January 2019
 ********************************************************************************************************************/
#pragma once

/* **************************************************************************************************************** */
/*                                                                                                                  */
/* Header files                                                                                                     */
/*                                                                                                                  */
/* **************************************************************************************************************** */
#include <ble-private-service-config.h>
#include <platform-config.h>

#if defined(SOFTDEVICE_PRESENT) && (MHFS_SUPPORT != 0)
#include <ble.h>
#include <ble_srv_common.h>
#include <nordic_common.h>

#include "mhfsdefinition.h"
#include "mhfsobserver.h"

#include "ble/blecontroller.h"
#include "ble/security.h"
#include "ble/services/privateservice.h"
#include "ble/services/gattcharacteristic.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>


/* **************************************************************************************************************** */
/*                                                                                                                  */
/* TYPEDEF DECLARATION                                                                                              */
/*                                                                                                                  */
/* **************************************************************************************************************** */


/* **************************************************************************************************************** */
/*                                                                                                                  */
/* Namespace Declaration                                                                                            */
/*                                                                                                                  */
/* **************************************************************************************************************** */
namespace ble 
{

    /* ************************************************************************************************************ */
    /*                                                                                                              */
    /* Class Declaration                                                                                            */
    /*                                                                                                              */
    /* ************************************************************************************************************ */
    class MhfsService : public PrivateService, public ISubject<MhfsObserver>
    {
    public:
        /* ******************************************************************************************************** */
        /* PUBLIC SECTION                                                                                           */
        /* ******************************************************************************************************** */
        MhfsService();
        virtual ~MhfsService();

        inline uint16_t connHandle() const                                  { return _connHandle; }
        inline uint16_t serviceHandle() const                               { return _serviceHandle; }

        inline ble_gatts_char_handles_t* adsValuesCharHandle()              { return _adsValuesChar.handle(); }
        inline ble_gatts_char_handles_t* adsConfigurationCharHandle()       { return _adsConfigurationChar.handle(); }
        inline ble_gatts_char_handles_t* commandCharHandle()                { return _commandChar.handle(); }
        
        /********************************************************************************************************//**
         * \brief   Update the value of the characteristic.
         *
         * Update the value of the characteristic and send the new value over BLE if the notification is active.
         *
         * \return  A pointer to the structure containing the value of the characteristic
         ************************************************************************************************************/
        uint32_t adsValuesUpdated(uint8_t adsId, uint8_t mux, uint8_t pga, bool isFloat, uint32_t value);
        
        /********************************************************************************************************//**
         * \brief   Update the value of the characteristic.
         *
         * Update the value of the characteristic and send the new value over BLE if the notification is active.
         *
         * \return  A pointer to the structure containing the value of the characteristic
         ************************************************************************************************************/
        uint32_t adsConfigurationUpdated(uint8_t adsId, uint16_t config);

        /********************************************************************************************************//**
         * \brief   Retrieves the new value for the characteristic
         *
         * \return  A pointer to the structure containing the value of the characteristic
         ************************************************************************************************************/
        mhfsAdsConfiguration_t* adsConfigurationCharValue();

        /********************************************************************************************************//**
         * \brief   Retrieves the new value for the characteristic
         *
         * \return  A pointer to the structure containing the value of the characteristic
         ************************************************************************************************************/
        mhfsAdsRecord_t* adsValuesCharValue();

        /********************************************************************************************************//**
         * \brief   Retrieves the new value for the characteristic
         *
         * \return  A pointer to the structure containing the value of the characteristic
         ************************************************************************************************************/
        mhfsCommand_t* commandCharValue();

        /********************************************************************************************************//**
         * \brief   Initialize the Soprod Button Service.
         ************************************************************************************************************/
        void bind(BleController* bleCtrl);

        /********************************************************************************************************//**
         * \brief   PrivateServices interface implementation
         ************************************************************************************************************/
        virtual void updateSecurityForCharacteristics(Security::eSecurityLevelId securityLevel);

        /********************************************************************************************************//**
         * \name    BleObserverInterface
         * \brief   BleObserver interface implementation
         * \{
         ************************************************************************************************************/
        virtual void onBleEvent(ble_evt_t const* pBleEvent, void* pContext);
        virtual void onSystemEvent(uint32_t systemEvent);
        /** \} */

        /********************************************************************************************************//**
         * \brief   addObserver ISubject interface implementation
         ************************************************************************************************************/
        virtual bool addObserver(MhfsObserver* observer);


    protected:
        /* ******************************************************************************************************** */
        /* PROTECTED SECTION                                                                                        */
        /* ******************************************************************************************************** */
        /********************************************************************************************************//**
         * \brief   Event handler for the BLE_GATTS_EVT_WRITE event.
         *
         * \param   pEvtWrite    Bluetooth stack write on characteristic's event
         ************************************************************************************************************/
        void onRead(ble_gatts_evt_read_t const* pEvtRead);

        /********************************************************************************************************//**
         * \brief   Event handler for the BLE_GATTS_EVT_WRITE event.
         *
         * \param   pEvtWrite    Bluetooth stack write on characteristic's event
         ************************************************************************************************************/
        void onWrite(ble_gatts_evt_write_t const* pEvtWrite);
        

    private:
        /* ******************************************************************************************************** */
        /*  PRIVATE SECTION                                                                                         */
        /* ******************************************************************************************************** */
        uint16_t                            _connHandle;
        uint16_t                            _serviceHandle;

        mhfsAdsConfiguration_t              _adsConfigurationCharValue;
        mhfsCommand_t                       _commandCharValue;

        GattCharacteristic                  _adsConfigurationChar;
        GattCharacteristic                  _adsValuesChar;
        GattCharacteristic                  _commandChar;

        /************************************************************************************************************
         * \brief   Initializes the BLE service
         ************************************************************************************************************/
        virtual uint32_t                    _initService();

        /************************************************************************************************************
         * \brief   Add the ADS Configuration characteristic to the Monitor HeatFlow Service.
         *
         * \return  NRF_SUCCESS on success, otherwise an error code.
         ************************************************************************************************************/
        uint32_t _addAdsConfigurationChar(GattCharacteristic* bleChar);

        /************************************************************************************************************
         * \brief   Add the ads value characteristic to the Monitor HeatFlow Service.
         *
         * \return  NRF_SUCCESS on success, otherwise an error code.
         ************************************************************************************************************/
        uint32_t _addAdsValuesChar(GattCharacteristic* bleChar);

        /************************************************************************************************************
         * \brief   Add the Command characteristic to the Monitor HeatFlow Service.
         *
         * \return  NRF_SUCCESS on success, otherwise an error code.
         ************************************************************************************************************/
        uint32_t _addCommandChar(GattCharacteristic* bleChar);

        void _notifyAdsConfigurationChange(mhfsAdsConfiguration_t* adsConfiguration);
        void _notifyCommandChange(mhfsCommand_t* command);
    };  // class MhfsService

}   // namespace ble

#endif  // if defined(SOFTDEVICE_PRESENT) && (SBS_SUPPORT != 0)

/** \} */

/** \} */   // Group: Private Services

/** \} */   // Group: BLE
