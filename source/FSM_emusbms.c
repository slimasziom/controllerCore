/*
 * fsmMainController.c
 *
 *  Created on: 16 lis 2020
 *      Author: Krzysztof Klimek
 */
#include "FSM_emusbms.h"
#include "APP_config.h"
#include "APP_queues.h"

#define DEFAULT_NODE_ID     0x01
#define CAN_TIMEOUT         2000U

/* Timers */
TimerHandle_t xTimerEmusBmsHandle = NULL;

/* FSM State Functions */
void * vEmusBmsOfflineState(FSM_EmusBms_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue );
void * vEmusBmsIdleState(FSM_EmusBms_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue );


/* Private Functions */
void vEmusBmsClearStateVariables(FSM_EmusBms_Definition_t * const me);
BaseType_t ReadOverallParameters(FSM_EmusBms_Definition_t * const me, uint8_t uiDataLen, uint8_t *uiData);
BaseType_t ReadDiagnosticCodes(FSM_EmusBms_Definition_t * const me, uint8_t uiDataLen, uint8_t *uiData);
BaseType_t ReadBatteryVoltageOverallParameters(FSM_EmusBms_Definition_t * const me, uint8_t uiDataLen, uint8_t *uiData);
BaseType_t ReadCellModuleTemperatureOverallParameters(FSM_EmusBms_Definition_t * const me, uint8_t uiDataLen, uint8_t *uiData);
BaseType_t ReadCellTemperatureOverallParameters(FSM_EmusBms_Definition_t * const me, uint8_t uiDataLen, uint8_t *uiData);
BaseType_t ReadCellBalancingRateOverallParameters(FSM_EmusBms_Definition_t * const me, uint8_t uiDataLen, uint8_t *uiData);
BaseType_t ReadIndividualCellVoltages(FSM_EmusBms_Definition_t * const me, uint8_t uiDataLen, uint8_t *uiData, uint8_t uiGroup);
BaseType_t ReadIndividualCellModuleTemperatures(FSM_EmusBms_Definition_t * const me, uint8_t uiDataLen, uint8_t *uiData);
BaseType_t ReadIndividualCellTemperatures(FSM_EmusBms_Definition_t * const me, uint8_t uiDataLen, uint8_t *uiData);
BaseType_t ReadIndividualCellBalancingRate(FSM_EmusBms_Definition_t * const me, uint8_t uiDataLen, uint8_t *uiData);
BaseType_t ReadSOC(FSM_EmusBms_Definition_t * const me, uint8_t uiDataLen, uint8_t *uiData);
BaseType_t ReadEnergyParameters(FSM_EmusBms_Definition_t * const me, uint8_t uiDataLen, uint8_t *uiData);


/* Function definitions */

void vEmusBmsFSMTask(void *pvParameters){
    FSM_EmusBms_Definition_t me;
    xAppMsgBaseType_t xMsg;
    void *(*activeState)(FSM_EmusBms_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue ) = &vEmusBmsOfflineState;
//    void *nextState = NULL;
    void *(*nextState)(FSM_EmusBms_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue ) = NULL;
    me.xEventQueue = xQueueBmsInputSignalHandle;
    me.pxTimer = &xTimerEmusBmsHandle;
    me.pCanNode = canREG1;
    me.uiCanID = canMESSAGE_BOX1;

    /* Initialization */
    me.xStateVariables.eState=NONE_STATE;
    snprintf( me.xStateVariables.cModuleName, 20, "bms-fsm");

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

        /* 10Hz refresh rate */
        vTaskDelay(pdMS_TO_TICKS(100));
        }
}
/*-----------------------------------------------------------*/

void * vEmusBmsOfflineState(FSM_EmusBms_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue ){
    void *state;
    state = NULL;
    xAppMsgCANType_t xCANMsg;

    switch(pxEventQueue->eSignal){
    case ENTRY_SIG:
        me->xStateVariables.eState = OFFLINE_STATE;
        /* clear state variables */
        vEmusBmsClearStateVariables(me);
        break;
    case GO_ONLINE_SIG:
    case TIMEOUT_SIG:
//        if (xQueueReceive(me->xCANQueue, &xCANMsg, 0) == pdTRUE){
//                state = &vEmusBmsIdleState;
//        }
        break;
    case GET_STATUS_SIG:
    case GET_PARS_OVERALL_SIG:
    case GET_DIAG_CODES_SIG:
    case GET_CELL_OVERALL_SIG:
    case GET_BAT_OVERALL_SIG:
    case GET_CELL_VOLS_SIG:
    case GET_CELL_TEMPS_SIG:
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

void * vEmusBmsIdleState(FSM_EmusBms_Definition_t * const me, xAppMsgBaseType_t *pxEventQueue ){
    void *state;
    state = NULL;
    xAppMsgCANType_t xCANMsg;
    static BaseType_t iter;

    switch(pxEventQueue->eSignal){
    case ENTRY_SIG:
        me->xStateVariables.eState = IDLE_STATE;
        iter = 0;
        //start timer
//        vFSMTimerStart(me->pxTimer, pdMS_TO_TICKS(100));   //check every 100ms
        break;
    case OFFLINE_SIG:
        state = &vEmusBmsOfflineState;
        break;
    case TIMEOUT_SIG:
        if (xQueueReceive(me->xCANQueue, &xCANMsg, 0) == pdTRUE){
            iter = 0;
            switch(xCANMsg.xBase.eSignal){
            case MOTOR_DRIVER_STATE_SIG:
                break;
            case MOTOR_RANGE_REF_SIG:
                break;
            case MOTOR_SUPPLY_1_SIG:
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
        else{
            iter++;
            if(iter > 100){
                state = &vEmusBmsOfflineState;
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
void vEmusBmsClearStateVariables(FSM_EmusBms_Definition_t * const me){
    BaseType_t i;
    for(i = 0; i<NUMBER_OF_CELLS; i++){
        me->xStateVariables.fEmusBmsIndividualCellVoltages[i] = 0.0;       // reg: 0-15; res: 0.1mV
        me->xStateVariables.uiEmusBmsIndividualCellTemperatures[i] = 0;
        me->xStateVariables.uiEmusBmsIndividualCellBalancingRate[i] = 0;
    }
    me->xStateVariables.uiEmusBmsIndividualCellModuleTemperatures[0] = 0;

    me->xStateVariables.xEmusBmsOverallPars.uiInputSignals = 0;
    me->xStateVariables.xEmusBmsOverallPars.uiOutputSignals = 0;
    me->xStateVariables.xEmusBmsOverallPars.uiNumberOfLiveCells = 0;
    me->xStateVariables.xEmusBmsOverallPars.uiChargingStage = 0;
    me->xStateVariables.xEmusBmsOverallPars.uiChargingStageDuration = 0;
    me->xStateVariables.xEmusBmsOverallPars.uiLastChargingError = 0;

    me->xStateVariables.xEmusBmsDiagnosticCodes.uiProtectionFlags = 0;
    me->xStateVariables.xEmusBmsDiagnosticCodes.uiWarningFlags = 0;
    me->xStateVariables.xEmusBmsDiagnosticCodes.uiBatteryStatusFlags = 0;
    me->xStateVariables.xEmusBmsBatVolOverallPars.fMinCellVoltage = 0.0;
    me->xStateVariables.xEmusBmsBatVolOverallPars.fMaxCellVoltage = 0.0;
    me->xStateVariables.xEmusBmsBatVolOverallPars.fAvgCellVoltage = 0.0;
    me->xStateVariables.xEmusBmsBatVolOverallPars.fTotalVoltage = 0.0;

    me->xStateVariables.xEmusBmsCellModuleTempOverallPars.iMinCellModuleTemp = 0;
    me->xStateVariables.xEmusBmsCellModuleTempOverallPars.iMaxCellModuleTemp = 0;
    me->xStateVariables.xEmusBmsCellModuleTempOverallPars.iAvgCellModuleTemp = 0;

    me->xStateVariables.xEmusBmsCellTempOverallPars.iMinCellTemp = 0;
    me->xStateVariables.xEmusBmsCellTempOverallPars.iMaxCellTemp = 0;
    me->xStateVariables.xEmusBmsCellTempOverallPars.iAvgCellTemp = 0;

    me->xStateVariables.xEmusBmsCellBalancingRateOverallPars.uiMinCellBalancing = 0;
    me->xStateVariables.xEmusBmsCellBalancingRateOverallPars.uiMaxCellBalancing = 0;
    me->xStateVariables.xEmusBmsCellBalancingRateOverallPars.uiAvgCellBalancing = 0;

    me->xStateVariables.xEmusBmsSOC.fCurrent = 0.0;
    me->xStateVariables.xEmusBmsSOC.fEstimatedCharge = 0.0;
    me->xStateVariables.xEmusBmsSOC.uiEstimatedSOC = 0;

    me->xStateVariables.xEmusBmsEnergyParameters.uiEstimatedConsumption = 0;
    me->xStateVariables.xEmusBmsEnergyParameters.fEstimatedEnergy = 0.0;
    me->xStateVariables.xEmusBmsEnergyParameters.fEstimatedDistanceLeft = 0.0;
    me->xStateVariables.xEmusBmsEnergyParameters.fEstimatedDistanceTraveled = 0.0;
}
/*-----------------------------------------------------------*/

BaseType_t ReadOverallParameters(FSM_EmusBms_Definition_t * const me, uint8_t uiDataLen, uint8_t *uiData){
    BaseType_t xReturn = pdTRUE;

    me->xStateVariables.xEmusBmsOverallPars.uiInputSignals = uiData[0];
    me->xStateVariables.xEmusBmsOverallPars.uiOutputSignals = uiData[1];
    me->xStateVariables.xEmusBmsOverallPars.uiNumberOfLiveCells = (uint16_t)((uiData[2] << 8) + uiData[7]);
    me->xStateVariables.xEmusBmsOverallPars.uiChargingStage = uiData[3];
    me->xStateVariables.xEmusBmsOverallPars.uiChargingStageDuration = (uint16_t)((uiData[4] << 8) + uiData[5]);
    me->xStateVariables.xEmusBmsOverallPars.uiLastChargingError = uiData[6];

    xReturn = pdFALSE;

    return xReturn;
}
/*-----------------------------------------------------------*/

BaseType_t ReadDiagnosticCodes(FSM_EmusBms_Definition_t * const me, uint8_t uiDataLen, uint8_t *uiData){
    BaseType_t xReturn = pdTRUE;

    me->xStateVariables.xEmusBmsDiagnosticCodes.uiProtectionFlags = uiData[2] << 8 + uiData[0];
    me->xStateVariables.xEmusBmsDiagnosticCodes.uiWarningFlags = uiData[1];
    me->xStateVariables.xEmusBmsDiagnosticCodes.uiBatteryStatusFlags = uiData[3];

    xReturn = pdFALSE;

    return xReturn;
}
/*-----------------------------------------------------------*/

BaseType_t ReadBatteryVoltageOverallParameters(FSM_EmusBms_Definition_t * const me, uint8_t uiDataLen, uint8_t *uiData){
    BaseType_t xReturn = pdTRUE;

    me->xStateVariables.xEmusBmsBatVolOverallPars.fMinCellVoltage = ((float_t)uiData[0]) / 100.0 + 2.0;
    me->xStateVariables.xEmusBmsBatVolOverallPars.fMaxCellVoltage = ((float_t)uiData[1]) / 100.0 + 2.0;
    me->xStateVariables.xEmusBmsBatVolOverallPars.fAvgCellVoltage = ((float_t)uiData[2]) / 100.0 + 2.0;
    me->xStateVariables.xEmusBmsBatVolOverallPars.fTotalVoltage = ((float_t)(uiData[5] << 24 + uiData[6] << 16 + uiData[3] << 8 + uiData[4])) / 100.0 + 2.0;

    xReturn = pdFALSE;

    return xReturn;
}
/*-----------------------------------------------------------*/

BaseType_t ReadCellModuleTemperatureOverallParameters(FSM_EmusBms_Definition_t * const me, uint8_t uiDataLen, uint8_t *uiData){
    BaseType_t xReturn = pdTRUE;

    me->xStateVariables.xEmusBmsCellModuleTempOverallPars.iMinCellModuleTemp = ((int8_t)uiData[0]) - 100;
    me->xStateVariables.xEmusBmsCellModuleTempOverallPars.iMaxCellModuleTemp = ((int8_t)uiData[1]) - 100;
    me->xStateVariables.xEmusBmsCellModuleTempOverallPars.iAvgCellModuleTemp = ((int8_t)uiData[2]) - 100;
    xReturn = pdFALSE;

    return xReturn;
}
/*-----------------------------------------------------------*/

BaseType_t ReadCellTemperatureOverallParameters(FSM_EmusBms_Definition_t * const me, uint8_t uiDataLen, uint8_t *uiData){
    BaseType_t xReturn = pdTRUE;

    me->xStateVariables.xEmusBmsCellTempOverallPars.iMinCellTemp = ((int8_t)uiData[0]) - 100;
    me->xStateVariables.xEmusBmsCellTempOverallPars.iMaxCellTemp = ((int8_t)uiData[1]) - 100;
    me->xStateVariables.xEmusBmsCellTempOverallPars.iAvgCellTemp = ((int8_t)uiData[2]) - 100;
    xReturn = pdFALSE;

    return xReturn;
}
/*-----------------------------------------------------------*/

BaseType_t ReadCellBalancingRateOverallParameters(FSM_EmusBms_Definition_t * const me, uint8_t uiDataLen, uint8_t *uiData){
    BaseType_t xReturn = pdTRUE;

    me->xStateVariables.xEmusBmsCellBalancingRateOverallPars.uiMinCellBalancing = uiData[0];
    me->xStateVariables.xEmusBmsCellBalancingRateOverallPars.uiMaxCellBalancing = uiData[1];
    me->xStateVariables.xEmusBmsCellBalancingRateOverallPars.uiAvgCellBalancing = uiData[2];
    xReturn = pdFALSE;

    return xReturn;
}
/*-----------------------------------------------------------*/

BaseType_t ReadIndividualCellVoltages(FSM_EmusBms_Definition_t * const me, uint8_t uiDataLen, uint8_t *uiData, uint8_t uiGroup){
    BaseType_t xReturn = pdTRUE;
    BaseType_t i;
//    static BaseType_t xCellStringNumber = -1;

    for(i = 0; i<uiDataLen; i++){
        if((uiGroup*8 + i)>NUMBER_OF_CELLS){
            break;
        }
        me->xStateVariables.fEmusBmsIndividualCellVoltages[uiGroup*8 + i] = uiData[i];
    }

    xReturn = pdFALSE;

    return xReturn;
}
/*-----------------------------------------------------------*/

BaseType_t ReadIndividualCellModuleTemperatures(FSM_EmusBms_Definition_t * const me, uint8_t uiDataLen, uint8_t *uiData){
    BaseType_t xReturn = pdTRUE;

    xReturn = pdFALSE;

    return xReturn;
}
/*-----------------------------------------------------------*/

BaseType_t ReadIndividualCellTemperatures(FSM_EmusBms_Definition_t * const me, uint8_t uiDataLen, uint8_t *uiData){
    BaseType_t xReturn = pdTRUE;

    xReturn = pdFALSE;

    return xReturn;
}
/*-----------------------------------------------------------*/

BaseType_t ReadIndividualCellBalancingRate(FSM_EmusBms_Definition_t * const me, uint8_t uiDataLen, uint8_t *uiData){
    BaseType_t xReturn = pdTRUE;

    xReturn = pdFALSE;

    return xReturn;
}
/*-----------------------------------------------------------*/

BaseType_t ReadSOC(FSM_EmusBms_Definition_t * const me, uint8_t uiDataLen, uint8_t *uiData){
    BaseType_t xReturn = pdTRUE;

    me->xStateVariables.xEmusBmsSOC.fCurrent = ((float_t)((uiData[0] << 8) + uiData[1])) / 10.0;
    me->xStateVariables.xEmusBmsSOC.fEstimatedCharge = ((float_t)((uiData[2] << 8) + uiData[3])) / 10.0;
    me->xStateVariables.xEmusBmsSOC.uiEstimatedSOC= uiData[6];

    xReturn = pdFALSE;

    return xReturn;
}
/*-----------------------------------------------------------*/

BaseType_t ReadEnergyParameters(FSM_EmusBms_Definition_t * const me, uint8_t uiDataLen, uint8_t *uiData){
    BaseType_t xReturn = pdTRUE;

    me->xStateVariables.xEmusBmsEnergyParameters.uiEstimatedConsumption = (uint16_t)((uiData[0] << 8) + uiData[1]);
    me->xStateVariables.xEmusBmsEnergyParameters.fEstimatedEnergy = ((float_t)((uiData[2] << 8) + uiData[3])) / 10.0;
    me->xStateVariables.xEmusBmsEnergyParameters.fEstimatedDistanceLeft = ((float_t)((uiData[4] << 8) + uiData[5])) / 10.0;
    me->xStateVariables.xEmusBmsEnergyParameters.fEstimatedDistanceTraveled = ((float_t)((uiData[6] << 8) + uiData[7])) / 10.0;

    xReturn = pdFALSE;

    return xReturn;
}
/*-----------------------------------------------------------*/

//BaseType_t ReadBatteryPackVoltage(FSM_EmusBms_Definition_t * const me){
//    BaseType_t xReturn;
//
//    xAppMsgCANType_t xCANMsg;
//    uint8_t puiDataBytes[] = {0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
//
//    xReturn = pdTRUE;
//
//    vSendCANFrame(me, sizeof(puiDataBytes), puiDataBytes);
//
//    if (xQueueReceive(xQueueBmsCANResponseHandle, &xCANMsg, CAN_TIMEOUT) == pdTRUE){
//        if (xCANMsg.uiData[0] == 0x01 && xCANMsg.uiData[1] == 0x14 )
//        {
////            me->xStateVariables.xBmsLiveData.fBatteryPackVoltage = (float_t)((xCANMsg.uiData[5] << 24) + (xCANMsg.uiData[4] << 16) + (xCANMsg.uiData[3] << 8) + xCANMsg.uiData[2]);
//            memcpy (&me->xStateVariables.xBmsLiveData.fBatteryPackVoltage, xCANMsg.uiData + 2, sizeof (float));
//            xReturn = pdFALSE;
//        }
//    }
//    return xReturn;
//}
///*-----------------------------------------------------------*/
//
//BaseType_t ReadBatteryPackCurrent(FSM_EmusBms_Definition_t * const me){
//    BaseType_t xReturn;
//
//    xAppMsgCANType_t xCANMsg;
//    uint8_t puiDataBytes[] = {0x15, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
//
//    xReturn = pdTRUE;
//
//    vSendCANFrame(me, sizeof(puiDataBytes), puiDataBytes);
//
//    if (xQueueReceive(xQueueBmsCANResponseHandle, &xCANMsg, CAN_TIMEOUT) == pdTRUE){
//        if (xCANMsg.uiData[0] == 0x01 && xCANMsg.uiData[1] == 0x15 )
//        {
////            me->xStateVariables.xBmsLiveData.fBatteryPackVoltage = (float_t)((xCANMsg.uiData[5] << 24) + (xCANMsg.uiData[4] << 16) + (xCANMsg.uiData[3] << 8) + xCANMsg.uiData[2]);
//            memcpy (&me->xStateVariables.xBmsLiveData.fBatteryPackCurrent, xCANMsg.uiData + 2, sizeof (float));
//            xReturn = pdFALSE;
//        }
//    }
//    return xReturn;
//}
///*-----------------------------------------------------------*/
//
//BaseType_t ReadBatteryPackMaxCellVoltage(FSM_EmusBms_Definition_t * const me){
//    BaseType_t xReturn;
//
//    xAppMsgCANType_t xCANMsg;
//    uint8_t puiDataBytes[] = {0x16, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
//
//    xReturn = pdTRUE;
//
//    vSendCANFrame(me, sizeof(puiDataBytes), puiDataBytes);
//
//    if (xQueueReceive(xQueueBmsCANResponseHandle, &xCANMsg, CAN_TIMEOUT) == pdTRUE){
//        if (xCANMsg.uiData[0] == 0x01 && xCANMsg.uiData[1] == 0x16 )
//        {
//            me->xStateVariables.xBmsLiveData.uiBatteryMaxCellVoltage = (uint16_t)((xCANMsg.uiData[3] << 8) + xCANMsg.uiData[2]);
//            xReturn = pdFALSE;
//        }
//    }
//    return xReturn;
//}
///*-----------------------------------------------------------*/
//
//BaseType_t ReadBatteryPackMinCellVoltage(FSM_EmusBms_Definition_t * const me){
//    BaseType_t xReturn;
//
//    xAppMsgCANType_t xCANMsg;
//    uint8_t puiDataBytes[] = {0x17, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
//
//    xReturn = pdTRUE;
//
//    vSendCANFrame(me, sizeof(puiDataBytes), puiDataBytes);
//
//    if (xQueueReceive(xQueueBmsCANResponseHandle, &xCANMsg, CAN_TIMEOUT) == pdTRUE){
//        if (xCANMsg.uiData[0] == 0x01 && xCANMsg.uiData[1] == 0x17 )
//        {
//            me->xStateVariables.xBmsLiveData.uiBatteryMaxCellVoltage = (uint16_t)((xCANMsg.uiData[3] << 8) + xCANMsg.uiData[2]);
//            xReturn = pdFALSE;
//        }
//    }
//    return xReturn;
//}
///*-----------------------------------------------------------*/
//
//BaseType_t ReadOnlineStatus(FSM_EmusBms_Definition_t * const me){
//    BaseType_t xReturn;
//    xAppMsgCANType_t xCANMsg;
//    uint16_t uiStatus;
//    uint8_t puiDataBytes[] = {0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
//
//    xReturn = pdTRUE;
//
//    vSendCANFrame(me, sizeof(puiDataBytes), puiDataBytes);
//
//    if (xQueueReceive(xQueueBmsCANResponseHandle, &xCANMsg, 20000) == pdTRUE){
//        if (xCANMsg.uiData[0] == 0x01 && xCANMsg.uiData[1] == 0x18 )
//        {
//            uiStatus = xCANMsg.uiData[3] * 256 + xCANMsg.uiData[2];
//            switch(uiStatus)
//            {
//            case 0x91:
//                me->xStateVariables.xBmsLiveData.eOnlineStatus = CHARGING_STATE;
//                break;
//            case 0x92:
//                me->xStateVariables.xBmsLiveData.eOnlineStatus = FULLY_CHARGED_STATE;
//                break;
//            case 0x93:
//                me->xStateVariables.xBmsLiveData.eOnlineStatus = DISCHARGING_STATE;
//                break;
//            case 0x96:
//                me->xStateVariables.xBmsLiveData.eOnlineStatus = REGENERATION_STATE;
//                break;
//            case 0x97:
//                me->xStateVariables.xBmsLiveData.eOnlineStatus = IDLE_STATE;
//                break;
//            case 0x9B:
//                me->xStateVariables.xBmsLiveData.eOnlineStatus = FAULT_STATE;
//                break;
//            default:
//                me->xStateVariables.xBmsLiveData.eOnlineStatus = NONE_STATE;
//                break;
//            }
//            xReturn = pdFALSE;
//        }
//    }
//    return xReturn;
//}
///*-----------------------------------------------------------*/
//
//BaseType_t ReadLifetimeCounter(FSM_EmusBms_Definition_t * const me){
//    BaseType_t xReturn;
//
//    xAppMsgCANType_t xCANMsg;
//    uint8_t puiDataBytes[] = {0x19, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
//
//    xReturn = pdTRUE;
//
//    vSendCANFrame(me, sizeof(puiDataBytes), puiDataBytes);
//
//    if (xQueueReceive(xQueueBmsCANResponseHandle, &xCANMsg, CAN_TIMEOUT) == pdTRUE){
//        if (xCANMsg.uiData[0] == 0x01 && xCANMsg.uiData[1] == 0x19 )
//        {
//            me->xStateVariables.xBmsLiveData.uiLifetimeCounter = (uint32_t)((xCANMsg.uiData[5] << 24) + (xCANMsg.uiData[4] << 16) + (xCANMsg.uiData[3] << 8) + xCANMsg.uiData[2]);
//            xReturn = pdFALSE;
//        }
//    }
//    return xReturn;
//}
///*-----------------------------------------------------------*/
//
//BaseType_t ReadEstimatedSOC(FSM_EmusBms_Definition_t * const me){
//    BaseType_t xReturn;
//
//    xAppMsgCANType_t xCANMsg;
//    uint8_t puiDataBytes[] = {0x1A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
//
//    xReturn = pdTRUE;
//
//    vSendCANFrame(me, sizeof(puiDataBytes), puiDataBytes);
//
//    if (xQueueReceive(xQueueBmsCANResponseHandle, &xCANMsg, CAN_TIMEOUT) == pdTRUE){
//        if (xCANMsg.uiData[0] == 0x01 && xCANMsg.uiData[1] == 0x1A )
//        {
//            me->xStateVariables.xBmsLiveData.uiStateOfCharge = (uint32_t)((xCANMsg.uiData[5] << 24) + (xCANMsg.uiData[4] << 16) + (xCANMsg.uiData[3] << 8) + xCANMsg.uiData[2]);
//            xReturn = pdFALSE;
//        }
//    }
//    return xReturn;
//}
///*-----------------------------------------------------------*/
//
//BaseType_t ReadDeviceTemperatures(FSM_EmusBms_Definition_t * const me){
//    BaseType_t xReturn;
//    BaseType_t i;
//    xAppMsgCANType_t xCANMsg;
//    uint8_t puiDataBytes[] = {0x1B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
//
//    xReturn = pdFALSE + 3;
//
//    vSendCANFrame(me, sizeof(puiDataBytes), puiDataBytes);
//
//    for(i=0; i<3; i++){
//        if (xQueueReceive(xQueueBmsCANResponseHandle, &xCANMsg, 500) == pdTRUE)
//        {
//            if (xCANMsg.uiData[0] == 0x01 && xCANMsg.uiData[1] == 0x1B )
//            {
//                switch(xCANMsg.uiData[5])
//                {
//                case 0x00:
//                    me->xStateVariables.xBmsLiveData.iTempInternal = (uint16_t)((xCANMsg.uiData[4] << 8) + xCANMsg.uiData[3]);
//                    break;
//                case 0x01:
//                    me->xStateVariables.xBmsLiveData.iExtSensorTemp1 = (uint16_t)((xCANMsg.uiData[4] << 8) + xCANMsg.uiData[3]);
//                    break;
//                case 0x02:
//                    me->xStateVariables.xBmsLiveData.iExtSensorTemp1 = (uint16_t)((xCANMsg.uiData[4] << 8) + xCANMsg.uiData[3]);
//                    break;
//                default:
//                    break;
//                }
//                xReturn--;
//            }
//        }
//    }
//    return xReturn;
//}
///*-----------------------------------------------------------*/
//
//BaseType_t ReadBatteryPackCellsVoltages(FSM_EmusBms_Definition_t * const me){
//    BaseType_t xReturn;
//    BaseType_t i;
//    xAppMsgCANType_t xCANMsg;
//    uint8_t puiDataBytes[] = {0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
//
//    xReturn = pdFALSE + NUMBER_OF_CELLS;
//
//    vSendCANFrame(me, sizeof(puiDataBytes), puiDataBytes);
//
//    for(i=0; i<NUMBER_OF_CELLS; i++){
//        if (xQueueReceive(xQueueBmsCANResponseHandle, &xCANMsg, CAN_TIMEOUT) == pdTRUE)
//        {
//            if (xCANMsg.uiData[0] == 0x01 && xCANMsg.uiData[1] == 0x1C )
//            {
//                me->xStateVariables.xBmsLiveData.uiCellVoltages[xCANMsg.uiData[5]] = (uint16_t)((xCANMsg.uiData[4] << 8) + xCANMsg.uiData[3]);
//                xReturn--;
//            }
//        }
//    }
//    return xReturn;
//}
///*-----------------------------------------------------------*/
//
//BaseType_t ReadSettingsValues(FSM_EmusBms_Definition_t * const me){
//    BaseType_t xReturn;
//
//    xAppMsgCANType_t xCANMsg;
//    uint8_t puiDataBytes[] = {0x1D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
//
//    xReturn = pdTRUE;
//
//    return xReturn;
//}
///*-----------------------------------------------------------*/
//
//BaseType_t ReadLiveData(FSM_EmusBms_Definition_t * const me){
//    BaseType_t xReturn;
//
//    xReturn = pdFALSE;
//
//    xReturn += ReadBatteryPackVoltage(me);
//    xReturn += ReadBatteryPackCurrent(me);
//    xReturn += ReadBatteryPackMaxCellVoltage(me);
//    xReturn += ReadBatteryPackMinCellVoltage(me);
//    xReturn += ReadOnlineStatus(me);
//    xReturn += ReadLifetimeCounter(me);
//    xReturn += ReadEstimatedSOC(me);
//    xReturn += ReadDeviceTemperatures(me);
//    xReturn += ReadBatteryPackCellsVoltages(me);
//
//    return xReturn;
//}
///*-----------------------------------------------------------*/

