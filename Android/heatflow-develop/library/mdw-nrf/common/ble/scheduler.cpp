/************************************************************************************************************
 * Copyright (C) Hes-so VALAIS/WALLIS, HEI Sion, Infotronics. 2019
 * Created by Patrice Rudaz
 * All rights reserved.
 ************************************************************************************************************/

/************************************************************************************************************/
/* Header files                                                                                             */
/************************************************************************************************************/
#include "scheduler.h"
#include <ble-scheduler-config.h>

#if defined(SOFTDEVICE_PRESENT)
#include <app_scheduler.h>


/************************************************************************************************************/
/* DEBUG COMPILATION OPTION & MACRO                                                                         */
/************************************************************************************************************/
#define DEBUG_SCHEDULER_ENABLE              0
#if (DEBUG_SCHEDULER_ENABLE != 0) && defined(DEBUG_NRF_USER)
    #include <tools/logtrace.h>
    #define SCH_DEBUG                       LOG_TRACE_DEBUG
#else
    #define SCH_DEBUG(...)                  {(void) (0);}
#endif


//------------------------------------------------------------------------------------------------------------
// Set this define to 1 to enable the check of the size of the queue used in `app_scheduler`...
#define CHECK_SCHEDULER_QUEUE_SIZE          0


/************************************************************************************************************/
/* Namespace's Declaration                                                                                  */
/************************************************************************************************************/
using ble::Scheduler;
    

/************************************************************************************************************/
/* PUBLIC SECTION                                                                                           */
/************************************************************************************************************/
//------------------------------------------------------------------------------------------------------------
void Scheduler::init()
{
    SCH_DEBUG("[Scheduler] init() > Calling `APP_SCHED_INIT()` ... ");
    APP_SCHED_INIT(BLE_SCHEDULER_MAX_EVT_SIZE, SCHED_QUEUE_SIZE);
    SCH_DEBUG("[Done]\r\n");
}

//------------------------------------------------------------------------------------------------------------
void Scheduler::schedule()
{
    // Check scheduler queue and processes events
#if defined(CHECK_SCHEDULER_QUEUE_SIZE) && (CHECK_SCHEDULER_QUEUE_SIZE != 0)
    uint16_t schedulerQueueFreeSpace;
    schedulerQueueFreeSpace = app_sched_queue_space_get();
    LOG_TRACE_DEBUG("[Scheduler] schedule() > `app_sched_execute()` [in]  (free space: %d)\r\n", schedulerQueueFreeSpace);

    if(schedulerQueueFreeSpace == 0) {
        LOG_TRACE_DEBUG("[Scheduler] schedule() > You should consider incrementing the size of the queue !!!\r\n");
        ASSERT(false);
    }
#else
    SCH_DEBUG("[Scheduler] schedule() > `app_sched_execute()` [in] ...\r\n");
#endif
    app_sched_execute();
    SCH_DEBUG("[Scheduler] schedule() > `app_sched_execute()` [out] (free space: %d)\r\n", app_sched_queue_space_get());
}

#endif  // #if defined(SOFTDEVICE_PRESENT)
