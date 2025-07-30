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

#ifndef PCAP_WRITER_H_INCLUDED
#define PCAP_WRITER_H_INCLUDED

/* ***************************************************************************
 * I N C L U D E   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* S Y S T E M   I N C L U D E S * * * * * * * * * * * * * * * * * * * * * * */
#include <stdbool.h>
#include <stdint.h>

/* P R O J E C T   I N C L U D E S * * * * * * * * * * * * * * * * * * * * * */
#include "pipehandling.h"

/* ***************************************************************************
 * D E F I N E   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* G L O B A L   C O N F I G   D E F I N I T I O N S * * * * * * * * * * * * */

/* G L O B A L   M A C R O   D E F I N I T I O N S * * * * * * * * * * * * * */

/* ***************************************************************************
 * T Y P E D E F   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* G L O B A L   T Y P E D E F S * * * * * * * * * * * * * * * * * * * * * * */

/** Link types to be used in the PCAP header */ 
typedef enum {
    PCAP_CAPTURINODEBUG = 147,
    PCAP_USER1UART      = 148,
    PCAP_SOCKETCAN      = 227
} PCAP_ValidLinkTypesType;

typedef struct
{
    uint32_t timestampSeconds;
    uint32_t timestampMicrosOrNanos;
    uint32_t protocolPayloadLength;
} PCAP_PacketRecordHeaderType;

/* ***************************************************************************
 * V A R I A B L E S   A N D   C O N S T A N T S   S E C T I O N * * * * * * *
 *************************************************************************** */

/* G L O B A L   V A R I A B L E   D E C L A R A T I O N S * * * * * * * * * */

/* ***************************************************************************
 * F U N C T I O N S   S E C T I O N * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* G L O B A L   I N L I N E   F U N C T I O N   D E F I N I T I O N S * * * */

/* G L O B A L   F U N C T I O N   P R O T O T Y P E S * * * * * * * * * * * */
int PCAP_WriteHeader           (      PipeHandleType               hFile,
                                      bool                         timestampNanos,
                                      uint32_t                     snapLength,
                                      PCAP_ValidLinkTypesType      linkType,
                                      uint8_t                      pFlag,
                                      uint8_t                      rFlag,
                                      uint16_t                     fcsLength);

int PCAP_FillPacketRecordHeader(const PCAP_PacketRecordHeaderType* packetRecordHeader,
                                      void*                        packetRecord);

/** Writes a packet record to a PCAP file
 * 
 * \param hFile Handle to the file to write the packet record to
 * \param packetData The data of the packet record to write
 * 
 * \note packetData must be a pointer to a buffer containing
 *       packetRecordHeader.packetLength bytes
 * \note within the packetData buffer, the bytes 8-12 must contain the captured
 *       packet length (as required by the PCAP standard). This amount of bytes
 *       will be written to the pipe.
 * \warning this function does not check if the file handle is valid
 */
int PCAP_WritePacketRecord     (      PipeHandleType               hFile,
                                      void*                        packetData);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#endif // PCAP_WRITER_H_INCLUDED

/* ***************************************************************************
 * E N D   O F   F I L E * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */
