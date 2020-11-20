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
//void * vTinyBmsOfflineState(FSM_TinyBms_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue );
//void * vTinyBmsIdleState(FSM_TinyBms_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue );
//void * vTinyBmsChargingState(FSM_TinyBms_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue );
//void * vTinyBmsFullyChargedState(FSM_TinyBms_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue );
//void * vTinyBmsDischargingState(FSM_TinyBms_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue );
//void * vTinyBmsRegenerationState(FSM_TinyBms_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue );
//void * vTinyBmsFaultState(FSM_TinyBms_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue );

void * vTinyBmsOfflineState(FSM_TinyBms_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue );
void * vTinyBmsGoOnlineState(FSM_TinyBms_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue );
void * vTinyBmsIdleState(FSM_TinyBms_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue );
void * vTinyBmsReadRegistersState(FSM_TinyBms_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue );
void * vTinyBmsWriteRegistersState(FSM_TinyBms_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue );

void vSendCANFrame(FSM_TinyBms_Definition_t * const me, uint8_t uiDataLen, uint8_t *uiData);
void vReadRegisterBlock(FSM_TinyBms_Definition_t * const me, uint16_t uiADDR, uint8_t uiRL);
void vWriteRegisterBlock(FSM_TinyBms_Definition_t * const me, uint16_t uiADDR, uint8_t uiRL, uint8_t *uiData, uint8_t uiDataLength);
void vReadResponseBlock(FSM_TinyBms_Definition_t * const me, uint8_t *uiData, uint8_t uiDataLength);

BaseType_t ReadBatteryPackVoltage(FSM_TinyBms_Definition_t * const me);
BaseType_t ReadBatteryPackCurrent(FSM_TinyBms_Definition_t * const me);
BaseType_t ReadBatteryPackMaxCellVoltage(FSM_TinyBms_Definition_t * const me);
BaseType_t ReadBatteryPackMinCellVoltage(FSM_TinyBms_Definition_t * const me);
BaseType_t ReadTinyBmsOnlineStatus(FSM_TinyBms_Definition_t * const me);


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
    me.pCanNode = canREG1;

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
/*-----------------------------------------------------------*/

void * vTinyBmsOfflineState(FSM_TinyBms_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue ){
    void *state;
    state = NULL;

    switch(pxEventQueue->eSignal){
    case ENTRY_SIG:
        me->xStateVariables.eState = OFFLINE_STATE;
        //start timer
//        vFSMTimerStart(me->pxTimer, pdMS_TO_TICKS(1000));   //check every 1s
        break;
    case GO_ONLINE_SIG:
    case TIMEOUT_SIG:
        state = &vTinyBmsGoOnlineState;
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

void * vTinyBmsGoOnlineState(FSM_TinyBms_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue ){
    void *state;
    state = NULL;

    switch(pxEventQueue->eSignal){
    case ENTRY_SIG:
    case GO_ONLINE_SIG:
        me->xStateVariables.eState = GOING_ONLINE_STATE;
        if( ReadTinyBmsOnlineStatus(me) == pdFALSE)
        {
            state = &vTinyBmsReadRegistersState;
        }
        else
        {
            state = &vTinyBmsOfflineState;
        }
        //start timer
        break;
    case OFFLINE_SIG:
        state = &vTinyBmsOfflineState;
        break;
    case TIMEOUT_SIG:
        state = &vTinyBmsOfflineState;
        break;
    case GET_STATUS_SIG:
        xQueueSend(pxEventQueue->pxReturnQueue, &me->xStateVariables, 0);
        break;
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
        break;
    case OFFLINE_SIG:
        state = &vTinyBmsOfflineState;
        break;
    case TIMEOUT_SIG:
    case READ_SIG:
        state = &vTinyBmsReadRegistersState;
        break;
    case WRITE_SIG:
        state = &vTinyBmsWriteRegistersState;
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

void * vTinyBmsReadRegistersState(FSM_TinyBms_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue ){
    void *state;
    state = NULL;

    switch(pxEventQueue->eSignal){
    case ENTRY_SIG:
        me->xStateVariables.eState = READING_STATE;
        break;
    case OFFLINE_SIG:
        state = &vTinyBmsOfflineState;
        break;
    case IDLE_SIG:
        state = &vTinyBmsIdleState;
        break;
    case WRITE_SIG:
        state = &vTinyBmsWriteRegistersState;
        break;
    case TIMEOUT_SIG:
        //either read all or one by one to not to block the rest of fsm
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

void * vTinyBmsWriteRegistersState(FSM_TinyBms_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue ){
    void *state;
    state = NULL;

    switch(pxEventQueue->eSignal){
    case ENTRY_SIG:
        me->xStateVariables.eState = WRITING_STATE;
        break;
    case OFFLINE_SIG:
        state = &vTinyBmsOfflineState;
        break;
    case IDLE_SIG:
        state = &vTinyBmsIdleState;
        break;
    case READ_SIG:
        //verify how get responses from bsp if message was ok
        state = &vTinyBmsReadRegistersState;
        break;
    case TIMEOUT_SIG:
        //verify how get responses from bsp if message was ok
        state = &vTinyBmsReadRegistersState;
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

//void * vTinyBmsRegenerationState(FSM_TinyBms_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue ){
//    void *state;
//    switch(pxEventQueue->eSignal){
//    case ENTRY_SIG:
//        me->xStateVariables.eState = REGENERATION_STATE;
//        //request bms live data
////        vFSMTimerStart(me->pxTimer, pdMS_TO_TICKS(1000));   //check every 100ms
//        break;
//    case OFFLINE_SIG:
//        state = &vTinyBmsOfflineState;
//        break;
//    case IDLE_SIG:
//        state = &vTinyBmsIdleState;
//        break;
//    case CHARGING_SIG:
//        state = &vTinyBmsChargingState;
//        break;
//    case FULLY_CHARGED_SIG:
//        state = &vTinyBmsFullyChargedState;
//        break;
//    case DISCHARGING_SIG:
//        state = &vTinyBmsDischargingState;
//        break;
//    case FAULT_SIG:
//        state = &vTinyBmsFaultState;
//        break;
//    case GET_STATUS_SIG:
//        xQueueSend(pxEventQueue->pxReturnQueue, &me->xStateVariables, 0);
//        break;
//    case EXIT_SIG:
////        vFSMTimerStop(me->pxTimer);
//    default:
//        break;
//    }
//
//    return state;
//}
///*-----------------------------------------------------------*/
//
//void * vTinyBmsFaultState(FSM_TinyBms_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue ){
//    void *state;
//    switch(pxEventQueue->eSignal){
//    case ENTRY_SIG:
//        me->xStateVariables.eState = FAULT_STATE;
////        vFSMTimerStart(me->pxTimer, pdMS_TO_TICKS(1000));   //check every 5s
//        break;
//    case OFFLINE_SIG:
//        state = &vTinyBmsOfflineState;
//        break;
//    case IDLE_SIG:
//        state = &vTinyBmsIdleState;
//        break;
//    case CHARGING_SIG:
//        state = &vTinyBmsChargingState;
//        break;
//    case FULLY_CHARGED_SIG:
//        state = &vTinyBmsFullyChargedState;
//        break;
//    case DISCHARGING_SIG:
//        state = &vTinyBmsDischargingState;
//        break;
//    case REGENERATION_SIG:
//        state = &vTinyBmsRegenerationState;
//        break;
//    case GET_STATUS_SIG:
//        xQueueSend(pxEventQueue->pxReturnQueue, &me->xStateVariables, 0);
//        break;
//    case EXIT_SIG:
////        vFSMTimerStop(me->pxTimer);
//    default:
//        break;
//    }
//
//    return state;
//}
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

void vSendCANFrame(FSM_TinyBms_Definition_t * const me, uint8_t uiDataLen, uint8_t *uiData){
    BSP_bmsCanSend(me->pCanNode, me->uiCanID, uiDataLen, uiData);
}
/*-----------------------------------------------------------*/

void vReadRegisterBlock(FSM_TinyBms_Definition_t * const me, uint16_t uiADDR, uint8_t uiRL){

}
/*-----------------------------------------------------------*/

void vWriteRegisterBlock(FSM_TinyBms_Definition_t * const me, uint16_t uiADDR, uint8_t uiRL, uint8_t *uiData, uint8_t uiDataLength){

}
/*-----------------------------------------------------------*/

void vReadResponseBlock(FSM_TinyBms_Definition_t * const me, uint8_t *uiData, uint8_t uiDataLength){

}
/*-----------------------------------------------------------*/
BaseType_t ReadBatteryPackVoltage(FSM_TinyBms_Definition_t * const me){
    BaseType_t xReturn;

    xAppMsgCANType_t xCANMsg;
    uint8_t puiDataBytes[] = {0x14};

    xReturn = pdTRUE;

    //send request for bms state: 2.1.10 in 'TinyBMS_Communication_Protocols.pdf'
    vSendCANFrame(me, sizeof(puiDataBytes), puiDataBytes);

    if (xQueueReceive(xQueueBmsCANResponseHandle, &xCANMsg, 500) == pdTRUE){
        if (xCANMsg.uiData[0] == 0x01 && xCANMsg.uiData[1] == 0x14 )
        {
//            me->xStateVariables.xBmsLiveData.fBatteryPackVoltage = (float_t)((xCANMsg.uiData[5] << 24) + (xCANMsg.uiData[4] << 16) + (xCANMsg.uiData[3] << 8) + xCANMsg.uiData[2]);
            memcpy (&me->xStateVariables.xBmsLiveData.fBatteryPackVoltage, xCANMsg.uiData + 2, sizeof (float));
            xReturn = pdFALSE;
        }
    }
    return xReturn;
}
/*-----------------------------------------------------------*/

BaseType_t ReadBatteryPackCurrent(FSM_TinyBms_Definition_t * const me){
    BaseType_t xReturn;

    xAppMsgCANType_t xCANMsg;
    uint8_t puiDataBytes[] = {0x15};

    xReturn = pdTRUE;

    //send request for bms state: 2.1.10 in 'TinyBMS_Communication_Protocols.pdf'
    vSendCANFrame(me, sizeof(puiDataBytes), puiDataBytes);

    if (xQueueReceive(xQueueBmsCANResponseHandle, &xCANMsg, 500) == pdTRUE){
        if (xCANMsg.uiData[0] == 0x01 && xCANMsg.uiData[1] == 0x15 )
        {
//            me->xStateVariables.xBmsLiveData.fBatteryPackVoltage = (float_t)((xCANMsg.uiData[5] << 24) + (xCANMsg.uiData[4] << 16) + (xCANMsg.uiData[3] << 8) + xCANMsg.uiData[2]);
            memcpy (&me->xStateVariables.xBmsLiveData.fBatteryPackCurrent, xCANMsg.uiData + 2, sizeof (float));
            xReturn = pdFALSE;
        }
    }
    return xReturn;
}
/*-----------------------------------------------------------*/

BaseType_t ReadBatteryPackMaxCellVoltage(FSM_TinyBms_Definition_t * const me){
    BaseType_t xReturn;

    xAppMsgCANType_t xCANMsg;
    uint8_t puiDataBytes[] = {0x16};

    xReturn = pdTRUE;

    //send request for bms state: 2.1.10 in 'TinyBMS_Communication_Protocols.pdf'
    vSendCANFrame(me, sizeof(puiDataBytes), puiDataBytes);

    if (xQueueReceive(xQueueBmsCANResponseHandle, &xCANMsg, 500) == pdTRUE){
        if (xCANMsg.uiData[0] == 0x01 && xCANMsg.uiData[1] == 0x16 )
        {
            me->xStateVariables.xBmsLiveData.uiBatteryMaxCellVoltage = (uint16_t)((xCANMsg.uiData[3] << 8) + xCANMsg.uiData[2]);
            xReturn = pdFALSE;
        }
    }
    return xReturn;
}
/*-----------------------------------------------------------*/

BaseType_t ReadBatteryPackMinCellVoltage(FSM_TinyBms_Definition_t * const me){
    BaseType_t xReturn;

    xAppMsgCANType_t xCANMsg;
    uint8_t puiDataBytes[] = {0x17};

    xReturn = pdTRUE;

    //send request for bms state: 2.1.10 in 'TinyBMS_Communication_Protocols.pdf'
    vSendCANFrame(me, sizeof(puiDataBytes), puiDataBytes);

    if (xQueueReceive(xQueueBmsCANResponseHandle, &xCANMsg, 500) == pdTRUE){
        if (xCANMsg.uiData[0] == 0x01 && xCANMsg.uiData[1] == 0x17 )
        {
            me->xStateVariables.xBmsLiveData.uiBatteryMaxCellVoltage = (uint16_t)((xCANMsg.uiData[3] << 8) + xCANMsg.uiData[2]);
            xReturn = pdFALSE;
        }
    }
    return xReturn;
}
/*-----------------------------------------------------------*/

BaseType_t ReadTinyBmsOnlineStatus(FSM_TinyBms_Definition_t * const me){
    BaseType_t xReturn;
    xAppMsgCANType_t xCANMsg;
    uint16_t uiStatus;
    uint8_t puiDataBytes[] = {0x18};

    xReturn = pdTRUE;

    //send request for bms state: 2.1.10 in 'TinyBMS_Communication_Protocols.pdf'
    vSendCANFrame(me, sizeof(puiDataBytes), puiDataBytes);

    if (xQueueReceive(xQueueBmsCANResponseHandle, &xCANMsg, 500) == pdTRUE){
        if (xCANMsg.uiData[0] == 0x01 && xCANMsg.uiData[1] == 0x18 )
        {
            //if status ok, go to read, but first check status
            uiStatus = xCANMsg.uiData[3] * 256 + xCANMsg.uiData[2];
            switch(uiStatus)
            {
            case 0x91:
                me->xStateVariables.xBmsLiveData.eOnlineStatus = CHARGING_STATE;
                break;
            case 0x92:
                me->xStateVariables.xBmsLiveData.eOnlineStatus = FULLY_CHARGED_STATE;
                break;
            case 0x93:
                me->xStateVariables.xBmsLiveData.eOnlineStatus = DISCHARGING_STATE;
                break;
            case 0x96:
                me->xStateVariables.xBmsLiveData.eOnlineStatus = REGENERATION_STATE;
                break;
            case 0x97:
                me->xStateVariables.xBmsLiveData.eOnlineStatus = IDLE_STATE;
                break;
            case 0x9B:
                me->xStateVariables.xBmsLiveData.eOnlineStatus = FAULT_STATE;
                break;
            default:
                me->xStateVariables.xBmsLiveData.eOnlineStatus = NONE_STATE;
                break;
            }
            xReturn = pdFALSE;
        }
    }
    return xReturn;
}
/*-----------------------------------------------------------*/
