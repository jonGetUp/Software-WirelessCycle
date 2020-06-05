/************************************************************************************************************
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2018
 * Created by Markus Salzmann, Patrice Rudaz & Thomas Sterren
 * All rights reserved.
 ************************************************************************************************************/

/************************************************************************************************************/
/* Header files                                                                                             */
/************************************************************************************************************/
#include <xf-config.h>

#if (XF_TM_PROTECT_ADD_TM != 0)
    #include "critical/critical.h"
#endif // XF_TM_PROTECT_ADD_TM

#include "core/xftimeoutmanager.h"
#include "core/ixfreactive.h"
#include "core/xftimeout.h"

    
/************************************************************************************************************/
/* DEBUG COMPILATION OPTION & MACRO                                                                         */
/************************************************************************************************************/
#define DEBUG_XFTIMEOUTMANAGER_ENABLE       0
#if (DEBUG_XFTIMEOUTMANAGER_ENABLE != 0) && defined(DEBUG_NRF_USER)
    #include <tools/logtrace.h>
    #define XFTM_DEBUG                      LOG_TRACE_DEBUG
#else
    #define XFTM_DEBUG(...)                 {(void) (0);}
#endif

#define DEBUG_XFTIMEOUTMANAGER_DEEP_ENABLE  0
#if (DEBUG_XFTIMEOUTMANAGER_DEEP_ENABLE != 0) && defined(DEBUG_NRF_USER)
    #include <tools/logtrace.h>
    #define XFTMD_DEBUG                     LOG_TRACE_DEBUG
#else
    #define XFTMD_DEBUG(...)                 {(void) (0);}
#endif

#define DEBUG_XFTIMEOUTMANAGER_TM_ENABLE    0
#if (DEBUG_XFTIMEOUTMANAGER_TM_ENABLE != 0) && defined(DEBUG_NRF_USER)
    #include <tools/logtrace.h>
    #include <critical/critical.h>
    #define XFTMTM_DEBUG                    LOG_TRACE_DEBUG
#else
    #define XFTMTM_DEBUG(...)               {(void) (0);}
#endif


/************************************************************************************************************/
/* Declaration section                                                                                      */
/************************************************************************************************************/
// Initialization of static class attributes
XFTimeoutManager* XFTimeoutManager::_pInstance = NULL;

#if (XF_TM_USE_ISLIST != 0) && (XF_TM_USE_ISLIST_MEMPOOL != 0)
    #if !defined(XF_TM_USE_ISLIST_MEMPOOL_ITEMCOUNT)
        #define XF_TM_USE_ISLIST_MEMPOOL_ITEMCOUNT  8
    #endif
    #define LISTITEM_REFCOUNT_SIZE (XF_TM_USE_ISLIST_MEMPOOL_ITEMCOUNT * sizeof(uint8_t))
    static const uint32_t LISTITEM_POOL_MEMORY_SIZE = XF_TM_USE_ISLIST_MEMPOOL_ITEMCOUNT * XF_TM_USE_ISLIST_MEMPOOL_ITEMSIZE;
#endif // XF_TM_USE_ISLIST_MEMPOOL

    
/************************************************************************************************************/
/* Constructor | Destructor                                                                                 */
/************************************************************************************************************/
// -----------------------------------------------------------------------------------------------------------
XFTimeoutManager::XFTimeoutManager()/* : _tickInterval(0)*/
{
    ASSERT(!_pInstance);
    _pInstance = this;

    xfTimerInit();

    #if (XF_TM_USE_ISLIST != 0) && (XF_TM_USE_ISLIST_MEMPOOL != 0)
    _timeouts.initialize(createTimeoutListItemPool(XF_TM_USE_ISLIST_MEMPOOL_ITEMCOUNT));
    #endif // #if (XF_TM_USE_ISLIST != 0) && (XF_TM_USE_ISLIST_MEMPOOL != 0)
}

// -----------------------------------------------------------------------------------------------------------
XFTimeoutManager::~XFTimeoutManager()
{
}


/************************************************************************************************************/
/* PUBLIC SECTION                                                                                           */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
XFTimeoutManager* XFTimeoutManager::getInstance()
{
    #if (XF_TM_STATIC_INSTANCE != 0)
    static XFTimeoutManager theXfTimeoutManager;
    #endif // XF_TM_STATIC_INSTANCE

    if(!_pInstance)
    {
        // _pInstance gets already initialized in the constructor. The assignment below gets only done for convenience.
        #if (XF_TM_STATIC_INSTANCE != 0)
        _pInstance = &theXfTimeoutManager;
        #else
        // Create instance on the heap
        _pInstance = new XFTimeoutManager;
        #endif // XF_TM_STATIC_INSTANCE
    }

    return _pInstance;
}

//------------------------------------------------------------------------------------------------------------
#if (XF_TM_USE_ISLIST != 0) && (XF_TM_USE_ISLIST_MEMPOOL != 0)
XFTimeoutManager::TimeoutList::ItemObjectPool* XFTimeoutManager::createTimeoutListItemPool(size_t objectCount)
{
    static uint8_t timeoutListItemPoolMemory[LISTITEM_POOL_MEMORY_SIZE + LISTITEM_REFCOUNT_SIZE] __attribute__((aligned(4)));
    static uint8_t* pMemory = (uint8_t*)timeoutListItemPoolMemory;
    static TimeoutList::ItemObjectPool timeoutListItemPool(objectCount, &pMemory);

    // Check that reserved memory is not too little
    ASSERT(reinterpret_cast<size_t>(pMemory) <= reinterpret_cast<size_t>(timeoutListItemPoolMemory) + LISTITEM_POOL_MEMORY_SIZE + LISTITEM_REFCOUNT_SIZE);
    return &timeoutListItemPool;
}
#endif // #if (XF_TM_USE_ISLIST != 0) && (XF_TM_USE_ISLIST_MEMPOOL != 0)

//------------------------------------------------------------------------------------------------------------
void XFTimeoutManager::start()
{
    // Nothing to do here
}

//------------------------------------------------------------------------------------------------------------
void XFTimeoutManager::scheduleTimeout(uint8_t timeoutId, int interval, IXFReactive* pReactive)
{
    // Create a new timeout out of the given parameters
    XFTimeout* pTimeout = new XFTimeout(timeoutId, interval, pReactive);
    if(pTimeout)
    {
        XFTM_DEBUG("[XFTimeoutManager] scheduleTimeout() > Timeout #%d added successfully (interval: %d) ...\r\n", timeoutId, interval);
        addTimeout(pTimeout);
    }
    else
    {
        XFTM_DEBUG("[XFTimeoutManager] scheduleTimeout() > XFTimeout instance is NULL !\r\n");
        ASSERT(false);
    }
}

//------------------------------------------------------------------------------------------------------------
void XFTimeoutManager::unscheduleTimeout(uint8_t timeoutId, IXFReactive* pReactive)
{
    // Remove timeout from list
    XFTM_DEBUG("[XFTimeoutManager] unscheduleTimeout() > id: %d\r\n", timeoutId);
    removeTimeouts(timeoutId, pReactive);
}

//------------------------------------------------------------------------------------------------------------
void XFTimeoutManager::timeout()
{
    // Checks if the timeout's list is empty
    if(_timeouts.empty())
    {
        return;
    }
    
    // Get first timeout and inject the timeout back to the behavioral class
    #if (XF_TM_USE_ISLIST != 0)
    XFTimeout* pTimeout = _timeouts.front();
    #else
    TimeoutList::iterator it = _timeouts.begin();
    XFTimeout* pTimeout = *it;
    #endif

    // Inject the timeout back to the behavioral class
    XFTMD_DEBUG("[XFTimeoutManager] timeout() > Timeout #%d pushed back (queue size: %u)\r\n", pTimeout->getId(), _timeouts.size());
    returnTimeout(pTimeout);

    // Remove timeout
    _timeouts.pop_front();

    #if (XF_TM_USE_ISLIST != 0)
    // Check if following timeouts are also timeouted
    for(TimeoutList::Item* pFirst = _timeouts.getFirst(); pFirst != _timeouts.getEnd();)
    {
        if(pFirst->concept->_relTicks <= 0)
        {
            _timeouts.pop_front();
            returnTimeout(pFirst->concept);
            pFirst = _timeouts.getFirst();
        }
        else
        {
            break;
        }
    }
    #else
    // Check if following timeouts are also timeouted
    for(TimeoutList::iterator it = _timeouts.begin(); it != _timeouts.end();/*Dot not increment here!*/)
    {
        if((*it)->_relTicks <= 0)
        {
            XFTimeout* pTimeout = *it;
            it = _timeouts.erase(it);
            returnTimeout(pTimeout);
        }
        else
        {
            break;
        }
    }
    #endif
    
    // If there are still timeouts waiting in the timeout list, a new app timer has to be started
    _startXfTimer();

    XFTM_DEBUG("[XFTimeoutManager] timeout() > _timeouts.size: %d\r\n", _timeouts.size());
}


/************************************************************************************************************/
/* PROTECTED SECTION                                                                                        */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
void XFTimeoutManager::addTimeout(XFTimeout* pNewTimeout)
{
    if(!_timeouts.empty())
    {
        #if (XF_TM_PROTECT_ADD_TM != 0)
        enterCritical();
        #endif // XF_TM_PROTECT_ADD_TM

        #if (XF_TM_USE_ISLIST != 0)
        TimeoutList::Item* pItem    = _timeouts.getFirst();
        XFTimeout* pTimeout         = pItem->concept;
        XFTimeout* pLastTimeout     = _timeouts.getEnd()->concept;
        #else
        TimeoutList::iterator i     = _timeouts.begin();
        XFTimeout* pTimeout         = (*i);
        XFTimeout* pLastTimeout     = (*(_timeouts.end()));
        #endif  // XF_TM_PROTECT_ADD_TM
     
        #if (XF_TM_PROTECT_ADD_TM != 0)
        exitCritical();
        #endif // XF_TM_PROTECT_ADD_TM
       
        if(pTimeout != NULL)
        {
            // Decrement remaining ticks by elapsed value of timer
            pTimeout->decrRelativeTicks(xfTimerGetElapsedTime());
            XFTMTM_DEBUG("[XFTM] addTimeout() > Item->UpdateTicks: %lu; newTimeout->ticks: %lu\r\n", pTimeout->_relTicks, pNewTimeout->_relTicks);

            // Check if new timeout is shorter than actually running one
            if(pNewTimeout->_relTicks <= pTimeout->_relTicks)
            {
                #if (XF_TM_PROTECT_ADD_TM != 0)
                enterCritical();
                #endif // XF_TM_PROTECT_ADD_TM

                // A new timeout at the beginning
                _timeouts.push_front(pNewTimeout);

                #if (XF_TM_PROTECT_ADD_TM != 0)
                exitCritical();
                #endif // XF_TM_PROTECT_ADD_TM

                // Remove time from following timeout
                pTimeout->decrRelativeTicks(pNewTimeout->_relTicks);
                
                // XF Timeout duration correction
                #if (XF_USE_CODE_EXECUTION_COMPENSATION	!= 0)
                pNewTimeout->decrRelativeTicks(_XF_CODE_COMPENSATION_NOT_EMPTY_LIST + XF_TM_PROTECT_ADD_TM);
                #endif // XF_USE_CODE_EXECUTION_COMPENSATION	
                
                // Restart the XF's timer with new timeout's duration
                XFTMTM_DEBUG("[XFTM] addTimeout() > FRONT; Item->UpdateTicks: %lu; newTimeout->ticks: %lu\r\n", pTimeout->_relTicks, pNewTimeout->_relTicks);
                xfTimerRestart(pNewTimeout->_relTicks);
            }
            else
            {
                unsigned int index = 0;
                do
                {
                    // Remove time from new timeout
                    pNewTimeout->decrRelativeTicks(pTimeout->_relTicks);
                    XFTMTM_DEBUG("[XFTM] addTimeout() > LOOP; Item->UpdateTicks: %lu; newTimeout->ticks: %lu\r\n", pNewTimeout->_relTicks, pNewTimeout->_relTicks);
                    
                    // Get next item
                    #if (XF_TM_USE_ISLIST != 0)
                    pItem       = pItem->getNext(); 
                    pTimeout    = pItem->concept;
                    #else
                    i++;
                    pTimeout    = (*i);
                    #endif  // XF_TM_USE_ISLIST
                    index++;    
                } 
                while(pTimeout != pLastTimeout && pTimeout->_relTicks < pNewTimeout->_relTicks);
                
                #if (XF_TM_PROTECT_ADD_TM != 0)
                enterCritical();
                #endif // XF_TM_PROTECT_ADD_TM

                // Insert new timeout before
                #if (XF_TM_USE_ISLIST != 0)
                _timeouts.addAt(index, pNewTimeout);
                XFTMTM_DEBUG("[XFTM] addTimeout() > ADD AT: %d; Item added: 0x%08lx\r\n", index, pNewTimeout);
                #else
                _timeouts.insert(i, pNewTimeout);
                XFTMTM_DEBUG("[XFTM] addTimeout() > Item added: 0x%08lx, at position: %d\r\n", pNewTimeout, i);
                #endif  // XF_TM_USE_ISLIST

                #if (XF_TM_PROTECT_ADD_TM != 0)
                exitCritical();
                #endif // XF_TM_PROTECT_ADD_TM

                if(_timeouts.size() > index + 1)
                {
                    // Remove time from following timeout
                    pTimeout->decrRelativeTicks(pNewTimeout->_relTicks);
                }
            }
        }
    }        
    else
    {
        XFTMD_DEBUG("[XFTM] addTimeout() > Empty list of Timeouts ...\r\n");
        
        #if (XF_USE_CODE_EXECUTION_COMPENSATION	!= 0)    
        // XF Timeout duration correction
        pNewTimeout->decrRelativeTicks(XF_CODE_COMPENSATION_EMPTY_LIST + XF_TM_PROTECT_ADD_TM);
        #endif // XF_USE_CODE_EXECUTION_COMPENSATION
        
        #if (XF_TM_PROTECT_ADD_TM != 0)
        enterCritical();
        #endif // XF_TM_PROTECT_ADD_TM

        // Insert timeout in the list
        _timeouts.push_front(pNewTimeout);
        XFTMD_DEBUG("[XFTM] addTimeout() > Item added: 0x%08lx\r\n", pNewTimeout);

        #if (XF_TM_PROTECT_ADD_TM != 0)
        exitCritical();
        #endif // XF_TM_PROTECT_ADD_TM

        // Starts the XF's timer again
        xfTimerStart(pNewTimeout->_relTicks);
    }

    XFTM_DEBUG("[XFTimeoutManager] addTimeout() > _timeouts.size: %d\r\n", _timeouts.size());
}

//------------------------------------------------------------------------------------------------------------
void XFTimeoutManager::removeTimeouts(uint8_t timeoutId, IXFReactive* pReactive)
{
    const XFTimeout timeout(timeoutId, 0, pReactive);
    XFTimeout* pTimeout;
    XFTMTM_DEBUG("[XFTM] removeTimeouts() > ||| (timeoutId: %d; size: %u)\r\n", timeoutId, _timeouts.size());
    
    #if (XF_TM_USE_ISLIST != 0)
    for(TimeoutList::Item* pItem = _timeouts.getFirst(); pItem != _timeouts.getEnd(); pItem = pItem->getNext())
    {
		pTimeout = pItem->concept;
        XFTMTM_DEBUG("[XFTM] removeTimeouts() > ... (pItem: 0x%08x)\r\n", pItem->concept);

        // Decrement remaining ticks of first timeout in list by already elapsed ticks
		if(pTimeout == _timeouts.getFirst()->concept)    
		{
            XFTMTM_DEBUG("[XFTM] removeTimeouts() > ... ...\r\n");
			pTimeout->decrRelativeTicks(xfTimerGetElapsedTime());
		}
    #else
    for(TimeoutList::iterator i  = _timeouts.begin(); i != _timeouts.end(); i++)
    {
        pTimeout = *i;

        // Decrement remaining ticks of first timeout in list by already elapsed ticks
		if(pTImeout == (*_timeouts.begin()))    
		{
            XFTMTM_DEBUG("[XFTM] removeTimeouts() > ... ...\r\n");
			pTimeout->decrRelativeTicks(xfTimerGetElapsedTime());
		}
    #endif  // XF_TM_USE_ISLIST
    
        // Check if behavior and timeout id are equal
        if(*pTimeout == timeout)
        {
            XFTMTM_DEBUG("[XFTM] removeTimeouts() > ... ... ...\r\n");
			// Only affect other elements in list if there are any
			if(_timeouts.size() > 1)
            {
				// Get iterator of next element in list
                #if (XF_TM_USE_ISLIST != 0)
                XFTimeout* pNextTimeout = (pItem->getNext())->concept;
                #else
                TimeoutList::iterator next = i;
                XFTimeout* pNextTimeout = *(++next);
                #endif

                if(pNextTimeout)
                {
                    XFTMTM_DEBUG("[XFTM] removeTimeouts() > ... ... ... pNextTimeout: %d\r\n", pNextTimeout->_relTicks);
                    // Check if remaining ticks can be given further
                    #if (XF_TM_USE_ISLIST != 0)
                    if(pNextTimeout != _timeouts.getEnd()->concept)
                    #else
                    if(pTimeout != (*_timeouts.end()))
                    #endif
                    {
                        // Add (remaining) ticks to next timeout in list
                        pNextTimeout->incrRelativeTicks(pTimeout->_relTicks);
                    }
                    
                    // if it's the first element in list, we need to restart the timer
                    #if (XF_TM_USE_ISLIST != 0)
                    if(pTimeout == _timeouts.getFirst()->concept)
                    #else
                    if(pTimeout != (*_timeouts.begin()))
                    #endif
                    {
                        // Restarts running XF timer
                        xfTimerRestart(pNextTimeout->_relTicks);
                    }
                }
            }
            else
            {
                // Stop timer
                xfTimerStop();
            }

            #if (XF_TM_PROTECT_ADD_TM != 0)
            enterCritical();
            #endif // XF_TM_PROTECT_ADD_TM

            // Erase element and delete pointer
            #if (XF_TM_USE_ISLIST != 0)
            XFTMTM_DEBUG("[XFTM] removeTimeouts() > ... ... ... ... size: %d\r\n", _timeouts.size());
            _timeouts.removeItem(pItem);
            XFTMTM_DEBUG("[XFTM] removeTimeouts() > ... ... ... ... size: %d\r\n", _timeouts.size());
            #else
            i = _timeouts.erase(i);
            #endif
            XFTMTM_DEBUG("[XFTM] removeTimeouts() > Item removed: id #%d\r\n", pTimeout->getId());

            #if (XF_TM_PROTECT_ADD_TM != 0)
            exitCritical();
            #endif // XF_TM_PROTECT_ADD_TM

            delete pTimeout;
            break;
        }
    }

    XFTMTM_DEBUG("[XFTM] removeTimeouts() > ... ... ... ||| \r\n");
    XFTM_DEBUG("[XFTimeoutManager] removeTimeouts() > _timeouts.size: %d\r\n", _timeouts.size());
    
}

//------------------------------------------------------------------------------------------------------------
void XFTimeoutManager::returnTimeout(XFTimeout* pTimeout)
{
    pTimeout->getBehaviour()->pushEvent(pTimeout);
}


/************************************************************************************************************/
/* PRIVATE SECTION                                                                                          */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
void XFTimeoutManager::_startXfTimer()
{
    if(!_timeouts.empty())
    {
        #if (XF_TM_USE_ISLIST != 0)
        uint32_t ticks  = _timeouts.getFirst()->concept->_relTicks;
        #else
        uint32_t ticks  = (*_timeouts.begin())->_relTicks;
        #endif
        XFTMTM_DEBUG("[XFTM] _startXfTimer() > Timer's tick: %lu\r\n", ticks);
        xfTimerStart(ticks);
    }
    else
    {
        XFTMTM_DEBUG("[XFTM] _stopXfTimer() > Timer's tick: %lu\r\n", ticks);
        xfTimerStop();
    }
}

