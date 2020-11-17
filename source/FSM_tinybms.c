/*
 * fsmMainController.c
 *
 *  Created on: 16 lis 2020
 *      Author: Krzysztof Klimek
 */
#include "FSM_tinybms.h"
#include "APP_config.h"
#include "APP_queues.h"

#define DEFAULT_NODE_ID     0x01

/* Timers */
TimerHandle_t xTimerTinyBmsHandle = NULL;

/* FSM State Functions */
void * vTinyBmsOfflineState(FSM_TinyBms_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue );
void * vTinyBmsIdleState(FSM_TinyBms_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue );
void * vTinyBmsChargingState(FSM_TinyBms_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue );
void * vTinyBmsFullyChargedState(FSM_TinyBms_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue );
void * vTinyBmsDischargingState(FSM_TinyBms_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue );
void * vTinyBmsRegenerationState(FSM_TinyBms_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue );
void * vTinyBmsFaultState(FSM_TinyBms_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue );

/* Private Functions */
void vTinyBmsFSMTaskInit(FSM_TinyBms_Definition_t * const me);
//void vFSMTimerFunctionCallback(TimerHandle_t xTimer);
//void vFSMTimerStart(TimerHandle_t *xTimer, uint32_t uiPeriod);
//void vFSMTimerStop(TimerHandle_t *xTimer);

void vTinyBmsFSMTask(void *pvParameters){
    FSM_TinyBms_Definition_t me;
    xAppMsgBaseType_t xMsg;
    void *(*activeState)(FSM_TinyBms_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue ) = &vTinyBmsOfflineState;
//    void *nextState = NULL;
    void *(*nextState)(FSM_TinyBms_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue ) = NULL;
    me.xEventQueue = xQueueBmsInputSignalHandle;
    me.pxTimer = &xTimerTinyBmsHandle;

    /* Initialization */
    me.xStateVariables.eState=NONE_STATE;
    snprintf( me.xStateVariables.cModuleName, 20, "bms-fsm");
    /* init state variables */
    vTinyBmsFSMTaskInit(&me);

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

void * vTinyBmsOfflineState(FSM_TinyBms_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue ){
    void *state;
    state = NULL;

    switch(pxEventQueue->eSignal){
    case ENTRY_SIG:
        me->xStateVariables.eState = OFFLINE_STATE;
        //request bms version
//        vFSMTimerStart(me->pxTimer, pdMS_TO_TICKS(1000));   //check every 1s until bms responds
        break;
    case IDLE_SIG:
        state = &vTinyBmsIdleState;
        break;
    case CHARGING_SIG:
        state = &vTinyBmsChargingState;
        break;
    case FULLY_CHARGED_SIG:
        state = &vTinyBmsFullyChargedState;
        break;
    case DISCHARGING_SIG:
        state = &vTinyBmsDischargingState;
        break;
    case REGENERATION_SIG:
        state = &vTinyBmsRegenerationState;
        break;
    case FAULT_SIG:
        state = &vTinyBmsFaultState;
        break;
    case GET_STATUS_SIG:
        xQueueSend(pxEventQueue->pxReturnQueue, &me->xStateVariables, 0);
        break;
    case EXIT_SIG:
//        vFSMTimerStop(me->pxTimer);
    default:
        break;
    }

    return state;
}
/*-----------------------------------------------------------*/

void * vTinyBmsIdleState(FSM_TinyBms_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue ){
    void *state;
    state = NULL;

    switch(pxEventQueue->eSignal){
    case ENTRY_SIG:
        me->xStateVariables.eState = IDLE_STATE;
        //request bms live data
//        vFSMTimerStart(me->pxTimer, pdMS_TO_TICKS(1000));   //check every 1s
        break;
    case OFFLINE_SIG:
        state = &vTinyBmsOfflineState;
        break;
    case CHARGING_SIG:
        state = &vTinyBmsChargingState;
        break;
    case FULLY_CHARGED_SIG:
        state = &vTinyBmsFullyChargedState;
        break;
    case DISCHARGING_SIG:
        state = &vTinyBmsDischargingState;
        break;
    case REGENERATION_SIG:
        state = &vTinyBmsRegenerationState;
        break;
    case FAULT_SIG:
        state = &vTinyBmsFaultState;
        break;
    case GET_STATUS_SIG:
        xQueueSend(pxEventQueue->pxReturnQueue, &me->xStateVariables, 0);
        break;
    case EXIT_SIG:
//        vFSMTimerStop(me->pxTimer);
    default:
        break;
    }

    return state;
}
/*-----------------------------------------------------------*/

void * vTinyBmsChargingState(FSM_TinyBms_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue ){
    void *state;
    state = NULL;

    switch(pxEventQueue->eSignal){
    case ENTRY_SIG:
        me->xStateVariables.eState = CHARGING_STATE;
        //request bms live data
//        vFSMTimerStart(me->pxTimer, pdMS_TO_TICKS(1000));   //check every 1s
        break;
    case OFFLINE_SIG:
        state = &vTinyBmsOfflineState;
        break;
    case IDLE_SIG:
        state = &vTinyBmsIdleState;
        break;
    case FULLY_CHARGED_SIG:
        state = &vTinyBmsFullyChargedState;
        break;
    case DISCHARGING_SIG:
        state = &vTinyBmsDischargingState;
        break;
    case REGENERATION_SIG:
        state = &vTinyBmsRegenerationState;
        break;
    case FAULT_SIG:
        state = &vTinyBmsFaultState;
        break;
    case GET_STATUS_SIG:
        xQueueSend(pxEventQueue->pxReturnQueue, &me->xStateVariables, 0);
        break;
    case EXIT_SIG:
//        vFSMTimerStop(me->pxTimer);
    default:
        break;
    }

    return state;
}
/*-----------------------------------------------------------*/

void * vTinyBmsFullyChargedState(FSM_TinyBms_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue ){
    void *state;
    state = NULL;

    switch(pxEventQueue->eSignal){
    case ENTRY_SIG:
        me->xStateVariables.eState = FULLY_CHARGED_STATE;
        //request bms live data
//        vFSMTimerStart(me->pxTimer, pdMS_TO_TICKS(1000));   //check every 5s
        break;
    case OFFLINE_SIG:
        state = &vTinyBmsOfflineState;
        break;
    case IDLE_SIG:
        state = &vTinyBmsIdleState;
        break;
    case CHARGING_SIG:
        state = &vTinyBmsChargingState;
        break;
    case DISCHARGING_SIG:
        state = &vTinyBmsDischargingState;
        break;
    case REGENERATION_SIG:
        state = &vTinyBmsRegenerationState;
        break;
    case FAULT_SIG:
        state = &vTinyBmsFaultState;
        break;
    case GET_STATUS_SIG:
        xQueueSend(pxEventQueue->pxReturnQueue, &me->xStateVariables, 0);
        break;
    case EXIT_SIG:
//        vFSMTimerStop(me->pxTimer);
    default:
        break;
    }

    return state;
}
/*-----------------------------------------------------------*/

void * vTinyBmsDischargingState(FSM_TinyBms_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue ){
    void *state;
    state = NULL;

    switch(pxEventQueue->eSignal){
    case ENTRY_SIG:
        me->xStateVariables.eState = DISCHARGING_STATE;
        //request bms live data
//        vFSMTimerStart(me->pxTimer, pdMS_TO_TICKS(100));   //check every 100ms
        break;
    case OFFLINE_SIG:
        state = &vTinyBmsOfflineState;
        break;
    case IDLE_SIG:
        state = &vTinyBmsIdleState;
        break;
    case CHARGING_SIG:
        state = &vTinyBmsChargingState;
        break;
    case FULLY_CHARGED_SIG:
        state = &vTinyBmsFullyChargedState;
        break;
    case REGENERATION_SIG:
        state = &vTinyBmsRegenerationState;
        break;
    case FAULT_SIG:
        state = &vTinyBmsFaultState;
        break;
    case GET_STATUS_SIG:
        xQueueSend(pxEventQueue->pxReturnQueue, &me->xStateVariables, 0);
        break;
    case EXIT_SIG:
//        vFSMTimerStop(me->pxTimer);
    default:
        break;
    }

    return state;
}
/*-----------------------------------------------------------*/

void * vTinyBmsRegenerationState(FSM_TinyBms_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue ){
    void *state;
    switch(pxEventQueue->eSignal){
    case ENTRY_SIG:
        me->xStateVariables.eState = REGENERATION_STATE;
        //request bms live data
//        vFSMTimerStart(me->pxTimer, pdMS_TO_TICKS(1000));   //check every 100ms
        break;
    case OFFLINE_SIG:
        state = &vTinyBmsOfflineState;
        break;
    case IDLE_SIG:
        state = &vTinyBmsIdleState;
        break;
    case CHARGING_SIG:
        state = &vTinyBmsChargingState;
        break;
    case FULLY_CHARGED_SIG:
        state = &vTinyBmsFullyChargedState;
        break;
    case DISCHARGING_SIG:
        state = &vTinyBmsDischargingState;
        break;
    case FAULT_SIG:
        state = &vTinyBmsFaultState;
        break;
    case GET_STATUS_SIG:
        xQueueSend(pxEventQueue->pxReturnQueue, &me->xStateVariables, 0);
        break;
    case EXIT_SIG:
//        vFSMTimerStop(me->pxTimer);
    default:
        break;
    }

    return state;
}
/*-----------------------------------------------------------*/

void * vTinyBmsFaultState(FSM_TinyBms_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue ){
    void *state;
    switch(pxEventQueue->eSignal){
    case ENTRY_SIG:
        me->xStateVariables.eState = FAULT_STATE;
//        vFSMTimerStart(me->pxTimer, pdMS_TO_TICKS(1000));   //check every 5s
        break;
    case OFFLINE_SIG:
        state = &vTinyBmsOfflineState;
        break;
    case IDLE_SIG:
        state = &vTinyBmsIdleState;
        break;
    case CHARGING_SIG:
        state = &vTinyBmsChargingState;
        break;
    case FULLY_CHARGED_SIG:
        state = &vTinyBmsFullyChargedState;
        break;
    case DISCHARGING_SIG:
        state = &vTinyBmsDischargingState;
        break;
    case REGENERATION_SIG:
        state = &vTinyBmsRegenerationState;
        break;
    case GET_STATUS_SIG:
        xQueueSend(pxEventQueue->pxReturnQueue, &me->xStateVariables, 0);
        break;
    case EXIT_SIG:
//        vFSMTimerStop(me->pxTimer);
    default:
        break;
    }

    return state;
}
/*-----------------------------------------------------------*/

void vTinyBmsFSMTaskInit(FSM_TinyBms_Definition_t * const me){
    BaseType_t i;
    for(i = 0; i<NUMBER_OF_CELLS; i++){
        me->xStateVariables.xBmsLiveData.uiCellVoltages[i]=0;       // reg: 0-15; res: 0.1mV
    }

    me->xStateVariables.xBmsLiveData.uiLifetimeCounter=0;
    me->xStateVariables.xBmsLiveData.uiTimeLeft=0;
    me->xStateVariables.xBmsLiveData.fBatteryPackVoltage=0.0;
    me->xStateVariables.xBmsLiveData.fBatteryPackCurrent=0.0;
    me->xStateVariables.xBmsLiveData.uiBatteryMinCellVoltage=0;
    me->xStateVariables.xBmsLiveData.uiBatteryMaxCellVoltage=0;
    me->xStateVariables.xBmsLiveData.iExtSensorTemp1=0;
    me->xStateVariables.xBmsLiveData.iExtSensorTemp2=0;
    me->xStateVariables.xBmsLiveData.uiDistanceLeftToEmptyBat=0;
    me->xStateVariables.xBmsLiveData.uiStateOfCharge=0;
    me->xStateVariables.xBmsLiveData.iTempInternal=0;
    me->xStateVariables.xBmsLiveData.eOnlineStatus=0;
    me->xStateVariables.xBmsLiveData.uiBalancingDecisionBits=0;
    me->xStateVariables.xBmsLiveData.uiRealBalancingBits=0;
    me->xStateVariables.xBmsLiveData.uiNoDetectedCells=0;
    me->xStateVariables.xBmsLiveData.fSpeed=0.0;

    me->xStateVariables.xBmsSettings.uiFullyChargedVoltage=0;
    me->xStateVariables.xBmsSettings.uiFullyDischargedVoltage=0;
    me->xStateVariables.xBmsSettings.uiEarlyBalancingThreshold=0;
    me->xStateVariables.xBmsSettings.uiChargeFinishedCurrent=0;
    me->xStateVariables.xBmsSettings.uiBatteryCapacity=0;
    me->xStateVariables.xBmsSettings.uiNumberOfSeriesCells=0;
    me->xStateVariables.xBmsSettings.uiAllowedDisbalance=0;
    me->xStateVariables.xBmsSettings.uiPulsesPerUnit=0;
    me->xStateVariables.xBmsSettings.uiDistanceUnitName=0;
    me->xStateVariables.xBmsSettings.uiOverVoltageCutoff=0;
    me->xStateVariables.xBmsSettings.uiUnderVoltageCutoff=0;
    me->xStateVariables.xBmsSettings.uiDischargeOverCurrentCutoff=0;
    me->xStateVariables.xBmsSettings.uiChargeOverCurrentCutoff=0;
    me->xStateVariables.xBmsSettings.iOverHeatCutoff=0;
    me->xStateVariables.xBmsSettings.iLowTempChargerCutoff=0;
}

//void vFSMTimerFunctionCallback(TimerHandle_t xTimer){
//    gioToggleBit(gioPORTB, 7);
//}
///*-----------------------------------------------------------*/
//
//void vFSMTimerStart(TimerHandle_t *xTimer, uint32_t uiPeriod){
//    //create software timer
//    if((*xTimer) == NULL){
//        (*xTimer) = xTimerCreate("FSM-TIMER", uiPeriod, pdTRUE, NULL, vFSMTimerFunctionCallback);
//        xTimerStart((*xTimer), portMAX_DELAY);
//    }else{
//        if(xTimerIsTimerActive((*xTimer)) == pdFALSE)
//        xTimerStart((*xTimer), portMAX_DELAY);
//    }
//}
///*-----------------------------------------------------------*/
//
//void vFSMTimerStop(TimerHandle_t *xTimer){
//    if((*xTimer) != NULL && xTimerIsTimerActive(xTimer) == pdTRUE)
//    xTimerStop((*xTimer), portMAX_DELAY);
//}
///*-----------------------------------------------------------*/

