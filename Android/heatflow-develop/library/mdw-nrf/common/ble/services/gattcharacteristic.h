/****************************************************************************************************************//**
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 *
 * \file    gattcharacteristic.h
 *
 * \addtogroup BLE
 * \{
 *
 * \class   ble::GattCharacteristic
 * \brief   Class handling the definition of any private BLE services.
 *
 * This file defines a new GATT Characteristic, server side, of a BLE private service
 *
 * Initial author: Patrice Rudaz
 * Creation date: 2017-01-15
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

#include <stdio.h>
#include "ble/security.h"

#include <ble_gatts.h>
#include <ble_gatt.h>
#include <nordic_common.h>


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
    class GattCharacteristic
    {
    public:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* CONSTRUCTOR SECTION                                                                                      */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        explicit GattCharacteristic();
        virtual ~GattCharacteristic();

        /* ******************************************************************************************************** */
        /*																										    */
        /* PUBLIC DECLARATION SECTION                                                                               */
        /*																										    */
        /* ******************************************************************************************************** */
        inline ble_gatts_char_handles_t*    handle()                        { return &_charHandle; }
        inline uint16_t                     uuid() const                    { return  _bleUUID.uuid; }
        inline uint8_t                      charLen() const                 { return MIN(_attrCharValue.max_len, _lastWrittenLen); }
        inline uint8_t*                     charVal() const                 { return  _attrCharValue.p_value; }
        inline bool                         isNotifyingOrIndicating() const { return  _isNotificationIndicationEnable; }

        
        /********************************************************************************************************//**
         * \brief   Add a characteristic to a service defined by its `service_handler` identifier.
         *
         * \param   serviceHandler              Identifier of the service
         *
         * \return \ref NRF_SUCCESS             Successfully added an include declaration.
         * \return \ref NRF_ERROR_INVALID_ADDR  Invalid pointer supplied.
         * \return \ref NRF_ERROR_INVALID_PARAM Invalid parameter(s) supplied, handle values need to match previously added services.
         * \return \ref NRF_ERROR_INVALID_STATE Invalid state to perform operation.
         * \return \ref NRF_ERROR_FORBIDDEN     Forbidden value supplied, self inclusions are not allowed.
         * \return \ref NRF_ERROR_NO_MEM        Not enough memory to complete operation.
         * \return \ref NRF_ERROR_NOT_FOUND     Attribute not found.
         ************************************************************************************************************/
        uint32_t addCharacteristic(uint16_t serviceHandler);

        /********************************************************************************************************//**
         * \brief   Specify the descriptor of a BLE Service's characteristic
         *
         * - charUUID           UUID of the characteristic to add to the service
         * - gattsCharHandles   Pointer to the handler of the characteristic
         * - charDescLen        Size of the user's description of the characteristic
         *  -charDesc           Pointer to the user's description of the characteristic
         ************************************************************************************************************/
        void setDescriptor(uint16_t charUUID, uint8_t uuidType, uint8_t* description, size_t charDescLen);

        /********************************************************************************************************//**
         * \brief   Specify the BLE Service's characteristic itself
         *
         * - gattsCharHandle    Pointer to the handle's structure of the characteristic
         * - charLen            Buffer's size of the characteristic's value
         * - charLenVariable    `true` means that the charactersitic's value length can be variable from 1 to `char_len`
         * - charValue          Pointer to the buffer of the characteristic's value
         ************************************************************************************************************/
        void setDefinition(uint8_t len, bool lenVariable, uint8_t* value);

        /********************************************************************************************************//**
         * \brief   Specify the properties of a BLE Service's characteristic.
         *
         * Specify the properties of a BLE Service's characteristic. Those properties can be:
         *  - propsRead     Read property:       1 means the characteristic's value is readable and 0 not readable.
         *  - propsNotify   Notify property:     1 means the characteristic's value can be notified and 0 can't be.
         *  - propsIndicate Indicate property:   1 means the characteristic's value can be indicated and 0 can't be.
         *  - propsWrite    Write property:      1 for write capability and 0 to prevent write access
         ************************************************************************************************************/
        void setProperties(bool writeWithoutResponse, bool read, bool notify, bool indicate, bool write, uint8_t inSdStack);

        /********************************************************************************************************//**
         * \brief   Specify the security of a BLE Service's characteristic
         *
         * For each charactersitic, you can specify how the security will be set on the read access, write access 
         * and, even, the write access on its CCCD, according to the Security level. The security level can be set
         * to none, just works bonding or bonding using Man in the Middle use case. 
         *
         * \param readAuth      Enable or disable the security for the read access on the characteristic
         * \param writeAuth     Enable or disable the security for the write access on the characteristic
         * \param cccdWriteAuth Enable or disable the security for the write access on the CCCD of the characteristic
         ************************************************************************************************************/
         void setSecurity(bool readAuth, bool writeAuth, bool cccdWriteAuth, Security::eSecurityLevelId securityLevel);
         
        /********************************************************************************************************//**
         * \brief   Update the right on the read, write and CCCD write access of the characteristic.
         *
         * According to the security level desired by the application, the method updates the rights on the read acces, 
         * write access and CCCD write access.
         *
         * \param securityLevel Can be set to  `noSecurityLevelId`, `justWorksLevel_id`,  `mitmLevelId`. 
         ************************************************************************************************************/
         void updateSecurityLevel(Security::eSecurityLevelId securityLevel);

         /********************************************************************************************************//**
         * \brief   Enable or disable notification/indication
         *
         * \param   enable  The new setting. `true` enables the notification or indication and `false` switches it off.
         ************************************************************************************************************/
        void enableNotificationIndication(uint16_t cccdValue);

        /********************************************************************************************************//**
         * \brief   Returns the actual characteristic's value from the database.
         *
         * \param[out]  pValue  Pointer to the GATT Attribute Value
         *
         * \return  A pointer of type `ble_gatts_value_t` containing the size, offset and pointer on the characteristic's
         *          value.
         ************************************************************************************************************/
        uint32_t getValue(ble_gatts_value_t* pValue);

        /********************************************************************************************************//**
         * \brief   Update the service's database with the new values for the characteristic.
         *
         * \param   size        Size of the charactersitic's value
         * \param   pValue      Pointer to the value
         *
         * \return NRF_SUCCESS on success, otherwise an error code.
         ************************************************************************************************************/
        uint32_t setValue(const uint16_t size, uint8_t* pValue);

        /********************************************************************************************************//**
         * \brief   Notify or indicate the peer device about the new value of the characteristic.
         *
         * \param   connHandle  Reference's number of the connection handler
         * \param   size        Size of the charactersitic's value
         * \param   pValue      Pointer to the value
         *
         * \return NRF_SUCCESS on success, otherwise an error code.
         ************************************************************************************************************/
        uint32_t notifyIndicateValue(uint16_t connHandle, const uint16_t size, uint8_t* pValue);

        /********************************************************************************************************//**
         * \brief   Send the new value of the characteristic over the air.
         *
         * \param   connHandle  Reference's number of the connection handler
         * \param   hvxType     Indication or Notification, see @ref BLE_GATT_HVX_TYPES.
         * \param   size        Size of the charactersitic's value
         * \param   pValue      Pointer to the value
         *
         * \return NRF_SUCCESS on success, otherwise an error code.
         ************************************************************************************************************/
        uint32_t sendValue(uint16_t connHandle, uint8_t hvxType, const uint16_t size, uint8_t* pValue);

    private:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PRIVATE DECLARATION SECTION                                                                              */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        // Attributes
        ble_gatts_char_handles_t    _charHandle;
        ble_gatts_char_md_t         _charMetaData;
        ble_gatts_attr_md_t         _attrMetaData;
        ble_gatts_attr_md_t         _cccdMetaData;
        ble_gatts_attr_t            _attrCharValue;
        ble_uuid_t                  _bleUUID;

        bool                        _isNotificationIndicationEnable;
        uint8_t                     _hvxType;
        uint8_t                     _lastWrittenLen;
    };

}    // namespace ble

#endif    // #if defined(SOFTDEVICE_PRESENT)

/** \} */
