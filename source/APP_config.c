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

//xThreadMapping_t xMainControllerMapping[] =
//{
//    { CONTROLLER_NONE_SIG,              CONTROLLER_NONE_STATE,          "unknown",      "unknown"   },
//    { CONTROLLER_RUN_SIG,               CONTROLLER_RUNNING_STATE,       "run",          "running"   },
//    { CONTROLLER_STOP_SIG,              CONTROLLER_STOP_STATE,          "stop",         "stopped"   },
//    { CONTROLLER_PAUSE_SIG,             CONTROLLER_PAUSED_STATE,        "pause",        "paused"    },
//    { CONTROLLER_EMERGENCY_SIG,         CONTROLLER_EMERGENCY_STATE,     "emergency",    "emergency" },
//    { CONTROLLER_GET_STATUS_SIG,        CONTROLLER_NONE_STATE,          "status",       ""          }
//};
//
//xThreadMapping_t xTinyBmsMapping[] =
//{
//    { BMS_NONE_SIG,                     BMS_NONE_STATE,                 "unknown",          "unknown"       },
//    { BMS_OFFLINE_SIG,                  BMS_OFFLINE_STATE,              "go-offline",       "offline"       },
//    { BMS_CHARGING_SIG,                 BMS_CHARGING_STATE,             "charge",           "charging"      },
//    { BMS_FULLY_CHARGED_SIG,            BMS_FULLY_CHARGED_STATE,        "finish-charge",    "charged"       },
//    { BMS_DISCHARGING_SIG,              BMS_DISCHARGING_STATE,          "discharge",        "discharging"   },
//    { BMS_REGENERATION_SIG,             BMS_REGENERATION_STATE,         "regenerate",       "regenerating"  },
//    { BMS_IDLE_SIG,                     BMS_IDLE_STATE,                 "go-idle",          "idle"          },
//    { BMS_FAULT_SIG,                    BMS_FAULT_STATE,                "go-fault",         "fault"         },
//    { BMS_GET_STATUS_SIG,               BMS_NONE_STATE,                 "status",           ""              }
//};

xThreadMapping_t xThreadMapping[] =
{
    { NONE_SIG,              NONE_STATE,             "unknown",             "unknown"       },
    { GET_STATUS_SIG,        NONE_STATE,             "status",              ""              },
    { RUN_SIG,               RUNNING_STATE,          "run",                 "running"       },
    { STOP_SIG,              STOP_STATE,             "stop",                "stopped"       },
    { PAUSE_SIG,             PAUSED_STATE,           "pause",               "paused"        },
    { EMERGENCY_SIG,         EMERGENCY_STATE,        "emergency",           "emergency"     },
    { OFFLINE_SIG,           OFFLINE_STATE,          "go-offline",          "offline"       },
    { GO_ONLINE_SIG,         GOING_ONLINE_STATE,     "go-online",           "going-online"  },
    { READ_SIG,              READING_STATE,          "read",                "reading"       },
    { WRITE_SIG,             WRITING_STATE,          "write",               "writing"       },
    { CHARGING_SIG,          CHARGING_STATE,         "charge",              "charging"      },
    { FULLY_CHARGED_SIG,     FULLY_CHARGED_STATE,    "finish-charge",       "charged"       },
    { DISCHARGING_SIG,       DISCHARGING_STATE,      "discharge",           "discharging"   },
    { REGENERATION_SIG,      REGENERATION_STATE,     "regenerate",          "regenerating"  },
    { IDLE_SIG,              IDLE_STATE,             "go-idle",             "idle"          },
    { FAULT_SIG,             FAULT_STATE,            "go-fault",            "fault"         },
    { TIMEOUT_SIG,           FAULT_STATE,            "timeout",             ""              },
    { GET_PARS_OVERALL_SIG,  NONE_STATE,             "parameters-overall",  ""              },
    { GET_DIAG_CODES_SIG,    NONE_STATE,             "diagnostic-codes",    ""              },
    { GET_BAT_OVERALL_SIG,   NONE_STATE,             "battery-overall",     ""              },
    { GET_INDIV_CELLS_SIG,   NONE_STATE,             "individual-cells",    ""              },
};

#if !defined( ARRAY_SIZE )
    #define ARRAY_SIZE(x) ( BaseType_t ) (sizeof ( x ) / sizeof ( x )[ 0 ] )
#endif

ESignal uiThreadSignalFromCommand(char *pcCommandString)
{
    ESignal eSignal = NONE_SIG;
    BaseType_t x;

    for( x = 0; x < ARRAY_SIZE( xThreadMapping ); x++ )
    {
        if (strcmp(xThreadMapping[x].cCommand, pcCommandString) == 0){
            eSignal = (ESignal) xThreadMapping[x].eSignal;
            break;
        }
    }
    return eSignal;
}
/*-----------------------------------------------------------*/

const char * pcStateNameFromThread(EState eState)
{
    const char *pcStateName = xThreadMapping[0].cState;
    BaseType_t x;

    for( x = 0; x < ARRAY_SIZE( xThreadMapping ); x++ )
        {
            if (xThreadMapping[x].eState == eState){
                pcStateName = xThreadMapping[x].cState;
                break;
            }
        }

    return pcStateName;
}
/*-----------------------------------------------------------*/
