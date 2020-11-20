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

#define CAN_DATA_MAX_SIZE   8

typedef struct {
    QueueHandle_t *pxReturnQueue;
    ESignal eSignal;
} xAppMsgBaseType_t;

typedef struct {
    xAppMsgBaseType_t xBase;
    uint8_t uiData[CAN_DATA_MAX_SIZE];
    uint8_t uiDataLength;
} xAppMsgCANType_t;

/* Queue handlers */
extern xQueueHandle xQueueCtrlInputSignalHandle;
extern xQueueHandle xQueueBmsInputSignalHandle;

extern xQueueHandle xQueueRestAPICtrlResponseHandle;
extern xQueueHandle xQueueCLICtrlResponseHandle;

extern xQueueHandle xQueueRestAPIBmsResponseHandle;
extern xQueueHandle xQueueCLIBmsResponseHandle;

#endif /* INCLUDE_APP_QUEUES_H_ */
