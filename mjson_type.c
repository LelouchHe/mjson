#include <stdlib.h>

#include "ref_str.h"
#include "map.h"
#include "vector.h"

#include "mjson_type.h"

#define MJSON_INI_FUN(type_pre, TYPE_T)                         \
mjson_t *type_pre##_ini() {                                     \
    mjson_t *mj = (mjson_t *)malloc(sizeof (mjson_t));          \
    if (mj == NULL) {                                           \
        return NULL;                                            \
    }                                                           \
    mj->v = (type_pre##_t *)calloc(1, sizeof (type_pre##_t));   \
    if (mj->v == NULL) {                                        \
        free(mj);                                               \
        return NULL;                                            \
    }                                                           \
    mj->v.type = TYPE_T;                                        \
    mj->ref = 1;                                                \
                                                                \
    return mj;                                                  \
}

#define MJSON_BASIC_FINI_FUN(type_pre, TYPE_T)      \
int type_pre##_fini(mjson_t *mj) {                  \
    if (!IS_TYPE(mj, TYPE_T)) {                     \
        return -1;                                  \
    }                                               \
    mj->ref--;                                      \
    if (mj->ref > 0) {                              \
        return 0;                                   \
    }                                               \
                                                    \
    if (mj->v != NULL && mj->v->text != NULL) {     \
        if (mj->v->is_str) {                        \
            free(mj->v->text);                      \
        } else {                                    \
            rs_fini(mj->v->text);                   \
        }                                           \
    }                                               \
    free(mj->v);                                    \
    free(mj);                                       \
                                                    \
    return 0;                                       \
}

#define MJSON_STATIC_FINI_FUN(type_pre, TYPE_T)     \
int type_pre##_fini(mjson_t *mj) {                  \
    if (!IS_TYPE(mj, TYPE_T)) {                     \
        return -1;                                  \
    }                                               \
    mj->ref--;                                      \
    if (mj->ref > 0) {                              \
        return 0;                                   \
    }                                               \
                                                    \
    free(mj);                                       \
    return 0;                                       \
}


/* 永远不做根节点  */
static mjson_value_t true_value = {MJSON_TRUE, 0, 0, 0, 0, "true"};
static mjson_value_t false_value = {MJSON_FALSE, 0, 0, 0, 0, "false"};
static mjson_value_t null_value = {MJSON_NULL, 0, 0, 0, 0, "null"};

MJSON_INI_FUN(mjson_object, MJSON_OBJECT)
MJSON_INI_FUN(mjson_array, MJSON_ARRAY)
MJSON_INI_FUN(mjson_str, MJSON_STRING)
MJSON_INI_FUN(mjson_int, MJSON_INTEGER)
MJSON_INI_FUN(mjson_object, MJSON_DOUBLE)

/*
 *
 * 只所以需要动态,是为了解决把其他类型值赋值为bool/null的情况
 * 此时很难分清是怎么來的
 * 如果不在这里统一,就需要再來一个标识位來指定
 * 只有从ini來的bool/null不能删除,其他的都需要
 * 这个可以以后作为优化加上
 *
 */
mjson_t *mjson_true_ini() {
    mjson_t *mj = (mjson_t *)malloc(sizeof (mjson_t));
    if (mj == NULL) {
        return NULL;
    }
    mj->v = &true_value;
    mj->ref = 1;

    return mj;
}

mjson_t *mjson_false_ini() {
    mjson_t *mj = (mjson_t *)malloc(sizeof (mjson_t));
    if (mj == NULL) {
        return NULL;
    }
    mj->v = &false_value;
    mj->ref = 1;

    return mj;
}

mjson_t *mjson_null_ini() {
    mjson_t *mj = (mjson_t *)malloc(sizeof (mjson_t));
    if (mj == NULL) {
        return NULL;
    }
    mj->v = &null_value;
    mj->ref = 1;

    return mj;
}

int mjson_object_fini(mjson_t *mj) {
    if (!IS_TYPE(mj, MJSON_OBJECT)) {
        return -1;
    }
    mj->ref--;
    if (mj->ref > 0) {
        return 0;
    }

    TO_TYPE(mj, mjson_object_t, mo);
    if (mo->m != NULL) {
        map_iter_t it = map_iter_next(mo->m, NULL);
        while (it.v != NULL) {
            mjson_t *v = (mjson_t *)map_iter_getv(&it);
            mjson_fini(v);

            it = map_iter_next(mo->m, &it);
        }
        map_fini(mo->m);
    }
    rs_fini(mo->h.text);
    free(mo);
    free(mj);

    return 0;
}

int mjson_array_fini(mjson_t *mj) {
    if (!IS_TYPE(mj, MJSON_ARRAY)) {
        return -1;
    }
    mj->ref--;
    if (mj->ref > 0) {
        return 0;
    }

    TO_TYPE(mj, mjson_array_t, ma);
    if (ma->v != NULL) {
        size_t i = 0;
        size_t n = vec_num(ma->v);
        for (i = 0; i < n; i++) {
            mjson_t *v = (mjson_t *)vec_get(ma->v, i);
            mjson_fini(v);
        }
        vec_fini(ma->v);
    }
    rs_fini(ma->h.text);
    free(ma);
    free(mj);

    return 0;
}

MJSON_BASIC_FINI_FUN(mjson_str, MJSON_STRING)
MJSON_BASIC_FINI_FUN(mjson_int, MJSON_INTEGER)
MJSON_BASIC_FINI_FUN(mjson_double, MJSON_DOUBLE)

MJSON_STATIC_FINI_FUN(mjson_true, MJSON_TRUE)
MJSON_STATIC_FINI_FUN(mjson_false, MJSON_FALSE)
MJSON_STATIC_FINI_FUN(mjson_null, MJSON_NULL)

