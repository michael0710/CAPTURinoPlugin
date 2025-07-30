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
 * \addtogroup linkedlist
 */
/* ************************************************************************* */

/* ***************************************************************************
 * I N C L U D E   S E C T I O N * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* P R O J E C T   I N C L U D E S * * * * * * * * * * * * * * * * * * * * * */
#include <stdlib.h>
#include <stdbool.h>

/* M O D U L E   H E A D E R   I N C L U D E * * * * * * * * * * * * * * * * */
#include "linkedlist.h"

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
int LLST_create_elem(LLST_ListEntryType** newElem, void* data, unsigned int id)
{
    LLST_ListEntryType* createdElem = (LLST_ListEntryType*)malloc(sizeof(LLST_ListEntryType));
    if (createdElem == NULL)
    {
        *newElem = NULL;
        return -1;
    }
    
    createdElem->data = data;
    createdElem->id = id;
    createdElem->next = NULL;
    *newElem = createdElem;
    return 0;
}

int LLST_delete_elem(LLST_ListEntryType* elem)
{
    free(elem);
    return 0;
}

bool LLST_has_next(const LLST_ListEntryType* const listEntry)
{
    return (listEntry->next != NULL);
}

int LLST_get_elem_with_id(LLST_ListEntryType* listStartEntry, LLST_ListEntryType** elem, unsigned int id)
{
    LLST_ListEntryType* listIterator = listStartEntry;
    
    while (listIterator != NULL)
    {
        if (listIterator->id == id)
        {
            *elem = listIterator;
            return 0;
        }
        listIterator = listIterator->next;
    }
    return -1;
}

int LLST_add_elem(LLST_ListEntryType* listStartEntry, LLST_ListEntryType* elem)
{
    LLST_ListEntryType* listIterator = listStartEntry;
    
    while (LLST_has_next(listIterator))
    {
        listIterator = listIterator->next;
    }
    
    listIterator->next = elem;
    
    return 0;
}

/** Removes the first element with the given id from the list without freeing
 * its memory.
 * \warning An application needs to make sure to hold a certain reference to
 *          the allocated memory before calling this function!
 */
int LLST_remove_elem_with_id(LLST_ListEntryType* listStartEntry, LLST_ListEntryType** newListStartEntry, unsigned int id)
{
    if (listStartEntry->id == id)
    {
        *newListStartEntry = listStartEntry->next;
        return 0;
    }
    else
    {
        *newListStartEntry = listStartEntry;
    }
    
    LLST_ListEntryType* prevEntry = listStartEntry;
    LLST_ListEntryType* currentEntry = listStartEntry->next;
    
    while (currentEntry != NULL)
    {
        if (currentEntry->id == id)
        {
            prevEntry->next = currentEntry->next;
            return 0;
        }
        currentEntry = currentEntry->next;
    }
    
    return -1;
}

#if 0
int get_element_count(const LLST_ListEntryType* const firstListEntry)
{
    LLST_ListEntryType* listEntry = firstListEntry;
    int listElementCount = 0;
    while (LLST_has_next(listEntry))
    {
        listEntry = listEntry->next;
        listElementCount++;
    }
    return listElementCount;
}

LLST_ListEntryType* get_last_element(const LLST_ListEntryType* const firstListEntry)
{
    LLST_ListEntryType* listEntry = firstListEntry;
    while (LLST_has_next(listEntry))
    {
        listEntry = listEntry->next;
    }
    return listEntry;
}

/** \todo this function shall return a status value so that it can fail if the list is not big enough */
LLST_ListEntryType* get_element_at(const LLST_ListEntryType* const firstListEntry,
                                         int                       pos)
{
    LLST_ListEntryType* listEntry = firstListEntry;
    for (int i=0; i<pos; i++)
    {
        if (LLST_has_next(listEntry) == false)
        {
            break;
        }
        listEntry = listEntry->next;
    }
    
    return listEntry;
}
#endif

/* ************************************************************************* */

/* ***************************************************************************
 * E N D   O F   F I L E * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */
