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
 * \addtogroup capturinoconn
 * \brief Module managing the connection and communication to the CAPTURino
 *        embedded system.
 * 
 * @{
 */
/* ************************************************************************* */

#ifndef CAPTURINOCONN_H_INCLUDED
#define CAPTURINOCONN_H_INCLUDED

/* ***************************************************************************
 * I N C L U D E   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* S Y S T E M   I N C L U D E S * * * * * * * * * * * * * * * * * * * * * * */
#include <stdbool.h>
#include <stddef.h>

/* P R O J E C T   I N C L U D E S * * * * * * * * * * * * * * * * * * * * * */
#include "serialhandling.h"

/* ***************************************************************************
 * D E F I N E   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */
#ifdef __cplusplus
extern "C"
{
#endif

/* G L O B A L   C O N F I G   D E F I N I T I O N S * * * * * * * * * * * * */

/* G L O B A L   M A C R O   D E F I N I T I O N S * * * * * * * * * * * * * */

/* ***************************************************************************
 * T Y P E D E F   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* G L O B A L   T Y P E D E F S * * * * * * * * * * * * * * * * * * * * * * */

/* ***************************************************************************
 * V A R I A B L E S   A N D   C O N S T A N T S   S E C T I O N * * * * * * *
 *************************************************************************** */

/* G L O B A L   V A R I A B L E   D E C L A R A T I O N S * * * * * * * * * */

/* ***************************************************************************
 * F U N C T I O N S   S E C T I O N * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* G L O B A L   F U N C T I O N   P R O T O T Y P E S * * * * * * * * * * * */
/** Opens a serial connection to the CAPTURino device.
 * 
 * \param[in] path serial port to the CAPTURino device.
 * \param[in] baudrate baudrate of the serial connection.
 * 
 * \returns 0: if the serial connection was opened successfully.
 * \returns -1: if the function failed.
 */
int CCON_Open            (const    char*         path,
                                   unsigned int  baudrate);

/** Initiates a session with the CAPTURino device.
 * 
 * \param[in] timeoutMS timeout in milliseconds.
 * \param[in] terminateFlag flag to indicate that the session should be
 *                          terminated. This flag might be set asynchronously.
 * 
 * \returns 0: if the session was initiated successfully.
 * \returns -1: if the function failed.
 */
int CCON_InitiateSession (         unsigned long timeoutMS,
                          volatile bool*         terminateFlag);

/** Gets the board ID of the CAPTURino device.
 * 
 * \param[in] timeoutMS timeout in milliseconds.
 * \param[in] terminateFlag flag to indicate that the command should be
 *                          terminated. This flag might be set asynchronously.
 * 
 * \returns 0: if the board ID was read successfully.
 * \returns -1: if the function failed.
 */
int CCON_GetBoardId      (         unsigned long timeoutMS,
                          volatile bool*         terminateFlag,
                                   uint32_t*     boardId);

/** Gets the board micros of the CAPTURino device.
 * 
 * \param[in] timeoutMS timeout in milliseconds.
 * \param[in] terminateFlag flag to indicate that the command should be
 *                          terminated. This flag might be set asynchronously.
 * \param[out] boardMicros current microseconds timestamp of the CAPTURino
 *                         device.
 *
 * \returns 0: if the board micros were read successfully.
 * \returns -1: if the function failed.
 */
int CCON_GetBoardMicros  (         unsigned long timeoutMS,
                          volatile bool*         terminateFlag,
                                   uint32_t*     boardMicros);

/** Gets a list of the supported link types of the CAPTURino device.
 * 
 * \param[in] timeoutMS timeout in milliseconds.
 * \param[in] terminateFlag flag to indicate that the command should be
 *                          terminated. This flag might be set asynchronously.
 * \param[out] dlts array of supported link types.
 * \param[in] maxDltsCount maximum number of supported link types.
 * \param[out] dltsCount number of supported link types.
 *
 * \returns 0: if the supported link types were read successfully.
 * \returns -1: if the function failed.
 */
int CCON_GetSupportedDlts(         unsigned long timeoutMS,
                          volatile bool*         terminateFlag,
                                   uint32_t*     dlts,
                                   size_t        maxDltsCount,
                                   size_t*       dltsCount);

/** Writes a given command to the CAPTURino device.
 * 
 * \warning In order for the CAPTURino device to execute the command, the last
 *          character of the command string must be a newline character.
 *
 * \param[in] cmd command to be executed.
 * \param[in] cmdLen number of characters in cmd.
 * \param[in] timeoutMS timeout in milliseconds.
 * \param[in] terminateFlag flag to indicate that the command should be
 *                          terminated. This flag might be set asynchronously.
 * 
 * \returns 0: if the command was executed successfully.
 * \returns -1: if the function failed.
 */
int CCON_Exec            (const    char*         cmd,
                                   size_t        cmdLen, 
                                   unsigned long timeoutMS,
                          volatile bool*         terminateFlag);

/** Reads data from the CAPTURino device.
 * 
 * \param[out] buf buffer to store the read data.
 * \param[in] bufLen size of the buffer.
 * \param[out] bytesRead number of bytes read.
 * 
 * \returns 0: if the data was read successfully.
 * \returns -1: if the function failed.
 */
int CCON_Read            (         char*         buf,
                                   size_t        bufLen,
                                   size_t*       bytesRead);

/** Writes a given command to the CAPTURino device and waits for a response.
 * 
 * \warning In order for the CAPTURino device to execute the command, the last
 *          character of the command string must be a newline character.
 * 
 * \param[in] cmd command to be executed.
 * \param[in] cmdLen number of characters in cmd.
 * \param[in] timeoutMS timeout in milliseconds.
 * \param[in] terminateFlag flag to indicate that the command should be
 *                          terminated. This flag might be set asynchronously.
 * \param[out] responseBuf buffer to store the response.
 * \param[out] responseLen number of characters in the response.
 * \param[in] responseBufLen size of the response buffer.
 * \param[in] terminateSequence sequence to return the function call if found
 *                              in the response.
 * \param[in] terminateSequenceLen number of characters in terminateSequence.
 * 
 * \returns 0: if the command was executed and the terminateSequence has been
 *             found.
 * \returns -1: if the function failed.
 * \returns -2: if the function failed due to a timeout.
 */
int CCON_ExecWithResponse(const    char*         cmd,
                                   size_t        cmdLen, 
                                   unsigned long timeoutMS,
                          volatile bool*         terminateFlag,
                                   char*         responseBuf,
                                   size_t*       responseLen,
                                   size_t        responseBufLen,
                          const    char*         terminateSequence,
                                   size_t        terminateSequenceLen);

/** Waits for a given amount of time or until a termination flag is set.
 * 
 * \param[in] timeMS time to wait in milliseconds.
 * \param[in] terminateFlag flag to indicate that the waiting should be
 *                          terminated. This flag might be set asynchronously.
 * 
 * \returns 0: everytime.
 */
int CCON_Wait            (         unsigned long timeMS,
                          volatile bool*         terminateFlag);

/** Closes the connection to the CAPTURino device.
 *
 * \returns 0: if the connection was closed successfully.
 * \returns -1: if the function failed.
 */
int CCON_Close           ();

/* G L O B A L   I N L I N E   F U N C T I O N   D E F I N I T I O N S * * * */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#ifdef __cplusplus
}
#endif

#endif /* CAPTURINOCONN_H_INCLUDED */

/**
 * @}
 */
/* ***************************************************************************
 * E N D   O F   F I L E * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */
