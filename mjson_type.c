#include "mjson_type.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "refp/refp.h"
#include "util/ref_str.h"
#include "util/map.h"
#include "util/vector.h"

#include "mjson_parser.h"

#define MJSON_INI_FUN_NAME(type) mjson_##type##_ini
#define MJSON_FINI_FUN_NAME(type) mjson_##type##_fini

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

/*
 *
 * get过程:
 * 0. text == NULL: 尚未赋值
 *                  组合类型:需要生成对应的结构(因为需要返回默认MJSON_NULL)
 *                  基本类型:默认值可以直接赋值
 * 1. text != NULL: 已经赋值
 *                  组合类型:进行parse即可,最后肯定有正确的内部结构(assert之)
 *                  基本类型:同上,需要parse
 * 
 * set过程:
 * 0. text == NULL: 尚未赋值
 *                  组合类型:不需要默认text,因为很快会进行赋值,这样成了浪费
 *                  基本类型:直接赋值即可,本身來进行解决
 * 1. text != NULL: 已经赋值 
 *                  组合类型:parse得正确结构,再进行处理
 *                  基本类型:释放原结构即可
 *
 * 为了减少浪费,所以要在这里深入json结构内部,而无法用parser隐藏
 * 不过set过程其实无法用parser來隐藏,同样是要暴露结构的,所以应该还好
 *
 */

mjson_t *MJSON_GET_FUN_NAME(object)(mjson_value_t *mv, const char *key, mjson_error_t *pe) {
    if (mv == NULL) {
        set_error(pe, MJSONE_NULL);
        return NULL;
    }
    if (mv->type != MJSON_OBJECT) {
        set_error(pe, MJSONE_TYPE);
        return NULL;
    }

    mjson_object_t *mo = (mjson_object_t *)mv;
    if (mo->h.text == NULL) {
        if (mo->m == NULL) {
            mo->m = map_ini(0);
            if (mo->m == NULL) {
                set_error(pe, MJSONE_MEM);
                return NULL;
            }
            /* 进行了修改 */
            mo->h.is_dirty = 1;
        }
    } else if (mjson_parse(mv, 0) < 0) {
        set_error(pe, MJSONE_PARSE);
        return NULL;
    }
    assert(mo->m != NULL);

    size_t key_len = strlen(key);
    mjson_t *v = (mjson_t *)map_get(mo->m, key, key_len);
    if (v == NULL) {
        v = mj_ini(MJSON_NULL);
        if (v == NULL) {
            set_error(pe, MJSONE_MEM);
            return NULL;
        }
        map_set(mo->m, key, key_len, v);
        mo->h.is_dirty = 1;
    }

    return v;
}

void MJSON_SET_FUN_NAME(object)(mjson_value_t *mv, const char *key, mjson_t *value, mjson_error_t *pe) {
    if (mv == NULL) {
        set_error(pe, MJSONE_NULL);
        return;
    }

    if (mv->type != MJSON_OBJECT) {
        set_error(pe, MJSONE_TYPE);
        return;
    }

    mjson_object_t *mo = (mjson_object_t *)mv;
    if (mo->h.text == NULL) {
        if (mo->m == NULL) {
            mo->m = map_ini(0);
            if (mo->m == NULL) {
                set_error(pe, MJSONE_MEM);
                return;
            }
            mo->h.is_dirty = 1;;
        }
    } else if (mjson_parse(mv, 0) < 0) {
        set_error(pe, MJSONE_PARSE);
        return;
    }

    assert(mo->m != NULL);
    map_set(mo->m, key, strlen(key), value);
}

/*
 *
 * 基本类型
 *
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
    } else if (mjson_parse(mv, 0) < 0) {
        set_error(pe, MJSONE_PARSE);
        /* 解析错误,仍然要返回已经解析的部分值 */
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
    } else if (mjson_parse(mv, 0) < 0) {
        set_error(pe, MJSONE_PARSE);
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

#define DECLARE_VALUE_INFO(type)                        \
{MJSON_INI_FUN_NAME(type), MJSON_FINI_FUN_NAME(type)}

typedef mjson_value_t *(*mjson_value_ini_fun)();
typedef void (*mjson_value_fini_fun)(mjson_value_t *mv);
static struct mjson_value_info_t {
    mjson_value_ini_fun ini_f;
    mjson_value_fini_fun fini_f;
} mjson_value_infos[] = {
    DECLARE_VALUE_INFO(object),
    DECLARE_VALUE_INFO(array),
    DECLARE_VALUE_INFO(str),
    DECLARE_VALUE_INFO(int),
    DECLARE_VALUE_INFO(double),
    DECLARE_VALUE_INFO(true),
    DECLARE_VALUE_INFO(false),
    DECLARE_VALUE_INFO(null),
};

mjson_value_t *mjson_ini(size_t type) {
    if (type < MJSON_OBJECT || type > MJSON_NULL) {
        return NULL;
    }

    return mjson_value_infos[type].ini_f();
}

void mjson_fini(mjson_value_t *mv) {
    if (mv == NULL) {
        return;
    }

    mjson_value_infos[mv->type].fini_f(mv);
}
