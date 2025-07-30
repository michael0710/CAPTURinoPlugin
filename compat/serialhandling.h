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
 * \addtogroup serialhandling
 * 
 * @{
 */
/* ************************************************************************* */


#ifndef SERIALHANDLING_H_INCLUDED
#define SERIALHANDLING_H_INCLUDED

/* ***************************************************************************
 * I N C L U D E   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* S Y S T E M   I N C L U D E S * * * * * * * * * * * * * * * * * * * * * * */
#include <stddef.h>

/* P R O J E C T   I N C L U D E S * * * * * * * * * * * * * * * * * * * * * */

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
typedef unsigned int SerialHandleType;

/* ***************************************************************************
 * V A R I A B L E S   A N D   C O N S T A N T S   S E C T I O N * * * * * * *
 *************************************************************************** */

/* G L O B A L   V A R I A B L E   D E C L A R A T I O N S * * * * * * * * * */

/* ***************************************************************************
 * F U N C T I O N S   S E C T I O N * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* G L O B A L   F U N C T I O N   P R O T O T Y P E S * * * * * * * * * * * */
extern const SerialHandleType INVALID_SERIAL_HANDLE;

/** Opens a serial port for reading and writing
 * 
 * \param[in] path The name of the serial port to be opened.
 * \param[in] baudrate The baudrate to be used for the serial port.
 * \param[out] serialHandleVal The handle to the opened serial port.
 *
 * \returns 0: if the initialization has been finished successfully
 * \returns -1: if the initialization failed
 */
int SERH_Open       (const char*             path,
                           unsigned int      baudrate,
                           SerialHandleType* serialHandleVal);

/** Closes a serial port
 *
 * \param[in] serialHandleVal The handle to the serial port to be closed.
 * 
 * \returns 0: if the closing operation was successful
 * \returns -1: if the closing operation failed
 */
int SERH_Close      (      SerialHandleType  serialHandleVal);

/** Retrieves a list of available serial ports and writes it to the given
 * memory.
 * 
 * \param[out] buffer The buffer to store the names of the available serial
 *                    ports.
 * \param[in] bufSize The size of the buffer.
 * \param[out] portNames The array to store the list of the available serial
 *                       ports.
 * \param[in] portNamesSize The size of the portNames array.
 * \param[out] portCount The number of available serial ports.
 * 
 * \returns 0: if the port list was successfully retrieved.
 * \returns -1: if the function failed.
 */
int SERH_GetPortList(      char*             buffer,
                           size_t            bufSize,
                           char**            portNames,
                           size_t            portNamesSize,
                           size_t*           portCount);

/** Clears the specified serial input.
 * 
 * \param[in] serialHandleVal handle to the serial port to be flushed.
 * 
 * \returns 0: if the flush operation was successful.
 * \returns -1: if the flush operation failed.
 */
int SERH_FlushInput (      SerialHandleType  serialHandleVal);

/** Reads from the specified serial port.
 *
 * \param[in] serialHandleVal handle to the serial port to be read from.
 * \param[out] buf character array to store the read data.
 * \param[in] chars2read amount of characters to be read from the serial port.
 *
 * \note If the specified buffer is too small to store all data, only the
 *       amount of characters that fit into the buffer will be read. The return
 *       value will be 0 in this case.
 * 
 * \returns 0: if the read operation was successful.
 * \returns -1: if the read operation failed.
 */
int SERH_Read       (      SerialHandleType  serialHandleVal,
                           char*             buf,
                           size_t            maxChars2read,
                           size_t*           charsRead);

/** Writes to the specified serial port.
 * 
 * \param[in] serialHandleVal handle to the serial port to be written to.
 * \param[in] buf character array to be written to the serial port.
 * \param[in] chars2write amount of characters to be written to the serial
 *                        port.
 * \returns 0: if the write operation was successful
 * \returns -1: if the write operation failed
 */
int SERH_Write      (      SerialHandleType  serialHandleVal,
                     const char*             buf,
                           size_t            chars2write);

/** Writes to the given serial port and appends a new line character or
 * character sequence, depending on the OS.
 * 
 * \param[in] serialHandleVal handle to the serial port to write to.
 * \param[in] buf character array to write to the serial port.
 * \param[in] chars2write amount of characters to write to the serial port.
 * 
 * \returns 0: if the write operation was successful
 * \returns -1: if the write operation failed
 */
int SERH_WriteLn    (      SerialHandleType  serialHandleVal,
                     const char*             buf,
                           size_t            chars2write);

/** Writes to the given serial port.
 * 
 * \param[in] serialHandleVal handle to the serial port to write to.
 * \param[in] fmtMsg The format string to write to the serial port.
 * \param[in] ... The arguments to be applied to the format string.
 * 
 * \returns 0: if the write operation was successful
 * \returns -1: if the write operation failed
 */
int SERH_WriteArg   (      SerialHandleType  serialHandleVal,
                     const char*             fmtMsg,
                                             ...);

/** Writes to the given serial port and appends a new line character or
 * character sequence, depending on the OS.
 * 
 * \param[in] serialHandleVal handle to the serial port to write to.
 * \param[in] fmtMsg The format string to write to the serial port.
 * \param[in] ... The arguments to be applied to the format string.
 * 
 * \returns 0: if the write operation was successful
 * \returns -1: if the write operation failed
 */
int SERH_WriteArgLn (      SerialHandleType  serialHandleVal,
                     const char*             fmtMsg,
                                             ...);

/* G L O B A L   I N L I N E   F U N C T I O N   D E F I N I T I O N S * * * */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#ifdef __cplusplus
}
#endif

#endif /* SERIALHANDLING_H_INCLUDED */

/**
 * @}
 */
/* ***************************************************************************
 * E N D   O F   F I L E * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */
