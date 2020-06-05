/* Copyright (c) 2018, HES-SO Valais
 * All rights reserved.
 */
#ifndef XF_CONFIG_H
#define XF_CONFIG_H

#include <sdk-tools-config.h>
#include <sdk_config.h>                                                 // For define APP_SCHEDULER_ENABLED

// <<< Use Configuration Wizard in Context Menu >>>\n
#ifdef USE_APP_CONFIG
#include "app_config.h"
#endif


//==========================================================
// <h> XF configuration

//==========================================================
// <q> XF_CRITICAL_QUEUE  -  Enable/Disable the usage of the Critical Queue for urgent events

#ifndef XF_CRITICAL_QUEUE
    #define XF_CRITICAL_QUEUE                       1
#endif

//==========================================================
// <q> XF_QUEUE_SIZE  -  Default XF's queue size used for both Critical and Standard queue

#ifndef XF_QUEUE_SIZE
    #define XF_QUEUE_SIZE                           10
#endif

//==========================================================
// <q> XF_COUNT_EVENTS_ON_QUEUE  -  Enable the counting of events in XFThread. This is for debug purpose

#ifndef XF_COUNT_EVENTS_ON_QUEUE
    #define XF_COUNT_EVENTS_ON_QUEUE                0
#endif


//==========================================================
// <q> XF_TM_USE_ISLIST  -  Enabled causes the XFTimeoutManager to use the IsList (interrupt safe list) class to queue timeouts.

#ifndef XF_TM_USE_ISLIST
    #define XF_TM_USE_ISLIST                        1
#endif

//==========================================================
// <q> XF_TM_USE_ISLIST_MEMPOOL  -  Enable to let the IsList take the memory from memory pool.

#ifndef XF_TM_USE_ISLIST_MEMPOOL
    #define XF_TM_USE_ISLIST_MEMPOOL                1
#endif

//==========================================================
// <o> XF_TM_USE_ISLIST_MEMPOOL_ITEMCOUNT  -  Maximum IsList items in XFTimeoutManager (XF_TM_USE_ISLIST_MEMPOOL needs to be enabled).

#ifndef XF_TM_USE_ISLIST_MEMPOOL_ITEMCOUNT
    #define XF_TM_USE_ISLIST_MEMPOOL_ITEMCOUNT      16
#endif

//==========================================================
// <o> XF_TM_USE_ISLIST_MEMPOOL_ITEMSIZE  -  Size of list item

#ifndef XF_TM_USE_ISLIST_MEMPOOL_ITEMSIZE
    #define XF_TM_USE_ISLIST_MEMPOOL_ITEMSIZE       8
#endif

//==========================================================
// <q> XF_TM_USE_MEMORYPOOL_FOR_TIMEOUTS  -  Enable to get memory from memory pool to create timeouts.

#ifndef XF_TM_USE_MEMORYPOOL_FOR_TIMEOUTS
    #define XF_TM_USE_MEMORYPOOL_FOR_TIMEOUTS       1
#endif

//==========================================================
// <q> XF_TM_STATIC_INSTANCE  -  Enable to statically allocate the instance for the XFTimeoutManager.

#ifndef XF_TM_STATIC_INSTANCE
    #define XF_TM_STATIC_INSTANCE                   1
#endif

//==========================================================
// <q> XF_USE_MEMORYPOOL_FOR_NULLTRANSITIONS  -  Enable/Disable the usage of the memory pool to create null transitions instead of creating them using heap.

#ifndef XF_USE_MEMORYPOOL_FOR_NULLTRANSITIONS
    #define XF_USE_MEMORYPOOL_FOR_NULLTRANSITIONS   1
#endif

//==========================================================
// <q> XF_USE_MEMORYPOOL_FOR_INITIALEVENTS  -  Enable/Disable the usage of the memory pool to create initial events instead of creating them using heap.

#ifndef XF_USE_MEMORYPOOL_FOR_INITIALEVENTS
    #define XF_USE_MEMORYPOOL_FOR_INITIALEVENTS     1
#endif

//==========================================================
// <q> XF_USE_MEMORYPOOL_FOR_DEFAULTEVENTS  -  Enable/Disable the usage of the memory pool to create default events instead of creating them using heap.

#ifndef XF_USE_MEMORYPOOL_FOR_DEFAULTEVENTS
    #define XF_USE_MEMORYPOOL_FOR_DEFAULTEVENTS     1
#endif


//==========================================================
// <q> XF_HOOK_PREEXECUTE_ENABLED  -  Enable/Disable the usage of HOOK pre-execution

#ifndef XF_HOOK_PREEXECUTE_ENABLED
    #define XF_HOOK_PREEXECUTE_ENABLED              1
#endif

//==========================================================
// <q> XF_HOOK_POSTEXECUTE_ENABLED  -  Enable/Disable the usage of HOOK post-execution

#ifndef XF_HOOK_POSTEXECUTE_ENABLED
    #define XF_HOOK_POSTEXECUTE_ENABLED             0
#endif


//==========================================================
// <q> XF_USE_CODE_EXECUTION_COMPENSATION  -  Enable/Disable the usage of code's compensation when scheduling a timeout

#ifndef XF_USE_CODE_EXECUTION_COMPENSATION
    #define XF_USE_CODE_EXECUTION_COMPENSATION      0
#endif

//==========================================================
// <o> XF_CODE_COMPENSATION_EMPTY_LIST  -  Amount of XF's timer ticks to be cut for code's compensation when timeouts list is empty

#ifndef XF_CODE_COMPENSATION_EMPTY_LIST
    #define XF_CODE_COMPENSATION_EMPTY_LIST         1
#endif

//==========================================================
// <o> XF_CODE_COMPENSATION_NOT_EMPTY_LIST  -  Amount of XF's timer ticks to be cut for code's compensation when timeouts list is NOT empty

#ifndef XF_CODE_COMPENSATION_NOT_EMPTY_LIST
    #define XF_CODE_COMPENSATION_NOT_EMPTY_LIST     3
#endif


//==========================================================
// <q> XF_TIMER_TICK_ON_PIO  -  Toggle PIO_18 to show the timer's clock of the XF

#ifndef XF_TIMER_TICK_ON_PIO
    #define XF_TIMER_TICK_ON_PIO                    0
#endif

//==========================================================
// <o> XF_TIMER_MAX_COUNTER_VAL  -  Defines the max counter value of the XF's timer

#ifndef XF_TIMER_MAX_COUNTER_VAL
    #define XF_TIMER_MAX_COUNTER_VAL                0x00ffffff
#endif

//==========================================================
// </h>


#if (APP_SCHEDULER_ENABLED != 0)
    #include "ble/scheduler.h"
    #define XF_TM_PROTECT_ADD_TM                    0
#else
    #define XF_TM_PROTECT_ADD_TM                    1
#endif // APP_SCHEDULER_ENABLED

#define XF_TIMER_MIN_TIMEOUT_TICKS                  TOOLS_APP_TIMER_MIN_TIMEOUT_TICKS

#endif // XF_CONFIG_H
