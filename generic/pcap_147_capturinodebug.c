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
#include <stdint.h>
#include <string.h>

/* P R O J E C T   I N C L U D E S * * * * * * * * * * * * * * * * * * * * * */
#include "diagnosis.h"
#include "pipehandling.h"
#include "pcap_writer.h"
#include "systemutils.h"

/* M O D U L E   H E A D E R   I N C L U D E * * * * * * * * * * * * * * * * */
#include "pcap_147_capturinodebug.h"

/* ***************************************************************************
 * D E F I N E   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* L O C A L   C O N F I G   D E F I N I T I O N S * * * * * * * * * * * * * */
#define PCAP_CAPTURINODEBUG_MAX_DEBUG_MSG_LENGTH 256

/* L O C A L   M A C R O   D E F I N I T I O N S * * * * * * * * * * * * * * */

/* ***************************************************************************
 * T Y P E D E F   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* ***************************************************************************
 * V A R I A B L E S   A N D   C O N S T A N T S   S E C T I O N * * * * * * *
 *************************************************************************** */

/* G L O B A L   V A R I A B L E   D E F I N I T I O N S * * * * * * * * * * */

/* L O C A L   C O N S T A N T   D E F I N I T I O N S * * * * * * * * * * * */
static const char* MODULE_NAME = "PCAP_147";

/* ***************************************************************************
 * F U N C T I O N S   S E C T I O N * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* L O C A L   F U N C T I O N   P R O T O T Y P E S * * * * * * * * * * * * */

/* L O C A L   I N L I N E   F U N C T I O N   D E F I N I T I O N S * * * * */

/* L O C A L   F U N C T I O N   D E F I N I T I O N S * * * * * * * * * * * */

/* G L O B A L   F U N C T I O N   D E F I N I T I O N S * * * * * * * * * * */
int PCAP_147_CapturinoDebug_writeDebugMsg(PipeHandleType pipe,
                                          const char* debugMsg)
{
    unsigned long long unixTime;
    unsigned long micros;

    int rv = SYSU_GetCurrentTime(&unixTime, &micros);

    PCAP_PacketRecordHeaderType recordHeader;
    recordHeader.timestampSeconds = (uint32_t)unixTime;
    recordHeader.timestampMicrosOrNanos = micros;
    
    PCAP_LinktypeReserved01_CapturinoDebugType debugFrame;
    debugFrame.reserved01_CapturinoDebugFrame.state = 0x0;

    debugFrame.reserved01_CapturinoDebugFrame.debugMsgLength = strnlen(debugMsg, 256) + 1;
    debugFrame.reserved01_CapturinoDebugFrame.debugMsg = debugMsg;

    return PCAP_147_CapturinoDebug_WriteRecord(pipe,
                                               recordHeader,
                                               debugFrame);
}

/* TODO: check if somehow libpcap can be used */
int PCAP_147_CapturinoDebug_WriteRecord(PipeHandleType hFile,
                                        PCAP_PacketRecordHeaderType packetRecordHeader,
                                        PCAP_LinktypeReserved01_CapturinoDebugType packetRecord)
{
    uint8_t buffer[PCAP_CAPTURINODEBUG_MAX_DEBUG_MSG_LENGTH] = {0};

    /* assign StatusDWORD */
    (*(uint32_t*)&buffer[0]) = packetRecord.reserved01_CapturinoDebugFrame.state;

    /* assign debug message length */
    (*(uint32_t*)&buffer[4]) = packetRecord.reserved01_CapturinoDebugFrame.debugMsgLength;

    /* assign debug message */
    for (size_t i = 0; i < packetRecord.reserved01_CapturinoDebugFrame.debugMsgLength; i++)
    {
        if ((8+i) >= PCAP_CAPTURINODEBUG_MAX_DEBUG_MSG_LENGTH)
        {
            DIAG_LogMsg(DIAG_WARNING, MODULE_NAME, __func__, "debug message too long!");
            return -1;
        }
        buffer[8+i] = packetRecord.reserved01_CapturinoDebugFrame.debugMsg[i];
    }

    packetRecordHeader.protocolPayloadLength = 8+packetRecord.reserved01_CapturinoDebugFrame.debugMsgLength;

    /* NOTE: debug message frame adds a 8 byte header to the message length */
    //return PCAP_WritePacketRecord(hFile, packetRecordHeader, buffer);
    return -1; // ONLY FOR DEBUGGING!
}
