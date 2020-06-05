/****************************************************************************************************************//**
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2018
 * Created by Thomas Sterren (thomas.sterren@hevs.ch) & Patrice Rudaz (patrice.rudaz@hevs.ch)
 * All rights reserved.
 *
 * \file    uptime.h
 * \class   UpTime
 * \brief   Calculates the time since system startup.
 *
 * UpTime calculates the time in minutes since system startup. This class can be used to check how long the system 
 * is already running.
 *
 * Only one instance of this class can be instantiated (singleton pattern). It is up to the developer when to create
 * this instance.
 *
 * Initial author: Thomas Sterren
 * Creation date: 2016-06-07
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
#include <platform-config.h>
#include <core/xfreactive.h>

/* **************************************************************************************************************** */
/*                                                                                                                  */
/* Class Declaration                                                                                                */
/*                                                                                                                  */
/* **************************************************************************************************************** */
class UpTime : public XFReactive
{
public:
    /************************************************************************************************************//**
     * \brief   Interface for UpTime observers
     ****************************************************************************************************************/
    class IObserver
    {
    public:
        /********************************************************************************************************//**
         * \brief   Called after each minute elapsed.
         ************************************************************************************************************/
        virtual void onUpTimeUpdate(const uint32_t days, const uint8_t hours, const uint8_t minutes) = 0;   
    };

public:
    /* ************************************************************************************************************ */
    /*                                                                                                              */
    /* CONSTRUCTOR DECALRATION SECTION                                                                              */
    /*                                                                                                              */
    /* ************************************************************************************************************ */
    UpTime();
    virtual ~UpTime();


    /* ************************************************************************************************************ */
    /*                                                                                                              */
    /* PUBLIC DECALRATION SECTION                                                                                   */
    /*                                                                                                              */
    /* ************************************************************************************************************ */
    /************************************************************************************************************//**
     * \brief   Returns a reference to the single object.
     ****************************************************************************************************************/
    static UpTime &instance()               { ASSERT(_pInstance); return *_pInstance; }

    /************************************************************************************************************//**
     * \brief   Starts the state machine.
     ****************************************************************************************************************/
    inline void start()                     { startBehaviour(); }

    /************************************************************************************************************//**
     * \brief   Subscribes observer to the class.
     ****************************************************************************************************************/
    bool subscribe(IObserver &observer);

    /************************************************************************************************************//**
     * \brief   Returns a well formatted string containing the up time.
     ****************************************************************************************************************/
    const char* upTimeString() const;

protected:
    /* ************************************************************************************************************ */
    /*                                                                                                              */
    /* PUBLIC DECALRATION SECTION                                                                                   */
    /*                                                                                                              */
    /* ************************************************************************************************************ */
    /************************************************************************************************************//**
     * \brief   XFReactive interface implementation
     ****************************************************************************************************************/
    virtual EventStatus processEvent();

    /************************************************************************************************************//**
     * \brief   Called every minute in state machine.
     ****************************************************************************************************************/
    void doMinuteElapsed();

    /************************************************************************************************************//**
     * \brief   Notifies observers about changes.
     ****************************************************************************************************************/
    void notifyObservers();

    // Attributes
    static UpTime* _pInstance;                      ///< \brief Pointer to single instance.

    static const uint8_t MAX_OBSERVERS = 2;         ///< \brief Maximum observers allowed.
    IObserver* _observers[MAX_OBSERVERS];           ///< \brief Observers subscribed to get notifications.
    uint8_t _observerCount;                         ///< \brief Number of registered observers.

    uint32_t _days;                                 ///< \brief Days of time since system startup.
    uint8_t _hours;                                 ///< \brief Hours of time since system startup.
    uint8_t _minutes;                               ///< \brief Minutes of time since system startup.
};
