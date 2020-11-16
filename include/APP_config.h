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
#include "math.h"

#define appRestAPI_BUFFER_SIZE ( 2048 )
#define appCOMMAND_LENGTH (32)

/************* MAIN CONTROLLER *************/
/* Controller input signals */
typedef enum {
    CONTROLLER_NONE_SIG,            // not used, first
    CONTROLLER_ENTRY_SIG,
    CONTROLLER_EXIT_SIG,
    CONTROLLER_RUN_SIG,
    CONTROLLER_STOP_SIG,
    CONTROLLER_PAUSE_SIG,
    CONTROLLER_EMERGENCY_SIG,
    CONTROLLER_SHORT_PRESS_SIG,     // last main controller
    CONTROLLER_GET_STATUS_SIG,      //generic to all tasks
    CONTROLLER_LAST_SIG             // not used, last
} ECtrlInputSignal;

/* Controller states */
typedef enum {
    CONTROLLER_NONE_STATE,          // not used, first
    CONTROLLER_RUNNING_STATE,
    CONTROLLER_STOP_STATE,
    CONTROLLER_PAUSED_STATE,
    CONTROLLER_EMERGENCY_STATE,
    CONTROLLER_LAST_STATE,          // not used, last
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
/************* MAIN CONTROLLER END *************/

/****************** BMS ******************/
#define NUMBER_OF_CELLS         16
/* BMS input signals */
typedef enum {
    BMS_NONE_SIG,            // not used, first
    BMS_ENTRY_SIG,
    BMS_EXIT_SIG,
    BMS_OFFLINE_SIG,
    BMS_CHARGING_SIG,
    BMS_FULLY_CHARGED_SIG,
    BMS_DISCHARGING_SIG,
    BMS_REGENERATION_SIG,
    BMS_IDLE_SIG,
    BMS_FAULT_SIG,
    BMS_GET_STATUS_SIG,      //generic to all tasks
    BMS_LAST_SIG             // not used, last
} EBmsInputSignal;

/* BMS states */
typedef enum {
    BMS_NONE_STATE,          // not used, first
    BMS_OFFLINE_STATE,
    BMS_CHARGING_STATE,
    BMS_FULLY_CHARGED_STATE,
    BMS_DISCHARGING_STATE,
    BMS_REGENERATION_STATE,
    BMS_IDLE_STATE,
    BMS_FAULT_STATE,
    BMS_LAST_STATE            // not used, last
} EBmsState;

/* Bms thread settings struct */
typedef struct {
    uint16_t uiCellVoltages[NUMBER_OF_CELLS];       // reg: 0-15; res: 0.1mV
    /****************************** RESERVED *******************************/
    uint32_t uiLifetimeCounter;                     // reg: 32/3; res: 1s
    uint32_t uiTimeLeft;                            // reg: 34/5; res: 1s
    float_t fBatteryPackVoltage;                    // reg: 36/7; res: 1V
    float_t fBatteryPackCurrent;                    // reg: 38/9; res: 1A
    uint16_t uiBatteryMinCellVoltage;               // reg: 40/1; res: 1mV
    uint16_t uiBatteryMaxCellVoltage;               // reg: 38/9; res: 1mV
    int16_t iExtSensorTemp1;                        // reg: 42;   res: 0.1C
    int16_t iExtSensorTemp2;                        // reg: 43;   res: 0.1C
    uint16_t uiDistanceLeftToEmptyBat;              // reg: 44;   res: 1km
    /****************************** RESERVED *******************************/
    uint32_t uiStateOfCharge;                       // reg: 46/7; res: 0.000001%
    int16_t iTempInternal;                          // reg: 48;   res: 0.1C
    /****************************** RESERVED *******************************/
    EBmsState eOnlineStatus;                        // reg: 50;   res: 0x91-Charging, 0x92-Fully Charged, 0x93-Discharging, 0x96-Regenertion, 0x97-Idle, 0x9B-Fault
    /****************************** RESERVED *******************************/
    uint16_t uiBalancingDecisionBits;               // reg: 51;   res: First Cell - LSB Bit of LSB Byte: 1 - need balancing, 0 - cell no need balance
    uint16_t uiRealBalancingBits;                   // reg: 52;   res: First Cell - LSB Bit of LSB Byte: 1 - need balancing, 0 - cell no need balance
    uint16_t uiNoDetectedCells;                     // reg: 53;
    float_t fSpeed;                                 // reg: 54/5; res: 1km/h
} xBmsLvdt_t;

/* Bms thread settings struct */
typedef struct {
    uint16_t uiFullyChargedVoltage;                 // reg: 300;  res: 1mV
    uint16_t uiFullyDischargedVoltage;              // reg: 301;  res: 1mV
    /****************************** RESERVED *******************************/
    uint16_t uiEarlyBalancingThreshold;             // reg: 303;  res: 1mV
    uint16_t uiChargeFinishedCurrent;               // reg: 304;  res: 1mV
    /****************************** RESERVED *******************************/
    uint16_t uiBatteryCapacity;                     // reg: 306;  res: 0.01Ah
    uint16_t uiNumberOfSeriesCells;                 // reg: 307;  res: 1 cell count
    uint16_t uiAllowedDisbalance;                   // reg: 308;  res: 1mV
    /****************************** RESERVED *******************************/
    uint32_t uiPulsesPerUnit;                       // reg: 312/3; res: 1 pulse/unit
    uint16_t uiDistanceUnitName;                    // reg: 314;  res: 0x01-Meter, 0x02-Kilometer, 0x03-Feet, 0x04-Mile, 0x05-Yard
    uint16_t uiOverVoltageCutoff;                   // reg: 315;  res: 1mV
    uint16_t uiUnderVoltageCutoff;                  // reg: 316;  res: 1mV
    uint16_t uiDischargeOverCurrentCutoff;          // reg: 317;  res: 1A
    uint16_t uiChargeOverCurrentCutoff;             // reg: 318;  res: 1A
    int16_t iOverHeatCutoff;                        // reg: 319;  res: 1C
    int16_t iLowTempChargerCutoff;                  // reg: 319;  res: 1C
} xBmsSettings_t;

/* Bms thread struct */
typedef struct {
    uint8_t eState;
    char cModuleName[20];
    xBmsLvdt_t xBmsLiveData;
    xBmsSettings_t xBmsSettings;
} xBmsStateVariables_t
/****************** BMS END ******************/;

typedef struct {
    uint8_t eSignal;
    uint8_t eState;
    const char *cCommand;
    const char *cState;
} xThreadMapping_t;

extern xThreadMapping_t xMainControllerMapping[];
extern xThreadMapping_t xTinyBmsMapping[];

/* Move to APP_thread_processing */
uint8_t uiThreadSignalFromCommand(xThreadMapping_t *xThread, char *pcCommandString);
const char * pcStateNameFromThread(uint8_t eState);

#endif /* INCLUDE_APP_CONFIG_H_ */
