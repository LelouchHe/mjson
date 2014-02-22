#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "refp/refp.h"
#include "util/ref_str.h"
#include "util/map.h"
#include "util/vector.h"

#include "mjson_parser.h"
#include "mjson_type.h"

#define INI_FUN(type_t, TYPE_T)                                                                         \
mjson_value_t *MJSON_INI_FUN_NAME(type_t)() {                                                           \
    MJSON_TYPE_NAME(type_t) *v = (MJSON_TYPE_NAME(type_t) *)calloc(1, sizeof (MJSON_TYPE_NAME(type_t)));\
    if (v == NULL) {                                                                                    \
        return NULL;                                                                                    \
    }                                                                                                   \
    v->h.type = TYPE_T;                                                                                 \
    return &v->h;                                                                                       \
}

#define FINI_FUN_BASIC(type_t, TYPE_T)                  \
void MJSON_FINI_FUN_NAME(type_t)(mjson_value_t *mv) {   \
    if (mv == NULL || mv->type != TYPE_T) {             \
        return;                                         \
    }                                                   \
                                                        \
    if (mv->text != NULL) {                             \
        if (mv->is_str) {                               \
            free(mv->text);                             \
        } else {                                        \
            rs_fini(mv->text);                          \
        }                                               \
        mv->text = NULL;                                \
    }                                                   \
    free(mv);                                           \
}

#define FINI_FUN_STATIC(type_t, TYPE_T)                 \
void MJSON_FINI_FUN_NAME(type_t)(mjson_value_t *mv) {   \
}

static mjson_bool_t true_value = {{MJSON_TRUE, 0, 1, 0, (ref_str_t *)"true"}};
static mjson_bool_t false_value = {{MJSON_FALSE, 0, 1, 0, (ref_str_t *)"false"}};
static mjson_null_t null_value = {{MJSON_NULL, 0, 1, 0, (ref_str_t *)"null"}};

INI_FUN(object, MJSON_OBJECT)
INI_FUN(array, MJSON_ARRAY)
INI_FUN(str, MJSON_STRING)
INI_FUN(int, MJSON_INTEGER)
INI_FUN(double, MJSON_DOUBLE)

mjson_value_t *MJSON_INI_FUN_NAME(true)() {
    return &true_value.h;
}

mjson_value_t *MJSON_INI_FUN_NAME(false)() {
    return &false_value.h;
}

mjson_value_t *MJSON_INI_FUN_NAME(null)() {
    return &null_value.h;
}

void MJSON_FINI_FUN_NAME(object)(mjson_value_t *mv) {
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
            mj_fini(v);

            it = map_iter_next(mo->m, &it);
        }
        map_fini(mo->m);
        mo->m = NULL;
    }

    free(mo);
}

void MJSON_FINI_FUN_NAME(array)(mjson_value_t *mv) {
    if (mv == NULL || mv->type != MJSON_ARRAY) {
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
            mj_fini(v);
        }
        vec_fini(ma->v);
        ma->v = NULL;
    }
    free(ma);
}

FINI_FUN_BASIC(str, MJSON_STRING)
FINI_FUN_BASIC(int, MJSON_INTEGER)
FINI_FUN_BASIC(double, MJSON_DOUBLE)

FINI_FUN_STATIC(true, MJSON_TRUE)
FINI_FUN_STATIC(false, MJSON_FALSE)
FINI_FUN_STATIC(null, MJSON_NULL)

void set_error(mjson_error_t *pe, int stat) {
    if (pe != NULL) {
        pe->stat = stat;
    }
}

mjson_value_t *MJSON_GET_FUN_NAME(object)(mjson_value_t *mv, const char *key, mjson_error_t *pe) {
    if (mv == NULL) {
        set_error(pe, MJSONE_NULL);
        return NULL;
    }
    if (mv->type != MJSON_OBJECT) {
        set_error(pe, MJSONE_TYPE);
        return NULL;
    }

    mjson_object_t *mo = (mjson_object_t *)mv;
    if (mo->m == NULL) {
        if (mo->h.text == NULL) {
            mo->m = map_ini(16);
            if (mo->m == NULL) {
                set_error(pe, MJSONE_MEM);
                return NULL;
            }
            mo->h.is_dirty = 1;
        }
    }

    mjson_value_t *nmv = (mjson_value_t *)map_get(mo->m, key);
    if (nmv == NULL) {
        nmv = mjson_null_ini();
        if (nmv == NULL) {
            set_error(pe, MJSONE_MEM);
            return NULL;
        }
        map_set(mo->m, key, nmv);
        mo->h.is_dirty = 1;
    }

    return nmv;
}

void MJSON_SET_FUN_NAME(object)(mjson_value_t *mv, const char *key, mjson_value_t *value, mjson_error_t *pe) {

}

/*
 *
 * 基本类型,2层判断
 * 0. 是否解析过了(看值是否为默认,本身即为默认值的,再劳烦解析下)
 * 1. 解析字符串是ref_str还是char*(看is_str字段即可)
 *
 * h.text有值,不代表已经被解析了,所以还是需要判断下是否为默认值
 * 如果是的话,就再次解析下(如果本身就是默认值,就只好再来一次了)
 *
 */

int MJSON_GET_FUN_NAME(int)(mjson_value_t *mv, mjson_error_t *pe) {
    if (mv == NULL) {
        set_error(pe, MJSONE_NULL);
        return 0;
    }

    if (mv->type != MJSON_INTEGER) {
        set_error(pe, MJSONE_TYPE);
        return 0;
    }

    mjson_int_t *mi = (mjson_int_t *)mv;
    if (mi->h.text == NULL) {
        mi->i = 0;
    } else if (mi->i == 0) {
        ref_str_data_t d;
        if (mi->h.is_str) {
            d.str = (char *)mi->h.text;
            d.begin = 0;
            d.end = strlen(d.str);
        } else {
            d = rs_get(mi->h.text);
        }

        mi->i = parser_int(d.str, d.begin, d.end, pe);
    }

    return mi->i;
}

void MJSON_SET_FUN_NAME(int)(mjson_value_t *mv, int value, mjson_error_t *pe) {
    if (mv == NULL) {
        set_error(pe, MJSONE_NULL);
        return;
    }
    if (mv->type != MJSON_INTEGER) {
        set_error(pe, MJSONE_TYPE);
        return;
    }
    
    mjson_int_t *mi = (mjson_int_t *)mv;
    if (mi->i == value) {
        set_error(pe, MJSONE_OK);
        return;
    }

    char str[32];
    size_t len = snprintf(str, 31, "%d", value);
    char *text = (char *)malloc((len + 1) * sizeof (char));
    if (text == NULL) {
        set_error(pe, MJSONE_MEM);
        return;
    }
    strncpy(text, str, len);
    text[len] = '\0';

    mi->i = value;
    if (mi->h.text != NULL) {
        if (mi->h.is_str) {
            free((char *)mi->h.text);
        } else {
            rs_fini(mi->h.text);
        }
    }
    mi->h.text = (ref_str_t *)text;
    mi->h.is_str = 1;
    mi->h.is_dirty = 1;

    set_error(pe, MJSONE_OK);
}

double MJSON_GET_FUN_NAME(double)(mjson_value_t *mv, mjson_error_t *pe) {
    if (mv == NULL) {
        set_error(pe, MJSONE_NULL);
        return 0;
    }

    if (mv->type != MJSON_DOUBLE) {
        set_error(pe, MJSONE_TYPE);
        return 0;
    }

    mjson_double_t *md = (mjson_double_t *)mv;
    if (md->h.text == NULL) {
        md->d = 0.0;
    } else if (md->d == 0.0) {
        if (!md->h.is_str) {
            ref_str_data_t d = rs_get(md->h.text);
            size_t len = d.end - d.begin;
            char *text = (char *)malloc((len + 1) * sizeof (char));
            if (text == NULL) {
                set_error(pe, MJSONE_MEM);
                return 0;
            }
            strncpy(text, d.str + d.begin, len);
            text[len] = '\0';

            rs_fini(md->h.text);

            md->h.text = (ref_str_t *)text;
            md->h.is_str = 1;
        }

        md->d = strtod((const char *)md->h.text, NULL);
    }

    return md->d;
}

void MJSON_SET_FUN_NAME(double)(mjson_value_t *mv, double value, mjson_error_t *pe) {
    if (mv == NULL) {
        set_error(pe, MJSONE_NULL);
        return;
    }
    if (mv->type != MJSON_DOUBLE) {
        set_error(pe, MJSONE_TYPE);
        return;
    }
    
    mjson_double_t *md = (mjson_double_t *)mv;
    if (md->d == value) {
        set_error(pe, MJSONE_OK);
        return;
    }

    char str[64];
    size_t len = snprintf(str, 63, "%g", value);
    char *text = (char *)malloc((len + 1) * sizeof (char));
    if (text == NULL) {
        set_error(pe, MJSONE_MEM);
        return;
    }
    strncpy(text, str, len);
    text[len] = '\0';

    md->d = value;
    if (md->h.text != NULL) {
        if (md->h.is_str) {
            free((char *)md->h.text);
        } else {
            rs_fini(md->h.text);
        }
    }
    md->h.text = (ref_str_t *)text;
    md->h.is_str = 1;
    md->h.is_dirty = 1;

    set_error(pe, MJSONE_OK);
}

