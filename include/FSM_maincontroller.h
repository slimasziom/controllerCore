/*
 * FSM_MainController.h
 *
 *  Created on: 16 lis 2020
 *      Author: Krzysztof Klimek
 */

#ifndef FSM_MAINCONTROLLER_H_
#define FSM_MAINCONTROLLER_H_

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
void vMainControllerFSMTask(void *pvParameters);

/* The structure that defines FSM structure */
typedef struct
{
    QueueHandle_t xEventQueue;
    TimerHandle_t *pxTimer;
//    void *pvActiveStateFunction(FSM_MainController_Definition_t * const me, QueueHandle_t *pxEventQueue );
    xControllerStateVariables_t xStateVariables;

} FSM_MainController_Definition_t;

/* FSM State Functions */
void * vMainControllerIdleState(FSM_MainController_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue );
void * vMainControllerStartState(FSM_MainController_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue );
void * vMainControllerEmergencyState(FSM_MainController_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue );
void * vMainControllerPauseState(FSM_MainController_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue );

#endif /* FSM_MAINCONTROLLER_H_ */
