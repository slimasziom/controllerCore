/*
 * FSM_MainController.h
 *
 *  Created on: 16 lis 2020
 *      Author: Krzysztof Klimek
 */

#ifndef FSM_MOTOR_CONTROLLER_H_
#define FSM_MOTOR_CONTROLLER_H_

#include "math.h"
#include "stdio.h"
#include "stdlib.h"
#include "HL_can.h"

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
void vMotorControllerFSMTask(void *pvParameters);
const char * pcMotorMode(uint8_t uiMotorMode);

typedef struct {
    uint8_t uiMode;
    const char *cMode;
} xMotorModeMapping_t;

extern xMotorModeMapping_t xMotorModeMapping[];

/* The structure that defines FSM structure */
typedef struct
{
    QueueHandle_t xEventQueue;
    TimerHandle_t *pxTimer;
    xMotorControllerStateVariables_t xStateVariables;
    canBASE_t *pCanNode;
    uint32_t uiCanID;
    QueueHandle_t xCANQueue;
} FSM_MotorController_Definition_t;

#endif /* FSM_MAINCONTROLLER_H_ */
