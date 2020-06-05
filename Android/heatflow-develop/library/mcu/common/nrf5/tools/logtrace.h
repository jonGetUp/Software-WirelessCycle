/****************************************************************************************************************//**
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 *
 * \file    logtrace.h
 *
 * \addtogroup Tools
 * \{
 *
 * \class   tools::LogTrace
 * \brief   Class agregating a couple of tool related to the RTT logger feature used by Nordic SDK v15
 *
 * Print debug/trace messages formated as String values. The messages can be sent over RTT (SEGGER's Real Time
 * Transfer), but a SEGGER probe is needed!
 *
 * This library provides methods that call the respective functions of the RTT protocol.
 * The transport layer is defined as `RTT` and enable by the global define `LOG_TRACE_RTT_ENABLED`. This define must 
 * be set for any of the macros to have effect. If you choose to not output information, all logging macros can be 
 * left in the code without any cost; they will just be ignored.
 *
 * Initial author: Patrice Rudaz
 * Creation date: 2018-01-15
 *
 * \author  Patrice RUDAZ
 * \date    July 2018
 ********************************************************************************************************************/
#pragma once

/* **************************************************************************************************************** */
/*                                                                                                                  */
/* Header files                                                                                                     */
/*                                                                                                                  */
/* **************************************************************************************************************** */
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include <assert-config.h>
#include <platform-config.h>
#include <sdk_config.h>

#if defined(LOG_TRACE_RTT_ENABLED) && (LOG_TRACE_RTT_ENABLED != 0)
    #include <SEGGER_RTT.h>
#endif


/* **************************************************************************************************************** */
/*                                                                                                                  */
/* MACRO definition                                                                                                 */
/*                                                                                                                  */
/* **************************************************************************************************************** */
#define LOG_TERMINAL_NORMAL                 (0)
#define LOG_TERMINAL_ERROR                  (1)

#define RTT_KEIL_CTRL_RESET                 "\x1B[0m"         // Reset to default colors
#define RTT_KEIL_CTRL_CLEAR                 "\x1B[2J"         // Clear screen, reposition cursor to top left

#define RTT_KEIL_CTRL_TEXT_BLACK            "\x1B[2;30m"
#define RTT_KEIL_CTRL_TEXT_RED              "\x1B[2;31m"
#define RTT_KEIL_CTRL_TEXT_GREEN            "\x1B[2;32m"
#define RTT_KEIL_CTRL_TEXT_YELLOW           "\x1B[2;33m"
#define RTT_KEIL_CTRL_TEXT_BLUE             "\x1B[2;34m"
#define RTT_KEIL_CTRL_TEXT_MAGENTA          "\x1B[2;35m"
#define RTT_KEIL_CTRL_TEXT_CYAN             "\x1B[2;36m"
#define RTT_KEIL_CTRL_TEXT_WHITE            "\x1B[2;37m"


#if defined(LOG_TRACE_RTT_ENABLED) && (LOG_TRACE_RTT_ENABLED != 0)
    #define LOG_TRACE_CLS                   tools::logTrace::cls()
    #define LOG_TRACE_DEBUG(...)            tools::LogTrace::debug(__VA_ARGS__)
    #define LOG_TRACE_ERROR(...)            tools::LogTrace::error(__VA_ARGS__)
    #define LOG_TRACE_WARNING(...)          tools::LogTrace::warn( __VA_ARGS__)
    #define LOG_TRACE_INFO(...)             tools::LogTrace::info( __VA_ARGS__)
#else
    #define LOG_TRACE_DEBUG(...)            {(void) (0);}
    #define LOG_TRACE_ERROR(...)            {(void) (0);}
    #define LOG_TRACE_WARNING(...)          {(void) (0);}
    #define LOG_TRACE_INFO(...)             {(void) (0);}
#endif

/* **************************************************************************************************************** */
/*                                                                                                                  */
/* NAMESPACE Declaration                                                                                            */
/*                                                                                                                  */
/* **************************************************************************************************************** */
namespace tools 
{

    /* ************************************************************************************************************ */
    /*                                                                                                              */
    /* Class Declaration                                                                                            */
    /*                                                                                                              */
    /* ************************************************************************************************************ */
    class LogTrace 
    {
    public:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PUBLIC DECLARATION SECTION                                                                               */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        /********************************************************************************************************//**
         * \brief   Initialization the SEGGER RTT module
         ************************************************************************************************************/
        static inline void init()
        {
            #if defined(LOG_TRACE_RTT_ENABLED) && (LOG_TRACE_RTT_ENABLED != 0)
            SEGGER_RTT_Init();
            #endif  // #if defined(LOG_TRACE_RTT_ENABLED) && (LOG_TRACE_RTT_ENABLED != 0)
        }

        /********************************************************************************************************//**
         * \brief   Clear screen.
         ************************************************************************************************************/
        static void cls()
        {
            #if defined(LOG_TRACE_RTT_ENABLED) && (LOG_TRACE_RTT_ENABLED != 0)
            out(LOG_TERMINAL_NORMAL, RTT_CTRL_CLEAR);
            #endif  // #if defined(LOG_TRACE_RTT_ENABLED) && (LOG_TRACE_RTT_ENABLED != 0)
        }

        /********************************************************************************************************//**
         * \brief   Trace a debug message.
         ************************************************************************************************************/
        static void debug(const char * format, ...) 
        {
            #if defined(LOG_TRACE_RTT_ENABLED) && (LOG_TRACE_RTT_ENABLED != 0)
            char str[255];
            va_list args;

            // Format string
            va_start(args, format);
            vsprintf(str, format, args);
            out(LOG_TERMINAL_NORMAL, str);
            va_end(args);
            #endif  // #if defined(LOG_TRACE_RTT_ENABLED) && (LOG_TRACE_RTT_ENABLED != 0)
        }

        /********************************************************************************************************//**
         * \brief   Trace an information message.
         ************************************************************************************************************/
        static void info(const char * format, ...) 
        {
            #if defined(LOG_TRACE_RTT_ENABLED) && (LOG_TRACE_RTT_ENABLED != 0)
            char str[255];
            va_list args;

            // Format string
            va_start(args, format);
            vsprintf(str, format, args);
            out(LOG_TERMINAL_NORMAL, str);
            va_end(args);
            #endif  // #if defined(LOG_TRACE_RTT_ENABLED) && (LOG_TRACE_RTT_ENABLED != 0)
        }

        /********************************************************************************************************//**
         * \brief   Trace a warning message.
         ************************************************************************************************************/
        static void warn(const char * format, ...)
        {
            #if defined(LOG_TRACE_RTT_ENABLED) && (LOG_TRACE_RTT_ENABLED != 0)
            char str[255];
            va_list args;

            // Format string
            va_start(args, format);
            vsprintf(str, format, args);
            sprintf(str, "%s%s", RTT_CTRL_TEXT_YELLOW, str);
            out(LOG_TERMINAL_NORMAL, str);
            va_end(args);
            #endif  // #if defined(LOG_TRACE_RTT_ENABLED) && (LOG_TRACE_RTT_ENABLED != 0)
        }

        /********************************************************************************************************//**
         * \brief   Trace an error message.
         ************************************************************************************************************/
        static void error(const char * format, ...)
        {
            #if defined(LOG_TRACE_RTT_ENABLED) && (LOG_TRACE_RTT_ENABLED != 0)
            char str[255];
            va_list args;

            // Format string
            va_start(args, format);
            vsprintf(str, format, args);
            sprintf(str, "%s%s", RTT_CTRL_TEXT_RED, str);
            out(LOG_TERMINAL_NORMAL, str);
            va_end(args);
            #endif  // #if defined(LOG_TRACE_RTT_ENABLED) && (LOG_TRACE_RTT_ENABLED != 0)
        }

    protected:
        /* ******************************************************************************************************** */
        /*                                                                                                          */
        /* PROTECTED DECLARATION SECTION                                                                            */
        /*                                                                                                          */
        /* ******************************************************************************************************** */
        /********************************************************************************************************//**
         * Constructor
         ************************************************************************************************************/
        LogTrace() 
        {
            // Utility class
        }

        /********************************************************************************************************//**
         * \brief   Print a message encoded as a String.
         *
         * The trace message is limited to 255 bytes. It is encoded as a String value.
         *  * If the VSM protocol is used as transport, the message is encoded as a VSM notification message, the 
         *    command type is set to `debug` and the Endpoint is fixed set to `broadcast`.
         *  * If RTT is used, only the String message is sent (the type is not used). A new line must be added.
         *  * Do nothing if no transport has been selected
         *
         * \param   index   The index of the buffer to be used in RTT.
         * \param   str     The string to print (Null-terminated String)
         ************************************************************************************************************/
        static void out(uint8_t index, const char * str);
    };

}   // Namespace tools    

/** \} */   // Group: Tools
