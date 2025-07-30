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
 * \addtogroup pipehandling
 * \brief Provides input and output operations for pipes.
 *
 * @{
 */
/* ************************************************************************* */

#ifndef PIPEHANDLING_H_INCLUDED
#define PIPEHANDLING_H_INCLUDED

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
typedef unsigned int PipeHandleType;

/* ***************************************************************************
 * V A R I A B L E S   A N D   C O N S T A N T S   S E C T I O N * * * * * * *
 *************************************************************************** */

/* G L O B A L   V A R I A B L E   D E C L A R A T I O N S * * * * * * * * * */
extern const PipeHandleType INVALID_PIPE_HANDLE;

/* ***************************************************************************
 * F U N C T I O N S   S E C T I O N * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* G L O B A L   F U N C T I O N   P R O T O T Y P E S * * * * * * * * * * * */
/** Opens a pipe for reading and writing.
 * 
 * \param[in] path The name of the pipe to be opened.
 * \param[out] pipeHandleVal The handle to the opened pipe.
 *
 * \returns 0: if the initialization has been finished successfully.
 * \returns -1: if the initialization failed.
 */
int PIPH_Open      (const char*           path,
                          PipeHandleType* pipeHandleVal);

/** Closes a previously opened pipe.
 *
 * \param[in] pipeHandleVal The handle to the pipe.
 *
 * \returns 0: if the closing operation was successful.
 * \returns -1: if the closing operation failed.
 */
int PIPH_Close     (      PipeHandleType  pipeHandleVal);

/** Writes a string to a pipe.
 * 
 * \param[in] pipeHandleVal The handle to the pipe.
 * \param[in] buf: the string to write to the pipe.
 * \param[in] chars2write: the amount of characters to write to the pipe.
 * 
 * \returns 0: if the write operation was successful.
 * \returns -1: if the write operation failed.
 */
int PIPH_Write     (      PipeHandleType  pipeHandleVal,
                    const char*           buf,
                          size_t          chars2write);

/** Writes a string to a pipe and appends a new line character or character
 * sequence, depending on the OS.
 * 
 * \param[in] pipeHandleVal The handle to the pipe.
 * \param[in] buf: the string to write to the pipe.
 * \param[in] chars2write: the amount of characters to write to the pipe.
 * 
 * \returns 0: if the write operation was successful.
 * \returns -1: if the write operation failed.
 */
int PIPH_WriteLn   (      PipeHandleType  pipeHandleVal,
                    const char*           buf,
                          size_t          chars2write);


/** Writes to the pipe.
 * 
 * \param[in] pipeHandleVal The handle to the pipe.
 * \param[in] fmtMsg The format string to write to the pipe.
 * \param[in] ... The arguments to be applied to the format string.
 * 
 * \returns 0: if the write operation was successful
 * \returns -1: if the write operation failed
 */
int PIPH_WriteArg  (      PipeHandleType  pipeHandleVal,
                    const char*           fmtMsg,
                                          ...);

/** Writes to the pipe and appends a new line character or character sequence,
 * depending on the OS.
 * 
 * \param[in] pipeHandleVal The handle to the pipe.
 * \param[in] fmtMsg The format string to write to the pipe.
 * \param[in] ... The arguments to be applied to the format string.
 * 
 * \returns 0: if the write operation was successful
 * \returns -1: if the write operation failed
 */
int PIPH_WriteArgLn(      PipeHandleType  pipeHandleVal,
                    const char*           fmtMsg,
                                          ...);

/* G L O B A L   I N L I N E   F U N C T I O N   D E F I N I T I O N S * * * */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#ifdef __cplusplus
}
#endif

#endif /* PIPEHANDLING_H_INCLUDED */

/**
 * @}
 */
/* ***************************************************************************
 * E N D   O F   F I L E * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */
