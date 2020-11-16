/*
 * FSM_MainController.h
 *
 *  Created on: 16 lis 2020
 *      Author: Krzysztof Klimek
 */

#ifndef FSM_TINYBMS_H_
#define FSM_TINYBMS_H_

#include "math.h"
#include "stdio.h"
#include "stdlib.h"

/* HALCoGen generated headers has moved to sys_main.h*/
#include "sys_main.h"

/* Time related functions */
#include "rti_runtimestats.h"

#include "FreeRTOS.h"
#include "os_task.h"
#include "os_queue.h"
#include "os_timer.h"
#include "os_semphr.h"

#include "APP_config.h"
#include "APP_queues.h"

/* FSM Task */
void vTinyBmsFSMTask(void *pvParameters);

/* The structure that defines FSM structure */
typedef struct
{
    QueueHandle_t xEventQueue;
    TimerHandle_t *pxTimer;
    xBmsStateVariables_t xStateVariables;
} FSM_TinyBms_Definition_t;

#endif /* FSM_MAINCONTROLLER_H_ */
