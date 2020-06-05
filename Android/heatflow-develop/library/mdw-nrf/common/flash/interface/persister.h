/****************************************************************************************************************//**
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Cyril Praz, Patrice Rudaz
 * All rights reserved.
 *
 * \file    persister.h
 *
 * \addtogroup FLASH
 * \{
 *
 * \class   Persister
 * \brief   This class is an abstract class defining a Persister Object which will be used to store information in
 *          the FALSH memory.
 *
 * Initial author: Cyril Praz
 * Creation date: 2016-12-27
 *
 * \author  Patrice Rudaz (patrice.rudaz@hevs.ch)
 * \date    April 2018
 ********************************************************************************************************************/
#pragma once


/* **************************************************************************************************************** */
/*                                                                                                                  */
/* Header files                                                                                                     */
/*                                                                                                                  */
/* **************************************************************************************************************** */
#include "fds.h"
#include <flashfileid.h>


/* **************************************************************************************************************** */
/*                                                                                                                  */
/* NAMESPACE defintion                                                                                              */
/*                                                                                                                  */
/* **************************************************************************************************************** */
namespace flash 
{

    /************************************************************************************************************//**
     * \brief   Enumeration used to have a unique identifier for every Persister state
     ****************************************************************************************************************/
    typedef enum
    {
        ST_PERSISTER_UNKNOW = 0,
        ST_PERSISTER_SAVING_REQUEST,
        ST_PERSISTER_SAVING,
        ST_PERSISTER_SAVED,
        ST_PERSISTER_RESTORING,
        ST_PERSISTER_RESTORED,
        ST_PERSISTER_UPDATING,
        ST_PERSISTER_UPDATED,
        ST_PERSISTER_CLEARED
    } ePersisterState;


    /* ************************************************************************************************************ */
    /*                                                                                                              */
    /* Class Declaration                                                                                            */
    /*                                                                                                              */
    /* ************************************************************************************************************ */
    class Persister
    {

    public:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* CONSTRUCTOR DECLARATION SECTION                                                                          */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        inline Persister() :  _state(ST_PERSISTER_UNKNOW)    {}
        virtual ~Persister()    {}

        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PUBLIC DECLARATION SECTION                                                                               */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        /********************************************************************************************************//**
         * \name    FlashAbstractMethods
         * \brief   Abstract methods of the FLASH obect...
         * \{
         ************************************************************************************************************/
        /********************************************************************************************************//**
         * \brief   Save
         ************************************************************************************************************/
        virtual uint32_t* save(uint16_t* size) = 0;

        /********************************************************************************************************//**
         * \brief   Restore
         ************************************************************************************************************/
        virtual void restore(uint32_t* data) = 0;
        /** \} */

        /********************************************************************************************************//**
         * \name    FlashSetterMethods
         * \brief   Setters
         * \{
         ************************************************************************************************************/
        inline void setStatus(ePersisterState state)    { _state = state; }

        inline void setFileIdAndKey(uint16_t fileId, uint16_t key)  
        {
            _record.file_id             = fileId;
            _record.key                 = key;
        }
        
        inline void setDataAndSize(void* pData, uint16_t size)
        {
            _record.data.p_data         = pData;
            _record.data.length_words   = size;
        }
        /** \} */
        
        /********************************************************************************************************//**
         * \name    FlashGetterMethods
         * \brief   Getters
         * \{
         ************************************************************************************************************/
        inline fds_record_t* getRecord()                { return &_record; }
        inline fds_record_desc_t* getRecordDescriptor() { return &_recordDescriptor; }
        inline uint16_t getFileId() const               { return _record.file_id; }    
        inline uint16_t getKey() const                  { return _record.key; }    
        inline uint32_t getLength() const               { return _record.data.length_words; }    
        inline ePersisterState getStatus() const        { return _state; }
        /** \} */

    private:
        fds_record_t        _record;
        fds_record_desc_t   _recordDescriptor;
        ePersisterState     _state;
    };

}  // namespace persister

/** \} */   // Group: FLASH
