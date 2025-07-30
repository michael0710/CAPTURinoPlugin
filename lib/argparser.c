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
 * \addtogroup argparser
 */
/* ************************************************************************* */

/* ***************************************************************************
 * I N C L U D E   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* S Y S T E M   I N C L U D E S * * * * * * * * * * * * * * * * * * * * * * */
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/* P R O J E C T   I N C L U D E S * * * * * * * * * * * * * * * * * * * * * */
#include "diagnosis.h"

/* M O D U L E   H E A D E R   I N C L U D E * * * * * * * * * * * * * * * * */
#include "argparser.h"

/* ***************************************************************************
 * D E F I N E   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* L O C A L   C O N F I G   D E F I N I T I O N S * * * * * * * * * * * * * */

/* L O C A L   M A C R O   D E F I N I T I O N S * * * * * * * * * * * * * * */

/* ***************************************************************************
 * T Y P E D E F   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* L O C A L   T Y P E D E F S * * * * * * * * * * * * * * * * * * * * * * * */

/* ***************************************************************************
 * V A R I A B L E S   A N D   C O N S T A N T S   S E C T I O N * * * * * * *
 *************************************************************************** */

/* G L O B A L   V A R I A B L E S * * * * * * * * * * * * * * * * * * * * */

/* L O C A L   C O N S T A N T   D E F I N I T I O N S * * * * * * * * * * * */
static const char* MODULE_NAME = "ARGP";

/* L O C A L   V A R I A B L E   D E F I N I T I O N S * * * * * * * * * * * */

/* ***************************************************************************
 * F U N C T I O N S   S E C T I O N * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* L O C A L   F U N C T I O N   P R O T O T Y P E S * * * * * * * * * * * * */

/* L O C A L   I N L I N E   F U N C T I O N   D E F I N I T I O N S * * * * */

/* L O C A L   F U N C T I O N   D E F I N I T I O N S * * * * * * * * * * * */

/* G L O B A L   F U N C T I O N   D E F I N I T I O N S * * * * * * * * * * */
int ARGP_constainsKey(int argc, char *argv[], const char* key, bool* isAvailable)
{
    DIAG_LogMsg(DIAG_DEBUG, MODULE_NAME, __func__, "function entered");
    *isAvailable = false;
    for (int i=0; i<argc; i++)
    {
        if (strncmp(argv[i], key, strnlen(key, 128)) == 0)
        {
            if ((argv[i][strnlen(key, 128)] == '=') || (argv[i][strnlen(key, 128)] == '\0'))
            {
                DIAG_LogMsgArg(DIAG_INFO, MODULE_NAME, __func__, "key \'%s\' found", key);
                *isAvailable = true;
                return 0;
            }
        }
    }
    return 0;
}

int ARGP_getLongOfArgs(int argc, char *argv[], const char* key, long* value)
{
    DIAG_LogMsg(DIAG_DEBUG, MODULE_NAME, __func__, "function entered");
    char* strValue;
    int rvGetStr = ARGP_getP2StringOfArgs(argc, argv, key, &strValue);
    if (rvGetStr != 0)
    {
        DIAG_LogMsg(DIAG_WARNING, MODULE_NAME, __func__, "call to ARGP_getP2StringOfArgs() failed");
        return -1;
    }

    char* endptr;
    *value = strtol(strValue, &endptr, 10);
    if (strValue == endptr)
    {
        DIAG_LogMsg(DIAG_WARNING, MODULE_NAME, __func__, "call to strtol failed");
        return -1;
    }

    return 0;
}

int ARGP_getUnsignedLongOfArgs(int argc, char *argv[], const char* key, unsigned long* value)
{
    DIAG_LogMsg(DIAG_DEBUG, MODULE_NAME, __func__, "function entered");
    char* strValue;
    int rvGetStr = ARGP_getP2StringOfArgs(argc, argv, key, &strValue);
    if (rvGetStr != 0)
    {
        DIAG_LogMsg(DIAG_WARNING, MODULE_NAME, __func__, "call to ARGP_getP2StringOfArgs() failed");
        return -1;
    }

    if (strValue[0] == '-')
    {
        DIAG_LogMsgArg(DIAG_WARNING, MODULE_NAME, __func__, "value \'%s\' is negative", strValue);
        return -1;
    }

    char* endptr;
    *value = strtoul(strValue, &endptr, 10);
    if (strValue == endptr)
    {
        DIAG_LogMsg(DIAG_WARNING, MODULE_NAME, __func__, "call to strtoul() failed");
        return -1;
    }

    return 0;
}

int ARGP_getP2StringOfArgs(int argc, char *argv[], const char* key, char** value)
{
    DIAG_LogMsg(DIAG_DEBUG, MODULE_NAME, __func__, "function entered");
    for (int i=0; i<argc; i++)
    {
        if (strncmp(argv[i], key, strnlen(key, 128)) == 0)
        {
            DIAG_LogMsgArg(DIAG_INFO, MODULE_NAME, __func__, "key \'%s\' found", key);
            /* the key was found but it is not clear if ... */
            if (argv[i][strnlen(key, 128)] == '=')
            {
                /* ... the value is given directly after the key separated by a '=' */
                *value = &argv[i][strnlen(key, 128) + 1];
                return 0;
            }
            else if ((argv[i][strnlen(key, 128)] == '\0') && (argc > i+1))
            {
                /* ... the value is given in the next argument */
                *value = argv[i+1];
                return 0;
            }
            DIAG_LogMsg(DIAG_INFO, MODULE_NAME, __func__, "no value found for key");
            /* ... for every other case just search the remaining arguments.
                   This can occur if the matched string is longer than the key,
                   e.g. the key being "--test" and the argument being "--test123" */
        }
    }
    return -1;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* ***************************************************************************
 * E N D   O F   F I L E * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */
