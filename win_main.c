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
 * \brief Main entry point for the windows extcap interface.
 */
/* ************************************************************************* */

/* ***************************************************************************
 * I N C L U D E   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* S Y S T E M   I N C L U D E S * * * * * * * * * * * * * * * * * * * * * * */
#include <windows.h>
#include <string.h>
#include <strsafe.h>
#include <processthreadsapi.h>

/* P R O J E C T   I N C L U D E S * * * * * * * * * * * * * * * * * * * * * */
#include "argparser.h"
#include "console.h"
#include "diagnosis.h"
#include "extcapmngr.h"
#include "genericutils.h"
#include "generic_main.h"

#pragma comment(lib, "OneCore.lib")

static const char MODULE_NAME[] = "WINMAIN";

/* L O C A L   C O N F I G   D E F I N I T I O N S * * * * * * * * * * * * * */
#define LOG_FROM_STARTUP    (0)

int main(int argc, char *argv[])
{
#if LOG_FROM_STARTUP == 1
    int rvDiagInit = DIAG_Open(TODO: specify path to the initial log file here if needed!);
    if (rvDiagInit != 0)
    {
        return -1;
    }
    DIAG_Enable();
    DIAG_LogMsg(DIAG_INFO, MODULE_NAME, __func__, "***************************************************************");
    DIAG_LogMsg(DIAG_INFO, MODULE_NAME, __func__, "New call of the extcap interface");
#endif

    int rv = generic_main(argc, argv);

    DIAG_LogMsgArg(DIAG_INFO, MODULE_NAME, __func__, "The Extcap plugin finished with return value %d", rv);
    DIAG_Close();
    return rv;
}
