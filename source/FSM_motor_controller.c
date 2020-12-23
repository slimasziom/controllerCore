/*
 * fsmMotorController.c
 *
 *  Created on: 23 Dec 2020
 *      Author: Krzysztof Klimek
 */
#include "FSM_motor_controller.h"
#include "APP_config.h"
#include "APP_queues.h"

#define DEFAULT_NODE_ID     0x04
#define CAN_TIMEOUT         2000U

/* Timers */
TimerHandle_t xTimerMotorControllerHandle = NULL;

void * vMotorControllerOfflineState(FSM_MotorController_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue );
void * vMotorControllerIdleState(FSM_MotorController_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue );

BaseType_t SendSamplingSetup(FSM_MotorController_Definition_t * const me);
BaseType_t ReadRangeAndRef(FSM_MotorController_Definition_t * const me, uint8_t uiDataLen, uint8_t *uiData);
BaseType_t ReadSupply1(FSM_MotorController_Definition_t * const me, uint8_t uiDataLen, uint8_t *uiData);

/* Private Functions */
void vMotorControllerFSMTaskInit(FSM_MotorController_Definition_t * const me);
//void vFSMTimerFunctionCallback(TimerHandle_t xTimer);
//void vFSMTimerStart(TimerHandle_t *xTimer, uint32_t uiPeriod);
//void vFSMTimerStop(TimerHandle_t *xTimer);

void vMotorControllerFSMTask(void *pvParameters){
    FSM_MotorController_Definition_t me;
    xAppMsgBaseType_t xMsg;
    void *(*activeState)(FSM_MotorController_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue ) = &vMotorControllerOfflineState;
//    void *nextState = NULL;
    void *(*nextState)(FSM_MotorController_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue ) = NULL;
    me.xEventQueue = xQueueMotorCtrlInputSignalHandle;
    me.xCANQueue = xQueueMotorCANResponseHandle;
    me.pxTimer = &xTimerMotorControllerHandle;
    me.pCanNode = canREG1;
    me.uiCanID = canMESSAGE_BOX3;

    /* Initialization */
    me.xStateVariables.eState=NONE_STATE;
    snprintf( me.xStateVariables.cModuleName, 20, "motor-ctrl-fsm");
    /* init state variables */
    vMotorControllerFSMTaskInit(&me);

    /* init first state */
    xMsg.eSignal = ENTRY_SIG;
    (*activeState)(&me, &xMsg);

    /* FSM execution */
    while(1)
        {
            /* controller logic here */
            if (xQueueReceive(me.xEventQueue, &xMsg, 0) == pdFALSE){
                xMsg.eSignal = TIMEOUT_SIG;
            }

            nextState = (*activeState)(&me, &xMsg);     //TODO: remove warning
            if (nextState != NULL){
                xMsg.eSignal = EXIT_SIG;
                (*activeState)(&me, &xMsg);
                activeState = nextState;
                xMsg.eSignal = ENTRY_SIG;
                (*activeState)(&me, &xMsg);
            }

        /* 100Hz refresh rate */
        vTaskDelay(pdMS_TO_TICKS(10));
        }
}
/*-----------------------------------------------------------*/

void * vMotorControllerOfflineState(FSM_MotorController_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue ){
    void *state;
    state = NULL;
    static BaseType_t iter;
    xAppMsgCANType_t xCANMsg;

    switch(pxEventQueue->eSignal){
    case ENTRY_SIG:
        me->xStateVariables.eState = OFFLINE_STATE;
        SendSamplingSetup(me);
        iter = 0;
        break;
    case GO_ONLINE_SIG:
    case TIMEOUT_SIG:
        iter++;
        if(iter > 100){
            // send data request frame
            SendSamplingSetup(me);
            iter = 0;
        }

        if (xQueueReceive(me->xCANQueue, &xCANMsg, 0) == pdTRUE){
            if (xCANMsg.xBase.eSignal == MOTOR_RANGE_REF_SIG){
                ReadRangeAndRef(me, 8, xCANMsg.uiData);
                state = &vMotorControllerIdleState;
            }
        }
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

void * vMotorControllerIdleState(FSM_MotorController_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue ){
    void *state;
    state = NULL;
    xAppMsgCANType_t xCANMsg;

    switch(pxEventQueue->eSignal){
    case ENTRY_SIG:
        me->xStateVariables.eState = IDLE_STATE;
        //start timer
//        vFSMTimerStart(me->pxTimer, pdMS_TO_TICKS(100));   //check every 100ms
        break;
    case OFFLINE_SIG:
        state = &vMotorControllerOfflineState;
        break;
    case TIMEOUT_SIG:
        if (xQueueReceive(me->xCANQueue, &xCANMsg, 0) == pdTRUE){
            switch(xCANMsg.xBase.eSignal){
            case MOTOR_DRIVER_STATE_SIG:
                break;
            case MOTOR_RANGE_REF_SIG:
                ReadRangeAndRef(me, 8, xCANMsg.uiData);
                break;
            case MOTOR_SUPPLY_1_SIG:
                ReadSupply1(me, 8, xCANMsg.uiData);
                break;
            case MOTOR_MOTOR_1_SIG:
                break;
            case MOTOR_MOTOR_2_SIG:
                break;
            case MOTOR_MOTOR_3_SIG:
                break;
            case MOTOR_MOTOR_4_SIG:
                break;
            }
        }
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

void vMotorControllerFSMTaskInit(FSM_MotorController_Definition_t * const me){
    me->xStateVariables.xMCLiveData.uiMotorConnected = 0;
    me->xStateVariables.xMCLiveData.uiMotorAlgorithm = 0;
    me->xStateVariables.xMCLiveData.uiHighPriorityLimiter = 0;
    me->xStateVariables.xMCLiveData.uiMotorMode = 0;
    me->xStateVariables.xMCLiveData.iTempMCU = 0;
    me->xStateVariables.xMCLiveData.uiLowPriorityLimiter = 0;
    me->xStateVariables.xMCLiveData.uiDeviceError = 0;
    me->xStateVariables.xMCLiveData.uiUrange = 0;
    me->xStateVariables.xMCLiveData.uiIrange = 0;
    me->xStateVariables.xMCLiveData.uiUref = 0;
    me->xStateVariables.xMCLiveData.uiIref = 0;
    me->xStateVariables.xMCLiveData.iRelCurrent = 0;
    me->xStateVariables.xMCLiveData.fCurrent = 0.0;
    me->xStateVariables.xMCLiveData.iRelVoltage = 0;
    me->xStateVariables.xMCLiveData.fVoltage = 0.0;
    me->xStateVariables.xMCLiveData.iTotalCharge = 0;
    me->xStateVariables.xMCLiveData.iRelAmplitudePhaseCurrent = 0;
    me->xStateVariables.xMCLiveData.iRelAplitudeLinkVoltage = 0;
    me->xStateVariables.xMCLiveData.iEnergyPassed = 0;
    me->xStateVariables.xMCLiveData.iRelMotorPower = 0;
    me->xStateVariables.xMCLiveData.fMotorPower = 0.0;
    me->xStateVariables.xMCLiveData.iMechanicalAngle = 0;
    me->xStateVariables.xMCLiveData.iRelRotorSpeed = 0;
    me->xStateVariables.xMCLiveData.iRotorSpeed = 0;
    me->xStateVariables.xMCLiveData.iOdometer = 0;
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

BaseType_t SendSamplingSetup(FSM_MotorController_Definition_t * const me){
    BaseType_t xReturn = pdTRUE;

    uint8_t puiDataBytes[] = {0x00, 0x08, 0xFF, 0xFF, 0x00, 0xFA, 0x00, 0x64};

    BSP_bmsCanSend(me->pCanNode, me->uiCanID, sizeof(puiDataBytes), puiDataBytes);

    xReturn = pdFALSE;

    return xReturn;
}
/*-----------------------------------------------------------*/

BaseType_t ReadRangeAndRef(FSM_MotorController_Definition_t * const me, uint8_t uiDataLen, uint8_t *uiData){
    BaseType_t xReturn = pdTRUE;

    me->xStateVariables.xMCLiveData.uiUrange = (uint16_t)((uiData[0] << 8) + uiData[1]);
    me->xStateVariables.xMCLiveData.uiIrange = (uint16_t)((uiData[2] << 8) + uiData[3]);
    me->xStateVariables.xMCLiveData.uiUref = (uint16_t)((uiData[4] << 8) + uiData[5]);
    me->xStateVariables.xMCLiveData.uiIref = (uint16_t)((uiData[6] << 8) + uiData[7]);

    xReturn = pdFALSE;

    return xReturn;
}
/*-----------------------------------------------------------*/

BaseType_t ReadSupply1(FSM_MotorController_Definition_t * const me, uint8_t uiDataLen, uint8_t *uiData){
    BaseType_t xReturn = pdTRUE;

    me->xStateVariables.xMCLiveData.iRelCurrent = (uint16_t)((uiData[0] << 8) + uiData[1]);
    me->xStateVariables.xMCLiveData.iRelVoltage = (uint16_t)((uiData[2] << 8) + uiData[3]);
    me->xStateVariables.xMCLiveData.iTotalCharge = (int32_t)((uiData[4] << 24) + (uiData[5] << 16) + (uiData[6] << 8) + uiData[7]);
    me->xStateVariables.xMCLiveData.fVoltage = (float_t)me->xStateVariables.xMCLiveData.iRelVoltage / 32768.0f * (float_t)me->xStateVariables.xMCLiveData.uiUrange;
    me->xStateVariables.xMCLiveData.fCurrent = (float_t)me->xStateVariables.xMCLiveData.iRelCurrent / 32768.0f * (float_t)me->xStateVariables.xMCLiveData.uiIrange;

    xReturn = pdFALSE;

    return xReturn;
}
/*-----------------------------------------------------------*/

