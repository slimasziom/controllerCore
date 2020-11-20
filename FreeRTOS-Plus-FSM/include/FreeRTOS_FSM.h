/*
 * FreeRTOS_FSM.h
 *
 *  Created on: 12 lis 2020
 *      Author: Krzysztof Klimek
 */

#ifndef FREERTOS_FSM_H_
#define FREERTOS_FSM_H_

#include "FreeRTOS.h"
#include "os_queue.h"

/*
 * The prototype to state function
 */
typedef portBASE_TYPE (*pdFSM_STATE_CALLBACK)(QueueHandle_t *pxEventQueue );

/* The structure that defines FSM structure */
typedef struct
{
    QueueHandle_t *pxEventQueue;                    /* Queue handler for events */
    const pdFSM_STATE_CALLBACK pxActiveState;       /* A pointer to FSM state functions */
} FSM_Definition_t;

#endif /* FREERTOS_FSM_H_ */
