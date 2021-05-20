#include <REST_commands.h>
#include "FreeRTOS.h"
#include "FreeRTOS_IO.h"
#include "os_task.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "HL_emac.h"
#include "HL_mdio.h"
#include "HL_phy_dp83640.h"


/* Application specific configuration */
#include "APP_config.h"
#include "APP_queues.h"

#include "FSM_emusbms.h"

extern hdkif_t hdkif_data[MAX_EMAC_INSTANCE];

/* Helper functions */
static void xMainControllerRESTStatusToJson( char *pcWriteBuffer, size_t xWriteBufferLen, xControllerStateVariables_t *xStateVariables);
static void xTinyBmsRESTStatusToJson( char *pcWriteBuffer, size_t xWriteBufferLen, xBmsStateVariables_t *xStateVariables);
static void xEmusBmsRESTStatusToJson( char *pcWriteBuffer, size_t xWriteBufferLen, xEmusBmsStateVariables_t *xStateVariables);
static void xEmusBmsRESTOverallParametersToJson( char *pcWriteBuffer, size_t xWriteBufferLen, xEmusBmsStateVariables_t *xStateVariables);
static void xEmusBmsRESTDiagnosticCodesToJson( char *pcWriteBuffer, size_t xWriteBufferLen, xEmusBmsStateVariables_t *xStateVariables);
static void xEmusBmsRESTBatteryOverallToJson( char *pcWriteBuffer, size_t xWriteBufferLen, xEmusBmsStateVariables_t *xStateVariables);
static void xEmusBmsRESTIndividualCellsToJson( char *pcWriteBuffer, size_t xWriteBufferLen, xEmusBmsStateVariables_t *xStateVariables);

void xMainControllerRESTStatusToJson( char *pcWriteBuffer, size_t xWriteBufferLen, xControllerStateVariables_t *xStateVariables){
    snprintf( pcWriteBuffer, xWriteBufferLen,
    "{\"success\": \"OK\", \"result\": "
        "{"
        "\"Module\": \"%s\","
        "\"State\": \"%s\","
        "\"Status\": "
            "{"
                "\"Driving-Mode\": %d, "
                "\"Errors\": %d, "
                "\"Warnings\": %d, "
                "\"Lights\": "
                "{"
                    "\"Break\": %d, "
                    "\"Daytime\": %d, "
                    "\"Head-High-Beam\": %d, "
                    "\"Head-Low-Beam\": %d, "
                    "\"Left-Indicator\": %d, "
                    "\"Right-Indicator\": %d, "
                    "\"Tail\": %d"
                "}, "
                "\"Battery\": "
                "{"
                    "\"Estimated-Distance-Left\": %.1f, "
                    "\"Estimated-Energy\": %.2f, "
                    "\"Max-Cell-Module-Temp\": %d, "
                    "\"Max-Cell-Temp\": %d, "
                    "\"Estimated-Consumption\": %d, "
                    "\"Estimated-SOC\": %d, "
                    "\"Errors\": %d, "
                    "\"Warnings\": %d"
                "}, "
                "\"Motor\": "
                "{"
                    "\"fMotorPower\": %.1f, "
                    "\"Odometer\": %d, "
                    "\"Rotor-Speed\": %d, "
                    "\"Speed\": %d, "
                    "\"Temp-MCU\": %d, "
                    "\"Temp-Motor\": %d, "
                    "\"Errors\": %d, "
                    "\"Warnings\": %d"
                "}"
            "}"
        "}"
    "}",
    xStateVariables->cModuleName,
    pcStateNameFromThread(xStateVariables->eState),
    xStateVariables->eDrivingMode = ECO_DRIVING_MODE,
    xStateVariables->uiErrors,
    xStateVariables->uiWarnings,

    xStateVariables->xLights.uiBreak,
    xStateVariables->xLights.uiDaytime,
    xStateVariables->xLights.uiHeadHighBeam,
    xStateVariables->xLights.uiHeadLowBeam,
    xStateVariables->xLights.uiLeftIndicator,
    xStateVariables->xLights.uiRightIndicator,
    xStateVariables->xLights.uiTail,

    xStateVariables->xBattery.fEstimatedDistanceLeft,
    xStateVariables->xBattery.fEstimatedEnergy,
    xStateVariables->xBattery.iMaxCellModuleTemp,
    xStateVariables->xBattery.iMaxCellTemp,
    xStateVariables->xBattery.uiEstimatedConsumption,
    xStateVariables->xBattery.uiEstimatedSOC,
    xStateVariables->xBattery.uiErrors,
    xStateVariables->xBattery.uiWarnings,

    xStateVariables->xMotor.fMotorPower,
    xStateVariables->xMotor.iOdometer,
    xStateVariables->xMotor.iRotorSpeed,
    xStateVariables->xMotor.iSpeed,
    xStateVariables->xMotor.iTempMCU,
    xStateVariables->xMotor.iTempMotor,
    xStateVariables->xMotor.uiErrors,
    xStateVariables->xMotor.uiWarnings
    );
}
/*-----------------------------------------------------------*/

BaseType_t xMainControllerREST( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    BaseType_t lParameterStringLength, xReturn;
    xControllerStateVariables_t xStateVariables;
    xAppMsgBaseType_t xMsg = {xQueueRestAPICtrlResponseHandle, GET_STATUS_SIG};
    char * pcParameter;

    /* Obtain the signal string. */
    pcParameter = ( char * ) FreeRTOS_RESTGetParameter
                            (
                                pcCommandString,        /* The command string itself. */
                                1,                      /* Return the first parameter. */
                                &lParameterStringLength /* Store the parameter string length. */
                            );

    ( void ) lParameterStringLength;

    xReturn = pdFALSE;
    ( void ) pcWriteBuffer;
    ( void ) xWriteBufferLen;
    ( void ) pcCommandString;

    xMsg.eSignal = (ESignal) uiThreadSignalFromCommand(pcParameter);

    switch(xMsg.eSignal){
    case RUN_SIG:
    case STOP_SIG:
    case PAUSE_SIG:
    case EMERGENCY_SIG:
        xQueueSend(xQueueCtrlInputSignalHandle, &xMsg, NULL);
        snprintf( pcWriteBuffer, xWriteBufferLen, "{\"success\": \"OK\", \"result\": {\"message\": \"executed\"}}" );
        break;
    case GET_STATUS_SIG:
        if(xQueueSend(xQueueCtrlInputSignalHandle, &xMsg, NULL) == pdTRUE){
            if(xQueueReceive(xQueueRestAPICtrlResponseHandle, &xStateVariables, 1000) == pdTRUE){
                xMainControllerRESTStatusToJson(pcWriteBuffer, xWriteBufferLen, &xStateVariables);
            }
            else
            {
                snprintf( pcWriteBuffer, xWriteBufferLen, "{\"success\": \"OK\", \"result\": \"response error\"}" );
            }
        }
        else
        {
            snprintf( pcWriteBuffer, xWriteBufferLen, "{\"success\": \"OK\", \"result\": \"send error\"}" );
        }
        break;
    default:
        break;
    }

    return xReturn;

}
/*-----------------------------------------------------------*/

static void xTinyBmsRESTStatusToJson( char *pcWriteBuffer, size_t xWriteBufferLen, xBmsStateVariables_t *xStateVariables){
    snprintf( pcWriteBuffer, xWriteBufferLen,
      "{\"success\": \"OK\", \"result\": "
          "{"
              "\"Module\": \"%s\", "
              "\"State\": \"%s\", "
              "\"Live-Data\": "
              "{"
                  "\"Cell-Voltages\": "
                      "{"
                          "\"0\": %d, "
                          "\"1\": %d, "
                          "\"2\": %d, "
                          "\"3\": %d, "
                          "\"4\": %d, "
                          "\"5\": %d, "
                          "\"6\": %d, "
                          "\"7\": %d, "
                          "\"8\": %d, "
                          "\"9\": %d, "
                          "\"10\": %d, "
                          "\"11\": %d, "
                          "\"12\": %d, "
                          "\"13\": %d, "
                          "\"14\": %d, "
                          "\"15\": %d"
                      "}, "
                  "\"Lifetime-Counter\": %d, "
                  "\"Time-Left\": %d, "
                  "\"Battery-Pack-Voltage\": %f, "
                  "\"Battery-Pack-Current\": %f, "
                  "\"Min-Cell-Voltage\": %d, "
                  "\"Max-Cell-Voltage\": %d, "
                  "\"Ext-Sensor-Temp-1\": %d, "
                  "\"Ext-Sensor-Temp-2\": %d, "
                  "\"Distance-Left\": %d, "
                  "\"SOC\": %d, "
                  "\"Int-Sensor-Temp\": %d, "
                  "\"Online-Status\": \"%s\", "
                  "\"Balancing-Decision-Bits\": %d, "
                  "\"Real-Balancing-Bits\": %d, "
                  "\"No-Detected-Cells\": %d, "
                  "\"Speed\": %f"
              "},"
              "\"Settings\": "
              "{"
                  "\"Fully-Charged-Voltage\": %d, "
                  "\"Fully-Discharged-Voltage\": %d, "
                  "\"Early-Balancing-Threshold\": %d, "
                  "\"Charge-Finished-Current\": %d, "
                  "\"Battery-Capacity\": %d, "
                  "\"Number-Of-Series-Cells\": %d, "
                  "\"Allowed-Disbalance\": %d, "
                  "\"Pulses-Per-Unit\": %d, "
                  "\"Distance-Unit-Name\": %d, "
                  "\"Over-Voltage-Cutoff\": %d, "
                  "\"Under-Voltage-Cutoff\": %d, "
                  "\"Discharge-Over-Current-Cutoff\": %d, "
                  "\"Charge-Over-Current-Cutoff\": %d, "
                  "\"OverHeat-Cutoff\": %d, "
                  "\"Low-Temp-Charger-Cutoff\": %d"
              "}"
          "}"
      "}",
      xStateVariables->cModuleName,
      pcStateNameFromThread(xStateVariables->eState),
      /* live-data cells */
      xStateVariables->xBmsLiveData.uiCellVoltages[0],
      xStateVariables->xBmsLiveData.uiCellVoltages[1],
      xStateVariables->xBmsLiveData.uiCellVoltages[2],
      xStateVariables->xBmsLiveData.uiCellVoltages[3],
      xStateVariables->xBmsLiveData.uiCellVoltages[4],
      xStateVariables->xBmsLiveData.uiCellVoltages[5],
      xStateVariables->xBmsLiveData.uiCellVoltages[6],
      xStateVariables->xBmsLiveData.uiCellVoltages[7],
      xStateVariables->xBmsLiveData.uiCellVoltages[8],
      xStateVariables->xBmsLiveData.uiCellVoltages[9],
      xStateVariables->xBmsLiveData.uiCellVoltages[10],
      xStateVariables->xBmsLiveData.uiCellVoltages[11],
      xStateVariables->xBmsLiveData.uiCellVoltages[12],
      xStateVariables->xBmsLiveData.uiCellVoltages[13],
      xStateVariables->xBmsLiveData.uiCellVoltages[14],
      xStateVariables->xBmsLiveData.uiCellVoltages[15],
      /* live-data other */
      xStateVariables->xBmsLiveData.uiLifetimeCounter,
      xStateVariables->xBmsLiveData.uiTimeLeft,
      xStateVariables->xBmsLiveData.fBatteryPackVoltage,
      xStateVariables->xBmsLiveData.fBatteryPackCurrent,
      xStateVariables->xBmsLiveData.uiBatteryMinCellVoltage,
      xStateVariables->xBmsLiveData.uiBatteryMaxCellVoltage,
      xStateVariables->xBmsLiveData.iExtSensorTemp1,
      xStateVariables->xBmsLiveData.iExtSensorTemp2,
      xStateVariables->xBmsLiveData.uiDistanceLeftToEmptyBat,
      xStateVariables->xBmsLiveData.uiStateOfCharge,
      xStateVariables->xBmsLiveData.iTempInternal,
      pcStateNameFromThread(xStateVariables->xBmsLiveData.eOnlineStatus),
      xStateVariables->xBmsLiveData.uiBalancingDecisionBits,
      xStateVariables->xBmsLiveData.uiRealBalancingBits,
      xStateVariables->xBmsLiveData.uiNoDetectedCells,
      xStateVariables->xBmsLiveData.fSpeed,
      /* settings */
      xStateVariables->xBmsSettings.uiFullyChargedVoltage,
      xStateVariables->xBmsSettings.uiFullyDischargedVoltage,
      xStateVariables->xBmsSettings.uiEarlyBalancingThreshold,
      xStateVariables->xBmsSettings.uiChargeFinishedCurrent,
      xStateVariables->xBmsSettings.uiBatteryCapacity,
      xStateVariables->xBmsSettings.uiNumberOfSeriesCells,
      xStateVariables->xBmsSettings.uiAllowedDisbalance,
      xStateVariables->xBmsSettings.uiPulsesPerUnit,
      xStateVariables->xBmsSettings.uiDistanceUnitName,
      xStateVariables->xBmsSettings.uiOverVoltageCutoff,
      xStateVariables->xBmsSettings.uiUnderVoltageCutoff,
      xStateVariables->xBmsSettings.uiDischargeOverCurrentCutoff,
      xStateVariables->xBmsSettings.uiChargeOverCurrentCutoff,
      xStateVariables->xBmsSettings.iOverHeatCutoff,
      xStateVariables->xBmsSettings.iLowTempChargerCutoff);
}
/*-----------------------------------------------------------*/

BaseType_t xTinyBmsREST(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString){
    BaseType_t lParameterStringLength, xReturn;
    xBmsStateVariables_t xStateVariables;
    xAppMsgBaseType_t xMsg = {xQueueRestAPIBmsResponseHandle, GET_STATUS_SIG};
    char * pcParameter;

    /* Obtain the signal string. */
    pcParameter = ( char * ) FreeRTOS_RESTGetParameter
                            (
                                pcCommandString,        /* The command string itself. */
                                1,                      /* Return the first parameter. */
                                &lParameterStringLength /* Store the parameter string length. */
                            );

    ( void ) lParameterStringLength;

    xReturn = pdFALSE;
    ( void ) pcWriteBuffer;
    ( void ) xWriteBufferLen;
    ( void ) pcCommandString;

    xMsg.eSignal = (ESignal) uiThreadSignalFromCommand(pcParameter);

    switch(xMsg.eSignal){
    case OFFLINE_SIG:
    case GO_ONLINE_SIG:
    case IDLE_SIG:
    case FAULT_SIG:
    case TIMEOUT_SIG:
        xQueueSend(xQueueBmsInputSignalHandle, &xMsg, NULL);
        snprintf( pcWriteBuffer, xWriteBufferLen, "{\"success\": \"OK\", \"result\": {\"message\": \"executed\"}}" );
        break;
    case GET_STATUS_SIG:
        if(xQueueSend(xQueueBmsInputSignalHandle, &xMsg, NULL) == pdTRUE){
            if(xQueueReceive(xQueueRestAPIBmsResponseHandle, &xStateVariables, 1000) == pdTRUE){
                xTinyBmsRESTStatusToJson(pcWriteBuffer, xWriteBufferLen, &xStateVariables);
            }
            else
            {
                snprintf( pcWriteBuffer, xWriteBufferLen, "{\"success\": \"OK\", \"result\": \"response error\"}" );
            }
        }
        else
        {
            snprintf( pcWriteBuffer, xWriteBufferLen, "{\"success\": \"OK\", \"result\": \"send error\"}" );
        }
        break;
    default:
        break;
    }

    return xReturn;
}
/*-----------------------------------------------------------*/

static void xEmusBmsRESTStatusToJson( char *pcWriteBuffer, size_t xWriteBufferLen, xEmusBmsStateVariables_t *xStateVariables){
    snprintf( pcWriteBuffer, xWriteBufferLen,
    "{\"success\": \"OK\", \"result\": "
        "{"
            "\"Module\": \"%s\", "
            "\"State\": \"%s\", "
            "\"Status\":"
            "{"
                "\"Total-Voltage\": %.2f, "
                "\"Current\": %.1f, "
                "\"Estimated-Energy\": %.2f, "
                "\"Estimated-State-Of-Charge\": %d, "
                "\"Charging-Stage\": %d, "
                "\"Charging-Stage-Duration\": %d, "
                "\"Protection-Flags\": %d, "
                "\"Warning-Reduction-Flags\": %d, "
                "\"Battery-Status-Flags\": %d, "
                "\"Min-Cell-Voltage\": %.2f, "
                "\"Max-Cell-Voltage\": %.2f, "
                "\"Min-Cell-Temperature\": %d, "
                "\"Max-Cell-Temperature\": %d"
            "}"
        "}"
    "}",
    xStateVariables->cModuleName,
    pcStateNameFromThread(xStateVariables->eState),

    xStateVariables->xEmusBmsBatVolOverallPars.fTotalVoltage,
    xStateVariables->xEmusBmsSOC.fCurrent,
    xStateVariables->xEmusBmsEnergyParameters.fEstimatedEnergy,
    xStateVariables->xEmusBmsSOC.uiEstimatedSOC,
    xStateVariables->xEmusBmsOverallPars.uiChargingStage,
    xStateVariables->xEmusBmsOverallPars.uiChargingStageDuration,
    xStateVariables->xEmusBmsDiagnosticCodes.uiProtectionFlags,
    xStateVariables->xEmusBmsDiagnosticCodes.uiWarningFlags,
    xStateVariables->xEmusBmsDiagnosticCodes.uiBatteryStatusFlags,
    xStateVariables->xEmusBmsBatVolOverallPars.fMinCellVoltage,
    xStateVariables->xEmusBmsBatVolOverallPars.fMaxCellVoltage,
    xStateVariables->xEmusBmsCellTempOverallPars.iMinCellTemp,
    xStateVariables->xEmusBmsCellTempOverallPars.iMaxCellTemp

    );
}
/*-----------------------------------------------------------*/

static void xEmusBmsRESTOverallParametersToJson( char *pcWriteBuffer, size_t xWriteBufferLen, xEmusBmsStateVariables_t *xStateVariables){
    snprintf( pcWriteBuffer, xWriteBufferLen,
    "{\"success\": \"OK\", \"result\": "
        "{"
            "\"Module\": \"%s\", "
            "\"State\": \"%s\", "
            "\"Overall-Parameters\": "
            "{"
                "\"Input-Signals\":"
                "{"
                    "\"Ignition-Key\": %d, "
                    "\"Charger-Mains\": %d, "
                    "\"Fast-Charge\": %d, "
                    "\"Leakage-Sensor\": %d "
                "}, "
                "\"Output-Signals\":"
                "{"
                    "\"Charger-Enable\": %d, "
                    "\"Heater-Enable\": %d, "
                    "\"Battery-Contactor\": %d, "
                    "\"Battery-Fan\": %d, "
                    "\"Power-Reduction\": %d, "
                    "\"Charging-Interlock\": %d, "
                    "\"DC-DC-Control\": %d, "
                    "\"Contactor-Pre-Charge\": %d "
                "}, "
                "\"Number-of-Live-Cells\": %d, "
                "\"Charging-Stage\": %d, "
                "\"Charging-Stage-Duration\": %d, "
                "\"Last-Charging-Error\": %d"
            "}"
        "}"
    "}",

    xStateVariables->cModuleName,
    pcStateNameFromThread(xStateVariables->eState),
    xStateVariables->xEmusBmsOverallPars.uiInputSignals && INPUT_SIGNALS_IGNITION_KEY_BIT,
    xStateVariables->xEmusBmsOverallPars.uiInputSignals && INPUT_SIGNALS_CHARGER_MAINS_BIT,
    xStateVariables->xEmusBmsOverallPars.uiInputSignals && INPUT_SIGNALS_FAST_CHARGE_BIT,
    xStateVariables->xEmusBmsOverallPars.uiInputSignals && INPUT_SIGNALS_LEAKAGE_SENSOR_BIT,

    xStateVariables->xEmusBmsOverallPars.uiOutputSignals && OUTPUT_SIGNALS_CHARGER_ENABLE_BIT,
    xStateVariables->xEmusBmsOverallPars.uiOutputSignals && OUTPUT_SIGNALS_HEATER_ENABLE_BIT,
    xStateVariables->xEmusBmsOverallPars.uiOutputSignals && OUTPUT_SIGNALS_BATTERY_CONTACTOR_BIT,
    xStateVariables->xEmusBmsOverallPars.uiOutputSignals && OUTPUT_SIGNALS_BATTERY_FAN_BIT,
    xStateVariables->xEmusBmsOverallPars.uiOutputSignals && OUTPUT_SIGNALS_POWER_REDUCTION_BIT,
    xStateVariables->xEmusBmsOverallPars.uiOutputSignals && OUTPUT_SIGNALS_CHARGING_INTERLOCK_BIT,
    xStateVariables->xEmusBmsOverallPars.uiOutputSignals && OUTPUT_SIGNALS_DCDC_CONTROL_BIT,
    xStateVariables->xEmusBmsOverallPars.uiOutputSignals && OUTPUT_SIGNALS_CONTACTOR_PRECHARGE_BIT,

    xStateVariables->xEmusBmsOverallPars.uiNumberOfLiveCells,
    xStateVariables->xEmusBmsOverallPars.uiChargingStage,
    xStateVariables->xEmusBmsOverallPars.uiChargingStageDuration,
    xStateVariables->xEmusBmsOverallPars.uiLastChargingError

    );
}
/*-----------------------------------------------------------*/

static void xEmusBmsRESTDiagnosticCodesToJson( char *pcWriteBuffer, size_t xWriteBufferLen, xEmusBmsStateVariables_t *xStateVariables){
    snprintf( pcWriteBuffer, xWriteBufferLen,
    "{\"success\": \"OK\", \"result\": "
        "{"
            "\"Module\": \"%s\", "
            "\"State\": \"%s\", "
            "\"Diagnostic-Codes\": "
            "{"
                "\"Protection-Flags\":"
                "{"
                    "\"Under-Voltage\": %d, "
                    "\"Over-Voltage\": %d, "
                    "\"Discharge-Over-Current\": %d, "
                    "\"Charge-Over-Current\": %d, "
                    "\"Cell-Module-Overheat\": %d, "
                    "\"Leakage\": %d, "
                    "\"No-Cell-Communication\": %d, "
                    "\"Reserved\": %d, "
                    "\"Reserved\": %d, "
                    "\"Reserved\": %d, "
                    "\"Reserved\": %d, "
                    "\"Cell-Overheat\": %d, "
                    "\"No-Current-Sensor\": %d, "
                    "\"Pack-Under-Voltage\": %d, "
                    "\"Reserved\": %d, "
                    "\"Reserved\": %d "
                "}, "
                "\"Warning-Reduction-Flags\":"
                "{"
                    "\"Low-Voltage\": %d, "
                    "\"High-Current\": %d, "
                    "\"High-Temperature\": %d "
                "}, "
                "\"Battery-Status-Flags\":"
                "{"
                    "\"Cell-Voltages-Validity\": %d, "
                    "\"Cell-Modules-Temperatures-Validity\": %d, "
                    "\"Cell-Balancing-Rates-Validity\": %d, "
                    "\"Number-Of-Live-Cells-Validity\": %d, "
                    "\"Battery-Charging-Finished\": %d, "
                    "\"Cell-Temperatures-Validity\": %d, "
                    "\"Reserved\": %d, "
                    "\"Reserved\": %d"
                "}"
            "}"
        "}"
    "}",
    xStateVariables->cModuleName,
    pcStateNameFromThread(xStateVariables->eState),

    xStateVariables->xEmusBmsDiagnosticCodes.uiProtectionFlags && PROTECTION_FLAG_UNDER_VOLTAGE_BIT,
    xStateVariables->xEmusBmsDiagnosticCodes.uiProtectionFlags && PROTECTION_FLAG_OVER_VOLTAGE_BIT,
    xStateVariables->xEmusBmsDiagnosticCodes.uiProtectionFlags && PROTECTION_FLAG_DISCHARGE_OVER_CURRENT_BIT,
    xStateVariables->xEmusBmsDiagnosticCodes.uiProtectionFlags && PROTECTION_FLAG_CHARGE_OVER_CURRENT_BIT,
    xStateVariables->xEmusBmsDiagnosticCodes.uiProtectionFlags && PROTECTION_FLAG_CELL_MODULE_OVERHEAT_BIT,
    xStateVariables->xEmusBmsDiagnosticCodes.uiProtectionFlags && PROTECTION_FLAG_LEAKAGE_BIT,
    xStateVariables->xEmusBmsDiagnosticCodes.uiProtectionFlags && PROTECTION_FLAG_NO_CELL_COMMUNICATION_BIT,
    xStateVariables->xEmusBmsDiagnosticCodes.uiProtectionFlags && PROTECTION_FLAG_RESERVED_7_BIT,
    xStateVariables->xEmusBmsDiagnosticCodes.uiProtectionFlags && PROTECTION_FLAG_RESERVED_8_BIT,
    xStateVariables->xEmusBmsDiagnosticCodes.uiProtectionFlags && PROTECTION_FLAG_RESERVED_9_BIT,
    xStateVariables->xEmusBmsDiagnosticCodes.uiProtectionFlags && PROTECTION_FLAG_RESERVED_10_BIT,
    xStateVariables->xEmusBmsDiagnosticCodes.uiProtectionFlags && PROTECTION_FLAG_CELL_OVERHEAT_BIT,
    xStateVariables->xEmusBmsDiagnosticCodes.uiProtectionFlags && PROTECTION_FLAG_NO_CURRENT_SENSOR_BIT,
    xStateVariables->xEmusBmsDiagnosticCodes.uiProtectionFlags && PROTECTION_FLAG_PACK_UNDER_VOLTAGE_BIT,
    xStateVariables->xEmusBmsDiagnosticCodes.uiProtectionFlags && PROTECTION_FLAG_RESERVED_14_BIT,
    xStateVariables->xEmusBmsDiagnosticCodes.uiProtectionFlags && PROTECTION_FLAG_RESERVED_15_BIT,

    xStateVariables->xEmusBmsDiagnosticCodes.uiWarningFlags && WARNING_REDUCTION_FLAG_LOW_VOLTAGE_BIT,
    xStateVariables->xEmusBmsDiagnosticCodes.uiWarningFlags && WARNING_REDUCTION_FLAG_HIGH_CURRENT_BIT,
    xStateVariables->xEmusBmsDiagnosticCodes.uiWarningFlags && WARNING_REDUCTION_FLAG_HIGH_TEMP_BIT,

    xStateVariables->xEmusBmsDiagnosticCodes.uiBatteryStatusFlags && BATTERY_STATUS_FLAG_CELL_VOLTAGES_VALIDITY_BIT,
    xStateVariables->xEmusBmsDiagnosticCodes.uiBatteryStatusFlags && BATTERY_STATUS_FLAG_CELL_MODULE_TEMPERATURES_VALIDITY_BIT,
    xStateVariables->xEmusBmsDiagnosticCodes.uiBatteryStatusFlags && BATTERY_STATUS_FLAG_CELL_BALANCING_RATES_VALIDITY_BIT,
    xStateVariables->xEmusBmsDiagnosticCodes.uiBatteryStatusFlags && BATTERY_STATUS_FLAG_NUMBER_OF_LIVE_CELLS_VALIDITY_BIT,
    xStateVariables->xEmusBmsDiagnosticCodes.uiBatteryStatusFlags && BATTERY_STATUS_FLAG_BATTERY_CHARGING_FINISHED_BIT,
    xStateVariables->xEmusBmsDiagnosticCodes.uiBatteryStatusFlags && BATTERY_STATUS_FLAG_CELL_TEMPERATURES_VALIDITY_BIT,
    xStateVariables->xEmusBmsDiagnosticCodes.uiBatteryStatusFlags && BATTERY_STATUS_FLAG_RESERVED_6_BIT,
    xStateVariables->xEmusBmsDiagnosticCodes.uiBatteryStatusFlags && BATTERY_STATUS_FLAG_RESERVED_7_BIT

    );
}
/*-----------------------------------------------------------*/

static void xEmusBmsRESTBatteryOverallToJson( char *pcWriteBuffer, size_t xWriteBufferLen, xEmusBmsStateVariables_t *xStateVariables){
    snprintf( pcWriteBuffer, xWriteBufferLen,
    "{\"success\": \"OK\", \"result\": "
        "{"
            "\"Module\": \"%s\", "
            "\"State\": \"%s\", "
            "\"Battery-Voltage-Overall-Parameters\":"
            "{"
                "\"Min-Cell-Voltage\": %.2f, "
                "\"Max-Cell-Voltage\": %.2f, "
                "\"Avg-Cell-Voltage\": %.2f, "
                "\"Total-Voltage\": %.2f"
            "}, "
            "\"Cell-Module-Temperature-Overall-Parameters\":"
            "{"
                "\"Min-Cell-Module-Temperature\": %d, "
                "\"Max-Cell-Module-Temperature\": %d, "
                "\"Avg-Cell-Module-Temperature\": %d"
            "}, "
            "\"Cell-Temperature-Overall-Parameters\":"
            "{"
                "\"Min-Cell-Temperature\": %d, "
                "\"Max-Cell-Temperature\": %d, "
                "\"Avg-Cell-Temperature\": %d"
            "}, "
            "\"Cell-Balancing-Rate-Overall-Parameters\":"
            "{"
                "\"Min-Cell-Balancing\": %d, "
                "\"Max-Cell-Balancing\": %d, "
                "\"Avg-Cell-Balancing\": %d"
            "}, "
            "\"State-of-Charge-Parameters\":"
            "{"
                "\"Current\": %.1f, "
                "\"Estimated-Charge\": %f, "
                "\"Estimated-State-Of-Charge\": %d"
            "}, "
            "\"Energy-Parameters\":"
            "{"
                "\"Estimated-Consumption\": %d, "
                "\"Estimated-Energy\": %.2f, "
                "\"Estimated-Distance-Left\": %.1f, "
                "\"Distance-Traveled\": %.1f"
            "}"
        "}"
    "}",
    xStateVariables->cModuleName,
    pcStateNameFromThread(xStateVariables->eState),

    xStateVariables->xEmusBmsBatVolOverallPars.fMinCellVoltage,
    xStateVariables->xEmusBmsBatVolOverallPars.fMaxCellVoltage,
    xStateVariables->xEmusBmsBatVolOverallPars.fAvgCellVoltage,
    xStateVariables->xEmusBmsBatVolOverallPars.fTotalVoltage,

    xStateVariables->xEmusBmsCellModuleTempOverallPars.iMinCellModuleTemp,
    xStateVariables->xEmusBmsCellModuleTempOverallPars.iMaxCellModuleTemp,
    xStateVariables->xEmusBmsCellModuleTempOverallPars.iAvgCellModuleTemp,

    xStateVariables->xEmusBmsCellTempOverallPars.iMinCellTemp,
    xStateVariables->xEmusBmsCellTempOverallPars.iMaxCellTemp,
    xStateVariables->xEmusBmsCellTempOverallPars.iAvgCellTemp,

    xStateVariables->xEmusBmsCellBalancingRateOverallPars.uiMinCellBalancing,
    xStateVariables->xEmusBmsCellBalancingRateOverallPars.uiMaxCellBalancing,
    xStateVariables->xEmusBmsCellBalancingRateOverallPars.uiAvgCellBalancing,

    xStateVariables->xEmusBmsSOC.fCurrent,
    xStateVariables->xEmusBmsSOC.fEstimatedCharge,
    xStateVariables->xEmusBmsSOC.uiEstimatedSOC,

    xStateVariables->xEmusBmsEnergyParameters.uiEstimatedConsumption,
    xStateVariables->xEmusBmsEnergyParameters.fEstimatedEnergy,
    xStateVariables->xEmusBmsEnergyParameters.fEstimatedDistanceLeft,
    xStateVariables->xEmusBmsEnergyParameters.fEstimatedDistanceTraveled
    );
}
/*-----------------------------------------------------------*/


static void xEmusBmsRESTIndividualCellsToJson( char *pcWriteBuffer, size_t xWriteBufferLen, xEmusBmsStateVariables_t *xStateVariables){
    BaseType_t i;
    snprintf( pcWriteBuffer, xWriteBufferLen,
    "{\"success\": \"OK\", \"result\": "
        "{"
            "\"Module\": \"%s\", "
            "\"State\": \"%s\", "
            "\"Expected-Number-of-Cells\": %d, "
            "\"Number-of-Live-Cells\": %d, "
            "\"Individual-Cells\":"
            "{",
    xStateVariables->cModuleName,
    pcStateNameFromThread(xStateVariables->eState),
    NUMBER_OF_CELLS,
    xStateVariables->xEmusBmsOverallPars.uiNumberOfLiveCells
    );


    for(i = 0; i<NUMBER_OF_CELLS; i++){
        snprintf( pcWriteBuffer + strlen(pcWriteBuffer), xWriteBufferLen - strlen(pcWriteBuffer),
            "\"%d\": "
            "{"
              "\"Voltage\": %.2f, "
              "\"Balancing\": %d, "
              "\"Temperature\": %d"
            "}, ",
            i,
            xStateVariables->fEmusBmsIndividualCellVoltages[i],
            xStateVariables->uiEmusBmsIndividualCellBalancingRate[i],
            xStateVariables->uiEmusBmsIndividualCellTemperatures[i]
         );
    }

    snprintf( pcWriteBuffer + strlen(pcWriteBuffer) - 2, xWriteBufferLen - strlen(pcWriteBuffer) + 2,
            "}"
        "}"
    "}"
     );
}
/*-----------------------------------------------------------*/

BaseType_t xEmusBmsREST(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString){
    BaseType_t lParameterStringLength, xReturn;
    xEmusBmsStateVariables_t xStateVariables;
    xAppMsgBaseType_t xMsg = {xQueueRestAPIEmusBmsResponseHandle, GET_STATUS_SIG};
    char * pcParameter;

    /* Obtain the signal string. */
    pcParameter = ( char * ) FreeRTOS_RESTGetParameter
                            (
                                pcCommandString,        /* The command string itself. */
                                1,                      /* Return the first parameter. */
                                &lParameterStringLength /* Store the parameter string length. */
                            );

    ( void ) lParameterStringLength;

    xReturn = pdFALSE;
    ( void ) pcWriteBuffer;
    ( void ) xWriteBufferLen;
    ( void ) pcCommandString;

    xMsg.eSignal = (ESignal) uiThreadSignalFromCommand(pcParameter);

    switch(xMsg.eSignal){
    case OFFLINE_SIG:
    case GO_ONLINE_SIG:
    case IDLE_SIG:
    case FAULT_SIG:
    case TIMEOUT_SIG:
        xQueueSend(xQueueBmsInputSignalHandle, &xMsg, NULL);
        snprintf( pcWriteBuffer, xWriteBufferLen, "{\"success\": \"OK\", \"result\": {\"message\": \"executed\"}}" );
        break;
    case GET_STATUS_SIG:
    case GET_PARS_OVERALL_SIG:
    case GET_DIAG_CODES_SIG:
    case GET_BAT_OVERALL_SIG:
    case GET_INDIV_CELLS_SIG:
        if(xQueueSend(xQueueBmsInputSignalHandle, &xMsg, NULL) == pdTRUE){
            if(xQueueReceive(xQueueRestAPIEmusBmsResponseHandle, &xStateVariables, 1000) == pdTRUE){
                switch(xMsg.eSignal){
                case GET_STATUS_SIG:
                    xEmusBmsRESTStatusToJson(pcWriteBuffer, xWriteBufferLen, &xStateVariables);
                    break;
                case GET_PARS_OVERALL_SIG:
                    xEmusBmsRESTOverallParametersToJson(pcWriteBuffer, xWriteBufferLen, &xStateVariables);
                    break;
                case GET_DIAG_CODES_SIG:
                    xEmusBmsRESTDiagnosticCodesToJson(pcWriteBuffer, xWriteBufferLen, &xStateVariables);
                    break;
                case GET_BAT_OVERALL_SIG:
                    xEmusBmsRESTBatteryOverallToJson(pcWriteBuffer, xWriteBufferLen, &xStateVariables);
                    break;
                case GET_INDIV_CELLS_SIG:
                    xEmusBmsRESTIndividualCellsToJson(pcWriteBuffer, xWriteBufferLen, &xStateVariables);
                    break;
                default:
                    xEmusBmsRESTStatusToJson(pcWriteBuffer, xWriteBufferLen, &xStateVariables);
                    break;
                }
            }
            else
            {
                snprintf( pcWriteBuffer, xWriteBufferLen, "{\"success\": \"OK\", \"result\": \"response error\"}" );
            }
        }
        else
        {
            snprintf( pcWriteBuffer, xWriteBufferLen, "{\"success\": \"OK\", \"result\": \"send error\"}" );
        }
        break;
    default:
        snprintf( pcWriteBuffer, xWriteBufferLen, "{\"success\": \"NOK\", \"result\": \"command not found\"}" );
        break;
    }

    return xReturn;
}
/*-----------------------------------------------------------*/

void xMotorControllerRESTStatusToJson( char *pcWriteBuffer, size_t xWriteBufferLen, xMotorControllerStateVariables_t *xStateVariables){
//    snprintf( pcWriteBuffer, xWriteBufferLen, "abc");
    snprintf( pcWriteBuffer, xWriteBufferLen,
      "{\"success\": \"OK\", \"result\": "
          "{"
              "\"Module\": \"%s\","
              "\"State\": \"%s\","
              "\"Live-Data\": "
              "{"
                  "\"Motor-Connected\": %s,"
                  "\"Motor-Algorithm\": \"%s\","
                  "\"High-Priority-Limiter\": %d,"
                  "\"Motor-Mode\": \"%s\","
                  "\"Temp-MCU\": %d,"
                  "\"Low-Priority-Limiter\": %d,"
                  "\"Device-Error\": %d,"
                  "\"Urange\": %d,"
                  "\"Irange\": %d,"
                  "\"Uref\": %d,"
                  "\"Iref\": %d,"
                  "\"Relative-Current\": %d,"
                  "\"Current\": %f,"
                  "\"Relative-Voltage\": %d,"
                  "\"Voltage\": %f,"
                  "\"Total-Charge\": %d,"
                  "\"Relative-Amplitude-Phase-Current\": %d,"
                  "\"Relative-Amplitude-Link-Voltage\": %d,"
                  "\"Energy-Passed\": %d,"
                  "\"Relative-Motor-Power\": %d,"
                  "\"Motor-Power\": %f,"
                  "\"Mechanical-Angle\": %d,"
                  "\"Relative-Rotor-Speed\": %d,"
                  "\"Rotor-Speed\": %d,"
                  "\"Odometer\": %d,"
                  "\"Temp-Motor1\": %d,"
                  "\"Temp-Motor2\": %d"
              "}"
          "}"
      "}",
      xStateVariables->cModuleName,
      pcStateNameFromThread(xStateVariables->eState),
      xStateVariables->xMCLiveData.uiMotorConnected ? "true" : "false",
      xStateVariables->xMCLiveData.uiMotorAlgorithm == 1 ? "bldc" : xStateVariables->xMCLiveData.uiMotorAlgorithm == 2 ? "vector" : "incorrect",
      xStateVariables->xMCLiveData.uiHighPriorityLimiter,
      pcMotorMode(xStateVariables->xMCLiveData.uiMotorMode),
      xStateVariables->xMCLiveData.iTempMCU,
      xStateVariables->xMCLiveData.uiLowPriorityLimiter,
      xStateVariables->xMCLiveData.uiDeviceError,
      xStateVariables->xMCLiveData.uiUrange,
      xStateVariables->xMCLiveData.uiIrange,
      xStateVariables->xMCLiveData.uiUref,
      xStateVariables->xMCLiveData.uiIref,
      xStateVariables->xMCLiveData.iRelCurrent,
      xStateVariables->xMCLiveData.fCurrent,
      xStateVariables->xMCLiveData.iRelVoltage,
      xStateVariables->xMCLiveData.fVoltage,
      xStateVariables->xMCLiveData.iTotalCharge,
      xStateVariables->xMCLiveData.iRelAmplitudePhaseCurrent,      //base (24 def) + 6
      xStateVariables->xMCLiveData.iRelAplitudeLinkVoltage,
      xStateVariables->xMCLiveData.iEnergyPassed,
      xStateVariables->xMCLiveData.iRelMotorPower,                 //base (24 def) + 7
      xStateVariables->xMCLiveData.fMotorPower,
      xStateVariables->xMCLiveData.iMechanicalAngle,
      xStateVariables->xMCLiveData.iRelRotorSpeed,
      xStateVariables->xMCLiveData.iRotorSpeed,
      xStateVariables->xMCLiveData.iOdometer,
      xStateVariables->xMCLiveData.iTempMotor1,
      xStateVariables->xMCLiveData.iTempMotor2);
}
/*-----------------------------------------------------------*/

BaseType_t xMotorControllerREST(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString){
    BaseType_t lParameterStringLength, xReturn;
    xMotorControllerStateVariables_t xStateVariables;
    xAppMsgBaseType_t xMsg = {xQueueRestAPIMotorCtrlResponseHandle, GET_STATUS_SIG};
    char * pcParameter;

    /* Obtain the signal string. */
    pcParameter = ( char * ) FreeRTOS_RESTGetParameter
                            (
                                pcCommandString,        /* The command string itself. */
                                1,                      /* Return the first parameter. */
                                &lParameterStringLength /* Store the parameter string length. */
                            );

    ( void ) lParameterStringLength;

    xReturn = pdFALSE;
    ( void ) pcWriteBuffer;
    ( void ) xWriteBufferLen;
    ( void ) pcCommandString;

    xMsg.eSignal = (ESignal) uiThreadSignalFromCommand(pcParameter);

    switch(xMsg.eSignal){
    case OFFLINE_SIG:
    case GO_ONLINE_SIG:
    case TIMEOUT_SIG:
        xQueueSend(xQueueMotorCtrlInputSignalHandle, &xMsg, NULL);
        snprintf( pcWriteBuffer, xWriteBufferLen, "{\"success\": \"OK\", \"result\": {\"message\": \"executed\"}}" );
        break;
    case GET_STATUS_SIG:
        if(xQueueSend(xQueueMotorCtrlInputSignalHandle, &xMsg, NULL) == pdTRUE){
            if(xQueueReceive(xQueueRestAPIMotorCtrlResponseHandle, &xStateVariables, 1000) == pdTRUE){
                xMotorControllerRESTStatusToJson(pcWriteBuffer, xWriteBufferLen, &xStateVariables);
            }
            else
            {
                snprintf( pcWriteBuffer, xWriteBufferLen, "{\"success\": \"OK\", \"result\": \"response error\"}" );
            }
        }
        else
        {
            snprintf( pcWriteBuffer, xWriteBufferLen, "{\"success\": \"OK\", \"result\": \"send error\"}" );
        }
        break;
    default:
        break;
    }

    return xReturn;
}
/*-----------------------------------------------------------*/
