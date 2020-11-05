/*
 * APP_config.h
 *
 *  Created on: 5 lis 2020
 *      Author: Krzysztof Klimek
 */

#ifndef APP_CONFIG_H_
#define APP_CONFIG_H_

/* Controller input signals */
typedef enum {
    CONTROLLER_NONE_SIG,    // not used, first
    CONTROLLER_START_SIG,
    CONTROLLER_STOP_SIG,
    CONTROLLER_PAUSE_SIG,
    CONTROLLER_EMERGENCY_SIG,
    CONTROLLER_SHORT_PRESS_SIG,
    CONTROLLER_LAST_SIG     // not used, last
} ECtrlInputSignal;

/* Controller states */
typedef enum {
    CONTROLLER_NONE_STATE,    // not used, first
    CONTROLLER_START_STATE,
    CONTROLLER_STOP_STATE,
    CONTROLLER_LAST_STATE     // not used, last
} EControllerState;

#endif /* INCLUDE_APP_CONFIG_H_ */
