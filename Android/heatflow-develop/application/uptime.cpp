/************************************************************************************************************
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz, Thomas Sterren
 * All rights reserved.
 ************************************************************************************************************/

/************************************************************************************************************/
/* Header files                                                                                             */
/************************************************************************************************************/
#include "uptime.h"
#include <stdio.h>
#include <string.h>


/************************************************************************************************************/
/* DEBUG COMPILATION OPTION & MACRO                                                                         */
/************************************************************************************************************/
#define DEBUG_UPTIME_ENABLE                 1
#if (DEBUG_UPTIME_ENABLE == 1) && defined(DEBUG_NRF_USER)
    #include <tools/logtrace.h>
    #define UPT_DEBUG                       LOG_TRACE_INFO
#else
    #define UPT_DEBUG(...)                  {(void) (0);}
#endif

/************************************************************************************************************/
/* Declaration section                                                                                      */
/************************************************************************************************************/
UpTime* UpTime::_pInstance(NULL);

/************************************************************************************************************/
/* CONSTRUCTOR                                                                                              */
/************************************************************************************************************/
UpTime::UpTime() : _observerCount(0), _days(0), _hours(0), _minutes(0)
{
    // Only one instance of this class allowed!
    ASSERT(!_pInstance);
    _pInstance = this;

    memset(_observers, 0, sizeof(_observers));
}

UpTime::~UpTime()
{
}

/************************************************************************************************************/
/* PUBLIC SECTION                                                                                           */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
bool UpTime::subscribe(IObserver &observer)
{
    if(_observerCount < MAX_OBSERVERS)
    {
        for(int i = 0; i < MAX_OBSERVERS; i++)
        {
            if(_observers[i] == NULL)
            {
                _observers[i] = &observer;
                _observerCount++;
                break;
            }
        }
        return true;
    }
    ASSERT(false);
    return false;
}

//------------------------------------------------------------------------------------------------------------
EventStatus UpTime::processEvent()
{
    switch(getCurrentEvent()->getEventType())
    {
        case IXFEvent::Initial:
            UPT_DEBUG("[UpTime] doInit() > Starting UpTime\r\n");
            getThread()->scheduleTimeout(1, 60000, this);
            break;

        case IXFEvent::Timeout:
            doMinuteElapsed();
            getThread()->scheduleTimeout(1, 60000, this);
            break;

        default:
            break;
    }
    return EventStatus::Consumed;
}

//------------------------------------------------------------------------------------------------------------
void UpTime::doMinuteElapsed()
{
    _minutes++;

    if(_minutes >= 60)
    {
        _hours++;
        _minutes -= 60;
    }

    if(_hours >= 24)
    {
        _days++;
        _hours -= 24;
    }

    UPT_DEBUG(upTimeString());
    notifyObservers();
}

//------------------------------------------------------------------------------------------------------------
void UpTime::notifyObservers()
{
    for(uint8_t i = 0; i < MAX_OBSERVERS; i++)
    {
        if(_observers[i])
        {
            _observers[i]->onUpTimeUpdate(_days, _hours, _minutes);
        }
    }
}

//------------------------------------------------------------------------------------------------------------
const char* UpTime::upTimeString() const
{
    // Needs to be at least 44
    static char str[44];

    if(_days == 0)
    {
        sprintf(str, "[UpTime] Uptime:            %02u:%02u (hh:mm)\r\n", _hours, _minutes);
    }
    else if(_days == 1)
    {
        sprintf(str, "[UpTime] Uptime:   1 day  - %02u:%02u (hh:mm)\r\n", _hours, _minutes);
    }
    else
    {
        sprintf(str, "[UpTime] Uptime: %3u days - %02u:%02u (hh:mm)\r\n", (uint8_t)_days, _hours, _minutes);
    }

    // Error: Actual string bigger then the buffer!
    ASSERT(strlen(str) < sizeof(str));
    return str;
}
