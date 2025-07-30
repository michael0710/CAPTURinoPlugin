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
#include <windows.h>
#include <handleapi.h>
#include <processthreadsapi.h>
#include <stdlib.h>
#include <strsafe.h>
#include <synchapi.h>

/* P R O J E C T   I N C L U D E S * * * * * * * * * * * * * * * * * * * * * */
#include "diagnosis.h"
#include "linkedlist.h"
#include "ringbuf.h"

/* M O D U L E   H E A D E R   I N C L U D E * * * * * * * * * * * * * * * * */
#include "pipehandling.h"

/* ***************************************************************************
 * D E F I N E   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* L O C A L   C O N F I G   D E F I N I T I O N S * * * * * * * * * * * * * */
#define TEMP_BUF_SIZE (1024)
#define MAX_FRAME_SIZE_TO_WRITE (4096)

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
static HANDLE mPipeHandle = INVALID_HANDLE_VALUE;
static HANDLE mWriteTaskHandle;
static HANDLE mStopEvent;

static size_t m_framesizememory[8192];
static char m_buffermemory[65536];

static volatile RingBufType mFrameSizeRingBuffer = 
{
    .head = 0,
    .tail = 0,
    .buffer = m_framesizememory,
    .bufferSize = 8192,
    .elementSize = sizeof(size_t)
};
static volatile RingBufType mDataRingBuffer =
{
    .head = 0,
    .tail = 0,
    .buffer = m_buffermemory,
    .bufferSize = 65536,
    .elementSize = 1
};

/* ***************************************************************************
 * F U N C T I O N S   S E C T I O N * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* L O C A L   F U N C T I O N   P R O T O T Y P E S * * * * * * * * * * * * */
static int vaWriteArg(      PipeHandleType pipeHandleVal,
                      const char*          fmtMsg,
                            va_list        argList);

/** Writes a specified amount of data to the given HANDLE (WinAPI type).
 * 
 * \param buf         [in] The array of characters that shall be written.
 * \param chars2write [in] The amount of characters to write to the pipe.
 * 
 * \note in order to work properly, the parameter chars2write must not exceed
 *       the range of the WinAPI type DWORD.
 * 
 * \returns 0: if the write attempt succeeds.
 * \returns -1: if the write attempt fails.
 */
static int write2handle(const char*  buf,
                              size_t chars2write);

/* L O C A L   I N L I N E   F U N C T I O N   D E F I N I T I O N S * * * * */

/* L O C A L   F U N C T I O N   D E F I N I T I O N S * * * * * * * * * * * */
static DWORD WINAPI WriteTask(LPVOID lpParam)
{
    while (WaitForSingleObject(mStopEvent, 0) != WAIT_OBJECT_0)
    {
        if (RingBuf_isEmpty(&mFrameSizeRingBuffer) == FALSE)
        {
            DWORD bytesWritten;
            size_t frameSize = *((size_t*)RingBuf_getTail(&mFrameSizeRingBuffer));
            if (RingBuf_getElementsCount(&mDataRingBuffer) < frameSize)
            {
                continue;
            }
            size_t bytes2end = RingBuf_getFullElementsTail2End(&mDataRingBuffer);
            if (bytes2end >= frameSize)
            {
                WriteFile(mPipeHandle, RingBuf_getTail(&mDataRingBuffer), frameSize, &bytesWritten, NULL);
                RingBuf_increaseTailMore(&mDataRingBuffer, (size_t)frameSize);
            }
            else /* this means the frame wraps over the end of the ring buffer */
            {
                char tempBuf[MAX_FRAME_SIZE_TO_WRITE];
                memcpy(tempBuf, RingBuf_getTail(&mDataRingBuffer), bytes2end);
                RingBuf_increaseTailMore(&mDataRingBuffer, bytes2end);
                memcpy(tempBuf + bytes2end, RingBuf_getTail(&mDataRingBuffer), frameSize - bytes2end);
                RingBuf_increaseTailMore(&mDataRingBuffer, frameSize - bytes2end);
                WriteFile(mPipeHandle, tempBuf, frameSize, &bytesWritten, NULL);
            }
            RingBuf_increaseTail(&mFrameSizeRingBuffer);
        }
        else
        {
            Sleep(10);
        }
    }

    return 0;
}

static int vaWriteArg(PipeHandleType pipeHandleVal, const char* fmtMsg, va_list argList)
{
    char tempBufMsg[TEMP_BUF_SIZE];

    HRESULT printfRetVal = StringCbVPrintf(tempBufMsg,
                                           TEMP_BUF_SIZE,
                                           fmtMsg,
                                           argList);

    size_t writtenBytes;
    HRESULT rvStrLen = StringCbLength(tempBufMsg, TEMP_BUF_SIZE, &writtenBytes);

    if ((printfRetVal == S_OK) && (rvStrLen == S_OK))
    {
        return PIPH_Write(pipeHandleVal, tempBufMsg, writtenBytes);
    }

    DIAG_LogMsg(DIAG_ERROR, MODULE_NAME, __func__,
        "unable to insert arguments to the format string! Logging the format string:");
    DIAG_LogMsg(DIAG_ERROR, MODULE_NAME, __func__, fmtMsg);
    return -1;
}

static int write2handle(const char* buf,
                        size_t      chars2write)
{
    if (chars2write >= 65536)
    {
        return -1;
    }
    size_t freeBytesAtOnce = RingBuf_getFreeElementsHead2End(&mDataRingBuffer);
    if (freeBytesAtOnce >= chars2write)
    {
        DIAG_LogMsgArg(DIAG_DEBUG, MODULE_NAME, __func__, "writing %d bytes buffer all at once", chars2write);
        DIAG_LogMsgArg(DIAG_DEBUG, MODULE_NAME, __func__, "bufSize: %d, head: %d, tail: %d", mDataRingBuffer.bufferSize, mDataRingBuffer.head, mDataRingBuffer.tail);
        memcpy(RingBuf_getHead(&mDataRingBuffer), buf, chars2write);
        RingBuf_increaseHeadMore(&mDataRingBuffer, chars2write);
    }
    else
    {
        DIAG_LogMsgArg(DIAG_DEBUG, MODULE_NAME, __func__, "writing to buffer in two steps: %d at first, %d afterwards", freeBytesAtOnce, chars2write - freeBytesAtOnce);
        DIAG_LogMsgArg(DIAG_DEBUG, MODULE_NAME, __func__, "bufSize: %d, head: %d, tail: %d", mDataRingBuffer.bufferSize, mDataRingBuffer.head, mDataRingBuffer.tail);
        memcpy(RingBuf_getHead(&mDataRingBuffer), buf, freeBytesAtOnce);
        RingBuf_increaseHeadMore(&mDataRingBuffer, freeBytesAtOnce);
        memcpy(RingBuf_getHead(&mDataRingBuffer), buf +  freeBytesAtOnce, chars2write - freeBytesAtOnce);
        RingBuf_increaseHeadMore(&mDataRingBuffer, chars2write - freeBytesAtOnce);
    }
    *((size_t*)RingBuf_getHead(&mFrameSizeRingBuffer)) = chars2write;
    RingBuf_increaseHead(&mFrameSizeRingBuffer);
    return 0;
}

/* G L O B A L   F U N C T I O N   D E F I N I T I O N S * * * * * * * * * * */
/**
 * \brief Opens an existing pipe for writing
 */
int PIPH_Open(const char* path, PipeHandleType* pipeHandleVal)
{
    DIAG_LogMsg(DIAG_DEBUG, MODULE_NAME, __func__, "function entered");

    if (mPipeHandle != INVALID_HANDLE_VALUE)
    {
        DIAG_LogMsg(DIAG_ERROR, MODULE_NAME, __func__, "There is already one pipe open. This module can only handle one pipe at a time!");
        return -1;
    }
    
    mPipeHandle = CreateFile(path,
                             GENERIC_WRITE,
                             FILE_SHARE_DELETE,
                             NULL,
                             OPEN_EXISTING,
                             0,
                             NULL);

    if (mPipeHandle == INVALID_HANDLE_VALUE)
    {
        DIAG_LogMsg(DIAG_FAILURE, MODULE_NAME, __func__, "unable to open pipe!");
        DWORD nErrId = GetLastError();
        DIAG_LogMsgArg(DIAG_INFO, MODULE_NAME, __func__, "CreateFile() returned INVALID_HANDLE_VALUE, GetLastError() is %ld", nErrId);
        return -1;
    }
    
    mFrameSizeRingBuffer.head = 0;
    mFrameSizeRingBuffer.tail = 0;
    mDataRingBuffer.head = 0;
    mDataRingBuffer.tail = 0;

    mStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (mStopEvent == NULL)
    {
        DIAG_LogMsg(DIAG_FAILURE, MODULE_NAME, __func__, "unable to create event for write task!");
        return -1;
    }

    mWriteTaskHandle = CreateThread(NULL, 0, WriteTask, NULL, 0, NULL);
    if (mWriteTaskHandle == NULL)
    {
        DIAG_LogMsg(DIAG_FAILURE, MODULE_NAME, __func__, "unable to create write task!");
        CloseHandle(mStopEvent);
    }

    *pipeHandleVal = 1;

    return 0;
}

/**
 * \brief Closes a pipe
 */
int PIPH_Close(PipeHandleType pipeHandleVal)
{
    if (pipeHandleVal != 1)
    {
        DIAG_LogMsg(DIAG_ERROR, MODULE_NAME, __func__, "invalid pipe handle");
        return -1;
    }

    /* stop the pipe writing thread */
    SetEvent(mStopEvent);
    WaitForSingleObject(mWriteTaskHandle, INFINITE);
    CloseHandle(mWriteTaskHandle);
    CloseHandle(mStopEvent);
    
    DIAG_LogMsg(DIAG_INFO, MODULE_NAME, __func__, "closing pipe");
    CloseHandle(mPipeHandle);
    
    mPipeHandle = INVALID_HANDLE_VALUE;

    return 0;
}

int PIPH_Write(PipeHandleType pipeHandleVal,
               const char*    buf,
               size_t         chars2write)
{
    if (pipeHandleVal != 1)
    {
        DIAG_LogMsg(DIAG_ERROR, MODULE_NAME, __func__, "invalid pipe handle");
        return -1;
    }
    return write2handle(buf, chars2write);
}

int PIPH_WriteLn(PipeHandleType pipeHandleVal,
                 const char*    buf,
                 size_t         chars2write)
{
    if (pipeHandleVal != 1)
    {
        DIAG_LogMsg(DIAG_ERROR, MODULE_NAME, __func__, "invalid pipe handle");
        return -1;
    }
    
    int rv = write2handle(buf, chars2write);
    if (rv == 0)
    {
        rv = write2handle("\r\n", 2);
    }
    return rv;
}

int PIPH_WriteArg(PipeHandleType pipeHandleVal,
                  const char*    fmtMsg,
                                 ...)
{
    va_list args;
    va_start(args, fmtMsg);
    int rvVaWriteArg = vaWriteArg(pipeHandleVal, fmtMsg, args);
    va_end(args);
    return rvVaWriteArg;
}

int PIPH_WriteArgLn(PipeHandleType pipeHandleVal,
                    const char*    fmtMsg,
                                   ...)
{
    va_list args;
    va_start(args, fmtMsg);
    int rvVaWriteArg = vaWriteArg(pipeHandleVal, fmtMsg, args);
    va_end(args);
    if (rvVaWriteArg == 0)
    {
        return PIPH_Write(pipeHandleVal, "\r\n", 2);
    }
    return rvVaWriteArg;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* ***************************************************************************
 * E N D   O F   F I L E * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */
