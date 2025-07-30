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
 */
/* ************************************************************************* */

/* ***************************************************************************
 * I N C L U D E   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* P R O J E C T   I N C L U D E S * * * * * * * * * * * * * * * * * * * * * */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <limits.h>

/* M O D U L E   H E A D E R   I N C L U D E * * * * * * * * * * * * * * * * */
#include "ringbuf.h"

/* ***************************************************************************
 * D E F I N E   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* L O C A L   C O N F I G   D E F I N I T I O N S * * * * * * * * * * * * * */

/* L O C A L   M A C R O   D E F I N I T I O N S * * * * * * * * * * * * * * */

/* ***************************************************************************
 * T Y P E D E F   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* L O C A L   T Y P E D E F S * * * * * * * * * * * * * * * * * * * * * * * */

/* ***************************************************************************
 * V A R I A B L E S   A N D   C O N S T A N T S   S E C T I O N * * * * * * *
 *************************************************************************** */

/* G L O B A L   V A R I A B L E   D E F I N I T I O N S * * * * * * * * * * */

/* L O C A L   C O N S T A N T   D E F I N I T I O N S * * * * * * * * * * * */

/* L O C A L   V A R I A B L E   D E F I N I T I O N S * * * * * * * * * * * */

/* ***************************************************************************
 * F U N C T I O N S   S E C T I O N * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* L O C A L   F U N C T I O N   P R O T O T Y P E S * * * * * * * * * * * * */

/* L O C A L   I N L I N E   F U N C T I O N   D E F I N I T I O N S * * * * */

/* L O C A L   F U N C T I O N   D E F I N I T I O N S * * * * * * * * * * * */

/* G L O B A L   F U N C T I O N   D E F I N I T I O N S * * * * * * * * * * */
bool RingBuf_isEmpty(const RingBufType* const context)
{
    /* if heads equals tails, the buffer is empty */
    if (context->head == context->tail)
    {
        return true;
    }

    return false;
}

bool RingBuf_isFull(const RingBufType* const context)
{
    /* if tails is one ahead of heads, the buffer is full ... */
    if (context->tail == context->head + 1)
    {
        return true;
    }
    /* ... also consider the wrap around */
    if (   (context->head == context->bufferSize - 1)
        && (context->tail == 0))
    {
        return true;
    }

    return false;
}

void* RingBuf_getHead(const RingBufType* const context)
{
    if (RingBuf_isFull(context) == false)
    {
        return (void*)(((uint64_t)context->buffer) + context->head * context->elementSize);
    }
    return NULL;
}

void* RingBuf_getTail(const RingBufType* const context)
{
    if (RingBuf_isEmpty(context) == false)
    {
        return (void*)(((uint64_t)context->buffer) + context->tail * context->elementSize);
    }
    return NULL;
}

void* RingBuf_getTailOffset(const RingBufType* const context, size_t offset)
{
    if (RingBuf_isEmpty(context) == false)
    {
        size_t tempTail = context->tail + offset;
        tempTail %= context->bufferSize;
        if ((context->tail < context->head) && (tempTail >= context->head))
        {
            return NULL;
        }
        else if ((context->tail > context->head) && (tempTail >= context->head) && (tempTail < context->tail))
        {
            return NULL;
        }
        else
        {
            return (void*)(((uint64_t)context->buffer) + tempTail * context->elementSize);
        }
    }
    return NULL;
}

int RingBuf_increaseTail(RingBufType* const context)
{
    if (RingBuf_isEmpty(context) == false)
    {
        context->tail += 1;
        context->tail %= context->bufferSize;
        return 0;
    }
    return -1;
}

int RingBuf_increaseHead(RingBufType* const context)
{
    if (RingBuf_isFull(context) == false)
    {
        context->head += 1;
        context->head %= context->bufferSize;
        return 0;
    }
    return -1;
}

int RingBuf_increaseTailMore(RingBufType* const context, size_t more)
{
    if (more > context->bufferSize)
    {
        return -1;
    }

    if (RingBuf_isEmpty(context) == false)
    {
        context->tail += more;
        context->tail %= context->bufferSize;
        return 0;
    }
    return -1;
}

int RingBuf_increaseHeadMore(RingBufType* const context, size_t more)
{
    if (more > context->bufferSize)
    {
        return -1;
    }
    
    if (RingBuf_isFull(context) == false)
    {
        context->head += more;
        context->head %= context->bufferSize;
        return 0;
    }
    return -1;
}

size_t RingBuf_getFreeElementsHead2End(const RingBufType* const context)
{
    if (context->head < context->tail)
    {
        return context->tail - context->head - 1;
    }
    else /* if (context->head >= context->tail) */
    {
        return context->bufferSize - context->head;
    }
}

size_t RingBuf_getFullElementsTail2End(const RingBufType* const context)
{
    if (context->tail <= context->head)
    {
        return context->head - context->tail;
    }
    else /* if (context->tail > context->head) */
    {
        return context->bufferSize - context->tail;
    }
}

size_t RingBuf_getElementsCount(const RingBufType* const context)
{
    if (context->head >= context->tail)
    {
        return context->head - context->tail;
    }
    else /* if (context->head < context->tail) */
    {
        return context->bufferSize - (context->tail - context->head);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* ***************************************************************************
 * E N D   O F   F I L E * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */
