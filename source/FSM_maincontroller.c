/*
 * fsmMainController.c
 *
 *  Created on: 16 lis 2020
 *      Author: Krzysztof Klimek
 */
#include "FSM_maincontroller.h"

#include "APP_config.h"
#include "APP_queues.h"

/* Timers */
TimerHandle_t xTimerMainControllerHandle = NULL;

/* Private Functions */
void vFSMTimerFunctionCallback(TimerHandle_t xTimer);
void vFSMTimerStart(TimerHandle_t *xTimer, uint32_t uiPeriod);
void vFSMTimerStop(TimerHandle_t *xTimer);

void vMainControllerFSMTask(void *pvParameters){
    FSM_MainController_Definition_t me;
    xAppMsgBaseType_t xMsg;
    void *(*activeState)(FSM_MainController_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue ) = &vMainControllerIdleState;
    me.xStateVariables.eState=NONE_STATE;

//    void *nextState = NULL;
    void *(*nextState)(FSM_MainController_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue ) = NULL;
    /* Initialization */
    me.xStateVariables.xSettings.uiPower=93;
    me.xStateVariables.xSettings.bOffset= true;
    me.xStateVariables.xSettings.xOffsetSettings.uiPar_a=32;
    me.xStateVariables.xSettings.xOffsetSettings.uiPar_b=64;
    me.xStateVariables.xSettings.xOffsetSettings.uiPar_c=12;
    snprintf( me.xStateVariables.cModuleName, 20, "main-controller-fsm");
    snprintf( me.xStateVariables.xSettings.xOffsetSettings.cOffsetType, 20, "default");

    me.xEventQueue = xQueueCtrlInputSignalHandle;
    me.pxTimer = &xTimerMainControllerHandle;

    /* init first state */
    xMsg.eSignal = ENTRY_SIG;
    (*activeState)(&me, &xMsg);

    /* FSM execution */
    while(1)
        {
            /* controller logic here */
            if (xQueueReceive(me.xEventQueue, &xMsg, 0) == pdTRUE)
            {
                nextState = (*activeState)(&me, &xMsg);     //TODO: remove warning
                if (nextState != NULL){
                    xMsg.eSignal = EXIT_SIG;
                    (*activeState)(&me, &xMsg);
                    activeState = nextState;
                    xMsg.eSignal = ENTRY_SIG;
                    (*activeState)(&me, &xMsg);
                }
            }
        /* 10Hz refresh rate */
        vTaskDelay(pdMS_TO_TICKS(100));
        }
}
/*-----------------------------------------------------------*/

void * vMainControllerIdleState(FSM_MainController_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue ){
    void *state;
    switch(pxEventQueue->eSignal){
    case ENTRY_SIG:
        me->xStateVariables.eState = STOP_STATE;
        break;
    case RUN_SIG:
    case SHORT_PRESS_SIG:
        state = &vMainControllerStartState;
        break;
    case EMERGENCY_SIG:
        state = &vMainControllerEmergencyState;
        break;
    case GET_STATUS_SIG:
        xQueueSend(pxEventQueue->pxReturnQueue, &me->xStateVariables, 0);
        state = NULL;
        break;
    default:
        state = NULL;
        break;
    }

    return state;
}
/*-----------------------------------------------------------*/

void * vMainControllerStartState(FSM_MainController_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue ){
    void *state;
    switch(pxEventQueue->eSignal){
    case ENTRY_SIG:
        me->xStateVariables.eState = RUNNING_STATE;
        gioSetBit(gioPORTB, 7, 1);
        state = NULL;
        break;
    case STOP_SIG:
    case SHORT_PRESS_SIG:
        state = &vMainControllerIdleState;
        break;
    case PAUSE_SIG:
        state = &vMainControllerPauseState;
        break;
    case EMERGENCY_SIG:
        state = &vMainControllerEmergencyState;
        break;
    case GET_STATUS_SIG:
        xQueueSend(pxEventQueue->pxReturnQueue, &me->xStateVariables, 0);
        state = NULL;
        break;
    case EXIT_SIG:
        gioSetBit(gioPORTB, 7, 0);
        state = NULL;
        break;
    default:
        state = NULL;
        break;
    }

    return state;
}
/*-----------------------------------------------------------*/

void * vMainControllerEmergencyState(FSM_MainController_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue ){
    void *state;
    switch(pxEventQueue->eSignal){
    case ENTRY_SIG:
        me->xStateVariables.eState = EMERGENCY_STATE;
        break;
    case RUN_SIG:
    case SHORT_PRESS_SIG:
        state = &vMainControllerStartState;
        break;
    case STOP_SIG:
        state = &vMainControllerIdleState;
        break;
    case GET_STATUS_SIG:
        xQueueSend(pxEventQueue->pxReturnQueue, &me->xStateVariables, 0);
        state = NULL;
        break;
    default:
        state = NULL;
        break;
    }

    return state;
}
/*-----------------------------------------------------------*/

void * vMainControllerPauseState(FSM_MainController_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue ){
    void *state;
    switch(pxEventQueue->eSignal){
    case ENTRY_SIG:
        me->xStateVariables.eState = PAUSED_STATE;
        vFSMTimerStart(me->pxTimer, pdMS_TO_TICKS(500));
        break;
    case RUN_SIG:
    case SHORT_PRESS_SIG:
        state = &vMainControllerStartState;
        break;
    case STOP_SIG:
        state = &vMainControllerIdleState;
        break;
    case EMERGENCY_SIG:
        state = &vMainControllerEmergencyState;
        break;
    case GET_STATUS_SIG:
        xQueueSend(pxEventQueue->pxReturnQueue, &me->xStateVariables, 0);
        state = NULL;
        break;
    case EXIT_SIG:
        vFSMTimerStop(me->pxTimer);
        gioSetBit(gioPORTB, 7, 0);
    default:
        state = NULL;
        break;
    }
    return state;
}
/*-----------------------------------------------------------*/

void vFSMTimerFunctionCallback(TimerHandle_t xTimer){
    gioToggleBit(gioPORTB, 7);
}
/*-----------------------------------------------------------*/

void vFSMTimerStart(TimerHandle_t *xTimer, uint32_t uiPeriod){
    //create software timer
    if((*xTimer) == NULL){
        (*xTimer) = xTimerCreate("FSM-TIMER", uiPeriod, pdTRUE, NULL, vFSMTimerFunctionCallback);
        xTimerStart((*xTimer), portMAX_DELAY);
    }else{
        if(xTimerIsTimerActive((*xTimer)) == pdFALSE)
        xTimerStart((*xTimer), portMAX_DELAY);
    }
}
/*-----------------------------------------------------------*/

void vFSMTimerStop(TimerHandle_t *xTimer){
    if((*xTimer) != NULL && xTimerIsTimerActive(xTimer) == pdTRUE)
    xTimerStop((*xTimer), portMAX_DELAY);
}
/*-----------------------------------------------------------*/

