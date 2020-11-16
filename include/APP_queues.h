/*
 * APP_queues.h
 *
 *  Created on: 5 lis 2020
 *      Author: Krzysztof Klimek
 */

#ifndef INCLUDE_APP_QUEUES_H_
#define INCLUDE_APP_QUEUES_H_

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "os_queue.h"
#include "APP_config.h"

typedef struct {
    QueueHandle_t *pxReturnQueue;
    ECtrlInputSignal eSignal;
} xAppMsgBaseType_t;

/* Queue handlers */
extern xQueueHandle xQueueCtrlInputSignalHandle;
extern xQueueHandle xQueueBmsInputSignalHandle;
extern xQueueHandle xQueueRestAPIResponseHandle;
extern xQueueHandle xQueueCLIResponseHandle;

#endif /* INCLUDE_APP_QUEUES_H_ */
