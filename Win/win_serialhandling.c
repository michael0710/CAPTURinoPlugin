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
#include <stdlib.h>
#include <strsafe.h>

/* P R O J E C T   I N C L U D E S * * * * * * * * * * * * * * * * * * * * * */
#include "diagnosis.h"
#include "linkedlist.h"

/* M O D U L E   H E A D E R   I N C L U D E * * * * * * * * * * * * * * * * */
#include "serialhandling.h"

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
const SerialHandleType INVALID_SERIAL_HANDLE = 0;

/* L O C A L   C O N S T A N T   D E F I N I T I O N S * * * * * * * * * * * */
static const char* MODULE_NAME = "SERH";

/* L O C A L   V A R I A B L E   D E F I N I T I O N S * * * * * * * * * * * */
static unsigned int serialHandlesIndexCounter = 1;
static LLST_ListEntryType* serialHandlesList = NULL;

/* ***************************************************************************
 * F U N C T I O N S   S E C T I O N * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* L O C A L   F U N C T I O N   P R O T O T Y P E S * * * * * * * * * * * * */
static int vaWriteArg(      SerialHandleType serialHandleVal,
                      const char*            fmtMsg,
                            va_list          argList);

/** Writes a specified amount of data to the given HANDLE (WinAPI type).
 * 
 * \param hStream     [in] Handle that refers to the serial port that shall be
 *                         written to.
 * \param buf         [in] The array of characters that shall be written.
 * \param chars2write [in] The amount of characters to write to the serial
 *                         port.
 * 
 * \note in order to work properly, the parameter chars2write must not exceed
 *       the range of the WinAPI type DWORD.
 * 
 * \returns 0: if the write attempt succeeds.
 * \returns -1: if the write attempt fails.
 */
static int write2handle(      HANDLE hStream,
                        const char*  buf,
                              size_t chars2write);

/* L O C A L   I N L I N E   F U N C T I O N   D E F I N I T I O N S * * * * */

/* L O C A L   F U N C T I O N   D E F I N I T I O N S * * * * * * * * * * * */
static int vaWriteArg(SerialHandleType serialHandleVal, const char* fmtMsg, va_list argList)
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
        return SERH_Write(serialHandleVal, tempBufMsg, writtenBytes);
    }

    DIAG_LogMsg(DIAG_ERROR, MODULE_NAME, __func__,
        "unable to insert arguments to the format string! Logging the format string:");
    DIAG_LogMsg(DIAG_ERROR, MODULE_NAME, __func__, fmtMsg);
    return -1;
}

static int write2handle(HANDLE      hStream,
                        const char* buf,
                        size_t      chars2write)
{
    unsigned long charsWritten;
    BOOL rvWriteFile = WriteFile(hStream, buf, (DWORD)chars2write, &charsWritten, NULL);
    return rvWriteFile ? 0 : -1;
}

/* G L O B A L   F U N C T I O N   D E F I N I T I O N S * * * * * * * * * * */
/**
 * \brief Opens a serial port for writing
 */
int SERH_Open(const char* path,
              unsigned int baudrate,
              SerialHandleType* serialHandleVal)
{
    DIAG_LogMsg(DIAG_DEBUG, MODULE_NAME, __func__, "function entered");
    ULONG comPortNo = 0;
    if (sscanf_s(path, "COM%lu", &comPortNo) != 1)
    {
        DIAG_LogMsg(DIAG_FAILURE, MODULE_NAME, __func__, "invalid COM port name specified!");
        return -1;
    }

    HANDLE* newSerialHandle = (HANDLE*)malloc(sizeof(HANDLE));
    LLST_ListEntryType* newListElement = NULL;
    if (newSerialHandle == NULL)
    {
        DIAG_LogMsg(DIAG_FAILURE, MODULE_NAME, __func__, "unable to allocate memory for serial handle");
        return -1;
    }
    if (serialHandlesList == NULL)
    {
        int rvCreateList = LLST_create_elem(&serialHandlesList, (void*)newSerialHandle, serialHandlesIndexCounter);
        if (rvCreateList != 0)
        {
            free(newSerialHandle);
            DIAG_LogMsg(DIAG_FAILURE, MODULE_NAME, __func__, "unable to allocate memory for serial handle");
            return -1;
        }
    }
    else
    {
        int rvCreateElem = LLST_create_elem(&newListElement, (void*)newSerialHandle, serialHandlesIndexCounter);
        if (rvCreateElem != 0)
        {
            free(newSerialHandle);
            DIAG_LogMsg(DIAG_FAILURE, MODULE_NAME, __func__, "unable to allocate memory for serial handle");
            return -1;
        }
        int rvAddElem = LLST_add_elem(serialHandlesList, newListElement);
        if (rvAddElem != 0)
        {
            free(newSerialHandle);
            LLST_delete_elem(newListElement);
            DIAG_LogMsg(DIAG_FAILURE, MODULE_NAME, __func__, "unable to allocate memory for serial handle");
            return -1;
        }
    }
    
    /* TODO: check if this works for any serial port even with number > 10
     * if not use CreateFile with the path \\.\COMn instead */
    HANDLE winSerialHandleVal = OpenCommPort(comPortNo,
                                             GENERIC_READ | GENERIC_WRITE,
                                             0x0);

    if (winSerialHandleVal == INVALID_HANDLE_VALUE)
    {
        DIAG_LogMsg(DIAG_FAILURE, MODULE_NAME, __func__, "unable to open serial port!");
        DWORD nErrId = GetLastError();
        DIAG_LogMsgArg(DIAG_INFO, MODULE_NAME, __func__, "CreateFile() returned INVALID_HANDLE_VALUE, GetLastError() is %ld", nErrId);
        
        /** reset the dynamic serialHandlesList to the previous state */
        LLST_ListEntryType* newListStart;
        int rvRemoveElem = LLST_remove_elem_with_id(serialHandlesList, &newListStart, serialHandlesIndexCounter);
        if (rvRemoveElem == 0)
        {
            serialHandlesList = newListStart;
        }
        free(newSerialHandle);
        LLST_delete_elem(newListElement);
        return -1;
    }

    DCB serialConfig;
    serialConfig.DCBlength = sizeof(DCB);
    serialConfig.BaudRate = baudrate;
    serialConfig.fBinary = TRUE;
    serialConfig.fParity = FALSE;
    serialConfig.fOutxCtsFlow = FALSE;
    serialConfig.fOutxDsrFlow = FALSE;
    serialConfig.fDtrControl = DTR_CONTROL_DISABLE;
    serialConfig.fDsrSensitivity = FALSE;
    serialConfig.fTXContinueOnXoff = FALSE;
    serialConfig.fOutX = FALSE;
    serialConfig.fInX = FALSE;
    serialConfig.fErrorChar = FALSE;
    serialConfig.fNull = FALSE;
    serialConfig.fRtsControl = RTS_CONTROL_DISABLE;
    serialConfig.fAbortOnError = FALSE;
    serialConfig.wReserved = 0;
    serialConfig.XonLim = 0;
    serialConfig.XoffLim = 0;
    serialConfig.ByteSize = 8;
    serialConfig.Parity = NOPARITY;
    serialConfig.StopBits = ONESTOPBIT;
    serialConfig.XonChar = 0x11;
    serialConfig.XoffChar = 0x13;
    serialConfig.ErrorChar = 0x00;
    serialConfig.EofChar = 0x00;
    serialConfig.EvtChar = 0x00;

    if (SetCommState(winSerialHandleVal, &serialConfig) == 0)
    {
        DIAG_LogMsg(DIAG_FAILURE, MODULE_NAME, __func__, "unable to open serial port!");
        DIAG_LogMsg(DIAG_INFO, MODULE_NAME, __func__, "CreateFile() returned INVALID_HANDLE_VALUE");
        DWORD nErrId = GetLastError();
        DIAG_LogMsgArg(DIAG_INFO, MODULE_NAME, __func__, "GetLastError() is %ld", nErrId);
        
        /** reset the dynamic serialHandlesList to the previous state */
        LLST_ListEntryType* newListStart;
        int rvRemoveElem = LLST_remove_elem_with_id(serialHandlesList, &newListStart, serialHandlesIndexCounter);
        if (rvRemoveElem == 0)
        {
            serialHandlesList = newListStart;
        }
        free(newSerialHandle);
        LLST_delete_elem(newListElement);
        return -1;
    }

    /* configured timeouts in milliseconds */
    COMMTIMEOUTS timeouts;
    /* make the Read function for the serial port non-blocking.
       see: https://learn.microsoft.com/en-us/windows/win32/api/winbase/ns-winbase-commtimeouts
       '[The configured value] specifies that the read operation is to return
        immediately with the bytes that have already been received, even if no
        bytes have been received.' */
    timeouts.ReadIntervalTimeout = MAXDWORD; /* max time to wait between two bytes */
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.ReadTotalTimeoutConstant = 0;

    timeouts.WriteTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 0;

    if(SetCommTimeouts(winSerialHandleVal, &timeouts) == 0)
    {
        DIAG_LogMsg(DIAG_FAILURE, MODULE_NAME, __func__, "unable to open serial port!");
        DWORD nErrId = GetLastError();
        DIAG_LogMsgArg(DIAG_INFO, MODULE_NAME, __func__, "CreateFile() returned INVALID_HANDLE_VALUE, GetLastError() is %ld", nErrId);
        
        /** reset the dynamic serialHandlesList to the previous state */
        LLST_ListEntryType* newListStart;
        int rvRemoveElem = LLST_remove_elem_with_id(serialHandlesList, &newListStart, serialHandlesIndexCounter);
        if (rvRemoveElem == 0)
        {
            serialHandlesList = newListStart;
        }
        free(newSerialHandle);
        LLST_delete_elem(newListElement);
        return -1;
    }

    DIAG_LogMsg(DIAG_INFO, MODULE_NAME, __func__, "Serial port opened successfully");
    
    *newSerialHandle = winSerialHandleVal;
    *serialHandleVal = (SerialHandleType)serialHandlesIndexCounter;
    serialHandlesIndexCounter++;
    
    return 0;
}

/**
 * \brief Closes a serial port
 */
int SERH_Close(SerialHandleType serialHandleVal)
{
    LLST_ListEntryType* elem;
    int rvGetElem = LLST_get_elem_with_id(serialHandlesList, &elem, (unsigned int)serialHandleVal);
    if (rvGetElem != 0)
    {
        DIAG_LogMsg(DIAG_ERROR, MODULE_NAME, __func__, "invalid serial port handle");
        return -1;
    }
    
    DIAG_LogMsg(DIAG_INFO, MODULE_NAME, __func__, "closing serial port");
    CloseHandle(*((HANDLE*)(elem->data)));
    free((HANDLE*)(elem->data));
    LLST_ListEntryType* newStart;
    int rvRemoveElem = LLST_remove_elem_with_id(serialHandlesList, &newStart, (unsigned int)serialHandleVal);
    if (rvRemoveElem == 0)
    {
        serialHandlesList = newStart;
    }
    LLST_delete_elem(elem);
    
    return 0;
}

int SERH_GetPortList(char* buffer,
                     size_t bufSize,
                     char** portNames,
                     size_t portNamesSize,
                     size_t* portCount)
{
    ULONG portsFound = 0;
    ULONG portNoArray[16];
    ULONG maxPortNos = (portNamesSize > 16) ? 16 : (ULONG)portNamesSize;
    ULONG rvGetCommPorts = GetCommPorts(portNoArray, maxPortNos, &portsFound);
    size_t bufIndex = 0;
    if ((rvGetCommPorts == ERROR_SUCCESS) || (rvGetCommPorts == ERROR_MORE_DATA))
    {
        *portCount = 0;

        for (size_t i=0; i<portsFound; i++)
        {
            HRESULT rvPrintf = StringCbPrintfA(&buffer[bufIndex], bufSize - bufIndex, "COM%lu", portNoArray[i]);
            if (rvPrintf == S_OK)
            {
                *portCount += 1;
                portNames[i] = &buffer[bufIndex];
                bufIndex += strnlen(&buffer[bufIndex], bufSize - bufIndex) + 1;
            }
            else
            {
                DIAG_LogMsgArg(DIAG_ERROR, MODULE_NAME, __func__, "unable to write port name \'COM%lu\' to buffer", portNoArray[i]);
                return 0;
            }
        }
        return 0;
    }
    return -1;
}

int SERH_FlushInput(SerialHandleType serialHandleVal)
{
    LLST_ListEntryType* elem;
    int rvGetElem = LLST_get_elem_with_id(serialHandlesList, &elem, (unsigned int)serialHandleVal);
    if (rvGetElem != 0)
    {
        DIAG_LogMsg(DIAG_ERROR, MODULE_NAME, __func__, "invalid serial port handle");
        return -1;
    }
    HANDLE comHandle = *((HANDLE*)(elem->data));
    /* TODO: check if PURGE_TXCLEAR is necessary for a function that is called FlushInput */
    BOOL rv = PurgeComm(comHandle, PURGE_RXCLEAR | PURGE_TXCLEAR);
    return rv ? 0 : -1;
}

int SERH_Read(SerialHandleType serialHandleVal,
              char* buf,
              size_t maxChars2read,
              size_t* charsRead)
{
    //DIAG_LogMsg(DIAG_DEBUG, MODULE_NAME, __func__, "function entered");
    LLST_ListEntryType* elem;
    int rvGetElem = LLST_get_elem_with_id(serialHandlesList, &elem, (unsigned int)serialHandleVal);
    if (rvGetElem != 0)
    {
        DIAG_LogMsg(DIAG_ERROR, MODULE_NAME, __func__, "invalid serial port handle");
        return -1;
    }
    HANDLE comHandle = *((HANDLE*)(elem->data));
    BOOL rvRead = ReadFile(comHandle, buf, (DWORD)maxChars2read, (DWORD*)charsRead, NULL);
    if ((rvRead == FALSE) && (GetLastError() == ERROR_MORE_DATA))
    {
        DIAG_LogMsg(DIAG_DEBUG, MODULE_NAME, __func__, "Serial buffer has contains more data than requested");
        return 0;
    }
    return (rvRead == TRUE) ? 0 : -1;
}

int SERH_Write(SerialHandleType serialHandleVal,
               const char*    buf,
               size_t         chars2write)
{
    DIAG_LogMsg(DIAG_DEBUG, MODULE_NAME, __func__, "function entered");
    LLST_ListEntryType* elem;
    int rvGetElem = LLST_get_elem_with_id(serialHandlesList, &elem, (unsigned int)serialHandleVal);
    if (rvGetElem != 0)
    {
        DIAG_LogMsg(DIAG_ERROR, MODULE_NAME, __func__, "invalid serial port handle");
        return -1;
    }
    return write2handle(*((HANDLE*)(elem->data)), buf, chars2write);
}

int SERH_WriteLn(SerialHandleType serialHandleVal,
                 const char*    buf,
                 size_t         chars2write)
{
    LLST_ListEntryType* elem;
    int rvGetElem = LLST_get_elem_with_id(serialHandlesList, &elem, (unsigned int)serialHandleVal);
    if (rvGetElem != 0)
    {
        DIAG_LogMsg(DIAG_ERROR, MODULE_NAME, __func__, "invalid serial port handle");
        return -1;
    }
    
    int rv = write2handle(*((HANDLE*)(elem->data)), buf, chars2write);
    if (rv == 0)
    {
        rv = write2handle(*((HANDLE*)(elem->data)), "\r\n", 2);
    }
    return rv;
}

int SERH_WriteArg(SerialHandleType serialHandleVal,
                  const char*    fmtMsg,
                                 ...)
{
    va_list args;
    va_start(args, fmtMsg);
    int rvVaWriteArg = vaWriteArg(serialHandleVal, fmtMsg, args);
    va_end(args);
    return rvVaWriteArg;
}

int SERH_WriteArgLn(SerialHandleType serialHandleVal,
                    const char*    fmtMsg,
                                   ...)
{
    va_list args;
    va_start(args, fmtMsg);
    int rvVaWriteArg = vaWriteArg(serialHandleVal, fmtMsg, args);
    va_end(args);
    if (rvVaWriteArg == 0)
    {
        return SERH_Write(serialHandleVal, "\r\n", 2);
    }
    return rvVaWriteArg;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* ***************************************************************************
 * E N D   O F   F I L E * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */
