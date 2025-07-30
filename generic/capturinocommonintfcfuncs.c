/* L I C E N S E * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * MIT License
 * 
 * Copyright (c) 2025 michael0710
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* ***************************************************************************
 * I N C L U D E   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* S Y S T E M   I N C L U D E S * * * * * * * * * * * * * * * * * * * * * * */
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

/* P R O J E C T   I N C L U D E S * * * * * * * * * * * * * * * * * * * * * */
#include "argparser.h"
#include "capturinoconn.h"
#include "console.h"
#include "diagnosis.h"
#include "genericutils.h"
#include "pipehandling.h"
#include "serialhandling.h"
#include "systemutils.h"

/* M O D U L E   H E A D E R   I N C L U D E * * * * * * * * * * * * * * * * */
#include "capturinocommonintfcfuncs.h"

/* ***************************************************************************
 * D E F I N E   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* L O C A L   C O N F I G   D E F I N I T I O N S * * * * * * * * * * * * * */

/* L O C A L   M A C R O   D E F I N I T I O N S * * * * * * * * * * * * * * */

/* ***************************************************************************
 * T Y P E D E F   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */
typedef enum
{
    CAPTURINO_STATE_PARSE_ARGS = 0,
    CAPTURINO_STATE_OPEN_FIFO = 1,
    CAPTURINO_STATE_INIT_FIFO = 2,
    CAPTURINO_STATE_OPEN_CONNECTION = 3,
    CAPTURINO_STATE_OPEN_CONNECTION_WAIT4RETRY = 4,
    CAPTURINO_STATE_INIT_CONNECTION = 5,
    CAPTURINO_STATE_WAIT_FOR_CONNECTION = 11,
    CAPTURINO_STATE_CONNECTED = 12,
    CAPTURINO_STATE_ERROR = 13
} CapturinoTestCaptureStmacStatesType;

/* ***************************************************************************
 * V A R I A B L E S   A N D   C O N S T A N T S   S E C T I O N * * * * * * *
 *************************************************************************** */

/* G L O B A L   V A R I A B L E   D E F I N I T I O N S * * * * * * * * * * */
const CapturinoId2PhyNameType mId2PhyNameMapping[] =
{
    {
        .boardId = 0x80000001,
        .physicalLayerName = "CAN (ISO 11898-2)"
    },
    {
        .boardId = 0x00000001,
        .physicalLayerName = "RS-485"
    }
};

/**
 * \todo Think about replacing this mapping with the libpcap library's pcap_datalink_val_to_name() function
 */
const Dlt2StringType mDlt2StringMapping[CAPTURino_KNOWN_DLTS_COUNT] =
{
    {
        .dlt = 148,
        .dltString = "UART"
    },
    {
        .dlt = 227,
        .dltString = "CAN (ISO 11898-1)"
    }
};

/* L O C A L   V A R I A B L E   D E F I N I T I O N S * * * * * * * * * * */
static volatile bool mTerminateFlag = false;
static unsigned long long mCapturinoBaseUnixTime = 0;
static unsigned long mCapturinoBaseMicros = 0;

/* L O C A L   C O N S T A N T   D E F I N I T I O N S * * * * * * * * * * * */
static const char* MODULE_NAME = "CAPT_COMM";

/* ***************************************************************************
 * F U N C T I O N S   S E C T I O N * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* L O C A L   F U N C T I O N   P R O T O T Y P E S * * * * * * * * * * * * */
static int capturinoExtcapConfig_addPortList(int argc, char *argv[], int configArgNo);

static int capturinoExtcapConfig_reloadInterfaceList(int argc, char *argv[], int configArgNo);

static int capturinoExtcapConfig_printUpdatedInterfaceDescription(int configArgNo);

/* L O C A L   I N L I N E   F U N C T I O N   D E F I N I T I O N S * * * * */

/* L O C A L   F U N C T I O N   D E F I N I T I O N S * * * * * * * * * * * */
static int capturinoExtcapConfig_addPortList(int argc, char *argv[], int configArgNo)
{
    char comPortBuf[128];
    char* comPorts[15];
    size_t comPortsFound = 0;

    int rvGetPortList = SERH_GetPortList(comPortBuf, 128, comPorts, 15, &comPortsFound);
    if (rvGetPortList == 0)
    {
        CNSL_WriteArgLn("value {arg=%d}{value=X}{display= }", configArgNo);
        for (size_t i=0; i<comPortsFound; i++)
        {
            CNSL_WriteArgLn("value {arg=%d}{value=%s}{display=%s}{default=false}",
                            configArgNo, comPorts[i],
                            comPorts[i]);
        }
    }
    else
    {
        DIAG_LogMsg(DIAG_WARNING, MODULE_NAME, __func__, "unable to retrieve COM port list!");
    }
    
    return 0;
}

static int capturinoExtcapConfig_reloadInterfaceList(int argc, char *argv[], int configArgNo)
{
    int rv;

    long baudrate;
    rv = ARGP_getLongOfArgs(argc, argv, "--baudrate", &baudrate);
    if (rv != 0)
    {
        CNSL_WriteArgLn("value {arg=%d}{value=%d}{display=%s}",
                        configArgNo, 0,
                        "No baudrate specified for CAPTURino communication.");
        return 0;
    }

    char* comPort;
    rv = ARGP_getP2StringOfArgs(argc, argv, "--port", &comPort);
    if (rv != 0)
    {
        CNSL_WriteArgLn("value {arg=%d}{value=%d}{display=%s}",
                        configArgNo, 0,
                        "No serial port specified for CAPTURino communication.");
        return 0;
    }
    
    DIAG_LogMsg(DIAG_INFO, MODULE_NAME, __func__, "open serial port to get supported linktypes");
    rv = CCON_Open(comPort, baudrate);
    if (rv != 0)
    {
        /* if no capture interface was found change the default selector text */
        CNSL_WriteArgLn("value {arg=%d}{value=%d}{display=%s}",
                        configArgNo, 0,
                        "No CAPTURino device found at the specified port.");
        return 0;
    }

    capturinoExtcapConfig_printUpdatedInterfaceDescription(configArgNo);

    DIAG_LogMsg(DIAG_INFO, MODULE_NAME, __func__, "closing serial port");
    CCON_Close();

    return 0;
}

static int capturinoExtcapConfig_printUpdatedInterfaceDescription(int configArgNo)
{
    int rv;

    rv = CCON_InitiateSession(200, &mTerminateFlag);
    if (rv != 0)
    {
        DIAG_LogMsgArg(DIAG_FAILURE, MODULE_NAME, __func__, "failed to initiate a session with the CAPTURino device. Return value was %d", rv);
        return 0;
    }

    uint32_t boardId;
    rv = CCON_GetBoardId(500, &mTerminateFlag, &boardId);
    if (rv != 0)
    {
        DIAG_LogMsgArg(DIAG_FAILURE, MODULE_NAME, __func__, "failed to get the board id from the CAPTURino device. Return value was %d", rv);
        return 0;
    }

    const char* phyName = "";
    DIAG_LogMsgArg(DIAG_INFO, MODULE_NAME, __func__, "CAPTURino device returned board id 0x%08X", boardId);
    for (size_t i = 0; i < sizeof(mId2PhyNameMapping) / sizeof(CapturinoId2PhyNameType); i++)
    {
        if (mId2PhyNameMapping[i].boardId == boardId)
        {
            phyName = mId2PhyNameMapping[i].physicalLayerName;
            break;
        }
    }

    uint32_t dlts[8];
    size_t dltsCount = 0;
    rv = CCON_GetSupportedDlts(500, &mTerminateFlag, dlts, 8, &dltsCount);
    if (rv != 0)
    {
        DIAG_LogMsgArg(DIAG_FAILURE, MODULE_NAME, __func__, "failed to get the supported dlts from the CAPTURino device. Return value was %d", rv);
        return 0;
    }
    const char* dltString = "";
    for (size_t i = 0; i < dltsCount; i++)
    {
        for (size_t j = 0; j < sizeof(mDlt2StringMapping) / sizeof(Dlt2StringType); j++)
        {
            if (mDlt2StringMapping[j].dlt == dlts[i])
            {
                dltString = mDlt2StringMapping[j].dltString;
                break;
            }
        }
        CNSL_WriteArgLn("value {arg=%d}{value=%d}{display=%s - %s}",
                        configArgNo, dlts[i],
                        phyName,
                        dltString);
    }

    return 0;
}

/* G L O B A L   F U N C T I O N   D E F I N I T I O N S * * * * * * * * * * */
/** TODO: The field of type "editselector" does not support the validation tag. Is that a bug in Wireshark?
 *  TODO: The field of type "editselector" does not show up on Debian. Is that a bug in Wireshark?
 *  \warning The field {required=true} leads to continuous errors on Debian. "Configure all extcaps before start of capture."
 *      This is Wireshark Issue #18487 resolved in version 4.2.4
 *      \see https://gitlab.com/wireshark/wireshark/-/issues/18487
 *
 * Due to some issues on the Debian platform, all fields are now of type
 * "string".
 */
int capturinoCommonExtcapConfig(int argc, char *argv[])
{
    bool hasReloadOption = false;
    char* reloadArg = NULL;
    int rvReloadOption = ARGP_getP2StringOfArgs(argc, argv, "--extcap-reload-option", &reloadArg);
    if (rvReloadOption == 0)
    {
        if (strcmp(reloadArg, "dlts") == 0)
        {
            capturinoExtcapConfig_reloadInterfaceList(argc, argv, 4);
        }
        else
        {
            DIAG_LogMsgArg(DIAG_ERROR, MODULE_NAME, __func__, "Unknown reload option \'%s\' found", reloadArg);
        }
    }
    else
    {
        DIAG_LogMsg(DIAG_DEBUG, MODULE_NAME, __func__, "function entered");

        /* NOTE: the call="..." argument must only consist of lower case letters. Otherwise Wireshark
        *       will crash with error 0xc0000409 */
        CNSL_WriteArgLn("arg {number=%d}{call=--port}{display=Serial port}{tooltip=Serial port for the communication}{type=editselector}{required=true}{group=Connection}", 0);
        capturinoExtcapConfig_addPortList(argc, argv, 0);
        CNSL_WriteArgLn("arg {number=%d}{call=--logfile}{display=Logfile}{tooltip=Log file of the CAPTURino plugin}{type=fileselect}{mustexist=false}{group=Connection}", 1);
        CNSL_WriteArgLn("arg {number=%d}{call=--loglevel}{display=Loglevel}{tooltip=Severity limit of the messages to be captured}{type=selector}{default=2}{group=Connection}", 2);
        CNSL_WriteArgLn("value {arg=%d}{value=0}{display=verbose}", 2);
        CNSL_WriteArgLn("value {arg=%d}{value=1}{display=debug}", 2);
        CNSL_WriteArgLn("value {arg=%d}{value=2}{display=info}", 2);
        CNSL_WriteArgLn("value {arg=%d}{value=3}{display=warning}", 2);
        CNSL_WriteArgLn("value {arg=%d}{value=4}{display=error}", 2);

        CNSL_WriteArgLn("arg {number=%d}{call=--baudrate}{display=Baudrate}{tooltip=Baudrate for the serial communication}{type=string}{default=115200}{required=true}{group=Connection}", 3);
        
        CNSL_WriteArgLn("arg {number=%d}{call=--dlts}{display=Capture interface}{tooltip=Click reload to search for available interfaces}{type=selector}{reload=true}{placeholder=Reload}{group=Connection}{required=true}", 4);
        CNSL_WriteArgLn("value {arg=%d}{value=-1}{display=No CAPTURino interface selected.}", 4);

        CNSL_WriteArgLn("arg {number=%d}{call=--canbaudrate}{display=CAN Bus Baudrate}{tooltip=Baudrate of the CAN bus to be captured}{type=string}{default=500000}{group=CAN}{required=true}", 5);
        CNSL_WriteArgLn("arg {number=%d}{call=--cansamplepoint}{display=Relative Sample Point (%%)}{tooltip=Relative sample point in percent}{type=string}{default=75}{group=CAN}{required=true}", 6);

        CNSL_WriteArgLn("arg {number=%d}{call=--serialbaudrate}{display=Serial Baudrate}{tooltip=Baudrate of the serial bus to be captured}{type=string}{default=19200}{group=UART}{required=true}", 7);
        
        CNSL_WriteArgLn("arg {number=%d}{call=--serialdatabits}{display=Databits}{tooltip=Number of databits in each frame to be captured}{type=selector}{default=8}{group=UART}{required=true}", 8);
        CNSL_WriteArgLn("value {arg=%d}{value=5}{display=5}", 8);
        CNSL_WriteArgLn("value {arg=%d}{value=6}{display=6}", 8);
        CNSL_WriteArgLn("value {arg=%d}{value=7}{display=7}", 8);
        CNSL_WriteArgLn("value {arg=%d}{value=8}{display=8}", 8);
        CNSL_WriteArgLn("value {arg=%d}{value=9}{display=9}", 8);

        CNSL_WriteArgLn("arg {number=%d}{call=--serialparity}{display=Parity}{tooltip=Parity configuration of the serial bus to be captured}{type=selector}{default=none}{group=UART}{required=true}", 9);
        CNSL_WriteArgLn("value {arg=%d}{value=n}{display=none}", 9);
        CNSL_WriteArgLn("value {arg=%d}{value=o}{display=odd}", 9);
        CNSL_WriteArgLn("value {arg=%d}{value=e}{display=even}", 9);
        CNSL_WriteArgLn("value {arg=%d}{value=sh}{display=stick high}", 9);
        CNSL_WriteArgLn("value {arg=%d}{value=sl}{display=stick low}", 9);
        
        CNSL_WriteArgLn("arg {number=%d}{call=--serialstopps}{display=Stopp bits}{tooltip=Number of stopp bits of the serial bus to be captured}{type=selector}{default=1}{group=UART}{required=true}", 10);
        CNSL_WriteArgLn("value {arg=%d}{value=1}{display=1 Stoppbit}", 10);
        CNSL_WriteArgLn("value {arg=%d}{value=1.5}{display=1.5 Stoppbits}", 10);
        CNSL_WriteArgLn("value {arg=%d}{value=2}{display=2 Stoppbits}", 10);

        CNSL_WriteArgLn("arg {number=%d}{call=--serialtimeout}{display=No new frame timeout (us)}{tooltip=Timeout to monitor if no more messages appear for a certain time}{type=string}{default=1750}{group=UART}{required=true}", 11);
    }
    return 0;
}

int capturinoCommonValidateParameters(const char* comPort,
                                      long baudrate,
                                      const char* fifopath,
                                      unsigned long dltValue)
{
    DIAG_LogMsg(DIAG_DEBUG, MODULE_NAME, __func__, "function entered");
    int retVal = 0;
    /* the comPort is just checked if the string is not of zero length */
    if (comPort[0] == 0)
    {
        DIAG_LogMsg(DIAG_FAILURE, MODULE_NAME, __func__, "no or invalid COM port specified!");
        retVal = -1;
    }
    if(baudrate <= 0)
    {
        DIAG_LogMsg(DIAG_FAILURE, MODULE_NAME, __func__, "no or invalid baudrate specified!");
        retVal = -1;
    }
    /* the fifoPath is just checked if the string is not of zero length */
    if (fifopath[0] == 0)
    {
        DIAG_LogMsg(DIAG_FAILURE, MODULE_NAME, __func__, "no or invalid FIFO specified!");
        retVal = -1;
    }

    if (dltValue == 0)
    {
        DIAG_LogMsg(DIAG_FAILURE, MODULE_NAME, __func__, "no or invalid DLT value specified!");
        CNSL_WriteErr("No CAPTURino device connected!\n",
                      STATIC_STRLEN("No CAPTURino device connected!\n"));
        return -1;
    }

    if (retVal == -1)
    {
        CNSL_WriteErr("Capture process stopped! Configuration parameters are not valid!\n",
                      STATIC_STRLEN("Capture process stopped! Configuration parameters are not valid!\n"));
    }

    return retVal;
}

int capturinoCommonGenerateCaptureCmd(uint32_t dlt,
                                      int argc,
                                      char *argv[],
                                      char* captureCmd,
                                      size_t maxCmdLen,
                                      size_t* cmdLen)
{
    *cmdLen = 0;
    int rv;
    rv = SYSU_StrNCpy_S(&captureCmd[*cmdLen],
                        maxCmdLen - (*cmdLen),
                        "capture ",
                        STATIC_STRLEN("capture "));
    if (rv != 0)
    {
        DIAG_LogMsgArg(DIAG_ERROR, MODULE_NAME, __func__, "Copying to command buffer failed. Return value = %d, Buffer size = %d, Generated command = \'%.*s\'", rv, maxCmdLen, *cmdLen, captureCmd);
        return -1;
    }
    *cmdLen += STATIC_STRLEN("capture ");

    /* depending on the DLT value to be captured, the configuration for the
       interface must be sent as well */
    switch (dlt)
    {
        case 148:
        {
            rv = SYSU_StrNCpy_S(&captureCmd[*cmdLen],
                                maxCmdLen - (*cmdLen),
                                "148 -b=",
                                STATIC_STRLEN("148 -b="));
            if (rv != 0)
            {
                DIAG_LogMsgArg(DIAG_ERROR, MODULE_NAME, __func__, "Copying to command buffer failed. Return value = %d, Buffer size = %d, Generated command = \'%.*s\'", rv, maxCmdLen, *cmdLen, captureCmd);
                return -1;
            }
            *cmdLen += STATIC_STRLEN("148 -b=");

            char* serialbaudrate;
            rv = ARGP_getP2StringOfArgs(argc, argv, "--serialbaudrate", &serialbaudrate);
            if (rv != 0)
            {
                DIAG_LogMsg(DIAG_ERROR, MODULE_NAME, __func__, "no serial baudrate specified!");
                return -1;
            }
            rv = SYSU_StrNCpy_S(&captureCmd[*cmdLen],
                                maxCmdLen - (*cmdLen),
                                serialbaudrate,
                                strnlen(serialbaudrate, 128));
            if (rv != 0)
            {
                DIAG_LogMsgArg(DIAG_ERROR, MODULE_NAME, __func__, "Copying to command buffer failed. Return value = %d, Buffer size = %d, Generated command = \'%.*s\'", rv, maxCmdLen, *cmdLen, captureCmd);
                return -1;
            }
            *cmdLen += strnlen(serialbaudrate, 128);

            rv = SYSU_StrNCpy_S(&captureCmd[*cmdLen],
                                maxCmdLen - (*cmdLen),
                                " -d=",
                                STATIC_STRLEN(" -d="));
            if (rv != 0)
            {
                DIAG_LogMsgArg(DIAG_ERROR, MODULE_NAME, __func__, "Copying to command buffer failed. Return value = %d, Buffer size = %d, Generated command = \'%.*s\'", rv, maxCmdLen, *cmdLen, captureCmd);
                return -1;
            }
            *cmdLen += STATIC_STRLEN(" -d=");

            char* serialdatabits;
            rv = ARGP_getP2StringOfArgs(argc, argv, "--serialdatabits", &serialdatabits);
            if (rv != 0)
            {
                DIAG_LogMsg(DIAG_ERROR, MODULE_NAME, __func__, "no serial databits specified!");
                return -1;
            }
            rv = SYSU_StrNCpy_S(&captureCmd[*cmdLen],
                                maxCmdLen - (*cmdLen),
                                serialdatabits,
                                strnlen(serialdatabits, 128));
            if (rv != 0)
            {
                DIAG_LogMsgArg(DIAG_ERROR, MODULE_NAME, __func__, "Copying to command buffer failed. Return value = %d, Buffer size = %d, Generated command = \'%.*s\'", rv, maxCmdLen, *cmdLen, captureCmd);
                return -1;
            }
            *cmdLen += strnlen(serialdatabits, 128);

            rv = SYSU_StrNCpy_S(&captureCmd[*cmdLen],
                                maxCmdLen - (*cmdLen),
                                " -p=",
                                STATIC_STRLEN(" -p="));
            if (rv != 0)
            {
                DIAG_LogMsgArg(DIAG_ERROR, MODULE_NAME, __func__, "Copying to command buffer failed. Return value = %d, Buffer size = %d, Generated command = \'%.*s\'", rv, maxCmdLen, *cmdLen, captureCmd);
                return -1;
            }
            *cmdLen += STATIC_STRLEN(" -p=");
            char* serialparity;
            rv = ARGP_getP2StringOfArgs(argc, argv, "--serialparity", &serialparity);
            if (rv != 0)
            {
                DIAG_LogMsg(DIAG_ERROR, MODULE_NAME, __func__, "no serial parity specified!");
                return -1;
            }
            rv = SYSU_StrNCpy_S(&captureCmd[*cmdLen],
                                maxCmdLen - (*cmdLen),
                                serialparity,
                                strnlen(serialparity, 128));
            if (rv != 0)
            {
                DIAG_LogMsgArg(DIAG_ERROR, MODULE_NAME, __func__, "Copying to command buffer failed. Return value = %d, Buffer size = %d, Generated command = \'%.*s\'", rv, maxCmdLen, *cmdLen, captureCmd);
                return -1;
            }
            *cmdLen += strnlen(serialparity, 128);

            rv = SYSU_StrNCpy_S(&captureCmd[*cmdLen],
                                maxCmdLen - (*cmdLen),
                                " -s=",
                                STATIC_STRLEN(" -s="));
            if (rv != 0)
            {
                DIAG_LogMsgArg(DIAG_ERROR, MODULE_NAME, __func__, "Copying to command buffer failed. Return value = %d, Buffer size = %d, Generated command = \'%.*s\'", rv, maxCmdLen, *cmdLen, captureCmd);
                return -1;
            }
            *cmdLen += STATIC_STRLEN(" -s=");
            char* serialstopps;
            rv = ARGP_getP2StringOfArgs(argc, argv, "--serialstopps", &serialstopps);
            if (rv != 0)
            {
                DIAG_LogMsg(DIAG_ERROR, MODULE_NAME, __func__, "no serial stop bits specified!");
                return -1;
            }
            rv = SYSU_StrNCpy_S(&captureCmd[*cmdLen],
                                maxCmdLen - (*cmdLen),
                                serialstopps,
                                strnlen(serialstopps, 128));
            if (rv != 0)
            {
                DIAG_LogMsgArg(DIAG_ERROR, MODULE_NAME, __func__, "Copying to command buffer failed. Return value = %d, Buffer size = %d, Generated command = \'%.*s\'", rv, maxCmdLen, *cmdLen, captureCmd);
                return -1;
            }
            *cmdLen += strnlen(serialstopps, 128);

            rv = SYSU_StrNCpy_S(&captureCmd[*cmdLen],
                                maxCmdLen - (*cmdLen),
                                " -t=",
                                STATIC_STRLEN(" -t="));
            if (rv != 0)
            {
                DIAG_LogMsgArg(DIAG_ERROR, MODULE_NAME, __func__, "Copying to command buffer failed. Return value = %d, Buffer size = %d, Generated command = \'%.*s\'", rv, maxCmdLen, *cmdLen, captureCmd);
                return -1;
            }
            *cmdLen += STATIC_STRLEN(" -t=");
            char* serialtimeout;
            rv = ARGP_getP2StringOfArgs(argc, argv, "--serialtimeout", &serialtimeout);
            if (rv != 0)
            {
                DIAG_LogMsg(DIAG_ERROR, MODULE_NAME, __func__, "no serial timeout specified!");
                return -1;
            }
            rv = SYSU_StrNCpy_S(&captureCmd[*cmdLen],
                                maxCmdLen - (*cmdLen),
                                serialtimeout,
                                strnlen(serialtimeout, 128));
            if (rv != 0)
            {
                DIAG_LogMsgArg(DIAG_ERROR, MODULE_NAME, __func__, "Copying to command buffer failed. Return value = %d, Buffer size = %d, Generated command = \'%.*s\'", rv, maxCmdLen, *cmdLen, captureCmd);
                return -1;
            }
            *cmdLen += strnlen(serialtimeout, 128);

            break;
        }

        case 227:
        {
            rv = SYSU_StrNCpy_S(&captureCmd[*cmdLen],
                                maxCmdLen - (*cmdLen),
                                "227 -b=",
                                STATIC_STRLEN("227 -b="));
            if (rv != 0)
            {
                DIAG_LogMsgArg(DIAG_ERROR, MODULE_NAME, __func__, "Copying to command buffer failed. Return value = %d, Buffer size = %d, Generated command = \'%.*s\'", rv, maxCmdLen, *cmdLen, captureCmd);
                return -1;
            }
            *cmdLen += STATIC_STRLEN("227 -b=");
            
            char* canbaudrate;
            rv = ARGP_getP2StringOfArgs(argc, argv, "--canbaudrate", &canbaudrate);
            if (rv != 0)
            {
                DIAG_LogMsg(DIAG_ERROR, MODULE_NAME, __func__, "no CAN baudrate specified!");
                return -1;
            }
            if (strnlen(canbaudrate, 128) == 1)
            {
                /* be careful with that easteregg. This code makes it impossible to
                   capture a CAN bus with a baudrate < 9. Which is actually wouldn't
                   make sense, but might be possible in some very rare cases! */
                char easterEggMsg[128];
                snprintf(easterEggMsg, 128, "Are you sure the baudrate is %.*s? I might fall asleep capturing that bus ...", 1, canbaudrate);
                CNSL_WriteErr(easterEggMsg, strnlen(easterEggMsg, 128));
            }

            rv = SYSU_StrNCpy_S(&captureCmd[*cmdLen],
                                maxCmdLen - (*cmdLen),
                                canbaudrate,
                                strnlen(canbaudrate, 128));
            if (rv != 0)
            {
                DIAG_LogMsgArg(DIAG_ERROR, MODULE_NAME, __func__, "Copying to command buffer failed. Return value = %d, Buffer size = %d, Generated command = \'%.*s\'", rv, maxCmdLen, *cmdLen, captureCmd);
                return -1;
            }
            *cmdLen += strnlen(canbaudrate, 128);

            rv = SYSU_StrNCpy_S(&captureCmd[*cmdLen],
                                maxCmdLen - (*cmdLen),
                                " -s=",
                                STATIC_STRLEN(" -s="));
            if (rv != 0)
            {
                DIAG_LogMsgArg(DIAG_ERROR, MODULE_NAME, __func__, "Copying to command buffer failed. Return value = %d, Buffer size = %d, Generated command = \'%.*s\'", rv, maxCmdLen, *cmdLen, captureCmd);
                return -1;
            }
            *cmdLen += STATIC_STRLEN(" -s=");

            char* cansamplepoint;
            rv = ARGP_getP2StringOfArgs(argc, argv, "--cansamplepoint", &cansamplepoint);
            if (rv != 0)
            {
                DIAG_LogMsg(DIAG_ERROR, MODULE_NAME, __func__, "no CAN sample point specified!");
                return -1;
            }
            rv = SYSU_StrNCpy_S(&captureCmd[*cmdLen],
                                maxCmdLen - (*cmdLen),
                                cansamplepoint,
                                strnlen(cansamplepoint, 128));
            if (rv != 0)
            {
                DIAG_LogMsgArg(DIAG_ERROR, MODULE_NAME, __func__, "Copying to command buffer failed. Return value = %d, Buffer size = %d, Generated command = \'%.*s\'", rv, maxCmdLen, *cmdLen, captureCmd);
                return -1;
            }
            *cmdLen += strnlen(cansamplepoint, 128);
            break;
        }
        default:
        {
            return -1;
        }
    }

    if (*cmdLen < maxCmdLen)
    {
        captureCmd[*cmdLen] = '\n';
        *cmdLen += 1;
        DIAG_LogMsgArg(DIAG_DEBUG, MODULE_NAME, __func__, "Generated command: \'%.*s\'", *cmdLen, captureCmd);
        return 0;
    }
    else
    {
        DIAG_LogMsgArg(DIAG_ERROR, MODULE_NAME, __func__, "specified command buffer too small! Buffer size = %d, Generated command = \'%.*s\'", maxCmdLen, *cmdLen, captureCmd);
        return -1;
    }
}

/** \warning microsOffset must not be > 1000000 */
int capturinoCommonUpdateTimebase(unsigned long long secondsOffset,
                                  unsigned long microsOffset)
{
    mCapturinoBaseUnixTime += secondsOffset;
    mCapturinoBaseMicros += microsOffset;
    if (mCapturinoBaseMicros > 1000000)
    {
        mCapturinoBaseUnixTime++;
        mCapturinoBaseMicros%1000000;
    }

    return 0;
}

int capturinoCommonSetTimebase(unsigned long long hostUnixTime,
                               unsigned long hostMicros,
                               unsigned long capturinoMicros)
{
    mCapturinoBaseUnixTime = hostUnixTime - (capturinoMicros / 1000000);
    mCapturinoBaseMicros = hostMicros - (capturinoMicros % 1000000);
    if (mCapturinoBaseMicros > hostMicros)
    {
        mCapturinoBaseUnixTime -= 1;
        mCapturinoBaseMicros += 1000000;
        DIAG_LogMsg(DIAG_DEBUG, MODULE_NAME, __func__, "Timebase micros needed correction");
    }
    DIAG_LogMsgArg(DIAG_INFO, MODULE_NAME, __func__, "Timebase is %llu seconds, %lu micros", mCapturinoBaseUnixTime, mCapturinoBaseMicros);
    return 0;
}

int capturinoCommonGetTimestamp(unsigned long capturinoMicros,
                                unsigned long long* unixSeconds,
                                unsigned long*      unixMicros)
{
    /** \todo how shall a overflow of the micros counter be handeled? Right now,
     *        the timestamp wraps back to the base time after ~71 minutes */
    *unixSeconds = mCapturinoBaseUnixTime + (unsigned long long)(capturinoMicros / 1000000);
    *unixMicros = mCapturinoBaseMicros + (capturinoMicros % 1000000);
    if (*unixMicros >= 1000000)
    {
        *unixSeconds += 1;
        *unixMicros -= 1000000;
        DIAG_LogMsg(DIAG_DEBUG, MODULE_NAME, __func__, "Timestamp micros needed correction");
    }
    DIAG_LogMsgArg(DIAG_VERBOSE, MODULE_NAME, __func__, "Timestamp was %lu micros => seconds = %llu, micros = %lu", capturinoMicros, *unixSeconds, *unixMicros);

    return 0;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* ***************************************************************************
 * E N D   O F   F I L E * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */
