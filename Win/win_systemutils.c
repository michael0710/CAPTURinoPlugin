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
 * \addtogroup systemutils
 */
/* ************************************************************************* */

/* ***************************************************************************
 * I N C L U D E   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* S Y S T E M   I N C L U D E S * * * * * * * * * * * * * * * * * * * * * * */
#include <windows.h>
#include <shlwapi.h>
#include <strsafe.h>
#include <synchapi.h>
#include <sysinfoapi.h>
#include <timeapi.h>

/* P R O J E C T   I N C L U D E S * * * * * * * * * * * * * * * * * * * * * */

/* M O D U L E   H E A D E R   I N C L U D E * * * * * * * * * * * * * * * * */
#include "systemutils.h"

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

/* G L O B A L   V A R I A B L E   D E F I N I T I O N S * * * * * * * * * * */

/* L O C A L   C O N S T A N T   D E F I N I T I O N S * * * * * * * * * * * */
static const char* MODULE_NAME = "SYSU";

/* L O C A L   V A R I A B L E   D E F I N I T I O N S * * * * * * * * * * * */

/* ***************************************************************************
 * F U N C T I O N S   S E C T I O N * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* L O C A L   F U N C T I O N   P R O T O T Y P E S * * * * * * * * * * * * */

/* L O C A L   I N L I N E   F U N C T I O N   D E F I N I T I O N S * * * * */

/* L O C A L   F U N C T I O N   D E F I N I T I O N S * * * * * * * * * * * */

/* G L O B A L   F U N C T I O N   D E F I N I T I O N S * * * * * * * * * * */
int SYSU_Sleep(unsigned int milliseconds)
{
    Sleep(milliseconds);
    return 0;
}

int SYSU_GetCurrentMillis(unsigned long* currentTime)
{
    *currentTime = timeGetTime();
    return 0;
}

int SYSU_GetCurrentTime(unsigned long long* unixTime, unsigned long* micros)
{
    SYSTEMTIME sysTime;
    GetSystemTime(&sysTime);

    FILETIME fileTime;
    BOOL rvSystime2Filetime = SystemTimeToFileTime(&sysTime, &fileTime);
    if (rvSystime2Filetime == FALSE)
    {
        return -1;
    }

    unsigned long long time = ((unsigned long long)fileTime.dwHighDateTime << 32) | ((unsigned long long)fileTime.dwLowDateTime);
    /* magic numbers necessary as unix time starts at January 1, 1970 00:00:00 UTC in 1s steps,
       while FILETIME starts at January 1, 1601 00:00:00 UTC in 100ns steps */
    *unixTime = (time - 116444736000000000) / 10000000;
    unsigned long long tempMicros = (time - 116444736000000000) / 10;
    *micros = (unsigned long)(tempMicros - (*unixTime * 1000000));

    return 0;
}

int SYSU_StrNCpy_S(char* dest,
                   size_t destSize,
                   const char* src,
                   size_t srcSize)
{
    HRESULT rv = StringCbCopyN(dest, destSize, src, srcSize);
    return (rv == S_OK) ? 0 : -1;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* ***************************************************************************
 * E N D   O F   F I L E * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */
