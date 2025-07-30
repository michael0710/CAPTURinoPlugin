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
 * \addtogroup ringbuf
 * 
 * @{
 */
/* ************************************************************************* */

#ifndef RINGBUF_H_INCLUDED
#define RINGBUF_H_INCLUDED

/* ***************************************************************************
 * I N C L U D E   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* P R O J E C T   I N C L U D E S * * * * * * * * * * * * * * * * * * * * * */
#include <stddef.h>
#include <stdint.h>

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
typedef struct
{
    size_t head;                /**< Next free location where another element
                                     can be stored. */
    size_t tail;                /**< Location of the last valid element in the
                                     buffer. */
    const size_t elementSize;   /**< Size of one element in bytes. */
    void* buffer;               /**< Pointer to the buffer. */
    const size_t bufferSize;    /**< Number of elements, each of size
                                     elementSize which can be stored in the
                                     buffer. */
} RingBufType;

/* ***************************************************************************
 * V A R I A B L E S   A N D   C O N S T A N T S   S E C T I O N * * * * * * *
 *************************************************************************** */

/* G L O B A L   C O N S T A N T S   D E C L A R A T I O N S * * * * * * * * */

/* G L O B A L   V A R I A B L E   D E C L A R A T I O N S * * * * * * * * * */

/* ***************************************************************************
 * F U N C T I O N S   S E C T I O N * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* G L O B A L   F U N C T I O N   P R O T O T Y P E S * * * * * * * * * * * */
/** Checks if the given buffer is empty.
 * 
 * \param[in] context The ring buffer to be checked.
 * 
 * \returns true: if the buffer is empty.
 * \returns false: if the buffer is not empty.
 */
bool RingBuf_isEmpty(const RingBufType* const context);

/** Checks if the given buffer is full.
 * 
 * \param[in] context The ring buffer to be checked.
 * 
 * \returns true: if the buffer is full.
 * \returns false: if the buffer is not full.
 */
bool RingBuf_isFull(const RingBufType* const context);

/** Retrieves the pointer to the head element of the buffer.
 * 
 * \param[in] context The ring buffer to get the pointer from.
 * 
 * \returns a valid address if the buffer is not full.
 * \returns NULL if the buffer is full.
 */
void* RingBuf_getHead(const RingBufType* const context);

/** Retrieves the pointer to the tail element of the buffer.
 * 
 * \param[in] context The ring buffer to get the pointer from.
 * 
 * \returns a valid address if the buffer is not empty.
 * \returns NULL if the buffer is empty.
 */
void* RingBuf_getTail(const RingBufType* const context);

/** Retrieves the pointer to the tail+offset element of the buffer.
 * 
 * \param[in] context The ring buffer to get the pointer from.
 * \param[in] offset The offset to the tail element.
 * 
 * \note a offset of 0 is equal to a call to RingBuf_getTail().
 * 
 * \returns a valid address if the buffer is contains at least 'offset'
 *          elements.
 * \returns NULL if the requested position does not contain valid data.
 */
void* RingBuf_getTailOffset(const RingBufType* const context, size_t offset);

/** Increases the tail pointer by one.
 * 
 * \param[in] context The ring buffer to be modified.
 * 
 * \returns 0: if the operation was successful.
 * \returns -1: if the operation failed.
 */
int RingBuf_increaseTail(RingBufType* const context);

/** Increases the head pointer by one.
 * 
 * \param[in] context The ring buffer to be modified.
 * 
 * \returns 0: if the operation was successful.
 * \returns -1: if the operation failed.
 */
int RingBuf_increaseHead(RingBufType* const context);

/** Increases the tail pointer by 'more'.
 * 
 * \param[in] context The ring buffer to be modified.
 * \param[in] more The number of elements to increase the tail pointer by.
 * 
 * \returns 0: if the operation was successful.
 * \returns -1: if the operation failed.
 */
int RingBuf_increaseTailMore(RingBufType* const context, size_t more);

/** Increases the head pointer by 'more'.
 * 
 * \param[in] context The ring buffer to be modified.
 * \param[in] more The number of elements to increase the head pointer by.
 * 
 * \returns 0: if the operation was successful.
 * \returns -1: if the operation failed.
 */
int RingBuf_increaseHeadMore(RingBufType* const context, size_t more);

/** Retrieves the number of free elements from the head to the end of the buffer.
 * 
 * \param[in] context The ring buffer to be checked.
 * 
 * \returns The number of free elements.
 */
size_t RingBuf_getFreeElementsHead2End(const RingBufType* const context);

/** Retrieves the number of full elements from the tail to the end of the buffer.
 * 
 * \param[in] context The ring buffer to be checked.
 * 
 * \returns The number of full elements.
 */
size_t RingBuf_getFullElementsTail2End(const RingBufType* const context);

/** Retrieves the number of elements in the buffer.
 * 
 * \param[in] context The ring buffer to be checked.
 * 
 * \returns The number of elements in the buffer.
 */
size_t RingBuf_getElementsCount(const RingBufType* const context);

/* G L O B A L   E X T E R N A L   F U N C T I O N   P R O T O T Y P E S * * */

/* G L O B A L   I N L I N E   F U N C T I O N   D E F I N I T I O N S * * * */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#ifdef __cplusplus
}
#endif

#endif /* RINGBUF_H_INCLUDED */

/**
 * @}
 */
/* ***************************************************************************
 * E N D   O F   F I L E * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */
