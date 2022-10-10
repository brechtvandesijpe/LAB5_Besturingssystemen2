/**
 * \author Mathieu Erbas
 */

#pragma once

#ifndef _GNU_SOURCE
    #define _GNU_SOURCE
#endif

#include <stdbool.h>

/**
 * dplist_t is a struct containing at least a head pointer to the start of the list;
 */
typedef struct dplist dplist_t;

typedef struct dplist_node dplist_node_t;

/* General remark on error handling
 * All functions below will:
 * - use assert() to check if memory allocation was successfully.
 */

/** Create and allocate memory for a new list
 * \param element_copy callback function to duplicate 'element'; If needed allocated new memory for the duplicated element.
 * \param element_free callback function to free memory allocated to element
 * \param element_compare callback function to compare two element elements; returns -1 if x<y, 0 if x==y, or 1 if x>y
 * \return a pointer to a newly-allocated and initialized list.
 */
dplist_t* dpl_create(void* (*element_copy)(void* element), void (*element_free)(void** element), int (*element_compare)(void* x, void* y));

/** Deletes all elements in the list
 * - Every list node of the list needs to be deleted. (free memory)
 * - The list itself also needs to be deleted. (free all memory)
 * - '*list' must be set to NULL.
 * \param list a double pointer to the list
 * \param free_element if true call element_free() on the element of the list node to remove
 */
void dpl_free(dplist_t** list, bool free_element);

/** Returns the number of elements in the list.
 * - If 'list' is is NULL, -1 is returned.
 * \param list a pointer to the list
 * \return the size of the list
 */
int dpl_size(dplist_t* list);

/** Inserts a new list node containing an 'element' in the list at position 'index'
 * - the first list node has index 0.
 * - If 'index' is 0 or negative, the list node is inserted at the start of 'list'.
 * - If 'index' is bigger than the number of elements in the list, the list node is inserted at the end of the list.
 * - If 'list' is is NULL, NULL is returned.
 * \param list a pointer to the list
 * \param element a pointer to the data that needs to be inserted
 * \param index the position at which the element should be inserted in the list
 * \param insert_copy if true use element_copy() to make a copy of 'element' and use the copy in the new list node, otherwise the given element pointer is added to the list
 */
void dpl_insert_at_index(dplist_t* list, void* element, int index, bool insert_copy);

void dpl_print(dplist_t* list, void (*element_print)(void* elem));

typedef struct {
    dplist_node_t* const node;
    const int index;
} iterator_t;

void* dpl_element_match(dplist_t* list, void* element);

iterator_t it_null();

iterator_t it_begin(dplist_t* list);

void* it_get_element(iterator_t* it);

bool it_is_null(iterator_t* it);

void it_incr(iterator_t*);

void dpl_remove_at_iterator(dplist_t* list, iterator_t* it, bool free_element);

void it_invalidate(iterator_t* it);

#define DPL_IT_NULL (it_null())
