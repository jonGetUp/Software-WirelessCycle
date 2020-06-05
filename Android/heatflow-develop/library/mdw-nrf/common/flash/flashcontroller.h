/****************************************************************************************************************//**
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Cyril Praz, Patrice Rudaz
 * All rights reserved.
 *
 * \file    flashcontroller.h
 *
 * \defgroup FLASH
 * \{
 *
 * \class   FlashController
 * \brief   Class to manage the flash
 *
 * Here is the class to handle everything about the flash
 *
 * Initial author: Cyril Praz
 * Creation date: 2017-05-10
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
#include <platform-config.h>

#if (USE_FLASH != 0)
#include <flash-config.h>
#include <flashfileid.h>
#include <isubject.hpp>
#include <xf-config.h>

#include "interface/persister.h"
#include "interface/flashobserver.h"

#include "fds.h"

#if defined(__cplusplus)
extern "C" 
{
#endif

    namespace flash 
    {
        /********************************************************************************************************//**
         * \brief   Callback function handling the Events from FlashStorage
         *
         * This function will be called for FlashStorage events which are passed to the application.
         *
         * \param   pFdsEvent     Flash data storage event.
         ************************************************************************************************************/
        void storageEventHandler(fds_evt_t const * pFdsEvent);

    } // namespace flash

#if defined(__cplusplus)
}
#endif


/* **************************************************************************************************************** */
/*                                                                                                                  */
/* NAMESPACE Declaration                                                                                            */
/*                                                                                                                  */
/* **************************************************************************************************************** */
namespace flash 
{
    
    /************************************************************************************************************//**
     * \brief    Enumeration used to have a unique identifier for every state in the state machine.
     ****************************************************************************************************************/
    typedef enum
    {
        ST_FLASH_UNKNOWN = 0,
        ST_FLASH_INIT,
        ST_FLASH_IDLE,
        ST_FLASH_WORKING,                   ///< currently writing or reading into the flash (not safe to shutdown now)
        ST_FLASH_FULL,                      ///< The FLASH is full, you should run the garbage collector (fds::gc());
        ST_FLASH_NO_SPACE_IN_QUEUE,         ///< The queue is currently full, just wait...
        ST_FLASH_ERROR
    } eFlashCtrlState;

    
    /* ************************************************************************************************************ */
    /*                                                                                                              */
    /* Class Declaration                                                                                            */
    /*                                                                                                              */
    /* ************************************************************************************************************ */
    class FlashController :  public ISubject<FlashObserver>
    {
        friend void storageEventHandler(fds_evt_t const * pFdsEvent);

    public:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* CONSTRUCTOR SECTION                                                                                      */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        FlashController();
        virtual ~FlashController();

        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PUBLIC DECLARATION SECTION                                                                               */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        /********************************************************************************************************//**
         * \brief   Singleton pattern: retrieves an instance of the class.
         ************************************************************************************************************/
        inline static FlashController* getInstance()    { return _instance; }

        /********************************************************************************************************//**
         * \brief   Get FlashController status
         ************************************************************************************************************/
        inline eFlashCtrlState getStatus()              { return _currentState; }

        /********************************************************************************************************//**
         * \brief   Initializes the components needed by this class.
         ************************************************************************************************************/
        void initialize();

        /********************************************************************************************************//**
         * \brief   Enable Flash Controller.
         ************************************************************************************************************/
        void enable();

        /********************************************************************************************************//**
         * \brief   Register a new Persister in the array of persisters.
         ************************************************************************************************************/
        bool addPersister(Persister* persister, FlashFileId::eFlashFileId fileId);

        /********************************************************************************************************//**
         * \brief   Remove the given Persister form the array of persisters.
         ************************************************************************************************************/
        bool removePersister(Persister* persister);

        /********************************************************************************************************//**
         * \brief   Save the given persister.
         ************************************************************************************************************/
        bool savePersister(Persister* persister);

        /********************************************************************************************************//**
         * \brief   Save all.
         ************************************************************************************************************/
        bool saveAll();

        /********************************************************************************************************//**
         * \brief   Update persister.
         ************************************************************************************************************/
        bool restorePersister(Persister* persister);

        /********************************************************************************************************//**
         * \brief   Update all.
         ************************************************************************************************************/
        bool restoreAll();

        /********************************************************************************************************//**
         * \brief   Clear persister.
         ************************************************************************************************************/
        bool clearPersister(Persister* persister);

        /********************************************************************************************************//**
         * \brief   Update all.
         ************************************************************************************************************/
        bool clearAll();
        
        /********************************************************************************************************//**
         * \brief   Flash Storage event handler.
         ************************************************************************************************************/
        void onFlashStorageEvent(fds_evt_t const * pFdsEvent);

    private:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PRIVATE DECLARATION SECTION                                                                              */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        // Attributes
        eFlashCtrlState                     _currentState;
        Persister*                          _persisters[FLASH_MAX_PERSISTER];
        uint8_t                             _nbPersisterRegistered;
        static  uint16_t                    _nextRecordId;
        static  FlashController*            _instance;

        // Methods
        inline bool                         _operationInProgress(Persister* persister);
        bool                                _savePersister(Persister* persister);
        void                                _processTransaction();
        
        // fds callback methods
        void                                _onInit(fds_evt_t const* pFdsEvent);
        void                                _onWrite(fds_evt_t const* pFdsEvent);
        void                                _onUpdate(fds_evt_t const* pFdsEvent);
        void                                _onRecordDeleted(fds_evt_t const* pFdsEvent);
        void                                _onFileDeleted(fds_evt_t const* pFdsEvent);
        void                                _onGarbageCollectorEvent(fds_evt_t const* pFdsEvent);

        void                                _onPersisterChanged(fds_evt_t const* pFdsEvent);
        void                                _checkPersisterSaved(fds_evt_t const* pFdsEvent);

        void                                _notifyWriteCompleted();
        void                                _notifyRestoreCompleted();
        void                                _notifyClearCompleted();
        void                                _notifyFlashInitialized();
        void                                _notifyFlashError();
    };
    
} // namespace flash
#endif // #if (USE_FLASH != 0)

/** \} */   // Group: FLASH
