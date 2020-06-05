/****************************************************************************************************************//**
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2018
 * All rights reserved.
 *
 * \file    eventid.h
 * \class   EventId
 * \brief   Class to specify the available ID for events
 *
 * This class is responsible to distribute to every event used in the project a different event's
 * identifier.
 *
 ********************************************************************************************************************/
#pragma once


/* **************************************************************************************************************** */
/*                                                                                                                  */
/* Header files                                                                                                     */
/*                                                                                                                  */
/* **************************************************************************************************************** */


/* **************************************************************************************************************** */
/*                                                                                                                  */
/* Class Declaration                                                                                                */
/*                                                                                                                  */
/* **************************************************************************************************************** */
class EventId
{
public:
    /* ************************************************************************************************************ */
    /*                                                                                                              */
    /* PUBLIC SECTION                                                                                               */
    /*                                                                                                              */
    /* ************************************************************************************************************ */

    ///< Enumeration of event's id available in the current application
    typedef enum
    {
        UnknownEvent = 0,               ///< Unknown event (not initialized).
        DefaultEvent,                   ///< Event ID to specify the default event.

        UartEvent,                      ///< Event ID to identify an event due to an UART activity...

        CalendarEvent,                  ///< Event ID related to a change in date and time

        MotorEvent,                     ///< Event ID dealing with motor's movement
        MotorInitEvent,                 ///< Event ID dealing with motor's movement
        MotorEndEvent,                  ///< Event ID dealing with motor's movement

        AncsNotificationEvent,          ///< Event ID dealing with ANCS notification events
        AncsAttributeEvent,             ///< Event ID dealing with ANCS attribute events
        AncsAppAttributeEvent,          ///< Event ID dealing with ANCS Application attribute events

        SensorEvent,                    ///< Event ID to specify the default event for the SensorController.

        // Add here your specific event ID
        TopEnumEventId                  ///< Last entry in the enumeration
    } eEventId;

    /* **************************************************************************************************************** *//**
    * \brief   Returns the string which describes the given event.
    *
    * \param   state    The identifier of the event to translate in string
    ****************************************************************************************************************/
    static const char* toString(eEventId eventId)
    {
        switch(eventId)
        {
            case DefaultEvent:          return "DefaultEvent";
            case UartEvent:             return "UartEvent";
            case CalendarEvent:         return "CalendarEvent";
            case MotorEvent:            return "MotorEvent";
            case MotorInitEvent:        return "MotorInitEvent";
            case MotorEndEvent:         return "MotorEndEvent";
            case AncsNotificationEvent: return "AncsNotificationEvent";
            case AncsAttributeEvent:    return "AncsAttributeEvent";
            case TopEnumEventId:        return "TopEnumEventId";
            default:                    return "UnknownEvent";
        }
    }
};
