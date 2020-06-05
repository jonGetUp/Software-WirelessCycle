/************************************************************************************************************
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2018
 * Created by Markus Salzmann, Patrice Rudaz & Thomas Sterren
 * All rights reserved.
 ************************************************************************************************************/

/************************************************************************************************************/
/* Header files                                                                                             */
/************************************************************************************************************/
#include "critical/critical.h"

#include "adapters/xf_adapter.h"
#include "core/xfthread.h"
#include "core/ixfreactive.h"
#include "core/xftimeoutmanager.h"


/************************************************************************************************************/
/* DEBUG COMPILATION OPTION & MACRO                                                                         */
/************************************************************************************************************/
#define DEBUG_XFTHREAD_ENABLE               0
#if (DEBUG_XFTHREAD_ENABLE != 0) && defined(DEBUG_NRF_USER)
    #include <tools/logtrace.h>
    #define XFT_DEBUG                       LOG_TRACE_DEBUG
#else // GPIOTE_CONFIG_LOG_ENABLED
    #define XFT_DEBUG(...)                  {(void) (0);}
#endif  // GPIOTE_CONFIG_LOG_ENABLED


/************************************************************************************************************/
/* Declaration section                                                                                      */
/************************************************************************************************************/
#if (XF_COUNT_EVENTS_ON_QUEUE != 0)
    #if (XF_CRITICAL_QUEUE != 0)
    static uint8_t criticalEventCounter     = 0;
    static uint8_t criticalEventCounterMax  = 0;
    #endif
    static uint8_t standardEventCounter     = 0;
    static uint8_t standardEventCounterMax  = 0;
#endif


/************************************************************************************************************/
/* CONSTRUCTOR                                                                                              */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
#if (XF_CRITICAL_QUEUE != 0)
XFThread::XFThread(uint8_t priorityLevelerThreshold)
    : _bExecuting(true), _priorityThreshold(priorityLevelerThreshold), _priorityCounter(0)
#else
XFThread::XFThread(uint8_t priorityLevelerThreshold)
    : _bExecuting(true), _priorityThreshold(priorityLevelerThreshold)
#endif  // XF_CRITICAL_QUEUE
{}

//------------------------------------------------------------------------------------------------------------
XFThread::~XFThread()
{}


/************************************************************************************************************/
/* PUBLIC SECTION                                                                                           */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
void XFThread::dispatchEvent(IXFEvent* pEvent) const
{
    EventStatus eventStatus = pEvent->getBehaviour()->process(pEvent);

    // Check if behavior should be deleted
    if(eventStatus == EventStatus::Terminate)
    {
        delete pEvent->getBehaviour();
    }
}

//------------------------------------------------------------------------------------------------------------
#if (XF_CRITICAL_QUEUE != 0)
void XFThread::pushCriticalEvent(IXFEvent* pEvent)
{
    enterCritical();
    _criticalEvents.push(pEvent);
    exitCritical();

    #if (XF_COUNT_EVENTS_ON_QUEUE != 0)
    // To estimate the size of the array of critical event
    criticalEventCounter++;
    #endif
}
#endif

//------------------------------------------------------------------------------------------------------------
void XFThread::pushEvent(IXFEvent* pEvent)
{
    enterCritical();
    _events.push(pEvent);
    exitCritical();

    #if (XF_COUNT_EVENTS_ON_QUEUE != 0)
    // To estimate the size of the array of critical event
    standardEventCounter++;
    #endif
}

//------------------------------------------------------------------------------------------------------------
void XFThread::scheduleTimeout(uint8_t timeoutId, uint16_t interval, IXFReactive* pReactive)
{
    // Forward timeout to the timeout manager
	XFT_DEBUG("[XFThread] scheduleTimeout() > id: %d, interval: %d\r\n", timeoutId, interval);
    XFTimeoutManager::getInstance()->scheduleTimeout(timeoutId, interval, pReactive);
}

//------------------------------------------------------------------------------------------------------------
void XFThread::unscheduleTimeout(uint8_t timeoutId, IXFReactive* pReactive)
{
    // Forward timeout to the timeout manager
	XFT_DEBUG("[XFThread] unscheduleTimeout() > id: %d\r\n", timeoutId);
    XFTimeoutManager::getInstance()->unscheduleTimeout(timeoutId, pReactive);
}

//------------------------------------------------------------------------------------------------------------
void XFThread::execute(const bool execOnce)
{
    while(_bExecuting)
    {
        IXFEvent* pEvent = NULL;

        #if (XF_HOOK_PREEXECUTE_ENABLED != 0)
        xfPreExecuteHook();
        #endif // XF_HOOK_PREEXECUTE_ENABLED

        #if (XF_CRITICAL_QUEUE != 0)
        while(_criticalEvents.empty() && _events.empty())
        #else
        while(_events.empty())
        #endif // XF_CRITICAL_QUEUE
        {
            if(execOnce)
            {
                break;
            }
            else
            {
                // Wait until something to do
                xfHookOnIdle();     // Needs to be implemented in the xf_adapter file
            #if (XF_HOOK_PREEXECUTE_ENABLED != 0)
                xfPreExecuteHook();
            #endif // XF_HOOK_PREEXECUTE_ENABLED
            }
        }

        // Deque next event from queue
        #if (XF_CRITICAL_QUEUE != 0)
        if(!_criticalEvents.empty() && _priorityCounter < _priorityThreshold)
        {
            pEvent = _criticalEvents.front(); _criticalEvents.pop();
            _priorityCounter++;

            #if (XF_COUNT_EVENTS_ON_QUEUE != 0)
            if(criticalEventCounter > criticalEventCounterMax)
            {
                criticalEventCounterMax = criticalEventCounter;
                XFT_DEBUG("[XFThread] execute() > criticalEventCounterMax = %d\r\n", criticalEventCounterMax);
            }
            criticalEventCounter--;
            #endif
        }
        else if(!_events.empty())
        {
            pEvent = _events.front(); _events.pop();
            _priorityCounter = 0;

            #if (XF_COUNT_EVENTS_ON_QUEUE != 0)
            if(standardEventCounter > standardEventCounterMax)
            {
                standardEventCounterMax = standardEventCounter;
                XFT_DEBUG("[XFThread] execute() > standardEventCounterMax = %d\r\n", standardEventCounterMax);
            }
            standardEventCounter--;
            #endif
        }
        else
        {
            _priorityCounter = 0;
            continue;
        }
        #else   // XF_CRITICAL_QUEUE
        if(!_events.empty())
        {
            pEvent = _events.front(); _events.pop();

            #if (XF_COUNT_EVENTS_ON_QUEUE != 0)
            if(standardEventCounter > standardEventCounterMax)
            {
                standardEventCounterMax = standardEventCounter;
                XFT_DEBUG("[XFThread] execute() > standardEventCounterMax = %d\r\n", standardEventCounterMax);
            }
            standardEventCounter--;
            #endif
        }
        else
        {
            if(execOnce)
            {
                // Do not loop
                break;
            }
            else
            {
                // Stay in loop
                continue;
            }
        }
        #endif  // XF_CRITICAL_QUEUE

        if(pEvent)
        {
            // Forward the event to the behavioral class
            dispatchEvent(pEvent);

            if(pEvent->getEventType() == IXFEvent::Terminate)
            {
                // Exit the event loop
                _bExecuting = false;
            }

            // Remove the consument event
            if(pEvent->mustBeDeleted())
            {
                delete pEvent;
            }
        }
        
        #if (XF_HOOK_POSTEXECUTE_ENABLED != 0)
        xfPostExecuteHook();
        #endif // XF_HOOK_POSTEXECUTE_ENABLED
    }
}
