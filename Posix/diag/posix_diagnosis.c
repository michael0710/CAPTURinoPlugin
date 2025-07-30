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
 * \brief Implementation of the Diagnosis component for POSIX compliant
 *        operating systems
 */
/* ************************************************************************* */

/* ***************************************************************************
 * I N C L U D E   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* S Y S T E M   I N C L U D E S * * * * * * * * * * * * * * * * * * * * * * */
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

/* P R O J E C T   I N C L U D E S * * * * * * * * * * * * * * * * * * * * * */

/* M O D U L E   H E A D E R   I N C L U D E * * * * * * * * * * * * * * * * */
#include "diagnosis.h"

/* ***************************************************************************
 * D E F I N E   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* L O C A L   C O N F I G   D E F I N I T I O N S * * * * * * * * * * * * * */
#define DEFAULT_LOGGING_SEVERITY    (DIAG_VERBOSE)
#define MAX_SEVERITY_VALUE          (5)
#define MAX_LOG_ENTRY_SIZE          (2048) /**< needs to fit in a size_t
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

/** Strings of the possible severities.
 *
 * \note the order of this array of strings must match the order of the enum
 *       defined in the diagnosis.h file.
 * \see DIAG_SeverityType
 */
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
static FILE* logFilePtr                             = NULL;
static DIAG_SeverityType loggingSeverity            = DEFAULT_LOGGING_SEVERITY;
static bool isOpen                                  = false;
static bool isEnabled                               = false;
static bool year2038appeared                        = false;

/* ***************************************************************************
 * F U N C T I O N S   S E C T I O N * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* L O C A L   F U N C T I O N   P R O T O T Y P E S * * * * * * * * * * * * */
static int tryLogEntry(DIAG_SeverityType    severity,
                       const char* restrict module,
                       const char* restrict function,
                       const char* restrict msg);

/* L O C A L   I N L I N E   F U N C T I O N   D E F I N I T I O N S * * * * */

/* L O C A L   F U N C T I O N   D E F I N I T I O N S * * * * * * * * * * * */
static int tryLogEntry(DIAG_SeverityType    severity,
                       const char* restrict module,
                       const char* restrict function,
                       const char* restrict msg)
{
    char logEntry[MAX_LOG_ENTRY_SIZE];
    struct timespec currentUnixTimeAndNanos;
    int rvClockGettime = clock_gettime(CLOCK_REALTIME, &currentUnixTimeAndNanos);
    if (   (year2038appeared == false)
        && (rvClockGettime == -1)
        && (errno == EOVERFLOW))
    {
        /* congratulations, your system just passed January 19 2038
           03:14:07 UTC with a time_t type that is defined as 32 bit signed
           integer ... */
        year2038appeared = true;
        size_t chars2write = STATIC_STRLEN("  -  -       :  :  .   ERROR   [Module DIAG]::Congratulations, the year-2038-problem just arrised!\n");
        size_t rvFwrite = fwrite("  -  -       :  :  .   ERROR   [Module DIAG]::Congratulations, the year-2038-problem just arrised!\n",
                                 sizeof(char),
                                 chars2write,
                                 logFilePtr);
        if (rvFwrite < chars2write)
        {
            fwrite("ERROR [Module DIAG]::error while writing to log file!\n",
                   sizeof(char),
                   STATIC_STRLEN("ERROR [Module DIAG]::error while writing to log file!\n"),
                   stderr);
        }
    }
    
    struct tm localTime;
    localtime_r(&currentUnixTimeAndNanos.tv_sec, &localTime);

    /* as the posix standard counts month from 0-11 we have to add 1 to get the
       actual month that every human being understands */
    int rvSnprintf = snprintf(logEntry,
                              MAX_LOG_ENTRY_SIZE,
                              "%02d-%02d-%04d %02d:%02d:%02d.%03ld %s [Module %s]::%s()::%s\n",
                              localTime.tm_mday,
                              localTime.tm_mon+1,
                              (1900 + localTime.tm_year),
                              localTime.tm_hour,
                              localTime.tm_min,
                              localTime.tm_sec,
                              currentUnixTimeAndNanos.tv_nsec/1000000, /* convert the nanos to millis */
                              severityStrings[severity],
                              module,
                              function,
                              msg);

    if (rvSnprintf < 0)
    {
        /* try to log error that happened while trying to log. Note that error
           handling the error handling somewhen makes no sense. If this does
           not work, the function will just fail */
        fwrite("  -  -       :  :  .   ERROR   [Module DIAG]::unable to create log message!\n",
               sizeof(char),
               STATIC_STRLEN("  -  -       :  :  .   ERROR   [Module DIAG]::unable to create log message!\n"),
               logFilePtr);
        return -1;
    }

    /* try to write correct log message */
    size_t chars2write = strnlen(logEntry, MAX_LOG_ENTRY_SIZE);
    size_t rvFwrite = fwrite(logEntry,
                             sizeof(char),
                             chars2write,
                             logFilePtr);
    
    if (rvFwrite < chars2write)
    {
        return -1;
    }

    return 0;
}

/* G L O B A L   F U N C T I O N   D E F I N I T I O N S * * * * * * * * * * */

int DIAG_Close()
{
    ASSERT_RETURN(isOpen, 0);

    int rvFclose = fclose(logFilePtr);
    if (rvFclose != 0)
    {
        fwrite("ERROR [Module DIAG]::error while closing log file!\n",
               sizeof(char),
               STATIC_STRLEN("ERROR [Module DIAG]::error while closing log file!\n"),
               stderr);
        return -1;
    }
    logFilePtr = NULL;

    return 0;
}

/* TODO: might think about also specifying the amount of characters in the string as param */
int DIAG_Open(const char* restrict logFilePath)
{
    if (logFilePath == NULL)
    {
        return -1;
    }
    
    /* stderr must actually never be NULL, but who knows ... */
    if (stderr == NULL)
    {
        return -1;
    }
    
    logFilePtr = fopen(logFilePath, "a");
    if (logFilePtr == NULL)
    {
        fwrite("ERROR [Module DIAG]::unable to create log file!\n",
               sizeof(char),
               STATIC_STRLEN("ERROR [Module DIAG]::unable to create log file!\n"),
               stderr);
        fprintf(stderr, "INFO [Module DIAG]::create log file failed with errno = %s\n", strerror(errno));
        return -1;
    }

    isOpen = true;
    return 0;
}

bool DIAG_IsOpen()
{
    return isOpen;
}

int DIAG_Enable()
{
    ASSERT_RETURN(isOpen, -1);
    isEnabled = true;
    return 0;
}

int DIAG_Disable()
{
    isEnabled = false;
    return 0;
}

int DIAG_SetSeverity(DIAG_SeverityType severity)
{
    ASSERT_RETURN(severity <= MAX_SEVERITY_VALUE, -1);
    loggingSeverity = severity;
    return 0;
}

int DIAG_LogMsg(DIAG_SeverityType       severity,
                const char* restrict    module,
                const char* restrict    function,
                const char* restrict    msg)
{
    ASSERT_RETURN(isOpen && isEnabled, 0);
    ASSERT_RETURN(severity >= loggingSeverity, 0);
    

    if (severity <= MAX_SEVERITY_VALUE)
    {
        return tryLogEntry(severity, module, function, msg);
    }
    
    tryLogEntry(DIAG_WARNING, MODULE_NAME, __func__, "invalid logging severity specified! Logging with highest severity");
    return tryLogEntry(MAX_SEVERITY_VALUE, module, function, msg);
}

int DIAG_LogMsgArg(DIAG_SeverityType    severity,
                   const char* restrict module,
                   const char* restrict function,
                   const char* restrict msg,
                   ...)
{
    ASSERT_RETURN(isOpen && isEnabled, 0);
    ASSERT_RETURN(severity >= loggingSeverity, 0);

    char tempBufMsg[TEMP_BUF_SIZE];

    va_list args;
    va_start(args, msg);
    int rvVsnprintf = vsnprintf(tempBufMsg, TEMP_BUF_SIZE, msg, args);
    va_end(args);
    
    if (rvVsnprintf >= 0)
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
