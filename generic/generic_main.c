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
 * \addtogroup main
 * \brief Generic implementation of the main program, that only relies on the
 *        existence of other source files from within this project.
 */
/* ************************************************************************* */

/* ***************************************************************************
 * I N C L U D E   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* S Y S T E M   I N C L U D E S * * * * * * * * * * * * * * * * * * * * * * */
#include <stdbool.h>
#include <string.h>

/* P R O J E C T   I N C L U D E S * * * * * * * * * * * * * * * * * * * * * */
#include "argparser.h"
#include "console.h"
#include "diagnosis.h"
#include "extcapmngr.h"
#include "genericutils.h"
#include "version.h"

/* M O D U L E   H E A D E R   I N C L U D E * * * * * * * * * * * * * * * * */
#include "generic_main.h"

/* ***************************************************************************
 * D E F I N E   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* L O C A L   C O N F I G   D E F I N I T I O N S * * * * * * * * * * * * * */
#define VERSION_STR        CMAKE_PROJECT_VERSION_MAJOR\
                        "."CMAKE_PROJECT_VERSION_MINOR\
                        "."CMAKE_PROJECT_VERSION_PATCH

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
static const char MODULE_NAME[] = "MAIN";

/* L O C A L   V A R I A B L E   D E F I N I T I O N S * * * * * * * * * * * */

/* ***************************************************************************
 * F U N C T I O N S   S E C T I O N * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* L O C A L   F U N C T I O N   P R O T O T Y P E S * * * * * * * * * * * * */
static int printHelp(int argc, char *argv[]);

/* L O C A L   I N L I N E   F U N C T I O N   D E F I N I T I O N S * * * * */

/* L O C A L   F U N C T I O N   D E F I N I T I O N S * * * * * * * * * * * */
static int printHelp(int argc, char *argv[])
{
    DIAG_LogMsg(DIAG_DEBUG, MODULE_NAME, __func__, "function entered");

    CNSL_Write("CAPTURino interface for Wireshark\nVersion ",
               STATIC_STRLEN("CAPTURino interface for Wireshark\nVersion "));
    CNSL_Write(VERSION_STR,
               STATIC_STRLEN(VERSION_STR));
    CNSL_Write("\n",
               STATIC_STRLEN("\n"));

    return 0;
}

/* G L O B A L   F U N C T I O N   D E F I N I T I O N S * * * * * * * * * * */
int generic_main(int argc, char *argv[])
{
    char* logFilePath = NULL;
    int rvHasLogFile = ARGP_getP2StringOfArgs(argc, argv, "--logfile", &logFilePath);
    if (rvHasLogFile == 0)
    {
        if (DIAG_IsOpen())
        {
            DIAG_LogMsgArg(DIAG_INFO, MODULE_NAME, __func__, "The call contained the --logfile argument. This log file is going to be closed. Further logs will be written to \'%s\'", logFilePath);
            DIAG_Close();
        }
        DIAG_Open(logFilePath); /* open the log file specified in the args */
        DIAG_Enable();
        DIAG_LogMsg(DIAG_INFO, MODULE_NAME, __func__, "***************************************************************");
        DIAG_LogMsg(DIAG_INFO, MODULE_NAME, __func__, "New call of the extcap interface");
        long logLevel = 0;
        int rvHasLogLevel = ARGP_getLongOfArgs(argc, argv, "--loglevel", &logLevel);
        if (rvHasLogLevel == 0)
        {
            DIAG_SetSeverity((DIAG_SeverityType)logLevel);
        }
        else
        {
            DIAG_LogMsg(DIAG_WARNING, MODULE_NAME, __func__, "The call contained the --logfile argument but no --loglevel argument. Using default log level DIAG_INFO");
            DIAG_SetSeverity(DIAG_INFO);
        }
    }
    
    if(CNSL_Init() != 0)
    {
        DIAG_LogMsg(DIAG_FAILURE, MODULE_NAME, __func__, "initialization of the module failed!");
        return -1;
    }
    
    DIAG_LogMsgArg(DIAG_INFO, MODULE_NAME, __func__, "interface called with %d arguments:", argc);
    for (int i=0; i<argc; i++)
    {
        DIAG_LogMsgArg(DIAG_INFO, MODULE_NAME, __func__, "    %s", argv[i]);
    }
    
    /* print help if no arguments has been passed */
    if (argc <= 1)
    {
        printHelp(argc, argv);
        return 0;
    }
    
    if (EXMG_execute(argc, argv) != 0)
    {
        DIAG_LogMsg(DIAG_WARNING, MODULE_NAME, __func__, "no valid command found");
        CNSL_Write("Command not found\n", STATIC_STRLEN("Command not found\n"));
        return -1;
    }
    
    return 0;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* ***************************************************************************
 * E N D   O F   F I L E * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */
