/************************************************************************************************************
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2018
 * Created by Markus Salzmann, Patrice Rudaz & Thomas Sterren
 * All rights reserved.
 ************************************************************************************************************/

/************************************************************************************************************/
/* Header files                                                                                             */
/************************************************************************************************************/
#include "xftimeout.h"
#include "adapters/xf_adapter.h"

/************************************************************************************************************/
/* Declaration section                                                                                      */
/************************************************************************************************************/
#if (XF_TM_USE_MEMORYPOOL_FOR_TIMEOUTS != 0)
    IMPLEMENT_MEMORYPOOL_OPERATORS(XFTimeout)
#endif // XF_TM_USE_MEMORYPOOL_FOR_TIMEOUTS

    
/************************************************************************************************************/
/* DEBUG COMPILATION OPTION & MACRO                                                                         */
/************************************************************************************************************/
#define DEBUG_XFTIMEOUT_ENABLE              0
#if (DEBUG_XFTIMEOUT_ENABLE != 0) && defined(DEBUG_NRF_USER)
    #include <tools/logtrace.h>
    #define XFT_DEBUG                       LOG_TRACE_DEBUG
#else
    #define XFT_DEBUG(...)                  {(void) (0);}
#endif


/************************************************************************************************************/
/* CONSTRUCTOR                                                                                              */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
XFTimeout::XFTimeout(uint8_t id, unsigned int interval, IXFReactive* pBehaviour)
    : IXFEvent(IXFEvent::Timeout, id, pBehaviour), _interval(interval)
{
    _relTicks       = 0;
    _overflow       = 0;
    
    if(interval > 0)
    {
        _relTicks       = interval2TimerTick(interval);
        if(_relTicks > xf_timer_max_counter_value)
        {
            _overflow   = _relTicks - xf_timer_max_counter_value;
            _relTicks   = xf_timer_max_counter_value;
        }
    }
    XFT_DEBUG("[XFTimeout] XFTimeout() > id: %d; _interval: %8d, _relTicks: %10lu, _overflow: %12lu\r\n", id, _interval, _relTicks, _overflow);
}

//------------------------------------------------------------------------------------------------------------
XFTimeout::XFTimeout(const XFTimeout &timeout)
    : IXFEvent(IXFEvent::Timeout, timeout._id, timeout._pBehaviour)
{
    ASSERT(false);
}

//------------------------------------------------------------------------------------------------------------
XFTimeout::~XFTimeout()
{}


/************************************************************************************************************/
/* PUBLIC SECTION                                                                                           */
/************************************************************************************************************/

//------------------------------------------------------------------------------------------------------------
void XFTimeout::incrRelativeTicks(unsigned int toAdd)
{
    if(toAdd > 0)
    {
        _relTicks = (_relTicks + toAdd) & xf_timer_max_counter_value;
        XFT_DEBUG("[XFTimeout] incrRelativeTicks() > _relTicks: %10lu (toAdd: %10lu)\r\n", _relTicks, toAdd);
    }
}
    
//------------------------------------------------------------------------------------------------------------
void XFTimeout::decrRelativeTicks(unsigned int toSubstract)
{
    if(toSubstract > 0)
    {
        if(_relTicks > XF_TIMER_MIN_TIMEOUT_TICKS && (_relTicks - XF_TIMER_MIN_TIMEOUT_TICKS) > toSubstract)
        {
            _relTicks -= toSubstract;
        }
        else
        {
            _relTicks = 0;
        }
    }
    XFT_DEBUG("[XFTimeout] decrRelativeTicks() > _relTicks: %10lu (toSubstract: %10lu)\r\n", _relTicks, toSubstract);
}
    
//------------------------------------------------------------------------------------------------------------
const XFTimeout &XFTimeout::operator = (const XFTimeout &timeout)
{
    ASSERT(false);
    return timeout;
}

//------------------------------------------------------------------------------------------------------------
bool XFTimeout::operator ==(const XFTimeout &timeout) const
{
    // Check behavior and timeout id attributes, if there are equal
    return (_pBehaviour == timeout._pBehaviour && getId() == timeout.getId()) ? true : false;
}
