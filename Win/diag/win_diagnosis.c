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
 * \addtogroup diagnosis
 * \brief Implementation of the Diagnosis component for Windows.
 */
/* ************************************************************************* */

/* ***************************************************************************
 * I N C L U D E   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* S Y S T E M   I N C L U D E S * * * * * * * * * * * * * * * * * * * * * * */
#include <windows.h>
#include <errhandlingapi.h>
#include <fileapi.h>
#include <handleapi.h>
#include <string.h>
#include <strsafe.h>
#include <sysinfoapi.h>
#include <stdbool.h>

/* P R O J E C T   I N C L U D E S * * * * * * * * * * * * * * * * * * * * * */

/* M O D U L E   H E A D E R   I N C L U D E * * * * * * * * * * * * * * * * */
#include "diagnosis.h"

/* ***************************************************************************
 * D E F I N E   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* L O C A L   C O N F I G   D E F I N I T I O N S * * * * * * * * * * * * * */
#define DEFAULT_LOGGING_SEVERITY    (DIAG_VERBOSE)
#define MAX_SEVERITY_VALUE          (5)
#define MAX_LOG_ENTRY_SIZE          (2048) /**< needs to fit in a DWORD
                                                variable, but that should not
                                                be a problem */
#define TEMP_BUF_SIZE               (1024)

/* L O C A L   M A C R O   D E F I N I T I O N S * * * * * * * * * * * * * * */
#define STATIC_STRLEN(x) (sizeof(x) - sizeof(""))
#define ASSERT_RETURN(cond, ret)    if (!(cond)) { return (ret); }

/* ***************************************************************************
 * T Y P E D E F   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* L O C A L   T Y P E D E F S * * * * * * * * * * * * * * * * * * * * * * * */

/* ***************************************************************************
 * V A R I A B L E S   A N D   C O N S T A N T S   S E C T I O N * * * * * * *
 *************************************************************************** */

/* G L O B A L   V A R I A B L E   D E F I N I T I O N S * * * * * * * * * * */

/* L O C A L   C O N S T A N T   D E F I N I T I O N S * * * * * * * * * * * */
static const char* MODULE_NAME = "DIAG";

static const char* severityStrings[MAX_SEVERITY_VALUE + 1] =
    {
        "VERBOSE",
        "DEBUG  ",
        "INFO   ",
        "WARNING",
        "ERROR  ",
        "FAILURE",
    };

/* L O C A L   V A R I A B L E   D E F I N I T I O N S * * * * * * * * * * * */
static HANDLE logFileHandle                 = NULL;
static HANDLE stdErrHandle                  = INVALID_HANDLE_VALUE;
static DIAG_SeverityType loggingSeverity    = DEFAULT_LOGGING_SEVERITY;
static BOOL isOpen                          = FALSE;
static BOOL isEnabled                       = FALSE;

/* ***************************************************************************
 * F U N C T I O N S   S E C T I O N * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* L O C A L   F U N C T I O N   P R O T O T Y P E S * * * * * * * * * * * * */
static int tryLogEntry(DIAG_SeverityType    severity,
                       const char*          module,
                       const char*          function,
                       const char*          msg);

/* L O C A L   I N L I N E   F U N C T I O N   D E F I N I T I O N S * * * * */

/* L O C A L   F U N C T I O N   D E F I N I T I O N S * * * * * * * * * * * */
static int tryLogEntry(DIAG_SeverityType    severity,
                       const char*          module,
                       const char*          function,
                       const char*          msg)
{
    DWORD actualBytesWritten = 0;
    char logEntry[MAX_LOG_ENTRY_SIZE];
    SYSTEMTIME localTime;

    GetLocalTime(&localTime);

    HRESULT printfRetVal =
            StringCbPrintf(logEntry,
                           MAX_LOG_ENTRY_SIZE,
                           "%02d-%02d-%04d %02d:%02d:%02d.%03d %s [Module %s]::%s()::%s\r\n",
                           localTime.wDay,
                           localTime.wMonth,
                           localTime.wYear,
                           localTime.wHour,
                           localTime.wMinute,
                           localTime.wSecond,
                           localTime.wMilliseconds,
                           severityStrings[severity],
                           module,
                           function,
                           msg);

    if (printfRetVal != S_OK)
    {
        /* try to log error that happened while trying to log. Note that it
         * does not make a difference if we check the return value of
         * WriteFile(...). What would we be supposed to do then? */
        WriteFile(logFileHandle,
                  "ERROR [Module DIAG]::unable to create log message!\n",
                  STATIC_STRLEN("ERROR [Module DIAG]::unable to create log message!\n"),
                  &actualBytesWritten,
                  NULL);
        return -1;
    }

    /* try to write correct log message */
    if (WriteFile(logFileHandle,
                  logEntry,
                  (DWORD)strnlen(logEntry, MAX_LOG_ENTRY_SIZE),
                  &actualBytesWritten,
                  NULL)
        == 0)
    {
        return -1;
    }

    return 0;
}

/* G L O B A L   F U N C T I O N   D E F I N I T I O N S * * * * * * * * * * */
int DIAG_Close()
{
    ASSERT_RETURN(isOpen, 0);
    ASSERT_RETURN(logFileHandle != INVALID_HANDLE_VALUE, -1);

    if (CloseHandle(logFileHandle) == 0)
    {
        return -1;
    }
    logFileHandle = INVALID_HANDLE_VALUE;
    isOpen = FALSE;

    return 0;
}

int DIAG_Open(const char* logFilePath)
{
    if (logFilePath == NULL)
    {
        return -1;
    }

    logFileHandle = CreateFile(logFilePath,
                               GENERIC_WRITE,
                               FILE_SHARE_DELETE,
                               NULL,
                               OPEN_ALWAYS,
                               FILE_ATTRIBUTE_NORMAL
                                   | FILE_FLAG_RANDOM_ACCESS,
                               NULL);

    if (logFileHandle == INVALID_HANDLE_VALUE)
    {
        stdErrHandle = GetStdHandle(STD_ERROR_HANDLE);
        if (stdErrHandle != INVALID_HANDLE_VALUE)
        {
            WriteFile(stdErrHandle,
                      "ERROR [Module DIAG]::unable to create log file!\n",
                      STATIC_STRLEN("ERROR [Module DIAG]::unable to create log file!\n"),
                      NULL,
                      NULL);
        }
        return -1;
    }

    /* try to set file pointer to the end of the file.
     * If this is not possible, the existing file is overwritten */
    SetFilePointer(logFileHandle,
                   0,
                   NULL,
                   FILE_END);
    isOpen = TRUE;
    return 0;
}

bool DIAG_IsOpen()
{
    return (isOpen == TRUE) ? true : false;
}

int DIAG_Enable()
{
    ASSERT_RETURN(isOpen, -1);
    isEnabled = TRUE;
    return 0;
}

int DIAG_Disable()
{
    isEnabled = FALSE;
    return 0;
}

int DIAG_SetSeverity(DIAG_SeverityType severity)
{
    ASSERT_RETURN(severity <= MAX_SEVERITY_VALUE, -1);
    loggingSeverity = severity;
    return 0;
}

int DIAG_LogMsg(DIAG_SeverityType   severity,
                const char*         module,
                const char*         function,
                const char*         msg)
{
    ASSERT_RETURN(isOpen && isEnabled, 0);
    ASSERT_RETURN(severity >= loggingSeverity, 0);
    ASSERT_RETURN(logFileHandle != INVALID_HANDLE_VALUE, -1);

    if (severity <= MAX_SEVERITY_VALUE)
    {
        return tryLogEntry(severity, module, function, msg);
    }

    tryLogEntry(DIAG_WARNING, MODULE_NAME, __func__, "invalid logging severity specified! Logging with highest severity");
    return tryLogEntry(MAX_SEVERITY_VALUE, module, function, msg);
}

int DIAG_LogMsgArg(DIAG_SeverityType   severity,
                   const char*         module,
                   const char*         function,
                   const char*         msg,
                   ...)
{
    ASSERT_RETURN(isOpen && isEnabled, 0);
    ASSERT_RETURN(severity >= loggingSeverity, 0);

    char tempBufMsg[TEMP_BUF_SIZE];

    va_list args;
    va_start(args, msg);
    HRESULT printfRetVal = StringCbVPrintf(tempBufMsg,
                                           TEMP_BUF_SIZE,
                                           msg,
                                           args);
    va_end(args);
    
    if (printfRetVal == S_OK)
    {
        return DIAG_LogMsg(severity, module, function, tempBufMsg);
    }

    DIAG_LogMsg(DIAG_WARNING, MODULE_NAME, __func__,
        "unable to insert arguments to the log string! Logging the format string instead:");
    DIAG_LogMsg(severity, module, function, msg);
    return -1;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* ***************************************************************************
 * E N D   O F   F I L E * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */
