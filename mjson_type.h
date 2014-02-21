#ifndef _MJSON_TYPE_H
#define _MJSON_TYPE_H

#include "mjson_core.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TO_REFP(mj, rp)  refp_t *(rp) = (refp_t *)(mj)
/* 本身是函数调用,不要调用频繁 */
#define TO_TYPE(mj, type_t, mt) type_t *(mt) = (type_t *)rp_get((refp_t *)(mj))

struct ref_str_t;
struct map_t;
struct vector_t;

typedef struct mjson_value_t {
    size_t type        : 3;
    size_t is_dirty    : 1;
    size_t is_str      : 1;
    size_t unused      : 3;

    ref_str_t *text;
} mjson_value_t;

typedef struct mjson_object_t {
    mjson_value_t h;
    map_t *m;
} mjson_object_t;

typedef struct mjson_array_t {
    mjson_value_t h;
    vector_t *v;
} mjson_array_t;

typedef struct mjson_str_t {
    mjson_value_t h;
} mjson_str_t;

typedef struct mjson_int_t {
    mjson_value_t h;
    int i;
} mjson_int_t;

typedef struct mjson_double_t {
    mjson_value_t h;
    double d;
} mjson_double_t;

typedef struct mjson_bool_t {
    mjson_value_t h;
} mjson_bool_t;

typedef struct mjson_null_t {
    mjson_value_t h;
} mjson_null_t;

mjson_value_t *mjson_object_ini();
mjson_value_t *mjson_array_ini();
mjson_value_t *mjson_str_ini();
mjson_value_t *mjson_int_ini();
mjson_value_t *mjson_double_ini();
mjson_value_t *mjson_true_ini();
mjson_value_t *mjson_false_ini();
mjson_value_t *mjson_null_ini();

void mjson_object_fini(mjson_value_t *mj);
void mjson_array_fini(mjson_value_t *mj);
void mjson_str_fini(mjson_value_t *mj);
void mjson_int_fini(mjson_value_t *mj);
void mjson_double_fini(mjson_value_t *mj);
void mjson_true_fini(mjson_value_t *mj);
void mjson_false_fini(mjson_value_t *mj);
void mjson_null_fini(mjson_value_t *mj);

#ifdef __cplusplus
}
#endif

#endif
