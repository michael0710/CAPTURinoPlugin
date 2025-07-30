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
 * \addtogroup console
 * \brief Provides standard console input and output.
 *
 * @{
 */
/* ************************************************************************* */

#ifndef CONSOLE_H_INCLUDED
#define CONSOLE_H_INCLUDED

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

/* ***************************************************************************
 * V A R I A B L E S   A N D   C O N S T A N T S   S E C T I O N * * * * * * *
 *************************************************************************** */

/* G L O B A L   V A R I A B L E   D E C L A R A T I O N S * * * * * * * * * */

/* ***************************************************************************
 * F U N C T I O N S   S E C T I O N * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* G L O B A L   F U N C T I O N   P R O T O T Y P E S * * * * * * * * * * * */

/** Initializes the Console module
 *
 * \returns 0: if the initialization has been finished successfully
 * \returns -1: if the initialization failed
 */
int CNSL_Init();

/** Writes to the stdout buffer
 * 
 * \param[in] buf: the string to write to stdout.
 * \param[in] chars2write: the amount of characters to write to stdout.
 * 
 * \returns 0: if the write operation was successful
 * \returns -1: if the write operation failed
 */
int CNSL_Write(const char* buf,
               size_t      chars2write);

/** Writes to the stdout buffer and appends a new line character or character
 * sequence, depending on the OS.
 * 
 * \param[in] buf: the string to write to stdout.
 * \param[in] chars2write: the amount of characters to write to stdout.
 *
 * \returns 0: if the write operation was successful
 * \returns -1: if the write operation failed
 */
int CNSL_WriteLn(const char* buf,
                 size_t      chars2write);

/** Writes a formatted string to the stdout buffer.
 *
 * \param[in] fmtMsg The format string to write to stdout.
 * \param[in] ... The arguments to be applied to the format string.
 *
 * \returns 0: if the write operation was successful
 * \returns -1: if the write operation failed
 */
int CNSL_WriteArg(const char* fmtMsg,
                  ...);

/** Writes a formatted string to the stdout buffer and appends a new line
 * character or character sequence, depending on the OS.
 *
 * \param[in] fmtMsg The format string to write to stdout.
 * \param[in] ... The arguments to be applied to the format string.
 *
 * \returns 0: if the write operation was successful
 * \returns -1: if the write operation failed
 */
int CNSL_WriteArgLn(const char* fmtMsg,
                    ...);

/** Writes to the stderr buffer.
 * 
 * \param[in] buf: the string to write to stdout.
 * \param[in] chars2write: the amount of characters to write to stdout.
 *
 * \returns 0: if the write operation was successful
 * \returns -1: if the write operation failed
 */
int CNSL_WriteErr(const char* buf,
                  size_t      chars2write);

/* G L O B A L   I N L I N E   F U N C T I O N   D E F I N I T I O N S * * * */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#ifdef __cplusplus
}
#endif

#endif /* CONSOLE_H_INCLUDED */

/**
 * @}
 */
/* ***************************************************************************
 * E N D   O F   F I L E * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */
