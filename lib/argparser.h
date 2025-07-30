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
 * \addtogroup argparser
 * 
 * @{
 */
/* ************************************************************************* */

#ifndef ARGPARSER_H_INCLUDED
#define ARGPARSER_H_INCLUDED

/* ***************************************************************************
 * I N C L U D E   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* P R O J E C T   I N C L U D E S * * * * * * * * * * * * * * * * * * * * * */
#include <stdbool.h>

/* E X T E R N   C   D E C L A R A T I O N * * * * * * * * * * * * * * * * * */
#ifdef __cplusplus
extern "C"
{
#endif

/* ***************************************************************************
 * D E F I N E   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

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
/** Checks if a given key is available in the argument list.
 * 
 * \param[in] argc the number of arguments in the argument list.
 * \param[in] argv the argument list.
 * \param[in] key  the key to be searched for.
 * \param[out] isAvailable a pointer to a boolean that is set to true if the
 *                         key was found in the argument list.
 * 
 * \returns 0: everytime.
 */
int ARGP_constainsKey(int argc, char *argv[], const char* key, bool* isAvailable);

/** Checks if a given key value pair is available in the argument list where
 * the value is of type long integer.
 * 
 * \param[in] argc the number of arguments in the argument list.
 * \param[in] argv the argument list.
 * \param[in] key the key to be searched for.
 * \param[out] value a pointer to a variable to store the value.
 *
 * \returns 0: if the key was found and the value was successfully converted to
 *             a long integer.
 * \returns -1: if the function failed.
 */
int ARGP_getLongOfArgs(int argc, char *argv[], const char* key, long* value);

/** Checks if a given key value pair is available in the argument list where
 * the value is of type unsigned long integer.
 * 
 * \param[in] argc the number of arguments in the argument list.
 * \param[in] argv the argument list.
 * \param[in] key the key to be searched for.
 * \param[out] value a pointer to a variable to store the value.
 *
 * \returns 0: if the key was found and the value was successfully converted to
 *             a long integer.
 * \returns -1: if the function failed.
 */
int ARGP_getUnsignedLongOfArgs(int argc, char *argv[], const char* key, unsigned long* value);

/** Checks if a given key value pair is available in the argument list. The
 * pointer which points to the start of the value string is stored in the
 * value parameter.
 * 
 * \param[in] argc the number of arguments in the argument list.
 * \param[in] argv the argument list.
 * \param[in] key the key to be searched for.
 * \param[out] value a pointer to a pointer variable to store the value.
 *
 * \returns 0: if the key was found and the pointer to the value was
 *             successfully retrieved.
 * \returns -1: if the function failed.
 */
int ARGP_getP2StringOfArgs(int argc, char *argv[], const char* key, char** value);

/* G L O B A L   I N L I N E   F U N C T I O N   D E F I N I T I O N S * * * */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#ifdef __cplusplus
}
#endif

#endif /* ARGPARSER_H_INCLUDED */

/**
 * @}
 */
/* ***************************************************************************
 * E N D   O F   F I L E * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */
