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
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

/* P R O J E C T   I N C L U D E S * * * * * * * * * * * * * * * * * * * * * */
#include "diagnosis.h"
#include "linkedlist.h"

/* M O D U L E   H E A D E R   I N C L U D E * * * * * * * * * * * * * * * * */
#include "posix_internal_filehandling.h"

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
const FileHandleType INVALID_FILE_HANDLE = 0;

/* L O C A L   C O N S T A N T   D E F I N I T I O N S * * * * * * * * * * * */
static const char* MODULE_NAME = "FILH";

/* L O C A L   V A R I A B L E   D E F I N I T I O N S * * * * * * * * * * * */
static unsigned int fileHandlesIndexCounter = 1;
static LLST_ListEntryType* fileHandlesList = NULL;

/* ***************************************************************************
 * F U N C T I O N S   S E C T I O N * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* L O C A L   F U N C T I O N   P R O T O T Y P E S * * * * * * * * * * * * */
/** Writes a specified amount of data to the given HANDLE (WinAPI type).
 * 
 * \param hStream     [in] Handle that refers to the pipe that shall be written
 *                         to.
 * \param buf         [in] The array of characters that shall be written.
 * \param chars2write [in] The amount of characters to write to the pipe.
 * 
 * \note in order to work properly, the parameter chars2write must not exceed
 *       the range of the WinAPI type DWORD.
 * 
 * \returns 0: if the write attempt succeeds.
 * \returns -1: if the write attempt fails.
 */
static int write2fildes(      int    fildes,
                        const char*  buf,
                              size_t chars2write);

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
int FILH_Open(const char* restrict path, int oflag, FileHandleType* restrict fileHandleVal)
{
    DIAG_LogMsg(DIAG_DEBUG, MODULE_NAME, __func__, "function entered");
    
    int* newFiledes = (int*)malloc(sizeof(int));
    LLST_ListEntryType* newListElement = NULL;
    if (newFiledes == NULL)
    {
        DIAG_LogMsg(DIAG_FAILURE, MODULE_NAME, __func__, "unable to allocate memory for file pointer");
        return -1;
    }

    int rvCreateElem = LLST_create_elem(&newListElement, (void*)newFiledes, fileHandlesIndexCounter);
    if (rvCreateElem != 0)
    {
        free(newFiledes);
        DIAG_LogMsg(DIAG_FAILURE, MODULE_NAME, __func__, "unable to allocate memory for list element");
        return -1;
    }

    if (fileHandlesList == NULL)
    {
        fileHandlesList = newListElement;
    }
    else
    {
        int rvAddElem = LLST_add_elem(fileHandlesList, newListElement);
        if (rvAddElem != 0)
        {
            free(newFiledes);
            LLST_delete_elem(newListElement);
            DIAG_LogMsg(DIAG_FAILURE, MODULE_NAME, __func__, "unable to add list element to the existing list");
            return -1;
        }
    }
    
    int posixFiledes = open(path, oflag);

    if (posixFiledes < 0)
    {
        DIAG_LogMsg(DIAG_FAILURE, MODULE_NAME, __func__, "unable to open file!");
        DIAG_LogMsgArg(DIAG_INFO, MODULE_NAME, __func__, "fopen() return value was < 0, strerror() is \'%s\'", strerror(errno));
        
        /** reset the dynamic pipeHandlesList to the previous state */
        LLST_ListEntryType* newListStart;
        int rvRemoveElem = LLST_remove_elem_with_id(fileHandlesList, &newListStart, fileHandlesIndexCounter);
        if (rvRemoveElem == 0)
        {
            fileHandlesList = newListStart; 
        }
        free(newFiledes);
        LLST_delete_elem(newListElement);
        return -1;
    }
    
    *newFiledes = posixFiledes;
    *fileHandleVal = (FileHandleType)fileHandlesIndexCounter;
    fileHandlesIndexCounter++;
    
    return 0;
}

int FILH_Close(FileHandleType fileHandleVal)
{
    LLST_ListEntryType* elem;
    int rvGetElem = LLST_get_elem_with_id(fileHandlesList, &elem, (unsigned int)fileHandleVal);
    if (rvGetElem != 0)
    {
        DIAG_LogMsg(DIAG_ERROR, MODULE_NAME, __func__, "invalid file handle");
        return -1;
    }
    
    DIAG_LogMsg(DIAG_INFO, MODULE_NAME, __func__, "closing file");
    close(*((int*)(elem->data)));
    free(elem->data);

    LLST_ListEntryType* newStart;
    int rvRemoveElem = LLST_remove_elem_with_id(fileHandlesList, &newStart, (unsigned int)fileHandleVal);
    if (rvRemoveElem == 0)
    {
        fileHandlesList = newStart;
    }
    LLST_delete_elem(elem);
    
    return 0;
}

int FILH_Read(FileHandleType fileHandleVal,
              char* buf,
              size_t chars2read,
              size_t* charsRead)
{
    LLST_ListEntryType* elem;
    int rvGetElem = LLST_get_elem_with_id(fileHandlesList, &elem, (unsigned int)fileHandleVal);
    if (rvGetElem != 0)
    {
        DIAG_LogMsg(DIAG_ERROR, MODULE_NAME, __func__, "invalid file handle");
        return -1;
    }
    /* read() returns 0 toindicate end-of-file on an empty pipe */
    /* not implemented yet */
    int rv = read(*((int*)(elem->data)), buf, chars2read);
    
    if (rv >= 0)
    {
        *charsRead = rv;
        rv = 0;
    }
    return rv;
}

int FILH_Write(FileHandleType fileHandleVal,
               const char*    buf,
               size_t         chars2write)
{
    LLST_ListEntryType* elem;
    int rvGetElem = LLST_get_elem_with_id(fileHandlesList, &elem, (unsigned int)fileHandleVal);
    if (rvGetElem != 0)
    {
        DIAG_LogMsg(DIAG_ERROR, MODULE_NAME, __func__, "invalid file handle");
        return -1;
    }
    return write2fildes(*((int*)(elem->data)), buf, chars2write);
}

int FILH_WriteLn(FileHandleType fileHandleVal,
                 const char*    buf,
                 size_t         chars2write)
{
    int rv = FILH_Write(fileHandleVal, buf, chars2write);
    if (rv == 0)
    {
        rv = FILH_Write(fileHandleVal, "\n", 1);
    }
    return rv;
}

int FILH_WriteArg(FileHandleType fileHandleVal,
                  const char*    fmtMsg,
                                 ...)
{
    va_list args;
    va_start(args, fmtMsg);
    int rvVaWriteArg = FILH_vaWriteArg(fileHandleVal, fmtMsg, args);
    va_end(args);
    return rvVaWriteArg;
}

int FILH_WriteArgLn(FileHandleType fileHandleVal,
                    const char*    fmtMsg,
                                   ...)
{
    va_list args;
    va_start(args, fmtMsg);
    int rvVaWriteArg = FILH_vaWriteArgLn(fileHandleVal, fmtMsg, args);
    va_end(args);
    return rvVaWriteArg;
}

/* TODO: actually having a public vaWriteArg and a private one is nonsense... */
int FILH_vaWriteArg(FileHandleType fileHandleVal,
                    const char*    fmtMsg,
                    va_list        args)
{
    char tempBufMsg[TEMP_BUF_SIZE];

    int rvVsnprintf = vsnprintf(tempBufMsg, TEMP_BUF_SIZE, fmtMsg, args);
    int bytesWritten = (rvVsnprintf < TEMP_BUF_SIZE) ? rvVsnprintf : TEMP_BUF_SIZE;
    if (rvVsnprintf >= 0)
    {
        return FILH_Write(fileHandleVal, tempBufMsg, bytesWritten);
    }

    DIAG_LogMsg(DIAG_ERROR, MODULE_NAME, __func__,
        "unable to insert arguments to the format string! Logging the format string:");
    DIAG_LogMsg(DIAG_ERROR, MODULE_NAME, __func__, fmtMsg);
    return -1;
}

int FILH_vaWriteArgLn(FileHandleType fileHandleVal,
                      const char*    fmtMsg,
                      va_list        args)
{
    int rvVaWriteArg = FILH_vaWriteArg(fileHandleVal, fmtMsg, args);
    if (rvVaWriteArg == 0)
    {
        return FILH_Write(fileHandleVal, "\n", 1);
    }
    return rvVaWriteArg;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* ***************************************************************************
 * E N D   O F   F I L E * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */
