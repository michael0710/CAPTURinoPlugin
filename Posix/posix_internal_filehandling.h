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

#ifndef POSIX_INTERNAL_FILEHANDLING_H_INCLUDED
#define POSIX_INTERNAL_FILEHANDLING_H_INCLUDED

/* ***************************************************************************
 * I N C L U D E   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* S Y S T E M   I N C L U D E S * * * * * * * * * * * * * * * * * * * * * * */
#include <stdarg.h>
#include <stddef.h>

/* P R O J E C T   I N C L U D E S * * * * * * * * * * * * * * * * * * * * * */

/* ***************************************************************************
 * D E F I N E   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* G L O B A L   C O N F I G   D E F I N I T I O N S * * * * * * * * * * * * */

/* G L O B A L   M A C R O   D E F I N I T I O N S * * * * * * * * * * * * * */

/* ***************************************************************************
 * T Y P E D E F   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* G L O B A L   T Y P E D E F S * * * * * * * * * * * * * * * * * * * * * * */
typedef unsigned int FileHandleType;

/* ***************************************************************************
 * V A R I A B L E S   A N D   C O N S T A N T S   S E C T I O N * * * * * * *
 *************************************************************************** */

/* G L O B A L   V A R I A B L E   D E C L A R A T I O N S * * * * * * * * * */
extern const FileHandleType INVALID_FILE_HANDLE;

/* ***************************************************************************
 * F U N C T I O N S   S E C T I O N * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* G L O B A L   F U N C T I O N   P R O T O T Y P E S * * * * * * * * * * * */

/** Opens a file.
 * 
 * \details write accesses to the opened file may only possible via this module
 *          by calling the specific function with the FileHandleType returned
 *          by this function.
 * 
 * \param path [in] path to the file in the underlying filesystem.
 * \param mode [in] mode to open the file.
 * \param fileHandleVal [out] a handle to the opened file needed for furhter
 *                            interactions with the file.
 * 
 * \note see the documentation for fopen() for detailed information about the
 *       mode argument.
 * 
 * \see https://pubs.opengroup.org/onlinepubs/9799919799/functions/fopen.html
 * 
 * \returns 0: if the initialization has been finished successfully.
 * \returns -1: if the initialization failed.
 */
int FILH_Open        (const char*           restrict  path,
                            int                       oflag,
                            FileHandleType* restrict  fileHandleVal);

/** Closes a pipe
 *
 */
int FILH_Close       (      FileHandleType  fileHandleVal);

/** Writes to the pipe
 * 
 * \returns 0: if the write operation was successful
 * \returns -1: if the write operation failed
 */
int FILH_Write       (      FileHandleType  fileHandleVal,
                      const char*           buf,
                            size_t          chars2write);

/**
 * 
 * \note this function calls the posix function read(). If this function fails,
 *       the error cause can be determined by reading the errno variable.
 */
int FILH_Read        (      FileHandleType  fileHandleVal,
                            char*           buf,
                            size_t          chars2read,
                            size_t*         charsRead);

/** Writes to the pipe and appends a new line character or character sequence,
 * depending on the OS
 * 
 * \todo needs to be implemented
 */
int FILH_WriteLn     (      FileHandleType  fileHandleVal,
                      const char*           buf,
                            size_t          chars2write);


int FILH_WriteArg    (      FileHandleType  fileHandleVal,
                      const char*           fmtMsg,
                                            ...);

int FILH_WriteArgLn  (      FileHandleType  fileHandleVal,
                      const char*           fmtMsg,
                                            ...);

int FILH_vaWriteArg  (      FileHandleType  fileHandleVal,
                      const char*           fmtMsg,
                            va_list         args);

int FILH_vaWriteArgLn(      FileHandleType  fileHandleVal,
                      const char*           fmtMsg,
                            va_list         args);

/* G L O B A L   I N L I N E   F U N C T I O N   D E F I N I T I O N S * * * */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#endif /* POSIX_INTERNAL_FILEHANDLING_H_INCLUDED */

/* ***************************************************************************
 * E N D   O F   F I L E * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */
