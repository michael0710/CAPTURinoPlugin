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
#include "capturinocommonintfcfuncs.h"
#include "pcap_writer.h"
#include "ringbuf.h"

/* M O D U L E   H E A D E R   I N C L U D E * * * * * * * * * * * * * * * * */
#include "capturino2pcapadptr.h"

/* ***************************************************************************
 * D E F I N E   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* L O C A L   C O N F I G   D E F I N I T I O N S * * * * * * * * * * * * * */

/* L O C A L   M A C R O   D E F I N I T I O N S * * * * * * * * * * * * * * */

/* ***************************************************************************
 * T Y P E D E F   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* ***************************************************************************
 * V A R I A B L E S   A N D   C O N S T A N T S   S E C T I O N * * * * * * *
 *************************************************************************** */

/* G L O B A L   V A R I A B L E   D E F I N I T I O N S * * * * * * * * * * */

/* L O C A L   V A R I A B L E   D E F I N I T I O N S * * * * * * * * * * */

/* L O C A L   C O N S T A N T   D E F I N I T I O N S * * * * * * * * * * * */
static const char* MODULE_NAME = "CAPT_ADPR";

/* ***************************************************************************
 * F U N C T I O N S   S E C T I O N * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* L O C A L   F U N C T I O N   P R O T O T Y P E S * * * * * * * * * * * * */

/* L O C A L   I N L I N E   F U N C T I O N   D E F I N I T I O N S * * * * */

/* L O C A L   F U N C T I O N   D E F I N I T I O N S * * * * * * * * * * * */
static int extract_148_data(PipeHandleType fifoPipe,
                            PCAP_PacketRecordHeaderType packetRecordHeader,
                            const uint8_t* data,
                            size_t dataLength)
{
    uint8_t buffer[16+8];
    uint8_t* UARTFrameBuffer = &buffer[16];
    memcpy(UARTFrameBuffer, data, dataLength);
    packetRecordHeader.protocolPayloadLength = (uint32_t)(dataLength);

    PCAP_FillPacketRecordHeader(&packetRecordHeader,
                                (void*)buffer);
    return PCAP_WritePacketRecord(fifoPipe, buffer);
}

static int extract_227_data(PipeHandleType fifoPipe,
                            PCAP_PacketRecordHeaderType packetRecordHeader,
                            const uint8_t* data,
                            size_t dataLength)
{
    if (dataLength < 3)
    {
        /* no CAN frame can be less than 3 bytes */
        return -1;
    }

    /* for now just assume that the given frame is a CAN2.0 frame */
    /** \todo there should be a check if the given frame is a CANFD or CANXL frame! */
    uint8_t buffer[16+16];
    uint8_t* CANFrameBuffer = &buffer[16];
    size_t i=0;
    if (data[i] >= 0x80)
    {
        /* captured frame is an extended frame and hence already formatted in the pcap format */
        memcpy(CANFrameBuffer, data, 4);
        i += 4;
    }
    else
    {
        /* captured frame is a standard frame and shortened by two bytes which are always 0 */
        CANFrameBuffer[0] = data[i] & 0xE0;
        CANFrameBuffer[1] = 0;
        CANFrameBuffer[2] = data[i++] & 0x1F;
        CANFrameBuffer[3] = data[i++];
    }

    CANFrameBuffer[4] = data[i++]; /* DLC in bytes! Not to confuse with the value of the CAN bus which is different for FD frames */
    CANFrameBuffer[5] = 0; /* FD flags */
    CANFrameBuffer[6] = 0; /* reserved */
    CANFrameBuffer[7] = 0; /* reserved */
    memcpy(CANFrameBuffer+8, data+i, dataLength-i);
    packetRecordHeader.protocolPayloadLength = (uint32_t)(8+dataLength-i);
    PCAP_FillPacketRecordHeader(&packetRecordHeader,
                                (void*)buffer);
    return PCAP_WritePacketRecord(fifoPipe, buffer);
}

/* G L O B A L   F U N C T I O N   D E F I N I T I O N S * * * * * * * * * * */
int captureDataFrame(PipeHandleType fifoPipe,
                     unsigned long dltValue,
                     uint32_t capturinoMicros,
                     size_t frameLength,
                     RingBufType* ringBuffer)
{
    PCAP_PacketRecordHeaderType packetRecordHeader;
    unsigned long long unixSeconds;
    unsigned long unixMicros;
    capturinoCommonGetTimestamp(capturinoMicros, &unixSeconds, &unixMicros);
    /* thanks to the PCAP standard, we must fall back to a 32 bit timestamp...
       at least its unsigned so we don't have a problem in 2038 but in 2106 */
    packetRecordHeader.timestampSeconds       = (uint32_t)unixSeconds;
    packetRecordHeader.timestampMicrosOrNanos = (uint32_t)unixMicros;

    uint8_t concatedData[64];
    size_t firstDataFractionLength = RingBuf_getFullElementsTail2End(ringBuffer);
    if (firstDataFractionLength >= frameLength)
    {
        memcpy(&concatedData[0], RingBuf_getTail(ringBuffer), frameLength);
        RingBuf_increaseTailMore(ringBuffer, frameLength);
    }
    else
    {
        memcpy(&concatedData[0], RingBuf_getTail(ringBuffer), firstDataFractionLength);
        RingBuf_increaseTailMore(ringBuffer, firstDataFractionLength);
        memcpy(&concatedData[firstDataFractionLength], RingBuf_getTail(ringBuffer), frameLength - firstDataFractionLength);
        RingBuf_increaseTailMore(ringBuffer, frameLength - firstDataFractionLength);
    }
    concatedData[frameLength] = '\0';

    switch ((PCAP_ValidLinkTypesType)dltValue)
    {
        case PCAP_USER1UART:
            /** \todo must be implemented */
            return extract_148_data(fifoPipe, packetRecordHeader, concatedData, frameLength);
        case PCAP_SOCKETCAN:
            return extract_227_data(fifoPipe, packetRecordHeader, concatedData, frameLength);

        default:
            return -1;
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* ***************************************************************************
 * E N D   O F   F I L E * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */
