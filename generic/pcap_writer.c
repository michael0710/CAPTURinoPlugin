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
#include <stdint.h>

/* P R O J E C T   I N C L U D E S * * * * * * * * * * * * * * * * * * * * * */
#include "diagnosis.h"
#include "pipehandling.h"
#include "systemutils.h"

/* M O D U L E   H E A D E R   I N C L U D E * * * * * * * * * * * * * * * * */
#include "pcap_writer.h"

/* ***************************************************************************
 * D E F I N E   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* L O C A L   C O N F I G   D E F I N I T I O N S * * * * * * * * * * * * * */
#define PCAP_MAX_SNAP_LENGTH 65535

/* L O C A L   M A C R O   D E F I N I T I O N S * * * * * * * * * * * * * * */

/* ***************************************************************************
 * T Y P E D E F   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* ***************************************************************************
 * V A R I A B L E S   A N D   C O N S T A N T S   S E C T I O N * * * * * * *
 *************************************************************************** */

/* G L O B A L   V A R I A B L E   D E F I N I T I O N S * * * * * * * * * * */
static uint32_t mCurrentSnapLength = 0;

/* L O C A L   C O N S T A N T   D E F I N I T I O N S * * * * * * * * * * * */
static const char* MODULE_NAME = "PCAP";

/* ***************************************************************************
 * F U N C T I O N S   S E C T I O N * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* L O C A L   F U N C T I O N   P R O T O T Y P E S * * * * * * * * * * * * */

/* L O C A L   I N L I N E   F U N C T I O N   D E F I N I T I O N S * * * * */

/* L O C A L   F U N C T I O N   D E F I N I T I O N S * * * * * * * * * * * */

/* G L O B A L   F U N C T I O N   D E F I N I T I O N S * * * * * * * * * * */
int PCAP_WriteHeader(PipeHandleType hFile,
                     bool timestampNanos,
                     uint32_t snapLength,
                     PCAP_ValidLinkTypesType linkType,
                     uint8_t pFlag,
                     uint8_t rFlag,
                     uint16_t fcsLength)
{
    unsigned char pcapHeader[24] = {0};
    /* assign magic number */
    if (timestampNanos)
    {
        /* magic number stating that the timestamp has nanoseconds precision */
        (*(uint32_t*)&pcapHeader[0]) = 0xa1b23c4d;
    }
    else
    {
        /* magic number stating that the timestamp has microseconds precision */
        (*(uint32_t*)&pcapHeader[0]) = 0xa1b2c3d4;
    }
    

    /* assign version number */
    (*(uint16_t*)&pcapHeader[4]) = 2; /* major version */
    (*(uint16_t*)&pcapHeader[6]) = 4; /* minor version */

    /* reserved1 field should be filled with 0 */
    (*(uint32_t*)&pcapHeader[8]) = 0;

    /* reserved2 field should be filled with 0 */
    (*(uint32_t*)&pcapHeader[12]) = 0;

    /* snap length */
    (*(uint32_t*)&pcapHeader[16]) = snapLength;
    mCurrentSnapLength = snapLength;

    DIAG_LogMsgArg(DIAG_DEBUG, MODULE_NAME, __func__, "snap length set to %u", snapLength);

    /* link type, P- and R-flag and fcs length */
    (*(uint32_t*)&pcapHeader[20]) = 0;
    (*(uint32_t*)&pcapHeader[20]) |= (0x0000000F & (uint32_t)fcsLength) << 28;
    (*(uint32_t*)&pcapHeader[20]) |= (0x00000001 & (uint32_t)rFlag) << 27;
    (*(uint32_t*)&pcapHeader[20]) |= (0x00000001 & (uint32_t)pFlag) << 26;
    /* NOTE: bits 16-25 are reserved */
    (*(uint32_t*)&pcapHeader[20]) |= (0x0000FFFF & (uint32_t)linkType);

    return PIPH_Write(hFile, (void*)pcapHeader, 24);
}

int PCAP_FillPacketRecordHeader(const PCAP_PacketRecordHeaderType* packetRecordHeader,
                                void* packetRecord)
{
    /* assign timestamp seconds */
    (((uint32_t*)packetRecord)[0]) = packetRecordHeader->timestampSeconds;

    /* assign timestamp nanoseconds */
    (((uint32_t*)packetRecord)[1]) = packetRecordHeader->timestampMicrosOrNanos;

    /* assign packet length */
    /* truncate the packet if the specified packetLength exceeds snapLength */
    uint32_t packetLength = (packetRecordHeader->protocolPayloadLength > mCurrentSnapLength)
                                ? mCurrentSnapLength : packetRecordHeader->protocolPayloadLength;
    (((uint32_t*)packetRecord)[2]) = packetLength;
    /* write original packet length */
    (((uint32_t*)packetRecord)[3]) = packetRecordHeader->protocolPayloadLength;
    return 0;
}

int PCAP_WritePacketRecord(PipeHandleType hFile,
                           void* packetData)
{
    uint32_t snapLength = ((uint32_t*)packetData)[2];
    return PIPH_Write(hFile, (void*)packetData, snapLength + 16);
}