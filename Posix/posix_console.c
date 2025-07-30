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
#include <stdarg.h>
#include <stdio.h>

/* P R O J E C T   I N C L U D E S * * * * * * * * * * * * * * * * * * * * * */
#include "diagnosis.h"

/* M O D U L E   H E A D E R   I N C L U D E * * * * * * * * * * * * * * * * */
#include "console.h"

/* ***************************************************************************
 * D E F I N E   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* L O C A L   C O N F I G   D E F I N I T I O N S * * * * * * * * * * * * * */
#define TEMP_BUF_SIZE (1024)

/* L O C A L   M A C R O   D E F I N I T I O N S * * * * * * * * * * * * * * */

/* ***************************************************************************
 * T Y P E D E F   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* L O C A L   T Y P E D E F S * * * * * * * * * * * * * * * * * * * * * * * */

/* ***************************************************************************
 * V A R I A B L E S   A N D   C O N S T A N T S   S E C T I O N * * * * * * *
 *************************************************************************** */

/* G L O B A L   V A R I A B L E   D E F I N I T I O N S * * * * * * * * * * */

/* L O C A L   C O N S T A N T   D E F I N I T I O N S * * * * * * * * * * * */
static const char* MODULE_NAME = "CNSL";

/* L O C A L   V A R I A B L E   D E F I N I T I O N S * * * * * * * * * * * */

/* ***************************************************************************
 * F U N C T I O N S   S E C T I O N * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* L O C A L   F U N C T I O N   P R O T O T Y P E S * * * * * * * * * * * * */
static int vaWriteArg(const char* fmtMsg, va_list argList);

static int write2stream(FILE*       ptrStream,
                        const char* buf,
                        size_t      chars2write);

/* L O C A L   I N L I N E   F U N C T I O N   D E F I N I T I O N S * * * * */

/* L O C A L   F U N C T I O N   D E F I N I T I O N S * * * * * * * * * * * */
static int vaWriteArg(const char* fmtMsg, va_list argList)
{
    char tempBufMsg[TEMP_BUF_SIZE];
    int rvVsnprintf = vsnprintf(tempBufMsg, TEMP_BUF_SIZE, fmtMsg, argList);

    int bytesWritten = (rvVsnprintf < TEMP_BUF_SIZE) ? rvVsnprintf : TEMP_BUF_SIZE;
    DIAG_LogMsgArg(DIAG_VERBOSE, MODULE_NAME, __func__, "writes %d characters to stdout", bytesWritten);
    if (rvVsnprintf >= 0)
    {
        DIAG_LogMsgArg(DIAG_VERBOSE, MODULE_NAME, __func__, "writes to STDOUT:\r\n%s", tempBufMsg);
        return write2stream(stdout, tempBufMsg, bytesWritten);
    }
    
    DIAG_LogMsg(DIAG_ERROR, MODULE_NAME, __func__,
        "unable to insert arguments to the console string! Logging the format string:");
    DIAG_LogMsg(DIAG_INFO, MODULE_NAME, __func__, fmtMsg);
    return -1;
}

static int write2stream(FILE*       ptrStream,
                        const char* buf,
                        size_t      chars2write)
{
    size_t rvFwrite = fwrite(buf, sizeof(char), chars2write, ptrStream);
    return (rvFwrite == chars2write) ? 0 : -1;
}

/* G L O B A L   F U N C T I O N   D E F I N I T I O N S * * * * * * * * * * */

int CNSL_Init()
{
    DIAG_LogMsg(DIAG_DEBUG, MODULE_NAME, __func__, "function entered");

    if (   (stdin  == NULL)
        || (stdout == NULL)
        || (stderr == NULL))
    {
        DIAG_LogMsg(DIAG_FAILURE, MODULE_NAME, __func__, "streams for stdin, stdout, stderr are not available");
        return -1;
    }

    return 0;
}

int CNSL_Write(const char* buf,
               size_t      chars2write)
{
    DIAG_LogMsgArg(DIAG_VERBOSE, MODULE_NAME, __func__, "writes to STDOUT:\r\n%s", buf);
    return write2stream(stdout, buf, chars2write);
}

int CNSL_WriteLn(const char* buf,
                 size_t      chars2write)
{
    DIAG_LogMsgArg(DIAG_VERBOSE, MODULE_NAME, __func__, "writes to STDOUT:\r\n%s", buf);
    int rv = write2stream(stdout, buf, chars2write);
    if (rv == 0)
    {
        rv = write2stream(stdout, "\n", 1);
    }
    return rv;
}

int CNSL_WriteArg(const char* fmtMsg,
                  ...)
{
    va_list args;
    va_start(args, fmtMsg);
    int rvVaWriteArg = vaWriteArg(fmtMsg, args);
    va_end(args);
    return rvVaWriteArg;
}

int CNSL_WriteArgLn(const char* fmtMsg,
                    ...)
{
    va_list args;
    va_start(args, fmtMsg);
    int rvVaWriteArg = vaWriteArg(fmtMsg, args);
    va_end(args);
    if (rvVaWriteArg == 0)
    {
        return write2stream(stdout, "\n", 1);
    }
    return -1;
}

int CNSL_WriteErr(const char* buf,
                  size_t      chars2write)
{
    DIAG_LogMsgArg(DIAG_VERBOSE, MODULE_NAME, __func__, "writes to STDERR:\r\n%s", buf);
    return write2stream(stderr, buf, chars2write);
}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* ***************************************************************************
 * E N D   O F   F I L E * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */
