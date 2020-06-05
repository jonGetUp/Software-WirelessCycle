/* Copyright (c) 2018, HES-SO Valais
 * All rights reserved.
 */
#ifndef BLE_SCHEDULER_CONFIG_H
#define BLE_SCHEDULER_CONFIG_H


#define SCHED_QUEUE_SIZE				    16												                            ///< \brief Maximum number of events in the scheduler queue.
#define BLE_SCHEDULER_MAX_EVT_SIZE		    MAX(SCHED_QUEUE_SIZE, APP_TIMER_SCHED_EVENT_DATA_SIZE)                      ///< \brief Max of event size given by application and the app_timer library.

#endif  // BLE_SCHEDULER_CONFIG_H
