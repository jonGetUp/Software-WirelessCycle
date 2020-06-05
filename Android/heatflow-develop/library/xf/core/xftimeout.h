/****************************************************************************************************************//**
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2018
 * Created by Thomas Sterren (thomas.sterren@hevs.ch) & Patrice Rudaz (patrice.rudaz@hevs.ch)
 * All rights reserved.
 *
 * \file    xftimeout.h
 *
 * \addtogroup XF eXecution Framework
 * \{
 *
 * \class   XFTimeout
 * \brief   Interface to be implemented by time-outs.
 *
 * Interface to be implemented by time-outs. Defined methods are needed by the framework classes 
 * (see also: \ref XFTimeoutManager).
 *
 * Initial author: Thomas Sterren
 * Creation date: 2010-11-26
 *
 * \author  Patrice Rudaz
 * \date    July 2018
 ********************************************************************************************************************/
#pragma once

/* **************************************************************************************************************** */
/*                                                                                                                  */
/* Header files                                                                                                     */
/*                                                                                                                  */
/* **************************************************************************************************************** */
#include <xf-config.h>
#include "core/ixfevent.h"
#if (XF_TM_USE_MEMORYPOOL_FOR_TIMEOUTS != 0)
    #include <pool/memorypoolmacros.h>
#endif // XF_TM_USE_MEMORYPOOL_FOR_TIMEOUTS


/* **************************************************************************************************************** */
/*                                                                                                                  */
/* Class Declaration                                                                                                */
/*                                                                                                                  */
/* **************************************************************************************************************** */
class XFTimeout : public IXFEvent
{
    friend class XFTimeoutManager;

public:
    /* ************************************************************************************************************ */
    /*                                                                                                              */
    /* CONSTRUCTOR SECTION                                                                                          */
    /*                                                                                                              */
    /* ************************************************************************************************************ */
    XFTimeout(uint8_t id, unsigned int interval, IXFReactive * pBehaviour);
    virtual ~XFTimeout();

    /* ************************************************************************************************************ */
    /*                                                                                                              */
    /* PUBLIC DECLARATION SECTION                                                                                   */
    /*                                                                                                              */
    /* ************************************************************************************************************ */
    /************************************************************************************************************//**
     * \brief   Operator checks if #_pBehaviour and #_id are equal.
     *
     * Operator checks if #_pBehaviour and #_id are equal and returns true if so. All other attributes get no 
     * attention.
     ****************************************************************************************************************/
    bool operator ==(const XFTimeout & timeout) const;

#if (XF_TM_USE_MEMORYPOOL_FOR_TIMEOUTS != 0)
    /************************************************************************************************************//**
     * \brief   Create custom new and delete operators using memory pool
     ****************************************************************************************************************/
    DECLARE_MEMORYPOOL_OPERATORS(XFTimeout)
#endif // XF_TM_USE_MEMORYPOOL_FOR_TIMEOUTS

    /************************************************************************************************************//**
     * \brief   Returns the interval value set for the current timeout
     ****************************************************************************************************************/
    inline unsigned int interval() const                { return _interval; }
    
    /************************************************************************************************************//**
     * \brief   Returns the overflow fo the interval value set for the current timeout
     ****************************************************************************************************************/
    inline unsigned int overflow() const                { return _overflow; }
    
    /************************************************************************************************************//**
     * \brief   Returns the timer tick's counter relative the interval of the current timeout
     ****************************************************************************************************************/
    inline unsigned int timerTicks() const              { return _relTicks; }
    
    /************************************************************************************************************//**
     * \brief   Increments the ticks value with the given parameter
     ****************************************************************************************************************/
    void incrRelativeTicks(unsigned int toAdd);
    
    /************************************************************************************************************//**
     * \brief   Decrement the ticks value...
     ****************************************************************************************************************/
    void decrRelativeTicks(unsigned int toSubstract);

protected:
    /* ************************************************************************************************************ */
    /*                                                                                                              */
    /* PROTECTED DECLARATION SECTION                                                                                */
    /*                                                                                                              */
    /* ************************************************************************************************************ */
    // Attributes
    unsigned int    _interval;
    unsigned int    _relTicks;  ///< \brief Used by the \ref XFTimeoutManager "TimeoutManager"  to calculate remaining time. Can get negative!
    unsigned int    _overflow;  ///< \brief stores the overflow value of the counter value. As the coutner value is a non-infinite value, we should need

    // Methods
    /************************************************************************************************************//**
     * \brief   Protected Constructor
     *
     * \warning Creating copies of XFTimeout is not allowed.
     ****************************************************************************************************************/
    XFTimeout(const XFTimeout & timeout);

    /************************************************************************************************************//**
     * \brief   Operator overload for XFTimeout
     *
     * \warning Creating copies of XFTimeout is not allowed.
     ****************************************************************************************************************/
    const XFTimeout & operator = (const XFTimeout & timeout);
};

/** \} */
