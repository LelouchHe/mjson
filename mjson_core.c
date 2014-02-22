#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <assert.h>

#include "util/ref_str.h"
#include "util/map.h"
#include "util/vector.h"
#include "refp/refp.h"

#include "mjson_type.h"
#include "mjson_parser.h"
#include "mjson_core.h"

static void set_error(mjson_error_t *pe, int stat) {
    if (pe != NULL) {
        pe->stat = stat;
    }
}

/*
 * 错误的实现
 * 看来double的比较很复杂,考虑专门新建一个文件描述各种比较
 * 要不就放到parser里?
 */
static int is_equal_double(double a, double b) {
    return 0;
}

/*
 *
 * 需要考虑mj可能被fini么?
 * 是不是需要inc一下,保证不会销毁呢?
 *
 */

/*
 *
 * 基本类型,2层判断
 * 0. 是否解析过了(看值是否为默认,本身即为默认值的,再劳烦解析下)
 * 1. 解析字符串是ref_str还是char*(看is_str字段即可)
 *
 */

const char *mj_str_get_error(mjson_t *mj, mjson_error_t *pe) {
    return NULL;
}

void mj_str_set_error(mjson_t *mj, const char *key, mjson_error_t *pe) {
}

int mj_int_get_error(mjson_t *mj, mjson_error_t *pe) {
    if (mj == NULL) {
        set_error(pe, MJSONE_NULL);
        return 0;
    }

    TO_TYPE(mj, mjson_int_t, mi);
    if (mi->h.type != MJSON_INTEGER) {
        set_error(pe, MJSONE_TYPE_ERROR);
        return 0;
    }

    if (mi->h.text == NULL) {
        /* 初始化时,text为NULL,默认输出0 */
        mi->i = 0;
    } else if (mi->i == 0) {
        if (!mi->h.is_str) {
            ref_str_data_t d = rs_get(mi->h.text);
            size_t len = d.end - d.begin;
            char *text = (char *)malloc((len + 1) * sizeof (char));
            if (text == NULL) {
                set_error(pe, MJSONE_MEM);
                return 0;
            }
            strncpy(text, d.str + d.begin, len);
            text[len] = '\0';

            rs_fini(mi->h.text);

            mi->h.text = (ref_str_t *)text;
            mi->h.is_str = 1;
        }

        mi->i = strtol((const char *)mi->h.text, NULL, 10);
    }

    return mi->i;
}

void mj_int_set_error(mjson_t *mj, int value, mjson_error_t *pe) {
    if (mj == NULL) {
        set_error(pe, MJSONE_NULL);
        return;
    }

    TO_TYPE(mj, mjson_int_t, mi);
    if (mi->h.type != MJSON_INTEGER) {
        set_error(pe, MJSONE_TYPE_ERROR);
        return;
    }
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

double mj_double_get_error(mjson_t *mj, mjson_error_t *pe) {
    if (mj == NULL) {
        set_error(pe, MJSONE_NULL);
        return 0.0;
    }

    TO_TYPE(mj, mjson_double_t, mi);
    if (mi->h.type != MJSON_DOUBLE) {
        set_error(pe, MJSONE_TYPE_ERROR);
        return 0.0;
    }

    if (mi->h.text == NULL) {
        /* 初始化时,text为NULL,默认输出0 */
        mi->d = 0.0;
    } else if (is_equal_double(mi->d, 0.0)) {
        if (!mi->h.is_str) {
            ref_str_data_t d = rs_get(mi->h.text);
            size_t len = d.end - d.begin;
            char *text = (char *)malloc((len + 1) * sizeof (char));
            if (text == NULL) {
                set_error(pe, MJSONE_MEM);
                return 0;
            }
            strncpy(text, d.str + d.begin, len);
            text[len] = '\0';

            rs_fini(mi->h.text);

            mi->h.text = (ref_str_t *)text;
            mi->h.is_str = 1;
        }

        mi->d = strtod((const char *)mi->h.text, NULL);
    }

    return mi->d;
}

void mj_double_set_error(mjson_t *mj, double value, mjson_error_t *pe) {
    if (mj == NULL) {
        set_error(pe, MJSONE_NULL);
        return;
    }

    TO_TYPE(mj, mjson_double_t, mi);
    if (mi->h.type != MJSON_DOUBLE) {
        set_error(pe, MJSONE_TYPE_ERROR);
        return;
    }
    if (is_equal_double(mi->d, value)) {
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

    mi->d = value;
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

/*
 *
 * 静态类型只用类型即可判断值
 *
 */
int mj_bool_get_error(mjson_t *mj, mjson_error_t *pe) {
    if (mj == NULL) {
        set_error(pe, MJSONE_NULL);
        return 0;
    }

    TO_TYPE(mj, mjson_bool_t, mb);
    if (mb->h.type != MJSON_TRUE && mb->h.type != MJSON_FALSE) {
        set_error(pe, MJSONE_TYPE_ERROR);
        return 0;
    }

    if (mb->h.type == MJSON_TRUE) {
        return 1;
    } else {
        return 0;
    }
}

void mj_bool_set_error(mjson_t *mj, int value, mjson_error_t *pe) {
    if (mj == NULL) {
        set_error(pe, MJSONE_NULL);
        return;
    }

    TO_TYPE(mj, mjson_bool_t, mb);
    if (mb->h.type != MJSON_TRUE && mb->h.type != MJSON_FALSE) {
        set_error(pe, MJSONE_TYPE_ERROR);
        return;
    }

    TO_REFP(mj, rp);
    if (value) {
        rp_reset(rp, mjson_true_ini(), (rp_fini_fun)mjson_true_fini);
    } else {
        rp_reset(rp, mjson_false_ini(), (rp_fini_fun)mjson_false_fini);
    }
    mb->h.is_dirty = 1;

    set_error(pe, MJSONE_OK);
}















