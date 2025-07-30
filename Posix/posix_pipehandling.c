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
 * \addtogroup pipehandling
 */
/* ************************************************************************* */

/* ***************************************************************************
 * I N C L U D E   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* S Y S T E M   I N C L U D E S * * * * * * * * * * * * * * * * * * * * * * */
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

/* P R O J E C T   I N C L U D E S * * * * * * * * * * * * * * * * * * * * * */
#include "diagnosis.h"
#include "linkedlist.h"
#include "posix_internal_filehandling.h"

/* M O D U L E   H E A D E R   I N C L U D E * * * * * * * * * * * * * * * * */
#include "pipehandling.h"

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
const PipeHandleType INVALID_PIPE_HANDLE = 0;

/* L O C A L   C O N S T A N T   D E F I N I T I O N S * * * * * * * * * * * */
static const char* MODULE_NAME = "PIPH";

/* L O C A L   V A R I A B L E   D E F I N I T I O N S * * * * * * * * * * * */
static unsigned int pipeHandlesIndexCounter = 1;
static LLST_ListEntryType* pipeHandlesList = NULL;

/* ***************************************************************************
 * F U N C T I O N S   S E C T I O N * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* L O C A L   F U N C T I O N   P R O T O T Y P E S * * * * * * * * * * * * */

/* L O C A L   I N L I N E   F U N C T I O N   D E F I N I T I O N S * * * * */

/* L O C A L   F U N C T I O N   D E F I N I T I O N S * * * * * * * * * * * */

/* G L O B A L   F U N C T I O N   D E F I N I T I O N S * * * * * * * * * * */
int PIPH_Open(const char* path, PipeHandleType* pipeHandleVal)
{
    DIAG_LogMsg(DIAG_DEBUG, MODULE_NAME, __func__, "function entered");

    FileHandleType fileHandle;
    /* the oflags are taken from the fopen documentation saying this is the same as "w" in fopen
       see https://pubs.opengroup.org/onlinepubs/9799919799/functions/fopen.html */
    int rv = FILH_Open(path, O_WRONLY | O_CREAT | O_TRUNC, &fileHandle);
    *pipeHandleVal = (PipeHandleType)fileHandle;
    return rv;
}

int PIPH_Close(PipeHandleType pipeHandleVal)
{
    return FILH_Close((FileHandleType) pipeHandleVal);
}

int PIPH_Write(PipeHandleType pipeHandleVal,
               const char*    buf,
               size_t         chars2write)
{
    return FILH_Write((FileHandleType)pipeHandleVal, buf, chars2write);
}

int PIPH_WriteLn(PipeHandleType pipeHandleVal,
                 const char*    buf,
                 size_t         chars2write)
{
    return FILH_WriteLn((FileHandleType)pipeHandleVal, buf, chars2write);
}

int PIPH_WriteArg(PipeHandleType pipeHandleVal,
                  const char*    fmtMsg,
                                 ...)
{
    va_list args;
    va_start(args, fmtMsg);
    int rvVaWriteArg = FILH_vaWriteArg((FileHandleType)pipeHandleVal, fmtMsg, args);
    va_end(args);
    return rvVaWriteArg;
}

int PIPH_WriteArgLn(PipeHandleType pipeHandleVal,
                    const char*    fmtMsg,
                                   ...)
{
    va_list args;
    va_start(args, fmtMsg);
    int rvVaWriteArgLn = FILH_vaWriteArgLn((FileHandleType)pipeHandleVal, fmtMsg, args);
    va_end(args);
    return rvVaWriteArgLn;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* ***************************************************************************
 * E N D   O F   F I L E * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */
