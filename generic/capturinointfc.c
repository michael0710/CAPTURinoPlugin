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
#include <stdbool.h>
#include <string.h>

/* P R O J E C T   I N C L U D E S * * * * * * * * * * * * * * * * * * * * * */
#include "capturino2pcapadptr.h"
#include "capturinoconn.h"
#include "console.h"
#include "diagnosis.h"
#include "genericutils.h"
#include "pipehandling.h"
#include "pcap_writer.h"
#include "serialhandling.h"
#include "systemutils.h"
#include "capturinocommonintfcfuncs.h"
#include "ringbuf.h"

/* M O D U L E   H E A D E R   I N C L U D E * * * * * * * * * * * * * * * * */
#include "capturinointfc.h"

/* ***************************************************************************
 * D E F I N E   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* L O C A L   C O N F I G   D E F I N I T I O N S * * * * * * * * * * * * * */

/* L O C A L   M A C R O   D E F I N I T I O N S * * * * * * * * * * * * * * */

/* ***************************************************************************
 * T Y P E D E F   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */
typedef enum
{
    CAPTURINO_STATE_RCV_HEADER_TIMESTAMP = 0,
    CAPTURINO_STATE_RCV_HEADER_PAYLOAD_LENGTH = 1,
    CAPTURINO_STATE_RCV_CONTENT = 2,
} LocalCaptureStmacStatesType;

/* ***************************************************************************
 * V A R I A B L E S   A N D   C O N S T A N T S   S E C T I O N * * * * * * *
 *************************************************************************** */

/* G L O B A L   V A R I A B L E   D E F I N I T I O N S * * * * * * * * * * */

/* L O C A L   V A R I A B L E   D E F I N I T I O N S * * * * * * * * * * */
static volatile bool mTerminateFlag = false;
static LocalCaptureStmacStatesType mCaptureState = CAPTURINO_STATE_RCV_HEADER_TIMESTAMP;

/* L O C A L   C O N S T A N T   D E F I N I T I O N S * * * * * * * * * * * */
static const char* MODULE_NAME = "CAPT_INTF";

/* ***************************************************************************
 * F U N C T I O N S   S E C T I O N * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* L O C A L   F U N C T I O N   P R O T O T Y P E S * * * * * * * * * * * * */
static int capturinoExtcapDlts(int argc, char *argv[]);

static int capturinoExtcapCapture(int argc, char *argv[]);

static int capturinoExtcapTerminateCb();

const EXMG_IntfcType capturinoIntfc =
{
    .val  = "CAPTURino",
    .disp = "CAPTURino (external bus capture)",
    .extcapDltsFunc    = capturinoExtcapDlts,
    .extcapConfigFunc  = capturinoCommonExtcapConfig,
    .extcapCaptureFunc = capturinoExtcapCapture,
    .extcapTerminateCb = capturinoExtcapTerminateCb
};

/* L O C A L   I N L I N E   F U N C T I O N   D E F I N I T I O N S * * * * */

/* L O C A L   F U N C T I O N   D E F I N I T I O N S * * * * * * * * * * * */

static int captureWithOpenFifoAndComm(PipeHandleType fifoPipe,
                                      unsigned long dltValue,
                                      int argc,
                                      char *argv[])
{
    int fcnRt = 0;

    fcnRt = CCON_InitiateSession(5000, &mTerminateFlag);
    if (fcnRt != 0)
    {
        DIAG_LogMsgArg(DIAG_FAILURE, MODULE_NAME, __func__, "error waiting for CAPTURino response! Return value=%d", fcnRt);
        return -1;
    }
    
    DIAG_LogMsg(DIAG_INFO, MODULE_NAME, __func__, "connection to CAPTURino established");
    
    /* get current time from the system ... */
    unsigned long long hostUnixTime = 0;
    unsigned long hostMicros = 0;
    SYSU_GetCurrentTime(&hostUnixTime, &hostMicros);
    /* ... get current millis from the device ... */
    uint32_t capturinoMicros = 0;
    fcnRt = CCON_GetBoardMicros(500, &mTerminateFlag, &capturinoMicros);
    if (fcnRt != 0)
    {
        DIAG_LogMsgArg(DIAG_FAILURE, MODULE_NAME, __func__, "error waiting for CAPTURino response! Return value=%d", fcnRt);
        return -1;
    }
    /* ... and calculate the base time to print the current timestamp within
       wireshark */
    capturinoCommonSetTimebase(hostUnixTime, hostMicros, capturinoMicros);

    /** \warning the command length must not be greater than the CLI_INPUT_BUFFER_SIZE
     *           of the embedded software */
    char captureCmd[128];
    size_t cmdLen = 0;
    fcnRt = capturinoCommonGenerateCaptureCmd(dltValue, argc, argv, captureCmd, 128, &cmdLen);
    if (fcnRt != 0)
    {
        DIAG_LogMsg(DIAG_FAILURE, MODULE_NAME, __func__, "error generating capture command!");
        return -1;
    }

    fcnRt = CCON_Exec(captureCmd, cmdLen, 200, &mTerminateFlag);
    if (fcnRt != 0)
    {
        DIAG_LogMsg(DIAG_FAILURE, MODULE_NAME, __func__, "error sending capture command!");
        return -1;
    }

    DIAG_LogMsgArg(DIAG_INFO, MODULE_NAME, __func__, "Sent capture command with %lu characters: \'%.*s\'", cmdLen, cmdLen, captureCmd);

    /* wait for the first character returned by the capture command. Must be
       ^F (0x06) to indicate a successful start of the capture process */
    while (mTerminateFlag == false)
    {
        size_t bytesRead = 0;
        char rcvChar;
        fcnRt = CCON_Read(&rcvChar, 1, &bytesRead);
        if (fcnRt != 0)
        {
            DIAG_LogMsg(DIAG_FAILURE, MODULE_NAME, __func__, "error reading from serial port!");
            return -1;
        }
        if (bytesRead > 0)
        {
            DIAG_LogMsgArg(DIAG_DEBUG, MODULE_NAME, __func__, "Received char: 0x%02X", rcvChar);
            if ((rcvChar == '\r') || (rcvChar == '\n'))
            {
                /* ignore new line characters \r and \n */
                continue;
            }
            else if (rcvChar == 0x06)
            {
                break;
            }
            else
            {
                DIAG_LogMsgArg(DIAG_FAILURE, MODULE_NAME, __func__, "Expected to receive %02X, but received %02X", 0x06, rcvChar);
                if (rcvChar == 'R')
                {
                    CNSL_WriteErr("The CAPTURino hardware is trying to communicate with a human, but I am a machine. Please flash the correct software to the CAPTURino hardware!",
                            STATIC_STRLEN("The CAPTURino hardware is trying to communicate with a human, but I am a machine. Please flash the correct software to the CAPTURino hardware!"));
                }
                return -1;
            }
        }
        else
        {
            /* sleep for a little to avoid high CPU usage, but only if no character has been received */
            SYSU_Sleep(1);
        }
    }

    size_t bytesToReceive = 0;
    uint8_t rcvBuffer[512];
    RingBufType buffer = {
        .head = 0,
        .tail = 0,
        .elementSize = 1,
        .buffer = rcvBuffer,
        .bufferSize = 512
    };
    uint32_t captureTimestampMicros = 0;
    uint32_t previousTimestampMicros = 0;
    bool previousNullFrameWasAllNull = false;
    
    while (mTerminateFlag == false)
    {
        size_t bytesRead = 0;
        fcnRt = CCON_Read(RingBuf_getHead(&buffer), RingBuf_getFreeElementsHead2End(&buffer), &bytesRead);
        if (fcnRt != 0)
        {
            DIAG_LogMsg(DIAG_FAILURE, MODULE_NAME, __func__, "error reading from serial port!");
            return -1;
        }
        if (bytesRead > 0)
        {
            DIAG_LogMsgArg(DIAG_VERBOSE, MODULE_NAME, __func__, "Received %d chars", bytesRead);
            RingBuf_increaseHeadMore(&buffer, bytesRead);
        }

        size_t bufferElements = RingBuf_getElementsCount(&buffer);
        switch (mCaptureState)
        {
            case CAPTURINO_STATE_RCV_HEADER_TIMESTAMP:
                if (bufferElements >= 4)
                {
                    previousTimestampMicros = captureTimestampMicros;
                    captureTimestampMicros = 0;
                    for (size_t i=0; i<4; i++)
                    {
                        captureTimestampMicros <<= 8;
                        captureTimestampMicros += *((uint8_t*)RingBuf_getTailOffset(&buffer, i));
                    }
                    RingBuf_increaseTailMore(&buffer, 4);
                    mCaptureState = CAPTURINO_STATE_RCV_HEADER_PAYLOAD_LENGTH;
                    if (captureTimestampMicros < previousTimestampMicros)
                    {
                        DIAG_LogMsgArg(DIAG_INFO, MODULE_NAME, __func__, "embedded timestamp wrapped from %d to %d", previousTimestampMicros, captureTimestampMicros);
                        const uint32_t secondsOffset = UINT32_MAX / 1000000;
                        const uint32_t microsOffset = UINT32_MAX - secondsOffset*1000000; 
                        capturinoCommonUpdateTimebase((unsigned long long)secondsOffset,
                                                      (unsigned long)microsOffset);
                    }
                }
                else
                {
                    SYSU_Sleep(1);
                }
                break;

            case CAPTURINO_STATE_RCV_HEADER_PAYLOAD_LENGTH:
                if (bufferElements >= 1)
                {
                    uint8_t tempByte;
                    tempByte = *((uint8_t*)RingBuf_getTail(&buffer));
                    if (tempByte >= 0x80)
                    {
                        /* MSB of PayloadLength1 is set, i.e. parts of the value are stored in PayloadLength2 */
                        if (bufferElements >= 2)
                        {
                            bytesToReceive = (tempByte & 0x7F) << 8;
                            bytesToReceive += *((uint8_t*)RingBuf_getTailOffset(&buffer, 5));
                            DIAG_LogMsgArg(DIAG_VERBOSE, MODULE_NAME, __func__, "large frame received. Length=%d", bytesToReceive);
                            RingBuf_increaseTailMore(&buffer, 2);
                            mCaptureState = CAPTURINO_STATE_RCV_CONTENT;
                        }
                    }
                    else if (tempByte > 0)
                    {
                        bytesToReceive = tempByte;
                        DIAG_LogMsgArg(DIAG_VERBOSE, MODULE_NAME, __func__, "short frame received. Length=%d", bytesToReceive);
                        RingBuf_increaseTail(&buffer);
                        mCaptureState = CAPTURINO_STATE_RCV_CONTENT;
                    }
                    else /* null frame received (used for resynchronisation) */
                    {
                        RingBuf_increaseTail(&buffer);
                        DIAG_LogMsg(DIAG_VERBOSE, MODULE_NAME, __func__, "Null frame received");
                        if (captureTimestampMicros == 0)
                        {
                            if (previousNullFrameWasAllNull == true)
                            {
                                /* two consecutive null frames received, i.e. the CAPTURino hardware indicates an error */
                                mTerminateFlag = true;
                                CNSL_WriteErr("Internal error in the CAPTURino hardware. Capture process stopped!",
                                              STATIC_STRLEN("Internal error in the CAPTURino hardware. Capture process stopped!"));
                            }
                            else 
                            {
                                previousNullFrameWasAllNull = true;
                            }
                        }
                        else
                        {
                            previousNullFrameWasAllNull = false;
                        }
                        mCaptureState = CAPTURINO_STATE_RCV_HEADER_TIMESTAMP;
                    }
                }
                break;

            case CAPTURINO_STATE_RCV_CONTENT:
                if (bufferElements >= bytesToReceive)
                {
                    /* write the received data to the fifo */
                    captureDataFrame(fifoPipe,
                                     dltValue,
                                     captureTimestampMicros,
                                     bytesToReceive,
                                     &buffer);
                    mCaptureState = CAPTURINO_STATE_RCV_HEADER_TIMESTAMP;
                }
                else if (bytesToReceive > 64) /* for now all values greater than 64 are malformed packets */
                {
                    DIAG_LogMsgArg(DIAG_FAILURE, MODULE_NAME, __func__, "Capture failed due to a possibly malformed packet! bytesToReceive=%lu, limit was %lu", bytesToReceive, 64);
                    /* instead of returning directly, leave the while loop so that the currently running command
                       on the embedded device is terminated */
                    mTerminateFlag = true;
                }
                break;
        }
        /** \todo in case of no captured data is available for a certain amount
                  of time, the CAPTURino control board shall send a null frame,
                  which can be used to check if the communication is alive and
                  also to resynchronize the transmitted timestamp */
    }

    /* terminate a possible running capture command */
    bool noTerminateFlag = false;
    CCON_Exec("\x03", 1, 50, &noTerminateFlag);

    return 0;
}

static int captureWithOpenFifo(PipeHandleType fifoPipe,
                               long baudrate,
                               char* comPort,
                               uint32_t dltValue,
                               int argc,
                               char *argv[])
{
    int fcnRt = 0;

    /** \todo move this function call to a DLT specific capture function */
    fcnRt = PCAP_WriteHeader(fifoPipe, false, 512, dltValue, 0, 0, 0);
    if (fcnRt != 0)
    {
        DIAG_LogMsg(DIAG_FAILURE, MODULE_NAME, __func__, "unable to write pcap header!");
        return -1;
    }

    fcnRt = CCON_Open(comPort, baudrate);
    if (fcnRt != 0)
    {
        DIAG_LogMsg(DIAG_FAILURE, MODULE_NAME, __func__, "Unable to open serial communication to CAPTURino!");
        return -1;
    }
    DIAG_LogMsg(DIAG_INFO, MODULE_NAME, __func__, "Opened communication to CAPTURino successfully");
    
    fcnRt = captureWithOpenFifoAndComm(fifoPipe, dltValue, argc, argv);
    if (fcnRt != 0)
    {
        DIAG_LogMsgArg(DIAG_ERROR, MODULE_NAME, __func__, "call to captureWithOpenFifoAndComm returned %d", fcnRt);
    }

    DIAG_LogMsg(DIAG_INFO, MODULE_NAME, __func__, "closing serial port");
    CCON_Close();

    return 0;
}

static int capturinoExtcapDlts(int argc, char *argv[])
{
    DIAG_LogMsg(DIAG_DEBUG, MODULE_NAME, __func__, "function entered");

#ifdef CAPTURINO_DLT_STRING
    #warning "CAPTURINO_DLT_STRING is already defined and will be redefined!"
#endif
#define CAPTURINO_DLT_STRING "dlt {number=148}{name=UART}{display=UART bus messages}" \
                             "dlt {number=227}{name=CAN}{display=CAN bus messages}"
    CNSL_WriteLn(CAPTURINO_DLT_STRING, STATIC_STRLEN(CAPTURINO_DLT_STRING));
    return 0;
}

static int capturinoExtcapCapture(int argc, char *argv[])
{
    DIAG_LogMsg(DIAG_DEBUG, MODULE_NAME, __func__, "function entered");
    int fcnRt = 0;

    /* local variables needed within the state machine */

    /* parse arguments */
    char* fifopath = NULL;
    fcnRt += ARGP_getP2StringOfArgs(argc, argv, "--fifo", &fifopath);
    /* note: as of the wireshark documentation the option '--extcap-capture-filter'
             must be supported.
       see: https://www.wireshark.org/docs/wsdg_html_chunked/ChCaptureExtcap.html
            chapter: 8.2.1.4 */
    
    long baudrate = -1;
    fcnRt  = ARGP_getLongOfArgs(argc, argv, "--baudrate", &baudrate);

    char* comPort = NULL;
    fcnRt += ARGP_getP2StringOfArgs(argc, argv, "--port", &comPort);

    unsigned long dltValue = 0;
    fcnRt += ARGP_getUnsignedLongOfArgs(argc, argv, "--dlts", &dltValue);

    fcnRt += capturinoCommonValidateParameters(comPort, baudrate, fifopath, dltValue);
    if (fcnRt == 0)
    {
        DIAG_LogMsgArg(DIAG_DEBUG, MODULE_NAME, __func__, "parsed arguments: baudrate=%ld, comPort=%s, fifoPath=%s, dltValue=%lu",
                                baudrate, comPort, fifopath, dltValue);
    }
    else
    {
        DIAG_LogMsg(DIAG_FAILURE, MODULE_NAME, __func__, "error parsing arguments!");
        DIAG_LogMsgArg(DIAG_INFO, MODULE_NAME, __func__, "parsed arguments: baudrate=%ld, comPort=%s, fifoPath=%s, dltValue=%lu",
                                baudrate, comPort, fifopath, dltValue);
        return -1;
    }

    PipeHandleType fifoPipe = 0;
    fcnRt = PIPH_Open(fifopath, &fifoPipe);
    if (fcnRt != 0)
    {
        DIAG_LogMsg(DIAG_FAILURE, MODULE_NAME, __func__, "unable to create fifo!");
        return -1;
    }
    
    fcnRt = captureWithOpenFifo(fifoPipe,
                                baudrate,
                                comPort,
                                dltValue,
                                argc,
                                argv);

    if (fcnRt != 0)
    {
        DIAG_LogMsgArg(DIAG_ERROR, MODULE_NAME, __func__, "call to captureWithOpenFifo returned %d", fcnRt);
        CNSL_WriteErr("Capture process stopped! An error occurred during the capture process!\n",
                      STATIC_STRLEN("Capture process stopped! An error occurred during the capture process!\n"));
    }

    if (fifoPipe != 0)
    {
        DIAG_LogMsg(DIAG_INFO, MODULE_NAME, __func__, "closing fifo");
        PIPH_Close(fifoPipe);
    }

    /* note that wireshark has no way to properly cleanup an extcap plugin on windows
       see Wireshark Issue #17131 https://gitlab.com/wireshark/wireshark/-/issues/17131 */

    return 0;
}

static int capturinoExtcapTerminateCb()
{
    DIAG_LogMsg(DIAG_DEBUG, MODULE_NAME, __func__, "function entered");
    mTerminateFlag = true;
    return 0;
}

/* G L O B A L   F U N C T I O N   D E F I N I T I O N S * * * * * * * * * * */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* ***************************************************************************
 * E N D   O F   F I L E * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */
