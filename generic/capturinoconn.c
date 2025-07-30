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
 * M O D U L E   I N F O R M A T I O N * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */
/**
 * \file
 * \addtogroup capturinoconn
 * \brief Module managing the connection and communication to the CAPTURino
 *        embedded system.
 */
/* ************************************************************************* */

/* ***************************************************************************
 * I N C L U D E   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* S Y S T E M   I N C L U D E S * * * * * * * * * * * * * * * * * * * * * * */
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

/* P R O J E C T   I N C L U D E S * * * * * * * * * * * * * * * * * * * * * */
#include "diagnosis.h"
#include "genericutils.h"
#include "serialhandling.h"
#include "systemutils.h"

/* M O D U L E   H E A D E R   I N C L U D E * * * * * * * * * * * * * * * * */
#include "capturinoconn.h"

/* ***************************************************************************
 * D E F I N E   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* L O C A L   C O N F I G   D E F I N I T I O N S * * * * * * * * * * * * * */
#define RCV_BUFFER_LENGTH       (256)
/* L O C A L   M A C R O   D E F I N I T I O N S * * * * * * * * * * * * * * */

/* ***************************************************************************
 * T Y P E D E F   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* L O C A L   T Y P E D E F S * * * * * * * * * * * * * * * * * * * * * * * */

/* ***************************************************************************
 * V A R I A B L E S   A N D   C O N S T A N T S   S E C T I O N * * * * * * *
 *************************************************************************** */

/* G L O B A L   C O N S T A N T   D E F I N I T I O N S * * * * * * * * * * */

/* G L O B A L   V A R I A B L E   D E F I N I T I O N S * * * * * * * * * * */

/* L O C A L   C O N S T A N T   D E F I N I T I O N S * * * * * * * * * * * */
static const char MODULE_NAME[] = "CCON";

/* L O C A L   V A R I A B L E   D E F I N I T I O N S * * * * * * * * * * * */
SerialHandleType mSerialHandle = 0;
static char mRcvBuffer[RCV_BUFFER_LENGTH];

/* ***************************************************************************
 * F U N C T I O N S   S E C T I O N * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* L O C A L   F U N C T I O N   P R O T O T Y P E S * * * * * * * * * * * * */

/* L O C A L   I N L I N E   F U N C T I O N   D E F I N I T I O N S * * * * */

/* L O C A L   F U N C T I O N   D E F I N I T I O N S * * * * * * * * * * * */
/**
 * Checks for an upcounting rollover timer if a certain timeout has been
 * reached.
 */
static bool isTimeoutReached(uint32_t timeoutStartPoint,
                             uint32_t timeoutReachedPoint,
                             uint32_t currentMillis)
{
    if (timeoutReachedPoint < timeoutStartPoint)
    {
        /* timeout occurs after counter overflow */
        if ((currentMillis > timeoutReachedPoint) && (currentMillis < timeoutStartPoint))
        {
            DIAG_LogMsg(DIAG_INFO, MODULE_NAME, __func__, "timeout reached after counter overflow");
            return true;
        }
        else 
        {
            return false;
        }
    }
    else
    {
        /* timeout occurs before counter overflow */
        if ((currentMillis > timeoutReachedPoint) || (currentMillis < timeoutStartPoint))
        {
            DIAG_LogMsg(DIAG_INFO, MODULE_NAME, __func__, "timeout reached before counter overflow");
            return true;
        }
        else
        {
            return false;
        }
    }
}

static int wait4Response(unsigned long timeoutMS,
                         volatile bool* terminateFlag,
                         char* responseBuf,
                         size_t* responseLen,
                         size_t responseBufLen,
                         const char* terminateSequence,
                         size_t terminateSequenceLen)
{
    unsigned long timeoutStartPoint;
    SYSU_GetCurrentMillis(&timeoutStartPoint);

    size_t totalBytesReceived = 0;
    while (*terminateFlag == false)
    {
        unsigned long currentMillis;
        SYSU_GetCurrentMillis(&currentMillis);
        if (isTimeoutReached(timeoutStartPoint,
                             timeoutStartPoint + timeoutMS,
                             currentMillis))
        {
            *responseLen = 0;
            return -2;
        }
        size_t bytesReceived = 0;
        int rvRead = SERH_Read(mSerialHandle,
                               responseBuf+totalBytesReceived,
                               responseBufLen-totalBytesReceived-1,
                               &bytesReceived);
        totalBytesReceived += bytesReceived;
        if (rvRead != 0)
        {
            DIAG_LogMsg(DIAG_FAILURE, MODULE_NAME, __func__, "reading from serial port failed!");
            return -1;
        }
        if (totalBytesReceived >= terminateSequenceLen)
        {
            if (strncmp(responseBuf+totalBytesReceived-terminateSequenceLen,
                        terminateSequence,
                        terminateSequenceLen) == 0)
            {
                break;
            }
        }
        if (totalBytesReceived >= responseBufLen - 1)
        {
            return -1;
        }
        /* TODO: i've heard about the sleep function might sleep more or less than specified. Is that really true? It might be an issue here */
        SYSU_Sleep(5); /* avoid high processor load */
    }

    *responseLen = totalBytesReceived;
    responseBuf[totalBytesReceived] = '\0';
    return 0;
}

/* G L O B A L   F U N C T I O N   D E F I N I T I O N S * * * * * * * * * * */
int CCON_Open(const char* path,
              unsigned int baudrate)
{
    return SERH_Open(path, baudrate, &mSerialHandle);
}

int CCON_InitiateSession(unsigned long timeoutMS,
                         volatile bool* terminateFlag)
{
    int rv;
    char tempRcvBuffer[128];
    size_t bytesReceived;

    /* send a newline character to the CAPTURino to get a response */
    DIAG_LogMsg(DIAG_DEBUG, MODULE_NAME, __func__, "function entered");
    if (SERH_FlushInput(mSerialHandle) != 0)
    {
        DIAG_LogMsg(DIAG_FAILURE, MODULE_NAME, __func__, "error clearing serial port buffers!");
        return -1;
    }
    
    if (SERH_Write(mSerialHandle, "\x03", STATIC_STRLEN("\x03")) != 0)
    {
        DIAG_LogMsg(DIAG_FAILURE, MODULE_NAME, __func__, "error sending command to CAPTURino!");
        return -1;
    }

    rv = wait4Response(timeoutMS/2,
                       terminateFlag,
                       tempRcvBuffer,
                       &bytesReceived,
                       128,
                       "CAPTURino>",
                       STATIC_STRLEN("CAPTURino>"));
    if (rv != -2)
    {
        /* return immediately if the function succeeded or failed totally */
        return rv;
    }
    else
    {
        /* take a second try with the ^C character if a timeout occured */
        if (SERH_FlushInput(mSerialHandle) != 0)
        {
            DIAG_LogMsg(DIAG_FAILURE, MODULE_NAME, __func__, "error clearing serial port buffers!");
            return -1;
        }
        
        if (SERH_Write(mSerialHandle, "\x03", STATIC_STRLEN("\x03")) != 0)
        {
            DIAG_LogMsg(DIAG_FAILURE, MODULE_NAME, __func__, "error sending command to CAPTURino!");
            return -1;
        }

        return wait4Response(timeoutMS/2,
                             terminateFlag,
                             tempRcvBuffer,
                             &bytesReceived,
                             128,
                             "CAPTURino>",
                             STATIC_STRLEN("CAPTURino>"));
    }
}

int CCON_GetBoardId(unsigned long timeoutMS,
                    volatile bool* terminateFlag,
                    uint32_t* boardId)
{
    int rv;
    char rcvBuffer[128];
    size_t bytesReceived = 0;
    rv = CCON_ExecWithResponse("idfcn\n",
                               STATIC_STRLEN("idfcn\n"),
                               timeoutMS,
                               terminateFlag,
                               rcvBuffer,
                               &bytesReceived,
                               128,
                               "CAPTURino>",
                               STATIC_STRLEN("CAPTURino>"));
    if ((rv != 0) || (bytesReceived < STATIC_STRLEN("idfcn") + 2 + 8))
    {
        DIAG_LogMsgArg(DIAG_ERROR, MODULE_NAME, __func__, "error waiting for CAPTURino response to command \'idfcn\'! rv=%d, received bytes=%d", rv, bytesReceived);
        DIAG_LogMsgArg(DIAG_ERROR, MODULE_NAME, __func__, "error waiting for CAPTURino response to command \'idfcn\'! Received %u chars: \'%.*s\'", bytesReceived, bytesReceived, rcvBuffer);
        return -1;
    }
    else
    {
        char* endptr = NULL;
        *boardId = strtoul(rcvBuffer+2, &endptr, 16);
        /* it is expected that the command returns eight digits, hence the
        endptr must match the rcvBuffer+STATIC_STRLEN("idfcn")+2+8 address */
        if (endptr != rcvBuffer+2+8)
        {
            DIAG_LogMsgArg(DIAG_ERROR, MODULE_NAME, __func__, "error parsing board id from CAPTURino response! endptr=%p, expected=%p", endptr, rcvBuffer+2+8);
            DIAG_LogMsgArg(DIAG_ERROR, MODULE_NAME, __func__, "error parsing board id from CAPTURino response! Received %u chars: \'%.*s\'", bytesReceived, bytesReceived, rcvBuffer);
            return -1;
        }
        else
        {
            return 0;
        }
    }
}

int CCON_GetBoardMicros(unsigned long timeoutMS,
                        volatile bool* terminateFlag,
                        uint32_t* boardMicros)
{
    int rv;
    char rcvBuffer[128];
    size_t bytesReceived = 0;
    rv = CCON_ExecWithResponse("time\n",
                               STATIC_STRLEN("time\n"),
                               timeoutMS,
                               terminateFlag,
                               rcvBuffer,
                               &bytesReceived,
                               128,
                               "CAPTURino>",
                               STATIC_STRLEN("CAPTURino>"));
    if ((rv != 0) || (bytesReceived < 3))
    {
        DIAG_LogMsgArg(DIAG_ERROR, MODULE_NAME, __func__, "error waiting for CAPTURino response to command \'time\'! rv=%d, received bytes=%d", rv, bytesReceived);
        DIAG_LogMsgArg(DIAG_ERROR, MODULE_NAME, __func__, "error waiting for CAPTURino response to command \'time\'! Received %u chars: \'%.*s\'", bytesReceived, bytesReceived, rcvBuffer);
        return -1;
    }
    else
    {
        char* endptr = NULL;
        *boardMicros = strtoul(rcvBuffer+2, &endptr, 10);
        if (endptr == rcvBuffer+2)
        {
            DIAG_LogMsgArg(DIAG_ERROR, MODULE_NAME, __func__, "error parsing board micros from CAPTURino response! endptr=%p, expected=%p", endptr, rcvBuffer+2+8);
            DIAG_LogMsgArg(DIAG_ERROR, MODULE_NAME, __func__, "error parsing board micros from CAPTURino response! Received %u chars: \'%.*s\'", bytesReceived, bytesReceived, rcvBuffer);
            return -1;
        }
        else
        {
            return 0;
        }
    }
}

int CCON_GetSupportedDlts(unsigned long timeoutMS,
                          volatile bool* terminateFlag,
                          uint32_t* dlts,
                          size_t maxDltsCount,
                          size_t* dltsCount)
{
    int rv;
    char rcvBuffer[512];
    size_t bytesReceived = 0;
    /* get supported linktypes */
    rv = CCON_ExecWithResponse("dlts\n",
                                STATIC_STRLEN("dlts\n"),
                                timeoutMS,
                                terminateFlag,
                                rcvBuffer,
                                &bytesReceived,
                                256,
                                "CAPTURino>",
                                STATIC_STRLEN("CAPTURino>"));
    if ((rv != 0) || (bytesReceived <= 2))
    {
        DIAG_LogMsgArg(DIAG_FAILURE, MODULE_NAME, __func__, "error waiting for CAPTURino response to command \'dlts\'! rv=%d, received bytes=%d", rv, bytesReceived);
        DIAG_LogMsgArg(DIAG_FAILURE, MODULE_NAME, __func__, "error waiting for CAPTURino response to command \'dlts\'! Received %u chars: \'%.*s\'", bytesReceived, bytesReceived, rcvBuffer);
        return -1;
    }

    size_t j = 0;
    bool skipCurrentLine = false;
    for (size_t i=0; i<(bytesReceived-STATIC_STRLEN("CAPTURino>"));)
    {
        if (skipCurrentLine == true)
        {
            if ((rcvBuffer[i] == '\r') || (rcvBuffer[i] == '\n'))
            {
                skipCurrentLine = false;
            }
        }
        else if ((rcvBuffer[i] == '\r') || (rcvBuffer[i] == '\n'))
        {
            i++;
            continue;
        }
        else
        {
            uint32_t dlt = 0;
            char* endptr = NULL;
            dlt = strtoul(rcvBuffer+i, &endptr, 10);
            if (endptr == rcvBuffer+i)
            {
                DIAG_LogMsgArg(DIAG_ERROR, MODULE_NAME, __func__, "error parsing dlts from CAPTURino response! endptr=%p, expected=%p", endptr, rcvBuffer+i);
                DIAG_LogMsgArg(DIAG_ERROR, MODULE_NAME, __func__, "error parsing dlts from CAPTURino response! Received %u chars: \'%.*s\'", bytesReceived, bytesReceived, rcvBuffer);
                skipCurrentLine = true;
                continue;
            }
            else
            {
                if (j < maxDltsCount)
                {
                    dlts[j] = dlt;
                    j++;
                }
                else
                {
                    DIAG_LogMsgArg(DIAG_ERROR, MODULE_NAME, __func__, "error parsing dlt from CAPTURino response! too many dlts received! received %u dlts, but only %u dlts are supported", j, maxDltsCount);
                    *dltsCount = j;
                    return 0;
                }
                i = (endptr - rcvBuffer) / sizeof(char);
            }
        }
    }
    *dltsCount = j;
    return 0;
}

int CCON_Exec(const char* cmd,
              size_t cmdLen, 
              unsigned long timeoutMS,
              volatile bool* terminateFlag)
{
    DIAG_LogMsg(DIAG_DEBUG, MODULE_NAME, __func__, "function entered");
    if (SERH_FlushInput(mSerialHandle) != 0)
    {
        DIAG_LogMsg(DIAG_FAILURE, MODULE_NAME, __func__, "error clearing serial port buffers!");
        return -1;
    }
    
    if (SERH_Write(mSerialHandle, cmd, cmdLen) != 0)
    {
        DIAG_LogMsg(DIAG_FAILURE, MODULE_NAME, __func__, "error sending command to CAPTURino!");
        return -1;
    }

    size_t totalBytesReceived = 0;
    unsigned long timeoutStartPoint;
    char responseBuf[128];
    SYSU_GetCurrentMillis(&timeoutStartPoint);
    while (*terminateFlag == false)
    {
        unsigned long currentMillis;
        SYSU_GetCurrentMillis(&currentMillis);
        if (isTimeoutReached(timeoutStartPoint,
                             timeoutStartPoint + timeoutMS,
                             currentMillis))
        {
            return -2;
        }
        size_t bytesReceived = 0;
        int rvRead = SERH_Read(mSerialHandle,
                               responseBuf+totalBytesReceived,
                               cmdLen-1-totalBytesReceived,
                               &bytesReceived);
        totalBytesReceived += bytesReceived;
        if (rvRead != 0)
        {
            DIAG_LogMsg(DIAG_FAILURE, MODULE_NAME, __func__, "reading from serial port failed!");
            return -1;
        }
        if (totalBytesReceived >= (cmdLen-1))
        {
            /* check if the returned characters match the command */
            return ((strncmp(responseBuf, cmd, cmdLen-1) == 0) ? 0 : -1);
        }
        /* TODO: i've heard about the sleep function might sleep more or less than specified. Is that really true? It might be an issue here */
        SYSU_Sleep(5); /* avoid high processor load */
    }
    return 0;
}

int CCON_Read(char* buf,
              size_t bufLen,
              size_t* bytesRead)
{
    return SERH_Read(mSerialHandle,
                     buf,
                     bufLen,
                     bytesRead);
}

int CCON_ExecWithResponse(const char* cmd,
                          size_t cmdLen, 
                          unsigned long timeoutMS,
                          volatile bool* terminateFlag,
                          char* responseBuf,
                          size_t* responseLen,
                          size_t responseBufLen,
                          const char* terminateSequence,
                          size_t terminateSequenceLen)
{
    DIAG_LogMsg(DIAG_DEBUG, MODULE_NAME, __func__, "function entered");
    unsigned long functionStartMillis;
    SYSU_GetCurrentMillis(&functionStartMillis);

    *responseLen = 0;

    if (CCON_Exec(cmd, cmdLen, timeoutMS, terminateFlag) != 0)
    {
        return -1;
    } 

    unsigned long currentMillis;
    SYSU_GetCurrentMillis(&currentMillis);
    /** \todo call probably does not work on a timer overflow */
    int rv = wait4Response(timeoutMS - (currentMillis - functionStartMillis),
                           terminateFlag,
                           responseBuf,
                           responseLen,
                           responseBufLen,
                           terminateSequence,
                           terminateSequenceLen);
    responseBuf[*responseLen] = '\0';
    return rv;
}

/** \todo this function has nothing to do with the capturino communication.
 * Think about moving it to another SWC e.g. systemutils */
int CCON_Wait(unsigned long timeMS,
              volatile bool* terminateFlag)
{
    DIAG_LogMsg(DIAG_DEBUG, MODULE_NAME, __func__, "function entered");
    unsigned long currentMillis;
    SYSU_GetCurrentMillis(&currentMillis);
    unsigned long timeoutPoint = currentMillis + timeMS;
    bool overflowFlag = timeoutPoint < currentMillis;

    while (*terminateFlag == false)
    {
        SYSU_GetCurrentMillis(&currentMillis);
        if ((overflowFlag == true) && (currentMillis < timeoutPoint))
        {
            overflowFlag = false;
        }
        if ((currentMillis > timeoutPoint) && (overflowFlag == false))
        {
            break;
        }
        SYSU_Sleep(10); /* avoid high processor load */
    }
    return 0;
}

int CCON_Close()
{
    int rv = -1;
    if (mSerialHandle != INVALID_SERIAL_HANDLE)
    {
        rv = SERH_Close(mSerialHandle);
        mSerialHandle = INVALID_SERIAL_HANDLE;
    }
    return rv;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* ***************************************************************************
 * E N D   O F   F I L E * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */
