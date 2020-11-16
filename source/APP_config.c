/*
 * APP_config.c
 *
 *  Created on: 5 lis 2020
 *      Author: Win 7 PL T430
 */

#include "APP_config.h"

/* Standard includes. */
#include <string.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "os_task.h"

//struct xTYPE_COUPLE
//{
//    const char *pcExtension;
//    const char *pcType;
//};
//
//struct xTYPE_COUPLE pxTypeCouples[ ] =
//{
//    { "html", "text/html" },
//    { "css",  "text/css" },
//    { "js",   "text/javascript" },
//    { "png",  "image/png" },
//    { "jpg",  "image/jpeg" },
//    { "gif",  "image/gif" },
//    { "txt",  "text/plain" },
//    { "mp3",  "audio/mpeg3" },
//    { "wav",  "audio/wav" },
//    { "flac", "audio/ogg" },
//    { "pdf",  "application/pdf" },
//    { "ttf",  "application/x-font-ttf" },
//    { "ttc",  "application/x-font-ttf" }
//};

xThreadMapping_t xMainControllerMapping[] =
{
    { CONTROLLER_NONE_SIG,              CONTROLLER_NONE_STATE,          "unknown",      "unknown"   },
    { CONTROLLER_RUN_SIG,               CONTROLLER_RUNNING_STATE,       "run",          "running"   },
    { CONTROLLER_STOP_SIG,              CONTROLLER_STOP_STATE,          "stop",         "stopped"   },
    { CONTROLLER_PAUSE_SIG,             CONTROLLER_PAUSED_STATE,        "pause",        "paused"    },
    { CONTROLLER_EMERGENCY_SIG,         CONTROLLER_EMERGENCY_STATE,     "emergency",    "emergency" },
    { CONTROLLER_GET_STATUS_SIG,        CONTROLLER_NONE_STATE,          "status",       ""          }
};

xThreadMapping_t xTinyBmsMapping[] =
{
    { BMS_NONE_SIG,                     BMS_NONE_STATE,                 "unknown",          "unknown"       },
    { BMS_OFFLINE_SIG,                  BMS_OFFLINE_STATE,              "go-offline",       "offline"       },
    { BMS_CHARGING_SIG,                 BMS_CHARGING_STATE,             "charge",           "charging"      },
    { BMS_FULLY_CHARGED_SIG,            BMS_FULLY_CHARGED_STATE,        "finish-charge",    "charged"       },
    { BMS_DISCHARGING_SIG,              BMS_DISCHARGING_STATE,          "discharge",        "discharging"   },
    { BMS_REGENERATION_SIG,             BMS_REGENERATION_STATE,         "regenerate",       "regenerating"  },
    { BMS_IDLE_SIG,                     BMS_IDLE_STATE,                 "go-idle",          "idle"          },
    { BMS_FAULT_SIG,                    BMS_FAULT_STATE,                "go-fault",         "fault"         },
    { BMS_GET_STATUS_SIG,               BMS_NONE_STATE,                 "status",           ""              }
};

#if !defined( ARRAY_SIZE )
    #define ARRAY_SIZE(x) ( BaseType_t ) (sizeof ( x ) / sizeof ( x )[ 0 ] )
#endif

ECtrlInputSignal uiThreadSignalFromCommand(xThreadMapping_t *xThread, char *pcCommandString)
{
    ECtrlInputSignal eSignal = CONTROLLER_NONE_SIG;
    BaseType_t x;

    for( x = 0; x < ARRAY_SIZE( xThread ); x++ )
    {
        if (strcmp(xThread[x].cCommand, pcCommandString) == 0){
            eSignal = (ECtrlInputSignal) xThread[x].eSignal;
            break;
        }
    }
    return eSignal;
}
/*-----------------------------------------------------------*/

const char * pcStateNameFromThread(ECtrlInputSignal eState)
{
    const char *pcStateName = xMainControllerMapping[0].cState;
    BaseType_t x;

    for( x = 0; x < ARRAY_SIZE( xMainControllerMapping ); x++ )
        {
            if (xMainControllerMapping[x].eState == eState){
                pcStateName = xMainControllerMapping[x].cState;
                break;
            }
        }

    return pcStateName;
}
/*-----------------------------------------------------------*/
