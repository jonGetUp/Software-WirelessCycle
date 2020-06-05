/************************************************************************************************************
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Cyril Praz, Patrice Rudaz
 * All rights reserved.
 ************************************************************************************************************/

/************************************************************************************************************/
/* Header files                                                                                             */
/************************************************************************************************************/
#include "flashcontroller.h"

#if (USE_FLASH != 0)

#include <string.h>


/************************************************************************************************************/
/* DEBUG COMPILATION OPTION & MACRO                                                                         */
/************************************************************************************************************/
#define DEBUG_FLASH_ENABLE                  0
#if (DEBUG_FLASH_ENABLE != 0) && defined(DEBUG_NRF_USER)
    #include <tools/logtrace.h>
    #define FLASH_DEBUG                      LOG_TRACE_DEBUG
#else
    #define FLASH_DEBUG(...)                {(void) (0);}
#endif

/************************************************************************************************************/
/* Namespace's Declaration                                                                                  */
/************************************************************************************************************/
using flash::FlashController;

/************************************************************************************************************/
/* Declaration section                                                                                      */
/************************************************************************************************************/
FlashController*                            FlashController::_instance      = NULL;
uint16_t                                    FlashController::_nextRecordId  = 1;

    
/************************************************************************************************************/
/* --- Extern "C" Section ---                                                                               */
/*     Callback's section                                                                                   */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
void flash::storageEventHandler(fds_evt_t const* pFdsEvent)
{
    FLASH_DEBUG("[FlashController] storageEventHandler() > storageEventHandler() > ... \r\n");
    FlashController* fc = FlashController::getInstance();
    if(fc != NULL)
    {
        fc->onFlashStorageEvent(pFdsEvent);
    }
}


/************************************************************************************************************/
/* CONSTRUCTOR|DESTRUCTOR                                                                                   */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
FlashController::FlashController()
{
    _currentState = ST_FLASH_UNKNOWN;
    _instance = this;
}

// -----------------------------------------------------------------------------------------------------------
FlashController::~FlashController() {}

    
/************************************************************************************************************/
/* PUBLIC METHODS                                                                                           */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
void FlashController::initialize()
{
    _currentState = ST_FLASH_INIT;
    _nbPersisterRegistered = 0;
}

//------------------------------------------------------------------------------------------------------------
void FlashController::enable()
{
    if(_currentState == ST_FLASH_INIT)
    {
        FLASH_DEBUG("[FlashController] enable() > ... \r\n");
        ret_code_t ret  = fds_register(flash::storageEventHandler);                                                     //register the callback
        if(ret != FDS_SUCCESS)
        {
            FLASH_DEBUG("[FlashController] enable() > fds_register() failed! (Error: 0x%04x)\r\n", ret);                // Registering of the event handler has failed.
            _notifyFlashError();                                                                                        // Notify the error and leaves...
            return;
        }

        fds_stat_t stat = { 0 };
        ret = fds_stat(&stat);
        if(ret == FDS_SUCCESS)
        {
            FLASH_DEBUG("[FlashController] enable() > fds_stat(): open: %d, valid: %d, dirty: %d, reserved: %d, used: %d, largest: %d, freeable: %d  \r\n", stat.open_records, stat.valid_records, stat.dirty_records, stat.words_reserved, stat.words_used, stat.largest_contig, stat.freeable_words);
            _currentState = ST_FLASH_IDLE;
            _notifyFlashInitialized();
        }
        else if(ret == FDS_ERR_NOT_INITIALIZED)
        {
            _processTransaction();
        }
        else
        {
            FLASH_DEBUG("[FlashController] enable() > fds_stat() failed! (Error 0x%04x.)\r\n", ret);
            _notifyFlashError();
        }
    }
}

//------------------------------------------------------------------------------------------------------------
bool FlashController::addPersister(Persister* persister, FlashFileId::eFlashFileId fileId)
{
    uint8_t i;
    for(i = 0; i < FLASH_MAX_PERSISTER; i++)
    {
        if(_persisters[i] == persister)
        {
            FLASH_DEBUG("[FlashController] addPersister() > Persister already in ! (file id: %2d; key: %2d; position %2d; %s)\r\n", persister->getFileId(), persister->getKey(), i, FlashFileId::toString(FlashFileId::fromInt(persister->getFileId())));
            return true;
        }
        
        if(_persisters[i] == NULL)
        {
            persister->setFileIdAndKey(fileId, _nextRecordId++);                                                        // configure the persister
            _persisters[i] = persister;                                                                                 // register the persister
            _nbPersisterRegistered++;
            FLASH_DEBUG("[FlashController] addPersister() > register the Persister (file id: %2d; key: %2d; position %2d; %s)\r\n", persister->getFileId(), persister->getKey(), i, FlashFileId::toString(FlashFileId::fromInt(persister->getFileId())));
            return true;
        }
    }
    FLASH_DEBUG("[FlashController] addPersister() > no more place to register this persister #%d (FLASH_MAX_PERSISTER: %d)\r\n", i, FLASH_MAX_PERSISTER);
    _notifyFlashError();
    return false;
}

//------------------------------------------------------------------------------------------------------------
bool FlashController::removePersister(Persister* persister)
{
    Persister* pToMove = NULL;
    for(uint8_t i = FLASH_MAX_PERSISTER; i > 0; i--)
    {
        if(_persisters[i - 1] != NULL)
        {
            if(_persisters[i - 1] == persister)
            {
                _persisters[i - 1] = pToMove;
                _nbPersisterRegistered--;
                FLASH_DEBUG("[FlashController] removePersiter() > persister successfully deleted \r\n");
                return true;
            }
            else if(pToMove == NULL)
            {
                pToMove = _persisters[i - 1];
                _persisters[i - 1] = NULL;
            }
        }
    }
    FLASH_DEBUG("[FlashController] removePersiter() > persister doesn't exist into the queue !\r\n");
    _notifyFlashError();
    return false;
}

//------------------------------------------------------------------------------------------------------------
bool FlashController::savePersister(Persister* persister)
{
    if(persister == NULL)
    {
        FLASH_DEBUG("[FlashController] savePersister() > persister is NULL !\r\n");
        return false;
    }

    if(!_savePersister(persister)) 
    {
        return false;
    }
    _processTransaction();
    return true;
}

//------------------------------------------------------------------------------------------------------------
bool FlashController::saveAll()
{
    for(uint8_t i = 0; i < _nbPersisterRegistered; i++)
    {
        if(!_savePersister(_persisters[i]))
        {
            return false;
        }
    }
    _processTransaction();
    return true;
}

//------------------------------------------------------------------------------------------------------------
bool FlashController::restorePersister(Persister* persister)
{
    if(persister == NULL)
    {
        FLASH_DEBUG("[FlashController] restorePersister() > persister is NULL !\r\n");
        return false;
    }

    //check if module is correctly initialized
    if(_currentState == ST_FLASH_UNKNOWN || _currentState == ST_FLASH_INIT)
    {
        FLASH_DEBUG("[FlashController] restorePersister() > FlashController is not initialized!\r\n");
        _notifyFlashError();
        return false;
    }

    uint16_t fileId = persister->getFileId();
    uint16_t key    = persister->getKey();

    //check if an operation is in progress for this persister
    if(!_operationInProgress(persister))
    {
        fds_flash_record_t flashRecord;
        fds_find_token_t   ftok             = {0};
        fds_record_desc_t* recordDescriptor = persister->getRecordDescriptor();

        _currentState                       = ST_FLASH_WORKING;
        persister->setStatus(ST_PERSISTER_RESTORING);

        // find the corresponding record
        ret_code_t ret                      = fds_record_find(fileId, key, recordDescriptor, &ftok);
        if(ret == FDS_ERR_NOT_FOUND)
        {
            FLASH_DEBUG("[FlashController] restorePersister() > nothing to restore (file id: %d, key: %d)\r\n", fileId, key);
            persister->setStatus(ST_PERSISTER_RESTORED); 
            _currentState                   = ST_FLASH_IDLE;
            return true;
        }
        
        // Open the record
        else if(ret == FDS_SUCCESS)
        {
            ret                             = fds_record_open(recordDescriptor, &flashRecord);
            if(ret != FDS_SUCCESS)
            {
                FLASH_DEBUG("[FlashController] restorePersister() > fds_record_open() failed! (file id: %d; key: %d; Error: 0x%04x)\r\n", fileId, key, ret);
                _notifyFlashError();
                return false;
            }
                
            // Access the record through the flash_record structure.
            persister->restore((uint32_t*) flashRecord.p_data);
            persister->setStatus(ST_PERSISTER_RESTORED);
            
#if (DEBUG_FLASH_ENABLE != 0)
            uint16_t    size                = flashRecord.p_header->length_words;
            FLASH_DEBUG("[FlashController] restorePersister() > Persister restored (file id: %d; key: %d), size = %d, data = [ ", fileId, key, size);
            for(uint8_t i = 0; i < size * 4; i++)
            {
                FLASH_DEBUG("0x%02x ", ((uint8_t*) flashRecord.p_data)[i]);
            }
            FLASH_DEBUG("]\r\n");
#endif  // #if (DEBUG_FLASH_ENABLE != 0)
            
            // Close the record when done.
            ret                             = fds_record_close(recordDescriptor);
            if(ret != FDS_SUCCESS)
            {
                FLASH_DEBUG("[FlashController] restorePersister() > fds_record_close() failed! (file id: %d; key: %d; Error: 0x%04x\r\n", fileId, key, ret);
                _notifyFlashError();
                return false;
            }
            _currentState                   = ST_FLASH_IDLE;
            return true;
        }
        else
        {
            FLASH_DEBUG("[FlashController] restorePersister() > fds_record_open() failed! (file id: %d; key: %d; Error: 0x%04x)\r\n", fileId, key, ret);
            _notifyFlashError();
        }
    }
    else
    {
        FLASH_DEBUG("[FlashController] restorePersister() > Persister busy... (file id = %d; record key = %d)\r\n", fileId, key);
    }
    return false;
}

//------------------------------------------------------------------------------------------------------------
bool FlashController::restoreAll()
{
    for(uint8_t i = 0; i < _nbPersisterRegistered; i++)
    {
        if(!restorePersister(_persisters[i]))
        {
            return false;
        }
    }
    _notifyRestoreCompleted();
    return true;
}

//------------------------------------------------------------------------------------------------------------
bool FlashController::clearPersister(Persister* persister)
{
    if(persister == NULL)
    {
        FLASH_DEBUG("[FlashController] clearPersister() > persister is NULL !\r\n");
        return false;
    }

    //check if module is correctly initialized
    if(_currentState == ST_FLASH_UNKNOWN || _currentState == ST_FLASH_INIT)
    {
        FLASH_DEBUG("[FlashController] clearPersister() > FlashController is not initialized!\r\n");
        _notifyFlashError();
        return false;
    }

    #if (DEBUG_FLASH_ENABLE != 0) && defined(DEBUG_NRF_USER)
    uint16_t fileId = persister->getFileId();
    uint16_t key    = persister->getKey();
    #endif

    //check if an operation is in progress for this persister
    if(!_operationInProgress(persister) && persister->getStatus() != ST_PERSISTER_CLEARED)
    {
        _currentState                       = ST_FLASH_WORKING;
        fds_record_desc_t* recordDescriptor = persister->getRecordDescriptor();
        ret_code_t ret                      = fds_record_delete(recordDescriptor);
        if(ret == FDS_SUCCESS)
        {
            persister->setStatus(ST_PERSISTER_CLEARED);
            FLASH_DEBUG("[FlashController] clearPersister() > fds_record_delete() succeeded! (file id: %d; key: %d)\r\n", fileId, key);
        }
        else
        {
            FLASH_DEBUG("[FlashController] clearPersister() > fds_record_delete() FAILED! (file id: %d; key: %d; Error: 0x%04x)\r\n", fileId, key, ret);
        }
    }
    else if(persister->getStatus() == ST_PERSISTER_CLEARED)
    {
        FLASH_DEBUG("[FlashController] clearPersister() > Persister already cleared ! (file id: %d; key: %d)\r\n", fileId, key);
    }
    else
    {
        FLASH_DEBUG("[FlashController] clearPersister() > Persister busy... (file id: %d; key: %d)\r\n", fileId, key);
        return false;
    }
    return true;
}

//------------------------------------------------------------------------------------------------------------
bool FlashController::clearAll()
{
    //check if module is correctly initialized
    if(_currentState == ST_FLASH_UNKNOWN || _currentState == ST_FLASH_INIT)
    {
        FLASH_DEBUG("[FlashController] clearAll() > FlashController is not initialized!\r\n");
        _notifyFlashError();
        return false;
    }
    
    if(_currentState == ST_FLASH_WORKING)
    {
        FLASH_DEBUG("[FlashController] clearAll() > FlashController is busy!\r\n");
        return false;
    }

    if(_nbPersisterRegistered > 0)
    {
        for(uint8_t i = 0; i < _nbPersisterRegistered; i++)
        {
            static uint16_t fileId  = 0xffff;
            if(_persisters[i]->getStatus() != ST_PERSISTER_CLEARED)
            {
                uint16_t currentFileId  = _persisters[i]->getFileId();
                if(currentFileId != fileId)
                {
                    _currentState       = ST_FLASH_WORKING;
                    FLASH_DEBUG("[FlashController] clearAll() > Clearing file id: %d ...\r\n", currentFileId);
                    ret_code_t errCode  = fds_file_delete(currentFileId);
                    
                    if(errCode == FDS_SUCCESS)
                    {
                        fileId          = currentFileId;
                        _persisters[i]->setStatus(ST_PERSISTER_CLEARED);
                    }
                    else if(errCode == FDS_ERR_INVALID_ARG)
                    {
                        FLASH_DEBUG("[FlashController] clearAll() > fds_file_delete() throws FDS_ERR_INVALID_ARG errCode ! (file id: %d)\r\n", currentFileId);
                        continue;
                    }
                }
            }            
        }
    }

    _notifyClearCompleted();
    return true;
}

//------------------------------------------------------------------------------------------------------------
void FlashController::onFlashStorageEvent(fds_evt_t const * pFdsEvent)
{
    switch(pFdsEvent->id)
    {
        case FDS_EVT_INIT:
            _onInit(pFdsEvent);
            break;
        
        case FDS_EVT_WRITE:
            if(FlashFileId::isValid(pFdsEvent->write.file_id))
            {
                _onWrite(pFdsEvent);
            }
            break;
            
        case FDS_EVT_UPDATE:
            if(FlashFileId::isValid(pFdsEvent->write.file_id))
            {
                _onUpdate(pFdsEvent);
            }
            break;
            
        case FDS_EVT_DEL_RECORD:
            if(FlashFileId::isValid(pFdsEvent->del.file_id))
            {
                _onRecordDeleted(pFdsEvent);
            }
            break;
            
        case FDS_EVT_DEL_FILE:
            if(FlashFileId::isValid(pFdsEvent->del.file_id))
            {
                _onFileDeleted(pFdsEvent);
            }
            clearAll();
            return;
            
        case FDS_EVT_GC:
            _onGarbageCollectorEvent(pFdsEvent);
            break;
        
        default:
            break;
    }
    
    _processTransaction();
}

/************************************************************************************************************/
/* Private section                                                                                          */
/************************************************************************************************************/
// -----------------------------------------------------------------------------------------------------------
inline bool FlashController::_operationInProgress(Persister* persister)
{
    return (persister->getStatus() == ST_PERSISTER_RESTORING    ||
            persister->getStatus() == ST_PERSISTER_SAVING       ||
            persister->getStatus() == ST_PERSISTER_UPDATING);
}

// -----------------------------------------------------------------------------------------------------------
bool FlashController::_savePersister(Persister* persister)
{
    //check if module is correctly initialized
    if(_currentState != ST_FLASH_UNKNOWN && _currentState != ST_FLASH_INIT)
    {
        //check if an operation is in progress for this persister
        if(!_operationInProgress(persister) && persister->getStatus() != ST_PERSISTER_SAVING_REQUEST)
        {
            uint16_t size = 0;
            uint32_t* data = persister->save(&size);
            persister->setDataAndSize(data, size);
            persister->setStatus(ST_PERSISTER_SAVING_REQUEST);
            return true;
        }
        else
        {
            FLASH_DEBUG("[FlashController] savePersister() > Persister busy ... (file id: %d; key = %d)\r\n", persister->getFileId(), persister->getKey());
        }
    }
    else
    {
        FLASH_DEBUG("[FlashController] savePersister() > FlashController is not correctly initialized \r\n");
    }
    _notifyFlashError();
    return false;
}

//------------------------------------------------------------------------------------------------------------
void FlashController::_processTransaction()
{
    ret_code_t ret = FDS_SUCCESS;

    if(_currentState == ST_FLASH_IDLE || _currentState == ST_FLASH_NO_SPACE_IN_QUEUE)
    {
        for(uint8_t i = 0; i < _nbPersisterRegistered; i++)
        {
            Persister * persister = _persisters[i];
            if(persister != NULL)
            {
                if(persister->getStatus() == ST_PERSISTER_SAVING_REQUEST)
                {
                    fds_find_token_t    ftok        = {0};
                    fds_record_t*       record      = persister->getRecord();
                    fds_record_desc_t*  recordDesc  = persister->getRecordDescriptor();
                    
                    _currentState = ST_FLASH_WORKING;

                    //check if the record already exist
                    ePersisterState state = ST_PERSISTER_UNKNOW;
                    ret = fds_record_find(record->file_id, record->key, recordDesc, &ftok);
                    if(ret == FDS_SUCCESS) 
                    {
                        FLASH_DEBUG("[FlashController] _processTransaction() > Updating Persister ... (file id: %d; key: %d)\r\n", record->file_id, record->key);
                        ret = fds_record_update(recordDesc, record);
                        state = ST_PERSISTER_UPDATING;
                    }
                    // ... if not
                    else if(ret == FDS_ERR_NOT_FOUND)
                    {
                        FLASH_DEBUG("[FlashController] _processTransaction() > Saving Persister ... (file id: %d; key: %d)\r\n", record->file_id, record->key);
                        ret = fds_record_write(recordDesc, record);
                        state = ST_PERSISTER_SAVING;
                    }
                    
                    // checks the possible errors...
                    switch(ret)
                    {
                        case FDS_SUCCESS:
                            FLASH_DEBUG("[FlashController] _processTransaction() > %s request sent for Persister (file id: %d; key: %d)\r\n", (state == ST_PERSISTER_SAVING)?"Save":"Update", record->file_id, record->key);
                            persister->setStatus(state);
                            break;
                        case FDS_ERR_NO_SPACE_IN_QUEUES:
                            FLASH_DEBUG("[FlashController] _processTransaction() > Error: FDS_ERR_NO_SPACE_IN_QUEUES (file id: %d; key: %d)\r\n", record->file_id, record->key);
                            _currentState = ST_FLASH_NO_SPACE_IN_QUEUE;
                            break;
                        case FDS_ERR_NO_SPACE_IN_FLASH:
                            FLASH_DEBUG("[FlashController] _processTransaction() > Error: FDS_ERR_NO_SPACE_IN_FLASH (file id: %d; key: %d)\r\n", record->file_id, record->key);
                            _currentState = ST_FLASH_FULL;
                            break;
                        default:
                            FLASH_DEBUG("[FlashController] _processTransaction() > Error: 0x%04x (file id: %d; key: %d)\r\n", ret, record->file_id, record->key);
                            _notifyFlashError();
                            break;
                    }
                    break;
                }
            }
            else
            {
                FLASH_DEBUG("[FlashController] _processTransaction() > Persister[%d] is NULL !\r\n", i);
            }
        }
    }
    else if(_currentState == ST_FLASH_INIT)
    {
        //initialize file data storage
        FLASH_DEBUG("[FlashController] _processTransaction() > Initializing File Data Storage... \r\n");
        ret = fds_init();
        if(ret != FDS_SUCCESS)
        {
            // Handle error.
            FLASH_DEBUG("[FlashController] _processTransaction() > Can't initialize File Data Sorage library ERROR: 0x%04x\r\n", ret);
            _notifyFlashError();
        }
    }
    
    if(_currentState == ST_FLASH_FULL)
    {
        ret = fds_gc();
        if(ret == FDS_SUCCESS)
        {
            FLASH_DEBUG("[FlashController] _process() > fds_gc() on going ...\r\n");
        }
        else if(ret == FDS_ERR_NO_SPACE_IN_QUEUES)
        {
            FLASH_DEBUG("[FlashController] _processTransaction() > fds_gc() Failed! (Error: FDS_ERR_NO_SPACE_IN_QUEUES)\r\n");
        }
        else
        {
            FLASH_DEBUG("[FlashController] _processTransaction() > fds_gc() Failed! (Error:  0x%04x)\r\n", ret);
            _notifyFlashError();
        }    
    }
        
}

//------------------------------------------------------------------------------------------------------------
void FlashController::_onInit(fds_evt_t const* pFdsEvent)
{
    if(pFdsEvent->result == FDS_SUCCESS)
    {
        FLASH_DEBUG("[FlashController] _onInit() > File storage initialised \r\n");
        _currentState = ST_FLASH_IDLE;
        _notifyFlashInitialized();
    }
    else if(pFdsEvent->result == FDS_ERR_BUSY)
    {
        FLASH_DEBUG("[FlashController] _onInit() > File Storage is BUSY ! Trying to init again ...\r\n");
    }
    else
    {
        FLASH_DEBUG("[FlashController] _onInit() > Initialisation failed ! (Error: 0x%04x)\r\n", pFdsEvent->result);
        _notifyFlashError();
    }
}

//------------------------------------------------------------------------------------------------------------
void FlashController::_onWrite(fds_evt_t const* pFdsEvent)
{
    _onPersisterChanged(pFdsEvent);
}

//------------------------------------------------------------------------------------------------------------
void FlashController::_onUpdate(fds_evt_t const* pFdsEvent)
{
    _onPersisterChanged(pFdsEvent);
}

//------------------------------------------------------------------------------------------------------------
void FlashController::_onRecordDeleted(fds_evt_t const* pFdsEvent)
{
    if(pFdsEvent->result == FDS_SUCCESS)
    {
        FLASH_DEBUG("[FlashController] _onRecordDeleted() > Record deleted.\r\n");
    }
    else if(pFdsEvent->result == FDS_ERR_BUSY)
    {
        FLASH_DEBUG("[FlashController] _onRecordDeleted() > FDS_ERR_BUSY\r\n");
        return;
    }
    else
    {
        FLASH_DEBUG("[FlashController] _onRecordDeleted() > Error: 0x%04x\r\n", pFdsEvent->result);
        _notifyFlashError();
    }
    
    _currentState = ST_FLASH_IDLE;
}

//------------------------------------------------------------------------------------------------------------
void FlashController::_onFileDeleted(fds_evt_t const* pFdsEvent)
{
    if(pFdsEvent->result == FDS_SUCCESS)
    {
        FLASH_DEBUG("[FlashController] _onFileDeleted() > File deleted.\r\n");
    }
    else if(pFdsEvent->result == FDS_ERR_BUSY)
    {
        FLASH_DEBUG("[FlashController] _onFileDeleted() > FDS_ERR_BUSY\r\n");
        return;
    }
    else
    {
        FLASH_DEBUG("[FlashController] _onFileDeleted() > Error: 0x%04x\r\n", pFdsEvent->result);
        _notifyFlashError();
    }
    
    _currentState = ST_FLASH_IDLE;
}

//------------------------------------------------------------------------------------------------------------
void FlashController::_onGarbageCollectorEvent(fds_evt_t const* pFdsEvent)
{
    if(pFdsEvent->result == FDS_SUCCESS)
    {
        FLASH_DEBUG("[FlashController] _onGarbageCollectorEvent() > Garbage Colllector operation done! \r\n");
        _currentState = ST_FLASH_IDLE;
    }
    else if(pFdsEvent->result == FDS_ERR_BUSY)
    {
        FLASH_DEBUG("[FlashController] _onGarbageCollectorEvent() > FDS_ERR_BUSY\r\n");
    }
    else
    {
        FLASH_DEBUG("[FlashController] _onGarbageCollectorEvent() > Error: 0x%04x\r\n", pFdsEvent->result);
        _notifyFlashError();
    }
}

//------------------------------------------------------------------------------------------------------------
void FlashController::_onPersisterChanged(fds_evt_t const* pFdsEvent)
{
    switch(pFdsEvent->result)
    {
        case FDS_SUCCESS:
        {
            FLASH_DEBUG("[FlashController] _onWrite() > Write operation succeeded.\r\n");
            _checkPersisterSaved(pFdsEvent);
            break;
        }
        case FDS_ERR_NO_SPACE_IN_QUEUES:
        {
            FLASH_DEBUG("[FlashController] _onWrite() > FDS_ERR_NO_SPACE_IN_QUEUES.\r\n");
            _currentState = ST_FLASH_NO_SPACE_IN_QUEUE;
            break;
        }
        case FDS_ERR_NO_SPACE_IN_FLASH:
        {
            FLASH_DEBUG("[FlashController] _onWrite() > FDS_ERR_NO_SPACE_IN_FLASH.\r\n");
            _currentState = ST_FLASH_FULL;
            break;
        }
        default:
        {
            FLASH_DEBUG("[FlashController] _onWrite() > Write operation failed! (Error: 0x%04x)\r\n", pFdsEvent->result);
            _notifyFlashError();
            break;
        }
    }
}

//------------------------------------------------------------------------------------------------------------
void FlashController::_checkPersisterSaved(fds_evt_t const* pFdsEvent)
{
    for(uint8_t i = 0; i < _nbPersisterRegistered; i++)
    {
        Persister* persister = _persisters[i];
        if(persister != NULL)
        {
            if((persister->getFileId() == pFdsEvent->write.file_id) &&
               (persister->getKey()    == pFdsEvent->write.record_key)) 
            {
                persister->setStatus(ST_PERSISTER_SAVED);
            }
            else if((persister->getStatus() != ST_PERSISTER_UNKNOW)     &&
                    (persister->getStatus() != ST_PERSISTER_SAVED)      &&
                    (persister->getStatus() != ST_PERSISTER_UPDATED)    &&
                    (persister->getStatus() != ST_PERSISTER_RESTORED))   
            {
                FLASH_DEBUG("[FlashController] _checkPersisterSaved() > Not all persistor saved.\r\n");
                _currentState = ST_FLASH_IDLE;
                return;
            }
        }
    }

    FLASH_DEBUG("[FlashController] _checkPersisterSaved() > All Persistor saved. Notify Observers...\r\n");
    _notifyWriteCompleted();
    _currentState = ST_FLASH_IDLE;
}

//------------------------------------------------------------------------------------------------------------
void FlashController::_notifyWriteCompleted()
{
    for(uint8_t i = 0; i < observersCount(); i++)
    {
        if(observer(i) != NULL)
        {
            observer(i)->onDataSaved(this);
        }
    }
}

//------------------------------------------------------------------------------------------------------------
void FlashController::_notifyRestoreCompleted()
{
    for(uint8_t i = 0; i < observersCount(); i++)
    {
        if(observer(i) != NULL)
        {
            observer(i)->onDataRestored(this);
        }
    }
}

//------------------------------------------------------------------------------------------------------------
void FlashController::_notifyClearCompleted()
{
    for(uint8_t i = 0; i < observersCount(); i++)
    {
        if(observer(i) != NULL)
        {
            observer(i)->onDataCleared(this);
        }
    }
}

//------------------------------------------------------------------------------------------------------------
void FlashController::_notifyFlashInitialized()
{
    for(uint8_t i = 0; i < observersCount(); i++)
    {
        if(observer(i) != NULL)
        {
            observer(i)->onFlashInitialized(this);
        }
    }
}

//------------------------------------------------------------------------------------------------------------
void FlashController::_notifyFlashError()
{
    _currentState = ST_FLASH_ERROR;
    ASSERT(false);
    
    for(uint8_t i = 0; i < observersCount(); i++)
    {
        if(observer(i) != NULL)
        {
            observer(i)->onFlashError(this);
        }
    }
}

#endif  // #if (USE_FLASH != 0)
