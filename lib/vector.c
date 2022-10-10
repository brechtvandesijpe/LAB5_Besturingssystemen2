
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
    vec->elements = realloc(vec->elements, vec->size);
    vec->elements[vec->size - 1] = element;
}

void vector_remove_at_index(vector_t* vec, size_t index) {
    assert(vec);
    assert(index < vec->size);
    memmove(vec->elements + index, vec->elements + index + 1, (vec->size - index) * sizeof(*vec->elements));
}

void** vector_elements(vector_t* vec) {
    assert(vec);
    return vec->elements;
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