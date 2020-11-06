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
    { CONTROLLER_START_SIG,             CONTROLLER_START_STATE,         "start",        "started"   },
    { CONTROLLER_STOP_SIG,              CONTROLLER_STOP_STATE,          "stop",         "stopped"   },
    { CONTROLLER_PAUSE_SIG,             CONTROLLER_PAUSE_STATE,         "pause",        "paused"    },
    { CONTROLLER_EMERGENCY_SIG,         CONTROLLER_EMERGENCY_STATE,     "emergency",    "emergency" },
    { CONTROLLER_GET_STATUS_CLI_SIG,    CONTROLLER_NONE_STATE,          "status",       ""          },
    { CONTROLLER_GET_STATUS_REST_SIG,   CONTROLLER_NONE_STATE,          "status",       ""          }
};

#if !defined( ARRAY_SIZE )
    #define ARRAY_SIZE(x) ( BaseType_t ) (sizeof ( x ) / sizeof ( x )[ 0 ] )
#endif

uint8_t uiThreadSignalFromCommand(xThreadMapping_t *xThread, char *pcCommandString)
{
    uint8_t eSignal = 0;
    BaseType_t x;

    for( x = 0; x < 5; x++ )
    {
        if (strcmp(xThread[x].cCommand, pcCommandString) == 0){
            eSignal = xThread[x].eSignal;
            break;
        }
    }
    return eSignal;
}
/*-----------------------------------------------------------*/

//const char * pcStateNameFromThread(uint8_t eState)
//{
//    char *pcStateName;
//
//
//
//    return NULL;
//}
/*-----------------------------------------------------------*/
