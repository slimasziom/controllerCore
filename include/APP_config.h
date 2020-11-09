/*
 * APP_config.h
 *
 *  Created on: 5 lis 2020
 *      Author: Krzysztof Klimek
 */

#ifndef APP_CONFIG_H_
#define APP_CONFIG_H_

#include "stdio.h"
#include "stdlib.h"
#include "stdint.h"

#define appRestAPI_BUFFER_SIZE ( 2048 )
#define appCOMMAND_LENGTH (32)

/* Controller input signals */
typedef enum {
    CONTROLLER_NONE_SIG,    // not used, first
    CONTROLLER_START_SIG,
    CONTROLLER_STOP_SIG,
    CONTROLLER_PAUSE_SIG,
    CONTROLLER_EMERGENCY_SIG,
    CONTROLLER_SHORT_PRESS_SIG,
    CONTROLLER_GET_STATUS_REST_SIG,
    CONTROLLER_GET_STATUS_CLI_SIG,
    CONTROLLER_LAST_SIG     // not used, last
} ECtrlInputSignal;

/* Controller states */
typedef enum {
    CONTROLLER_NONE_STATE,    // not used, first
    CONTROLLER_START_STATE,
    CONTROLLER_STOP_STATE,
    CONTROLLER_PAUSE_STATE,
    CONTROLLER_EMERGENCY_STATE,
    CONTROLLER_LAST_STATE     // not used, last
} EControllerState;

/* Controller thread settings struct */
typedef struct {
    uint8_t uiPar_a;
    uint8_t uiPar_b;
    uint8_t uiPar_c;
    char cOffsetType[20];
} xOffset_t;

/* Controller thread settings struct */
typedef struct {
    uint8_t uiPower;
    uint8_t bOffset;
    xOffset_t xOffsetSettings;
} xCtsv_t;

/* Controller thread struct */
typedef struct {
    uint8_t eState;
    char cModuleName[20];
    xCtsv_t xSettings;
} xControllerStateVariables_t;

typedef struct {
    uint8_t eSignal;
    uint8_t eState;
    const char *cCommand;
    const char *cState;
} xThreadMapping_t;

extern xThreadMapping_t xMainControllerMapping[];


/* Move to APP_thread_processing */
uint8_t uiThreadSignalFromCommand(xThreadMapping_t *xThread, char *pcCommandString);
//char * pcStateNameFromThread(uint8_t sState);

#endif /* INCLUDE_APP_CONFIG_H_ */
