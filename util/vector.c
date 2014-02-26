#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#include "vector.h"

struct vector_t {
    const void **data;
    size_t size;
    size_t num;
};

vector_t *vec_ini(size_t size) {
    vector_t *vec = (vector_t *)malloc(sizeof (vector_t));
    if (vec == NULL) {
        return NULL;
    }
    if (size == 0) {
        size = 11;
    }
    vec->data = (const void **)calloc(size, sizeof (void *));
    if (vec->data == NULL) {
        free(vec);
        return NULL;
    }
    vec->size = size;
    vec->num = 0;

    return vec;
}

void vec_fini(vector_t *vec) {
    if (vec == NULL) {
        return;
    }
    assert(vec->data != NULL);

    free(vec->data);
    vec->data = NULL;

    free(vec);
}

size_t vec_size(vector_t *vec) {
    if (vec == NULL) {
        return 0;
    }
    assert(vec->data != NULL);

    return vec->size;
}

size_t vec_num(vector_t *vec) {
    if (vec == NULL) {
        return 0;
    }
    assert(vec->data != NULL);

    return vec->num;
}

const void *vec_get(vector_t *vec, size_t offset) {
    if (vec == NULL || offset >= vec->num) {
        return NULL;
    }
    assert(vec->data != NULL);

    return vec->data[offset];
}

static size_t grow(vector_t *vec, size_t size) {
    if (vec->size >= size) {
        if (vec->num > size) {
            vec->num = size;
        }
        return vec->size;
    }

    size_t nsize = vec->size;
    while (nsize < size) {
        nsize *= 2;
    }

    const void **n = (const void **)realloc(vec->data, nsize * sizeof (void *));
    if (n == NULL) {
        return vec->size;
    }
    /* 默认值 */
    memset((void *)n + vec->size, 0, nsize - vec->size);

    vec->size = nsize;
    free(vec->data);
    vec->data = n;

    return nsize;
}

size_t vec_set(vector_t *vec, size_t offset, const void *value) {
    if (vec == NULL) {
        return 0;
    }
    assert(vec->data != NULL);

    if (offset >= vec->size) {
        if (grow(vec, offset + 1) < offset + 1) {
            return 0;
        }
    }

    if (offset >= vec->num) {
        vec->num = offset + 1;
    }
    vec->data[offset] = value;

    return 1;
}

const void *vec_erase(vector_t *vec, size_t offset) {
    if (vec == NULL || offset >= vec->num) {
        return NULL;
    }
    assert(vec->data != NULL);

    const void *v = vec->data[offset];
    size_t move_num = vec->num - (offset + 1);
    if (move_num > 0) {
        memmove(vec->data + offset, vec->data + offset + 1, move_num * sizeof (void *));
    }
    vec->num--;

    return v;
}

const void **vec_data(vector_t *vec) {
    if (vec == NULL) {
        return NULL;
    }

    return vec->data;
}

size_t vec_push(vector_t *vec, const void *value) {
    return vec_set(vec, vec->num, value);
}

const void *vec_pop(vector_t *vec) {
    return vec_erase(vec, vec->num - 1);
}

const void *vec_front(vector_t *vec) {
    return vec_get(vec, 0);
}

const void *vec_back(vector_t *vec) {
    return vec_get(vec, vec->num - 1);
}

size_t vec_append_private(vector_t *vec, ...) {
    if (vec == NULL) {
        return 0;
    }

    size_t num = 0;
    va_list args;
    va_start(args, vec);
    const void *v = NULL;
    while ((v = va_arg(args, const void *)) != NULL) {
        if (vec_push(vec, v) == 1) {
            num++;
        }
    }
    va_end(args);

    return num;
}
