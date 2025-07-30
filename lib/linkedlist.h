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
 *
 * @{
 */
/* ************************************************************************* */

#ifndef LINKEDLIST_H_INCLUDED
#define LINKEDLIST_H_INCLUDED

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
/** Type definition that represents a single element in a linked list.
 * 
 * \note this is an anonymous typedef so that it can be used within the struct
 *       definition.
 */
typedef struct LLST_strListEntryType LLST_ListEntryType;

/** Structure that represents a single element in a linked list.
 * 
 * \var unsigned int $id used to identify the element in the list.
 * \var void* $data a pointer to the data that the element refers to.
 * \var LLST_ListEntryType* $next a pointer to the next element in the list.
 */
struct LLST_strListEntryType {
    unsigned int        id;
    void*               data;
    LLST_ListEntryType* next;
};

/* ***************************************************************************
 * V A R I A B L E S   A N D   C O N S T A N T S   S E C T I O N * * * * * * *
 *************************************************************************** */

/* G L O B A L   V A R I A B L E   D E C L A R A T I O N S * * * * * * * * * */

/* ***************************************************************************
 * F U N C T I O N S   S E C T I O N * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */

/* G L O B A L   F U N C T I O N   P R O T O T Y P E S * * * * * * * * * * * */
/** Reserves memory for a new element of the type ListEntryType and assigns the
 * given parameters (data, id) to the structure.
 *
 * \note this function does neither allocate memory for the data itself nor
 *       copies it the content to any reserved memory. This memory must be
 *       handeled by any overlaying application.
 *
 * \param[out] newElem a pointer to the structure, which was created within the
 *                     function call.
 * \param[in] data a pointer to some data, that the new list entry shall
 *                 refer to.
 * \param[in] id an unsigned integer that shall be assigned to the new list
 *               entry. Note that this should be an unique number, otherwise
 *               some objects in the list might not be directly accessible by
 *               the provided functions.
 *
 * \returns 0: if the function was executed successfully and a new element has
 *             been created.
 * \returns -1: if the function failed.
 *
 */
int  LLST_create_elem         (      LLST_ListEntryType**      newElem,
                                     void*                     data,
                                     unsigned int              id);

/** Frees the memory allocated by a ListEntryType that was created with
 * create_elem().
 * 
 * \param[in] elem a pointer to the structure, whose memory shall be freed.
 *
 * \returns 0: everytime.
 */
int  LLST_delete_elem         (      LLST_ListEntryType*       elem);

/** Returns wheather or not the given list entry has a next element it points
 * to.
 *
 * \param[in] listEntry a pointer to the structure that shall be checked for a
 *                      next element.
 *
 * \returns true: if the list entry has a next item.
 * \returns false: if the list has no next item.
 */
bool LLST_has_next            (const LLST_ListEntryType* const listEntry);

/** Retrieves the element with the matching id from a given list.
 *
 * \param[in] listStartEntry a pointer to the first element of a list.
 * \param[out] elem a pointer where the found element is written to.
 * \param[in] id the id to be searched for in the list.
 *
 * \returns 0: if the function was executed successfully and an element was
 *             returned.
 * \returns -1: if the function failed.
 */
int  LLST_get_elem_with_id    (      LLST_ListEntryType*       listStartEntry,
                                     LLST_ListEntryType**      elem,
                                     unsigned int              id);

/** Adds an element at the end of the given list.
 * 
 * \note This function does not allocate any memory.
 *
 * \param[in] listStartEntry a pointer to the first element of a list.
 * \param[in] elem a pointer that refers to the new element to be added to the
 *                 list.
 *
 * \returns 0: if the function was executed successfully.
 * \returns -1: if the function failed.
 */
int  LLST_add_elem            (      LLST_ListEntryType*       listStartEntry,
                                     LLST_ListEntryType*       elem);
              
/** Removese the element with the matching id from a given list.
 *
 * \param[in] listStartEntry a pointer to the first element of a list.
 * \param[out] newListStartEntry the pointer with the new start of the list
 *                               after the element was removed.
 * \param[in] id the id to be removed from the list.
 *
 * \note in case that the fist element is removed, the newListStartEntry must
 *       be used for further accesses to the list. In any other case, this is
 *       the identical pointer as the parameter given in listStartEntry.
 *
 * \returns 0: if the function was executed successfully and an element was
 *             removed.
 * \returns -1: if the function failed.
 */         
int  LLST_remove_elem_with_id (      LLST_ListEntryType*       listStartEntry,
                                     LLST_ListEntryType**      newListStartEntry,
                                     unsigned int              id);

/* G L O B A L   I N L I N E   F U N C T I O N   D E F I N I T I O N S * * * */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#ifdef __cplusplus
}
#endif

#endif /* LINKEDLIST_H_INCLUDED */

/**
 * @}
 */
/* ***************************************************************************
 * E N D   O F   F I L E * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *************************************************************************** */
