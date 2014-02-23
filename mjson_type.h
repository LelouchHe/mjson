#ifndef _MJSON_TYPE_H
#define _MJSON_TYPE_H

#include <stddef.h>

#include "mjson_core.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MJSON_TYPE_NAME(type) mjson_##type##_t

#define MJSON_GET_FUN_NAME(type) mjson_##type##_get
#define MJSON_SET_FUN_NAME(type) mjson_##type##_set

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

    struct ref_str_t *text;
} mjson_value_t;

typedef struct mjson_object_t {
    mjson_value_t h;
    struct map_t *m;
} mjson_object_t;

typedef struct mjson_array_t {
    mjson_value_t h;
    struct vector_t *v;
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

/*
 *
 * 构造函数/析构函数
 * 这些函数构造统一
 *
 */

mjson_value_t *mjson_ini(size_t type);
void mjson_fini(mjson_value_t *mv);

/*
 * get/set
 */

mjson_value_t *mjson_object_get(mjson_value_t *mv, const char *key, mjson_error_t *pe);
void mjson_object_set(mjson_value_t *mv, const char *key, mjson_value_t *value, mjson_error_t *pe);

/*
mjson_value_t *mjson_array_get(mjson_value_t *mv, size_t index, mjson_error_t *pe);
void mjson_array_set(mjson_value_t *mv, size_t index, mjson_value_t *value, mjson_error_t *pe);

const char *mjson_str_get(mjson_value_t *mv, mjson_error_t *pe);
void mjson_str_set(mjson_value_t *mv, const char *value, mjson_error_t *pe);
*/

int mjson_int_get(mjson_value_t *mv, mjson_error_t *pe);
void mjson_int_set(mjson_value_t *mv, int value, mjson_error_t *pe);

double mjson_double_get(mjson_value_t *mv, mjson_error_t *pe);
void mjson_double_set(mjson_value_t *mv, double value, mjson_error_t *pe);

/* null的正常值为-1 */
/*
int mjson_null_get(mjson_value_t *mv, mjson_error_t *pe);
void mjson_null_set(mjson_value_t *mv, int value, mjson_error_t *pe);
*/

/*
 * 辅助函数
 */

void set_error(mjson_error_t *pe, int stat);

#ifdef __cplusplus
}
#endif

#endif
