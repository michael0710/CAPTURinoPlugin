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
#include <stdbool.h>
#include <string.h>

/* P R O J E C T   I N C L U D E S * * * * * * * * * * * * * * * * * * * * * */
#include "console.h"
#include "capturinotestintfc.h"
#include "capturinointfc.h"
#include "diagnosis.h"
#include "genericutils.h"
#include "systemutils.h"

/* M O D U L E   H E A D E R   I N C L U D E * * * * * * * * * * * * * * * * */
#include "extcapmngr.h"

/* ***************************************************************************
 * D E F I N E   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* L O C A L   C O N F I G   D E F I N I T I O N S * * * * * * * * * * * * * */
#define MAX_CMD_LENGTH 32       ///< max length of a single cmd command or argument

/* L O C A L   M A C R O   D E F I N I T I O N S * * * * * * * * * * * * * * */

/* ***************************************************************************
 * T Y P E D E F   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* L O C A L   T Y P E D E F S * * * * * * * * * * * * * * * * * * * * * * * */

/* ***************************************************************************
 * V A R I A B L E S   A N D   C O N S T A N T S   S E C T I O N * * * * * * *
 *************************************************************************** */

/* G L O B A L   V A R I A B L E S * * * * * * * * * * * * * * * * * * * * */
#define REG_INTFCS_COUNT 2
const EXMG_IntfcType *registeredInterfaces[REG_INTFCS_COUNT] =
{
    &capturinoTestIntfc,
    &capturinoIntfc
};

/* L O C A L   C O N S T A N T   D E F I N I T I O N S * * * * * * * * * * * */
static const char* MODULE_NAME = "EXMG";

/* L O C A L   V A R I A B L E   D E F I N I T I O N S * * * * * * * * * * * */
static int mCalledExtcapIntfc = -1;

/* ***************************************************************************
 * F U N C T I O N S   S E C T I O N * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* L O C A L   F U N C T I O N   P R O T O T Y P E S * * * * * * * * * * * * */
/** Implements the --extcap-interfaces command
 *
 *
 * \see https://www.wireshark.org/docs/wsdg_html_chunked/ChCaptureExtcap.html chapter 8.2.1
 *
 * \returns "extcap {version=1.0}{help=Some help url}"
 *          "interface {value=example1}{display=Example interface 1 for extcap}"
 *          "interface {value=example2}{display=Example interface 2 for extcap}"
 */
static int extcapInterfaces(int argc, char *argv[]);

/* L O C A L   I N L I N E   F U N C T I O N   D E F I N I T I O N S * * * * */

/* L O C A L   F U N C T I O N   D E F I N I T I O N S * * * * * * * * * * * */
static int extcapInterfaces(int argc, char *argv[])
{
    DIAG_LogMsg(DIAG_DEBUG, MODULE_NAME, __func__, "function entered");
    /* NOTE: if argv contains "--extcap-version=x.x" then it might be used for a version check of wireshark */

    CNSL_WriteLn("extcap {version=0.1.0}{help=no online help available}",
                 STATIC_STRNLEN("extcap {version=0.1.0}{help=no online help available}", 256));

    for (int i=0; i<REG_INTFCS_COUNT; i++)
    {
        int rvWriteArgLn = CNSL_WriteArgLn("interface {value=%s}{display=%s}",
                                           registeredInterfaces[i]->val,
                                           registeredInterfaces[i]->disp);
        if (rvWriteArgLn != 0)
        {
            DIAG_LogMsg(DIAG_ERROR, MODULE_NAME, __func__, "unable to write format string to console out!");
            return -1;
        }
    }

    return 0;
}

/* G L O B A L   F U N C T I O N   D E F I N I T I O N S * * * * * * * * * * */
int EXMG_execute(int argc, char *argv[])
{
    DIAG_LogMsg(DIAG_DEBUG, MODULE_NAME, __func__, "function entered");

    bool isExtcapInterfaces = false;
    bool isExtcapDlts = false;
    bool isExtcapConfig = false;
    bool isCapture = false;
    ARGP_constainsKey(argc, argv, "--extcap-interfaces", &isExtcapInterfaces);
    ARGP_constainsKey(argc, argv, "--extcap-dlts", &isExtcapDlts);
    ARGP_constainsKey(argc, argv, "--extcap-config", &isExtcapConfig);
    ARGP_constainsKey(argc, argv, "--capture", &isCapture);

    if (isExtcapInterfaces)
    {
        return extcapInterfaces(argc, argv);
    }
    else
    {
        char* intfc = NULL;
        ARGP_getP2StringOfArgs(argc, argv, "--extcap-interface", &intfc);
        for (int i=0; i<REG_INTFCS_COUNT; i++)
        {
            if (strncmp(intfc, registeredInterfaces[i]->val, MAX_INTFCVAL_LENGTH) == 0)
            {
                mCalledExtcapIntfc = i;
            }
        }
        if (mCalledExtcapIntfc < 0)
        {
            return -1;
        }

        if (isExtcapDlts)
        {
            return registeredInterfaces[mCalledExtcapIntfc]->extcapDltsFunc(argc, argv);
        }
        else if (isExtcapConfig)
        {
            return registeredInterfaces[mCalledExtcapIntfc]->extcapConfigFunc(argc, argv);
        }
        else if (isCapture)
        {
            return registeredInterfaces[mCalledExtcapIntfc]->extcapCaptureFunc(argc, argv);
        }
    }

    return -1;
}

int EXMG_terminateIntfc()
{
    if ((mCalledExtcapIntfc >= 0) && (mCalledExtcapIntfc < REG_INTFCS_COUNT))
    {
        registeredInterfaces[mCalledExtcapIntfc]->extcapTerminateCb();
    }
    return 0;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* ***************************************************************************
 * E N D   O F   F I L E * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */
