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
 * \addtogroup serialhandling
 */
/* ************************************************************************* */

/* ***************************************************************************
 * I N C L U D E   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* S Y S T E M   I N C L U D E S * * * * * * * * * * * * * * * * * * * * * * */
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

/* P R O J E C T   I N C L U D E S * * * * * * * * * * * * * * * * * * * * * */
#include "diagnosis.h"
#include "linkedlist.h"

/* M O D U L E   H E A D E R   I N C L U D E * * * * * * * * * * * * * * * * */
#include "serialhandling.h"

/* ***************************************************************************
 * D E F I N E   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* L O C A L   C O N F I G   D E F I N I T I O N S * * * * * * * * * * * * * */
#define TEMP_BUF_SIZE   (512)

/* L O C A L   M A C R O   D E F I N I T I O N S * * * * * * * * * * * * * * */

/* ***************************************************************************
 * T Y P E D E F   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* L O C A L   T Y P E D E F S * * * * * * * * * * * * * * * * * * * * * * * */
typedef struct
{
    unsigned int baudrate;
    speed_t CFG_VAL;
} BaudrateLutType;
/* ***************************************************************************
 * V A R I A B L E S   A N D   C O N S T A N T S   S E C T I O N * * * * * * *
 *************************************************************************** */

/* G L O B A L   V A R I A B L E   D E F I N I T I O N S * * * * * * * * * * */
const SerialHandleType INVALID_SERIAL_HANDLE = 0;

/* L O C A L   C O N S T A N T   D E F I N I T I O N S * * * * * * * * * * * */
static const char* MODULE_NAME = "SERH";
static const BaudrateLutType BAUDRATE_LUT[] =
{
    { .baudrate =     50, .CFG_VAL =     B50 },
    { .baudrate =     75, .CFG_VAL =     B50 },
    { .baudrate =    110, .CFG_VAL =     B50 },
    { .baudrate =    150, .CFG_VAL =    B150 },
    { .baudrate =    200, .CFG_VAL =    B200 },
    { .baudrate =    300, .CFG_VAL =    B300 },
    { .baudrate =    600, .CFG_VAL =    B600 },
    { .baudrate =   1200, .CFG_VAL =   B1200 },
    { .baudrate =   1800, .CFG_VAL =   B1800 },
    { .baudrate =   2400, .CFG_VAL =   B2400 },
    { .baudrate =   4800, .CFG_VAL =   B4800 },
    { .baudrate =   9600, .CFG_VAL =   B9600 },
    { .baudrate =  19200, .CFG_VAL =  B19200 },
    { .baudrate =  38400, .CFG_VAL =  B38400 },
    { .baudrate = 115200, .CFG_VAL = B115200 },
};


/* L O C A L   V A R I A B L E   D E F I N I T I O N S * * * * * * * * * * * */
static int mSerialFildes = -1;

/* ***************************************************************************
 * F U N C T I O N S   S E C T I O N * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* L O C A L   F U N C T I O N   P R O T O T Y P E S * * * * * * * * * * * * */

/* L O C A L   I N L I N E   F U N C T I O N   D E F I N I T I O N S * * * * */

/* L O C A L   F U N C T I O N   D E F I N I T I O N S * * * * * * * * * * * */
static int write2fildes(int         fildes,
                        const char* buf,
                        size_t      chars2write)
{
    size_t rvFwrite = write(fildes, buf, sizeof(char)*chars2write);
    DIAG_LogMsgArg(DIAG_INFO, MODULE_NAME, __func__, "write() was called to write %ld chars and wrote %ld chars", chars2write, rvFwrite);
    return (rvFwrite == chars2write) ? 0 : -1;
}

/* G L O B A L   F U N C T I O N   D E F I N I T I O N S * * * * * * * * * * */
/**
 * \brief Opens an existing pipe for writing
 */
int SERH_Open(const char* path, unsigned int baudrate, SerialHandleType* serialHandleVal)
{
    DIAG_LogMsg(DIAG_DEBUG, MODULE_NAME, __func__, "function entered");

    if (mSerialFildes >= 0)
    {
        DIAG_LogMsg(DIAG_ERROR, MODULE_NAME, __func__, "There is already one serial port open. This module can only handle one serial port at a time!");
        return -1;
    }
    int baudrateConstant = 0;
    for (size_t i=0; i<sizeof(BAUDRATE_LUT); i++)
    {
        if (baudrate == BAUDRATE_LUT[i].baudrate)
        {
            baudrateConstant = BAUDRATE_LUT[i].CFG_VAL;
        }
    }
    if (baudrateConstant == 0)
    {
        DIAG_LogMsg(DIAG_FAILURE, MODULE_NAME, __func__, "invalid baudrate specified!");
        DIAG_LogMsgArg(DIAG_INFO, MODULE_NAME, __func__, "baudrate was %d", baudrate);
        return -1;
    }

    mSerialFildes = open(path, O_RDWR | O_NOCTTY | O_NONBLOCK);

    if (mSerialFildes < 0)
    {
        DIAG_LogMsg(DIAG_FAILURE, MODULE_NAME, __func__, "unable to open file!");
        DIAG_LogMsgArg(DIAG_INFO, MODULE_NAME, __func__, "fopen() return value was < 0, strerror() is \'%s\'", strerror(errno));
        
        *serialHandleVal = INVALID_SERIAL_HANDLE;
        return -1;
    }
    *serialHandleVal = 1;

    /* set the communication parameters */
    int rv;
    struct termios commAttr;
    rv = tcgetattr(mSerialFildes, &commAttr);
    if (rv == -1)
    {
        close(mSerialFildes);
        mSerialFildes = -1;
        DIAG_LogMsg(DIAG_FAILURE, MODULE_NAME, __func__, "unable to get TTY attributes! Are you sure you specified an existing serial port?");
        return -1;
    }

    rv = cfsetispeed(&commAttr, baudrateConstant);
    if (rv == -1)
    {
        close(mSerialFildes);
        mSerialFildes = -1;
        DIAG_LogMsg(DIAG_FAILURE, MODULE_NAME, __func__, "Failed to set baudrate. The baudrate is probably not supported!");
        return -1;
    }
    rv = cfsetospeed(&commAttr, baudrateConstant);
    if (rv == -1)
    {
        close(mSerialFildes);
        mSerialFildes = -1;
        DIAG_LogMsg(DIAG_FAILURE, MODULE_NAME, __func__, "Failed to set baudrate. The baudrate is probably not supported!");
        return -1;
    }

    commAttr.c_cflag &= ~CSTOPB;
    commAttr.c_cflag &= ~PARENB;
    commAttr.c_cflag &= ~CSIZE;
    commAttr.c_cflag |= CS8;
    commAttr.c_cflag |= CLOCAL | CREAD;
    commAttr.c_cflag &= ~CRTSCTS;

    commAttr.c_lflag &= ~ICANON;
    commAttr.c_lflag &= ~ECHO;
    commAttr.c_lflag &= ~ECHOE;
    commAttr.c_lflag &= ~ISIG;

    commAttr.c_iflag &= ~IXON;
    commAttr.c_iflag &= ~IXOFF;
    commAttr.c_iflag &= ~IXANY;

    commAttr.c_oflag &= ~OPOST;

    tcsetattr(mSerialFildes, TCSAFLUSH, &commAttr);
    if (rv == -1)
    {
        close(mSerialFildes);
        mSerialFildes = -1;
        DIAG_LogMsg(DIAG_FAILURE, MODULE_NAME, __func__, "unable to set TTY attributes!");
        return -1;
    }

    return 0;
}

int SERH_Close(SerialHandleType serialHandleVal)
{
    DIAG_LogMsg(DIAG_DEBUG, MODULE_NAME, __func__, "function entered");

    if (serialHandleVal != 1)
    {
        DIAG_LogMsg(DIAG_ERROR, MODULE_NAME, __func__, "invalid serial port handle");
        return -1;
    }

    if (mSerialFildes == -1)
    {
        DIAG_LogMsg(DIAG_ERROR, MODULE_NAME, __func__, "no serial port open");
        return -1;
    }
    close(mSerialFildes);
    mSerialFildes = -1;
    return 0;
}

int SERH_GetPortList(char* buffer, size_t bufSize, char** portNames, size_t portNamesSize, size_t* portCount)
{
#if 0
    /* TODO: this code only adds a single string to the list. It is a temporary workaround for the plugin
             running on POSIX complying OSes.
             Two solutions are possible:
               1. implement the gathering of all available serial ports here
               2. fix issue #11 so that any port can be specified by the user
    */
    if ((bufSize < 26) || (portNamesSize < 2))
    {
        *portCount = 0;
        return -1;
    }
    strncpy(buffer,    "/dev/ttyUSB0", 13);
    strncpy(buffer+13, "/dev/ttyUSB1", 13);
    portNames[0] = &buffer[0];
    portNames[1] = &buffer[13];
    *portCount = 2;
    return 0;
#else
    DIR* dirp = opendir("/dev");
    if (dirp == NULL)
    {
        DIAG_LogMsg(DIAG_ERROR, MODULE_NAME, __func__, "unable to open directory /dev");
        return -1;
    }

    struct dirent* dp;
    size_t bufIndex = 0;
    while ((dp = readdir(dirp)))
    {
        if (strncmp(dp->d_name, "ttyUSB", 6) == 0)
        {
            size_t fileNameLen = strnlen(dp->d_name, 128);
            if (   (*portCount < portNamesSize)
                && ((bufIndex+fileNameLen+5) < bufSize))
            {
                strncpy(&buffer[bufIndex], "/dev/", 5);
                strncpy(&buffer[bufIndex+5], dp->d_name, fileNameLen);
                buffer[bufIndex+5+fileNameLen] = '\0';
                portNames[*portCount] = &buffer[bufIndex];
                *portCount += 1;
                bufIndex += fileNameLen + 5 + 1;
            }
            else
            {
                DIAG_LogMsg(DIAG_WARNING, MODULE_NAME, __func__, "provided buffer is too small. Not all available serial ports have been written!");
                break;
            }
        }
    }

    return 0;
#endif
}

int SERH_FlushInput(SerialHandleType serialHandleVal)
{
    if (serialHandleVal != 1)
    {
        DIAG_LogMsg(DIAG_ERROR, MODULE_NAME, __func__, "invalid serial port handle");
        return -1;
    }
    return tcflush(mSerialFildes, TCIOFLUSH);
}

/** \todo check if the return value is compliant to the requirement stated in the function documentation (see header file) */
int SERH_Read(SerialHandleType serialHandleVal,
              char* buf,
              size_t maxChars2read,
              size_t* charsRead)
{
    if (serialHandleVal != 1)
    {
        DIAG_LogMsg(DIAG_ERROR, MODULE_NAME, __func__, "invalid serial port handle");
        return -1;
    }

    /* read() returns 0 toindicate end-of-file on an empty pipe */
    /* not implemented yet */
    int rv = read(mSerialFildes, buf, maxChars2read);
    
    if (rv >= 0)
    {
        *charsRead = rv;
        return 0;
    }
    else if ((rv == -1) && (errno == EAGAIN))
    {
        *charsRead = 0;
        return 0;
    }
    else
    {
        return -1;
    }
}

int SERH_Write(SerialHandleType serialHandleVal,
               const char*    buf,
               size_t         chars2write)
{
    if (serialHandleVal != 1)
    {
        DIAG_LogMsg(DIAG_ERROR, MODULE_NAME, __func__, "invalid serial port handle");
        return -1;
    }
    return write2fildes(mSerialFildes, buf, chars2write);
}

int SERH_WriteLn(SerialHandleType serialHandleVal,
                 const char*    buf,
                 size_t         chars2write)
{
    if (serialHandleVal != 1)
    {
        DIAG_LogMsg(DIAG_ERROR, MODULE_NAME, __func__, "invalid serial port handle");
        return -1;
    }
    int rv = write2fildes(mSerialFildes, buf, chars2write);
    if (rv != 0)
    {
        return rv;
    }
    return write2fildes(mSerialFildes, "\n", 1);
}

int SERH_WriteArg(SerialHandleType serialHandleVal,
                  const char*    fmtMsg,
                                 ...)
{
    if (serialHandleVal != 1)
    {
        DIAG_LogMsg(DIAG_ERROR, MODULE_NAME, __func__, "invalid serial port handle");
        return -1;
    }

    char tempBufMsg[TEMP_BUF_SIZE];
    va_list args;
    va_start(args, fmtMsg);
    int rvVsnprintf = vsnprintf(tempBufMsg, TEMP_BUF_SIZE, fmtMsg, args);
    int bytesWritten = (rvVsnprintf < TEMP_BUF_SIZE) ? rvVsnprintf : TEMP_BUF_SIZE;
    int rv = -1;
    if (rvVsnprintf >= 0)
    {
        rv = write2fildes(mSerialFildes, tempBufMsg, bytesWritten);
    }
    va_end(args);
    return rv;
}

int SERH_WriteArgLn(SerialHandleType serialHandleVal,
                    const char*    fmtMsg,
                                   ...)
{
    if (serialHandleVal != 1)
    {
        DIAG_LogMsg(DIAG_ERROR, MODULE_NAME, __func__, "invalid serial port handle");
        return -1;
    }

    char tempBufMsg[TEMP_BUF_SIZE];
    va_list args;
    va_start(args, fmtMsg);
    int rvVsnprintf = vsnprintf(tempBufMsg, TEMP_BUF_SIZE, fmtMsg, args);
    int bytesWritten = (rvVsnprintf < TEMP_BUF_SIZE) ? rvVsnprintf : TEMP_BUF_SIZE;
    int rv = -1;
    if (rvVsnprintf >= 0)
    {
        rv = write2fildes(mSerialFildes, tempBufMsg, bytesWritten);
    }
    va_end(args);

    if (rv != 0)
    {
        return rv;
    }
    return write2fildes(mSerialFildes, "\n", 1);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* ***************************************************************************
 * E N D   O F   F I L E * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */
