#ifndef _MJSON_TYPE_H
#define _MJSON_TYPE_H

#include "mjson_core.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IS_TYPE(mj, TYPE_T) ((mj) != NULL && (mj)->v != NULL && (mj)->v.type == TYPE_T)
#define TO_TYPE(mj, type_t, o) type_t *(o) = (type_t *)((mj)->v)

struct ref_str_t;
struct map_t;
struct vector_t;

typedef struct mjson_value_t {
    int type        : 3;
    int is_dirty    : 1;
    int is_str      : 1;
    int unused      : 3;

    ref_str_t *text;
} mjson_value_t;

struct mjson_t {
    size_t ref;
    mjson_value_t *v;
};

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
} mjson_int_t;

typedef struct mjson_bool_t {
    mjson_value_t h;
} mjson_bool_t;

typedef struct mjson_null_t {
    mjson_value_t h;
} mjson_null_t;

mjson_t *mjson_object_ini();
mjson_t *mjson_array_ini();
mjson_t *mjson_str_ini();
mjson_t *mjson_int_ini();
mjson_t *mjson_true_ini();
mjson_t *mjson_false_ini();
mjson_t *mjson_null_ini();

int mjson_object_fini(mjson_t *mj);
int mjson_array_fini(mjson_t *mj);
int mjson_str_fini(mjson_t *mj);
int mjson_int_fini(mjson_t *mj);
int mjson_true_fini(mjson_t *mj);
int mjson_false_fini(mjson_t *mj);
int mjson_null_fini(mjson_t *mj);

#ifdef __cplusplus
}
#endif

#endif
