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
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* P R O J E C T   I N C L U D E S * * * * * * * * * * * * * * * * * * * * * */
#include "capturinoconn.h"
#include "console.h"
#include "diagnosis.h"
#include "genericutils.h"
#include "pipehandling.h"
#include "pcap_writer.h"
#include "pcap_147_capturinodebug.h"
#include "serialhandling.h"
#include "systemutils.h"
#include "capturinocommonintfcfuncs.h"
#include "ringbuf.h"

/* M O D U L E   H E A D E R   I N C L U D E * * * * * * * * * * * * * * * * */
#include "capturinotestintfc.h"

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
static volatile bool mTerminateFlag = false;

/* L O C A L   C O N S T A N T   D E F I N I T I O N S * * * * * * * * * * * */
static const char* MODULE_NAME = "CAPT_TEST";

/* ***************************************************************************
 * F U N C T I O N S   S E C T I O N * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* L O C A L   F U N C T I O N   P R O T O T Y P E S * * * * * * * * * * * * */
static int capturinoExtcapDlts(int argc, char *argv[]);

static int capturinoExtcapCapture(int argc, char *argv[]);

static int captureWithOpenFifo(PipeHandleType fifoPipe,
                               long baudrate,
                               char* comPort,
                               unsigned long dltValue,
                               int argc,
                               char *argv[]);

static int captureWithOpenFifoAndComm(PipeHandleType fifoPipe,
                                      unsigned long dltValue,
                                      int argc,
                                      char *argv[]);

static int capturinoExtcapTerminateCb();

const EXMG_IntfcType capturinoTestIntfc =
{
    .val  = "CAPTURinoTEST",
    .disp = "CAPTURino (external device debug messages)",
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
    
    DIAG_LogMsg(DIAG_INFO, MODULE_NAME, __func__, "connected to CAPTURino ...");
    PCAP_147_CapturinoDebug_writeDebugMsg(fifoPipe, "CLI on CAPTURino started successfully");
    
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
    PCAP_147_CapturinoDebug_writeDebugMsg(fifoPipe, "Time offset calculated successfully");

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
    PCAP_147_CapturinoDebug_writeDebugMsg(fifoPipe, "Capture command sent");

    /* wait for the first character returned by the capture command. Must be
       ^F (0x06) to indicate a successful start of the capture process */
    while (mTerminateFlag == false)
    {
        size_t bytesRead = 0;
        char rcvChar;
        fcnRt = CCON_Read(&rcvChar, 1, &bytesRead);
        if (bytesRead > 0)
        {
            DIAG_LogMsgArg(DIAG_DEBUG, MODULE_NAME, __func__, "Received char: \'%c\'", rcvChar);
            if ((rcvChar == '\r') || (rcvChar == '\n'))
            {
                /* ignore new line characters \r and \n */
                continue;
            }
            else if (rcvChar == 0x06)
            {
                PCAP_147_CapturinoDebug_writeDebugMsg(fifoPipe, "Received signal for successful initialization");
                break;
            }
            else
            {
                DIAG_LogMsgArg(DIAG_FAILURE, MODULE_NAME, __func__, "Expected to receive %02x, but received %02x", 0x06, rcvChar);
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
    char rcvBuffer[256];
    RingBufType buffer = {
        .head = 0,
        .tail = 0,
        .elementSize = 1,
        .buffer = rcvBuffer,
        .bufferSize = 256
    };

    PCAP_147_CapturinoDebug_writeDebugMsg(fifoPipe, "Waiting for message frames ...");
    while (mTerminateFlag == false)
    {
        size_t bytesRead = 0;
        fcnRt = CCON_Read(RingBuf_getHead(&buffer), RingBuf_getFreeElementsHead2End(&buffer), &bytesRead);
        if (bytesRead > 0)
        {
            DIAG_LogMsgArg(DIAG_DEBUG, MODULE_NAME, __func__, "Received %d chars", bytesRead);
        }
        RingBuf_increaseHeadMore(&buffer, bytesRead);
        size_t bufferElements = RingBuf_getElementsCount(&buffer);
        if (bytesToReceive == 0)
        {
            if (bufferElements >= 5)
            {
                DIAG_LogMsgArg(DIAG_VERBOSE, MODULE_NAME, __func__, "buffer has %d elements", bufferElements);
                /* extract timestamp */
                uint32_t timestamp = 0;
                uint8_t tempByte;
                for (size_t i=0; i<4; i++)
                {
                    tempByte = *((uint8_t*)RingBuf_getTailOffset(&buffer, i));
                    timestamp <<= 8;
                    timestamp += tempByte;
                }
                tempByte = *((uint8_t*)RingBuf_getTailOffset(&buffer, 4));
                if (tempByte >= 0x80)
                {
                    /* MSB of PayloadLength1 is set, i.e. parts of the value are stored in PayloadLength2 */
                    if (bufferElements >= 6)
                    {
                        bytesToReceive = (tempByte & 0x7F) << 8;
                        bytesToReceive += *((uint8_t*)RingBuf_getTailOffset(&buffer, 5));
                        DIAG_LogMsgArg(DIAG_VERBOSE, MODULE_NAME, __func__, "large frame received. Length=%d", bytesToReceive);
                        RingBuf_increaseTailMore(&buffer, 6);
                    }
                }
                else if (tempByte > 0)
                {
                    bytesToReceive = tempByte;
                    DIAG_LogMsgArg(DIAG_VERBOSE, MODULE_NAME, __func__, "short frame received. Length=%d", bytesToReceive);
                    RingBuf_increaseTailMore(&buffer, 5);
                }    
                else /* null frame received (used for resynchronisation) */
                {
                    bytesToReceive = 0;
                    RingBuf_increaseTailMore(&buffer, 5);
                    PCAP_147_CapturinoDebug_writeDebugMsg(fifoPipe, "Null frame received");
                }
            }
        }
        else if (bufferElements >= bytesToReceive)
        {
            /* write the received data to the fifo */
            PCAP_147_CapturinoDebug_writeDebugMsg(fifoPipe, "Frame received");
            /* shift the remaining received bytes to the beginning */
            RingBuf_increaseTailMore(&buffer, bytesToReceive);
            /* reset the variable to trigger a new detection of the next frame length */
            bytesToReceive = 0;
        }

        /* for now this case shall lead to an error */
        if (bufferElements >= 250)
        {
            return -1;
        }
        SYSU_Sleep(1);

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
                               unsigned long dltValue,
                               int argc,
                               char *argv[])
{
    int fcnRt = 0;

    fcnRt = PCAP_WriteHeader(fifoPipe, 0, 512, PCAP_CAPTURINODEBUG, 0, 0, 0);
    if (fcnRt != 0)
    {
        DIAG_LogMsg(DIAG_FAILURE, MODULE_NAME, __func__, "unable to write pcap header!");
        return -1;
    }

    fcnRt = CCON_Open(comPort, baudrate);
    while (fcnRt != 0)
    {
        PCAP_147_CapturinoDebug_writeDebugMsg(fifoPipe, "Unable to open serial communication to CAPTURino! Retrying in 5 seconds ...");
        CCON_Wait(5000, &mTerminateFlag);
        if (mTerminateFlag == true)
        {
            return 0;
        }
        fcnRt = CCON_Open(comPort, baudrate);
    }
    PCAP_147_CapturinoDebug_writeDebugMsg(fifoPipe, "Serial port opened successfully");
    
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
#define CAPTURINO_DLT_STRING "dlt {number=147}{name=USER0}{display=CAPTURino external device debug messages}"
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

    unsigned long dltValue = 147;

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
