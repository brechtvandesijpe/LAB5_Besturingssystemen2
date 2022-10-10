#include "vector.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

typedef struct vector {
    void** elements;
    size_t size;
} vector_t;

vector_t* vector_create() {
    return calloc(1, sizeof(vector_t));
}

void vector_add(vector_t* vec, void* element) {
    assert(vec);
    vec->size++;
    vec->elements = realloc(vec->elements, vec->size * sizeof(*vec->elements));
    vec->elements[vec->size - 1] = element;
}

void vector_remove_at_index(vector_t* vec, size_t index) {
    assert(vec);
    assert(index < vec->size);
    if (index < vec->size - 1)
        memmove(vec->elements + index, vec->elements + index + 1, (vec->size - index - 1) * sizeof(*vec->elements));
    vec->elements[vec->size - 1] = NULL; // to help debugging
    vec->size--;
}

void* vector_at(vector_t* vec, size_t index) {
    assert(vec);
    assert(index < vec->size);
    return vec->elements[index];
}

void* vector_find(vector_t* vec, void* element_to_match, bool (*match_elements)(void*, void*)) {
    for (size_t i = 0; i < vec->size; i++) {
        void* element = vector_at(vec, i);
        if (match_elements(element, element_to_match))
            return element;
    }
    return NULL;
}

size_t vector_size(vector_t* vec) {
    assert(vec);
    return vec->size;
}

void vector_destroy(vector_t* vec) {
    assert(vec);
    free(vec->elements);
    free(vec);
}