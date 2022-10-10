/**
 * \author Mathieu Erbas
 */

#include "dplist.h"

#include <assert.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef DEBUG
    #define DEBUG_PRINTF(...)                                                         \
        do {                                                                          \
            fprintf(stderr, "\nIn %s - function %s at line %d: ", __FILE__, __func__, \
                    __LINE__);                                                        \
            fprintf(stderr, __VA_ARGS__);                                             \
            fflush(stderr);                                                           \
        } while (0)
#else
    #define DEBUG_PRINTF(...) (void) 0
#endif

#define DPLIST_ERR_HANDLER(condition, err_code)   \
    do {                                          \
        if ((condition))                          \
            DEBUG_PRINTF(#condition " failed\n"); \
        assert(!(condition));                     \
    } while (0)

#define NODE_HANDLE_NULL ((node_handle){.node = NULL, .index = -1})
/*
 * The real definition of struct list / struct node
 */

struct dplist_node {
    dplist_node_t *prev, *next;
    void* element;
};

struct dplist {
    dplist_node_t* head;
    void* (*element_copy)(void* src_element);
    void (*element_free)(void** element);
    int (*element_compare)(void* x, void* y);
    int size;
};

typedef struct {
    dplist_node_t* node;
    int index;
} node_handle;

dplist_t* dpl_create(void* (*element_copy)(void* src_element),
                     void (*element_free)(void** element),
                     int (*element_compare)(void* x, void* y)) {
    dplist_t* list;
    list = malloc(sizeof(struct dplist));
    DPLIST_ERR_HANDLER(list == NULL, DPLIST_MEMORY_ERROR);
    list->head = NULL;
    list->element_copy = element_copy;
    list->element_free = element_free;
    list->element_compare = element_compare;
    list->size = 0;
    return list;
}

static node_handle dpl_get_if(dplist_t* list, bool (*func)(dplist_t*, node_handle, void*), void* arg) {
    assert(list != NULL);

    dplist_node_t* current = list->head;
    for (int index = 0; current != NULL; index++) {
        // cache `next` up front if `func` removes `current`
        dplist_node_t* next = current->next;
        node_handle handle = {.node = current, .index = index};
        if (func(list, handle, arg))
            return handle;
        current = next;
    }

    return NODE_HANDLE_NULL;
}

void dpl_free(dplist_t** list, bool free_element) {
    assert(list && *list);

    while ((*list)->head != NULL) {
        dplist_node_t* temp = (*list)->head;
        if (free_element) {
            (*list)->element_free(&(temp->element));
        } else {
            free(temp->element);
        }
        (*list)->head = temp->next;
        free(temp);
    }
    free(*list);
    *list = NULL;
}

static void dpl_insert_node_after(dplist_t* list, dplist_node_t* reference, void* element, bool copy) {
    assert(list != NULL);
    dplist_node_t* node = malloc(sizeof(dplist_node_t));
    node->element = copy ? list->element_copy(element) : element;
    node->prev = reference;
    node->next = reference ? reference->next : list->head;

    // modify list
    if (reference == NULL) {
        if (list->head != NULL)
            list->head->prev = node; // right-side assignment
        list->head = node;           // left-side
    } else {
        if (reference->next != NULL) {
            reference->next->prev = node; // right-side
        }
        reference->next = node; // left-side
    }
    list->size++;
}

static bool has_index_or_last(dplist_t* list, node_handle handle, void* arg) {
    (void) list;
    int argument = *((int*) arg);
    return (handle.index == argument || handle.node->next == NULL);
}

void dpl_insert_at_index(dplist_t* list, void* element, int index, bool insert_copy) {
    assert(list);
    int index_plus_one = index + 1;
    node_handle handle = index <= 0 ? NODE_HANDLE_NULL : dpl_get_if(list, has_index_or_last, &index_plus_one);
    dpl_insert_node_after(list, handle.node, element, insert_copy);
}

static void dpl_remove_node(dplist_t* list, dplist_node_t* const node, bool free_element) {
    assert(list != NULL && node != NULL);

    if (node->next != NULL)
        node->next->prev = node->prev;

    if (node->prev != NULL) {
        node->prev->next = node->next;
    } else {
        (list->head = node->next);
    }

    if (free_element == true) {
        list->element_free(&(node->element));
    } else {
        node->element = NULL;
        free(node->element);
    }
    list->size--;
    free(node);
}

int dpl_size(dplist_t* list) {
    assert(list);
    return list->size;
}

void dpl_print(dplist_t* list, void (*element_print)(void* elem)) {
    printf("Printing the list:\n");
    int i = 0;
    for (dplist_node_t* node = list->head; node != NULL; node = node->next) {
        printf("%d: ", i);
        element_print(node->element);
        i++;
    }
}

static bool element_is_equal(dplist_t* list, node_handle handle, void* arg) {
    return (!list->element_compare(handle.node->element, arg));
}

void* dpl_element_match(dplist_t* list, void* element) {
    if (list == NULL)
        return NULL;
    node_handle handle = dpl_get_if(list, element_is_equal, element);
    if (handle.node == NULL)
        return NULL;
    return handle.node->element;
}

iterator_t it_null() {
    return (iterator_t){.node = NULL, .index = -1};
}

iterator_t* it_assign(iterator_t* it, iterator_t value) {
    memcpy(it, &value, sizeof(value));
    return it;
}

void it_incr(iterator_t* it) {
    if (it_is_null(it))
        return;
    it_assign(it, it->node->next ? (iterator_t){.node = it->node->next, .index = it->index + 1} : DPL_IT_NULL);
}

iterator_t it_begin(dplist_t* list) {
    if (list->head == NULL)
        return DPL_IT_NULL;
    return (iterator_t){.node = list->head, .index = 0};
}

void* it_get_element(iterator_t* it) {
    assert(!it_is_null(it));
    return it->node->element;
}

bool it_is_null(iterator_t* it) {
    return it->node == DPL_IT_NULL.node && it->index == DPL_IT_NULL.index;
}
void dpl_remove_at_iterator(dplist_t* list, iterator_t* it, bool free_element) {
    assert(!it_is_null(it));
    dpl_remove_node(list, it->node, free_element);
    it_invalidate(it);
}

void it_invalidate(iterator_t* it) {
    it_assign(it, DPL_IT_NULL);
}
