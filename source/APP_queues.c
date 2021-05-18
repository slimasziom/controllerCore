/*
 * APP_queues.c
 *
 *  Created on: 5 lis 2020
 *      Author: Win 7 PL T430
 */

#include "APP_queues.h"

xQueueHandle xQueueCtrlInputSignalHandle;

xQueueHandle xQueueBmsInputSignalHandle;

xQueueHandle xQueueMotorCtrlInputSignalHandle;

xQueueHandle xQueueRestAPICtrlResponseHandle;

xQueueHandle xQueueCLICtrlResponseHandle;

xQueueHandle xQueueRestAPIBmsResponseHandle;

xQueueHandle xQueueCLIBmsResponseHandle;

xQueueHandle xQueueRestAPIEmusBmsResponseHandle;

xQueueHandle xQueueCLIEmusBmsResponseHandle;

xQueueHandle xQueueBmsCANResponseHandle;

xQueueHandle xQueueMotorCANResponseHandle;

xQueueHandle xQueueRestAPIMotorCtrlResponseHandle;

xQueueHandle xQueueCLIMotorCtrlResponseHandle;

