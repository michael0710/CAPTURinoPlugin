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

#ifndef EXTCAPMNGR_H_INCLUDED
#define EXTCAPMNGR_H_INCLUDED

#include "argparser.h"

#define MAX_INTFCVAL_LENGTH 20
#define MAX_INTFCDISP_LENGTH 64
#define MAX_INTFCDLT_LENGTH 256

#define MAX_PREFC_LENGTH 512

typedef struct
{
    /** Short name of the interface used to reference it in the extcap command. */
    char val[MAX_INTFCVAL_LENGTH];

    /** Display name of the interface shown in the Wireshark GUI. */
    char disp[MAX_INTFCDISP_LENGTH];

    /** Pointer to a function that prints the dlts of the extcap interface to
     * the console.
     *
     * \param argc number of arguments the main function was called with.
     * \param argv array of arguments the main function was called with.
     * 
     * \returns 0 if the call was successful.
     * \returns -1 if the call failed.
     */
    int (*extcapDltsFunc)(int argc, char *argv[]);

    /** Pointer to a function that prints the configuration of the extcap
     * interface to the console.
     *
     * \param argc number of arguments the main function was called with.
     * \param argv array of arguments the main function was called with.
     * 
     * \returns 0 if the call was successful.
     * \returns -1 if the call failed.
     */
    int (*extcapConfigFunc)(int argc, char *argv[]);

    /** Pointer to a function that starts the capture process of the extcap
     * interface.
     *
     * \param argc number of arguments the main function was called with.
     * \param argv array of arguments the main function was called with.
     * 
     * \returns 0 if the call was successful.
     * \returns -1 if the call failed.
     */
    int (*extcapCaptureFunc)(int argc, char *argv[]);

    /** Pointer to a function that shall be when the extcap interface is going
     * to be terminated by wireshark.
     * 
     * \warning This function can only be called on Unix systems, as Wireshark
     *          uses a different mechanism on Windows to terminate the extcap
     *          interface's process.
     * 
     * \returns 0 if the call was successful.
     * \returns -1 if the call failed.
     */
    int (*extcapTerminateCb)();
} EXMG_IntfcType;

int EXMG_execute(int argc, char *argv[]);

int EXMG_terminateIntfc();

#endif // EXTCAPMNGR_H_INCLUDED
