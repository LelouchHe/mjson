#include <stdlib.h>
#include <assert.h>

#include "util/ref_str.h"
#include "util/map.h"
#include "util/vector.h"

#include "mjson_type.h"

#define MJSON_INI_FUN(type_pre, TYPE_T)                                 \
mjson_value_t *type_pre##_ini() {                                       \
    type_pre##_t *v = (type_pre##_t *)calloc(1, sizeof (type_pre##_t)); \
    if (v == NULL) {                                                    \
        return NULL;                                                    \
    }                                                                   \
    v->h.type = TYPE_T;                                                 \
    return &v.h;                                                        \
}

#define MJSON_BASIC_FINI_FUN(type_pre, TYPE_T)      \
void type_pre##_fini(mjson_value_t *mv) {           \
    if (mv == NULL || mv->type != TYPE_T) {         \
        return;                                     \
    }                                               \
                                                    \
    if (mv->text != NULL) {                         \
        if (mv->is_str) {                           \
            free(mv->text);                         \
        } else {                                    \
            rs_fini(mv->text);                      \
        }                                           \
        mv->text = NULL;                            \
    }                                               \
    free(mv);                                       \
}

#define MJSON_STATIC_FINI_FUN(type_pre, TYPE_T)     \
void type_pre##_fini(mjson_value_t *mv) {           \
    if (mv == NULL || mv->type != TYPE_T) {         \
        return;                                     \
    }                                               \
    mv->text = NULL;                                \
    free(mv);                                       \
}

static mjson_bool_t true_value = {{MJSON_TRUE, 0, 0, 1, (ref_str_t *)"true"}};
static mjson_bool_t false_value = {{MJSON_FALSE, 0, 0, 1, (ref_str_t *)"false"}};
static mjson_null_t null_value = {{MJSON_NULL, 0, 0, 1, (ref_str_t *)"null"}};

MJSON_INI_FUN(mjson_object, MJSON_OBJECT)
MJSON_INI_FUN(mjson_array, MJSON_ARRAY)
MJSON_INI_FUN(mjson_str, MJSON_STRING)
MJSON_INI_FUN(mjson_int, MJSON_INTEGER)
MJSON_INI_FUN(mjson_double, MJSON_DOUBLE)

mjson_value_t *mjson_true_ini() {
    return &true_value.h;
}

mjson_value_t *mjson_false_ini() {
    return &false_value.h;
}

mjson_value_t *mjson_null_ini() {
    return &null_value.h;
}

void mjson_object_fini(mjson_value_t *mv) {
    if (mv == NULL || mv->type != MJSON_OBJECT) {
        return;
    }
    if (mv->text != NULL) {
        rs_fini(mv->text);
        mv->text = NULL;
    }

    mjson_object_t *mo = (mjson_object_t *)mv;
    if (mo->m != NULL) {
        map_iter_t it = map_iter_next(mo->m, NULL);
        while (it.v != NULL) {
            mjson_t *v = (mjson_t *)map_iter_getv(&it);
            mjson_fini(v);

            it = map_iter_next(mo->m, &it);
        }
        map_fini(mo->m);
        mo->m = NULL;
    }

    free(mo);
}

void mjson_array_fini(mjson_value_t *mv) {
    if (mj == NULL || mv->type != MJSON_ARRAY)
        return;
    }
    if (mv->text != NULL) {
        rs_fini(mv->text);
        mv->text = NULL;
    }

    mjson_array_t *ma = (mjson_array_t *)mv;
    if (ma->v != NULL) {
        size_t i = 0;
        size_t n = vec_num(ma->v);
        for (i = 0; i < n; i++) {
            mjson_t *v = (mjson_t *)vec_get(ma->v, i);
            mjson_fini(v);
        }
        vec_fini(ma->v);
        ma->v = NULL;
    }
    free(ma);
}

MJSON_BASIC_FINI_FUN(mjson_str, MJSON_STRING)
MJSON_BASIC_FINI_FUN(mjson_int, MJSON_INTEGER)
MJSON_BASIC_FINI_FUN(mjson_double, MJSON_DOUBLE)

MJSON_STATIC_FINI_FUN(mjson_true, MJSON_TRUE)
MJSON_STATIC_FINI_FUN(mjson_false, MJSON_FALSE)
MJSON_STATIC_FINI_FUN(mjson_null, MJSON_NULL)

