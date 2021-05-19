/*
 * FSM_MainController.h
 *
 *  Created on: 16 lis 2020
 *      Author: Krzysztof Klimek
 */

#ifndef FSM_EMUSBMS_H_
#define FSM_EMUSBMS_H_

#include "math.h"
#include "stdio.h"
#include "stdlib.h"
#include "HL_can.h"

/* HALCoGen generated headers has moved to sys_main.h*/
#include "sys_main.h"

/* Time related functions */
#include "rti_runtimestats.h"

#include "FreeRTOS.h"
#include "os_task.h"
#include "os_queue.h"
#include "os_timer.h"
#include "os_semphr.h"

#include "APP_config.h"
#include "APP_queues.h"

/* FSM Task */
void vEmusBmsFSMTask(void *pvParameters);

#define INPUT_SIGNALS_IGNITION_KEY_BIT             (0u)
#define INPUT_SIGNALS_CHARGER_MAINS_BIT            (1u)
#define INPUT_SIGNALS_FAST_CHARGE_BIT              (2u)
#define INPUT_SIGNALS_LEAKAGE_SENSOR_BIT           (3u)

#define OUTPUT_SIGNALS_CHARGER_ENABLE_BIT          (0u)
#define OUTPUT_SIGNALS_HEATER_ENABLE_BIT           (1u)
#define OUTPUT_SIGNALS_BATTERY_CONTACTOR_BIT       (2u)
#define OUTPUT_SIGNALS_BATTERY_FAN_BIT             (3u)
#define OUTPUT_SIGNALS_POWER_REDUCTION_BIT         (4u)
#define OUTPUT_SIGNALS_CHARGING_INTERLOCK_BIT      (5u)
#define OUTPUT_SIGNALS_DCDC_CONTROL_BIT            (6u)
#define OUTPUT_SIGNALS_CONTACTOR_PRECHARGE_BIT     (7u)

#define PROTECTION_FLAG_UNDER_VOLTAGE_BIT          (0u)
#define PROTECTION_FLAG_OVER_VOLTAGE_BIT           (1u)
#define PROTECTION_FLAG_DISCHARGE_OVER_CURRENT_BIT (2u)
#define PROTECTION_FLAG_CHARGE_OVER_CURRENT_BIT    (3u)
#define PROTECTION_FLAG_CELL_MODULE_OVERHEAT_BIT   (4u)
#define PROTECTION_FLAG_LEAKAGE_BIT                (5u)
#define PROTECTION_FLAG_NO_CELL_COMMUNICATION_BIT  (6u)
#define PROTECTION_FLAG_RESERVED_7_BIT             (7u)
#define PROTECTION_FLAG_RESERVED_8_BIT             (8u)
#define PROTECTION_FLAG_RESERVED_9_BIT             (9u)
#define PROTECTION_FLAG_RESERVED_10_BIT            (10u)
#define PROTECTION_FLAG_CELL_OVERHEAT_BIT          (11u)
#define PROTECTION_FLAG_NO_CURRENT_SENSOR_BIT      (12u)
#define PROTECTION_FLAG_PACK_UNDER_VOLTAGE_BIT     (13u)
#define PROTECTION_FLAG_RESERVED_14_BIT            (14u)
#define PROTECTION_FLAG_RESERVED_15_BIT            (15u)

#define WARNING_REDUCTION_FLAG_LOW_VOLTAGE_BIT     (0u)
#define WARNING_REDUCTION_FLAG_HIGH_CURRENT_BIT    (1u)
#define WARNING_REDUCTION_FLAG_HIGH_TEMP_BIT       (2u)

#define BATTERY_STATUS_FLAG_CELL_VOLTAGES_VALIDITY_BIT              (0u)
#define BATTERY_STATUS_FLAG_CELL_MODULE_TEMPERATURES_VALIDITY_BIT   (1u)
#define BATTERY_STATUS_FLAG_CELL_BALANCING_RATES_VALIDITY_BIT       (2u)
#define BATTERY_STATUS_FLAG_NUMBER_OF_LIVE_CELLS_VALIDITY_BIT       (3u)
#define BATTERY_STATUS_FLAG_BATTERY_CHARGING_FINISHED_BIT           (4u)
#define BATTERY_STATUS_FLAG_CELL_TEMPERATURES_VALIDITY_BIT          (5u)
#define BATTERY_STATUS_FLAG_RESERVED_6_BIT                          (6u)
#define BATTERY_STATUS_FLAG_RESERVED_7_BIT                          (7u)

enum eChargingStage {
    CHARGING_STAGE_DISCONNECTED,
    CHARGING_STAGE_PRE_HEATING,
    CHARGING_STAGE_PRE_CHARGING,
    CHARGING_STAGE_MAIN_CHARGING,
    CHARGING_STAGE_BALANCING,
    CHARGING_STAGE_CHARGING_FINISHED,
    CHARGING_STAGE_CHARGING_ERROR
    // etc.
};

static inline char *stringFromChargingStage(enum eChargingStage ecs)
{
    static const char *strings[] = {
        "disconnected",
        "pre-heating",
        "pre-charging",
        "main-charging",
        "balancing",
        "charging finished",
        "charging error"
        /* continue for rest of values */
    };

    return strings[ecs];
}

enum eLastChargingError {
    CHARGING_ERROR_NO_ERROR,
    CHARGING_ERROR_NO_COMM_OR_LOST_PRECHARGING,
    CHARGING_ERROR_NO_COMM,
    CHARGING_ERROR_MAX_CHG_DURATION_EXPIRED,
    CHARGING_ERROR_NO_COMM_OR_LOST_CHARGING_BALANCING,
    CHARGING_ERROR_SET_MODULE_BALANCING_TRESHOLD,
    CHARGING_ERROR_CELL_TEMP_TOO_HIGH,
    CHARGING_ERROR_NO_COMM_OR_LOST_PREHEATING,
    CHARGING_ERROR_CELL_NO_MISMATCH,
    CHARGING_ERROR_CELL_OVER_VOLTAGE,
    CHARGING_ERROR_CELL_PROTECTION_EVENT
    // etc.
};

static inline char *stringFromLastChargingError(enum eChargingStage ecs)
{
    static const char *strings[] = {
        "No error",
        "No cell communication at the start of charging or communication lost during Pre-charging (using CAN charger), cannot charge",
        "No cell communication (using non-CAN charger), cannot charge",
        "Maximum charging stage duration expired",
        "Cell communication lost during Main Charging or Balancing stage (using CAN charger), cannot continue charging",
        "Cannot set cell module balancing threshold",
        "Cell or cell module temperature too high",
        "Cell communication lost during Pre-heating stage (using CAN charger)",
        "Number of cells mismatch",
        "Cell over-voltage",
        "Cell protection event occurred, see “Diagnostic Codes” message for determining specific protection reason"
        /* continue for rest of values */
    };

    return strings[ecs];
}

typedef struct {
    uint8_t uiInputSignals;
    uint8_t uiOutputSignals;
    uint16_t uiNumberOfLiveCells;
    uint8_t uiChargingStage;
    uint16_t uiChargingStageDuration;
    uint8_t uiLastChargingError;
} xEmusBmsOverallPars_t;

typedef struct {
    uint16_t uiProtectionFlags;
    uint8_t uiWarningFlags;
    uint8_t uiBatteryStatusFlags;
} xEmusBmsDiagnosticCodes_t;

typedef struct {
    float_t fMinCellVoltage;
    float_t fMaxCellVoltage;
    float_t fAvgCellVoltage;
    float_t fTotalVoltage;
} xEmusBmsBatVolOverallPars_t;

typedef struct {
    int8_t iMinCellModuleTemp;
    int8_t iMaxCellModuleTemp;
    int8_t iAvgCellModuleTemp;
} xEmusBmsCellModuleTempOverallPars_t;

typedef struct {
    int8_t iMinCellTemp;
    int8_t iMaxCellTemp;
    int8_t iAvgCellTemp;
} xEmusBmsCellTempOverallPars_t;

typedef struct {
    uint8_t uiMinCellBalancing;
    uint8_t uiMaxCellBalancing;
    uint8_t uiAvgCellBalancing;
} xEmusBmsCellBalancingRateOverallPars_t;

typedef struct {
    float_t fCurrent;
    float_t fEstimatedCharge;
    uint8_t uiEstimatedSOC;
} xEmusBmsSOC_t;

typedef struct {
    uint16_t uiEstimatedConsumption;
    float_t fEstimatedEnergy;
    float_t fEstimatedDistanceLeft;
    float_t fEstimatedDistanceTraveled;
} xEmusBmsEnergyParameters_t;

/* EmusBms thread struct */
typedef struct {
    EState eState;
    char cModuleName[20];

    float_t fEmusBmsIndividualCellVoltages[NUMBER_OF_CELLS];
    uint8_t uiEmusBmsIndividualCellModuleTemperatures[1];
    uint8_t uiEmusBmsIndividualCellTemperatures[NUMBER_OF_CELLS];
    uint8_t uiEmusBmsIndividualCellBalancingRate[NUMBER_OF_CELLS];

    xEmusBmsOverallPars_t xEmusBmsOverallPars;
    xEmusBmsDiagnosticCodes_t xEmusBmsDiagnosticCodes;
    xEmusBmsBatVolOverallPars_t xEmusBmsBatVolOverallPars;
    xEmusBmsCellModuleTempOverallPars_t xEmusBmsCellModuleTempOverallPars;
    xEmusBmsCellTempOverallPars_t xEmusBmsCellTempOverallPars;
    xEmusBmsCellBalancingRateOverallPars_t xEmusBmsCellBalancingRateOverallPars;
    xEmusBmsSOC_t xEmusBmsSOC;
    xEmusBmsEnergyParameters_t xEmusBmsEnergyParameters;

} xEmusBmsStateVariables_t;

/* The structure that defines FSM structure */
typedef struct
{
    QueueHandle_t xEventQueue;
    TimerHandle_t *pxTimer;
    xEmusBmsStateVariables_t xStateVariables;
    canBASE_t *pCanNode;
    uint32_t uiCanID;
    QueueHandle_t xCANQueue;
} FSM_EmusBms_Definition_t;
/****************** EmusBMS END ******************/


#endif /* FSM_EMUSBMS_H_ */
