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
 * \addtogroup diagnosis
 * \brief Provides functions to write log messages to a file.
 *
 * @{
 */
/* ************************************************************************* */

#ifndef DIAGNOSIS_H_INCLUDED
#define DIAGNOSIS_H_INCLUDED

/* ***************************************************************************
 * I N C L U D E   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* S Y S T E M   I N C L U D E S * * * * * * * * * * * * * * * * * * * * * * */
#include <stdbool.h>

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

typedef enum {
    DIAG_VERBOSE = 0,
    DIAG_DEBUG   = 1,
    DIAG_INFO    = 2,
    DIAG_WARNING = 3,   ///< abnormal condition that might affect the user
                        ///  experience but the overall functionality is not
                        ///  affected
    DIAG_ERROR   = 4,   ///< abnormal condition in the program flow detected so
                        ///  that some parts of the program might not be
                        ///  available anymore
    DIAG_FAILURE = 5    ///< the normal program flow can not be continued
                        ///  Program flow will be stopped right after this
                        ///  message
} DIAG_SeverityType;

/* ***************************************************************************
 * V A R I A B L E S   A N D   C O N S T A N T S   S E C T I O N * * * * * * *
 *************************************************************************** */

/* G L O B A L   V A R I A B L E   D E C L A R A T I O N S * * * * * * * * * */

/* ***************************************************************************
 * F U N C T I O N S   S E C T I O N * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* G L O B A L   F U N C T I O N   P R O T O T Y P E S * * * * * * * * * * * */

/** Closes the Diagnosis module when shutting down the program
 *
 * \returns 0: if the log file has been closed successfully
 * \returns -1: if the function failed
 */
int DIAG_Close();

/** Opens or creates a file to write log messages to.
 * 
 * \note If the log file already exists, new log messages will be appended.
 *
 * \param[in] logFilePathArg The name of the log file to be opened or created.
 * 
 * \returns 0: if the log file has been opened successfully
 * \returns -1: if the function failed
 */
int DIAG_Open(const char* logFilePathArg);

/** Returns whether a log file is open or not.
 * 
 * \returns true: if a log file is open
 * \returns false: if no log file is open
 */
bool DIAG_IsOpen();

/** Enables writes to the opened log file.
 * 
 * \returns 0: if the logging has been enabled successfully
 * \returns -1: if no log file is open and therefore the logging was not
 *              enabled
 */
int DIAG_Enable();

/** Disables writes to the opened log file.
 * 
 * \returns 0: everytime
 */
int DIAG_Disable();

/** Sets the severity level for log messages.
 * 
 * Log messages with a severity level lower than the set level will not be
 * captured.
 * 
 * \param severity The severity level to be set.
 * 
 * \returns 0: if the severity level has been set successfully
 * \returns -1: if the severity level is invalid
 */
int DIAG_SetSeverity(DIAG_SeverityType severity);

/**
 * \brief Writes a log message to the log file
 *
 * \param severity  severity of the event that shall result in a log message
 * \param module    argument to specify the module name. This is usually done
 *                  by defining a static constant string called MODULE_NAME in
 *                  the module's source file.
 * \param function  argument to specify the function name. This is usually done
 *                  by using the predefined value of __func__ as parameter.
 * \param msg       message to write to the log file (must not contain any
 *                  format characters)
 *
 * \returns 0: if the log message has been written successfully. Note that it
 *             also returns 0 if the given severity is lower than the logging
 *             severity
 * \returns -1: if an error occured
 */
int DIAG_LogMsg(DIAG_SeverityType   severity,
                const char*         module,
                const char*         function,
                const char*         msg);

/**
 * \brief Writes a log message with arguments to the log file
 *
 * \param severity  severity of the event that shall result in a log message
 * \param module    argument to specify the module name. This is usually done
 *                  by defining a static constant string called MODULE_NAME in
 *                  the module's source file.
 * \param function  argument to specify the function name. This is usually done
 *                  by using the predefined value of __func__ as parameter.
 * \param msg       message to write to the log file (must not contain more
 *                  than one format character for strings)
 * \param ...       variadic args in accordance to the format string
 *
 * \returns 0: if the log message has been written successfully. Note that it
 *             also returns 0 if the given severity is lower than the logging
 *             severity
 * \returns -1: if an error occured
 */
int DIAG_LogMsgArg(DIAG_SeverityType   severity,
                   const char*         module,
                   const char*         function,
                   const char*         fmtMsg,
                   ...);

/* G L O B A L   I N L I N E   F U N C T I O N   D E F I N I T I O N S * * * */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#ifdef __cplusplus
}
#endif

#endif /* DIAGNOSIS_H_INCLUDED */

/**
 * @}
 */
/* ***************************************************************************
 * E N D   O F   F I L E * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */
