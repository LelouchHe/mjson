#include "mjson_core.h"

#include <stdlib.h>
#include <assert.h>

#include "util/ref_str.h"
#include "util/map.h"
#include "util/vector.h"
#include "refp/refp.h"

#include "mjson_type.h"
#include "mjson_parser.h"

mjson_t *mj_ini(size_t type) {
    mjson_value_t *mv = mjson_ini(type);
    if (mv == NULL) {
        return NULL;
    }

    refp_t *mj = rp_ini(mv, (rp_fini_fun)mjson_fini);
    if (mj == NULL) {
        mjson_fini(mv);
        return NULL;
    }

    return mj;
}

mjson_t *mj_parse(const char *str, size_t len) {
    if (str == NULL || len == 0) {
        return NULL;
    }

    mjson_value_t *mv = mjson_ini_with_str(str, len);
    if (mv == NULL) {
        return mv;
    }

    refp_t *mj = rp_ini(mv, (rp_fini_fun)mjson_fini);
    if (mj == NULL) {
        mjson_fini(mv);
        return NULL;
    }

    return mj;
}

void mj_fini(mjson_t *mj) {
    if (mj == NULL) {
        return;
    }

    rp_fini((refp_t *)mj);
}

int mj_type(mjson_t *mj) {
    if (mj == NULL) {
        return MJSONE_NULL;
    }

    TO_TYPE(mj, mjson_value_t, mv);
    return mv->type;
}

/*
 *
 * 需要考虑mj可能被fini么?
 * 是不是需要inc一下,保证不会销毁呢?
 *
 */

#define GET_FUN_NAME(type) mj_get_##type##_error
#define SET_FUN_NAME(type) mj_set_##type##_error

mjson_t *GET_FUN_NAME(kv)(mjson_t *mj, const char *key, mjson_error_t *pe) {
    if (mj == NULL) {
        set_error(pe, MJSONE_NULL);
        return NULL;
    }

    TO_TYPE(mj, mjson_value_t, mv);
    if (mv->type != MJSON_OBJECT) {
        mjson_value_t *nmv = mjson_ini(MJSON_OBJECT);
        if (nmv == NULL) {
            set_error(pe, MJSONE_MEM);
            return NULL;
        }

        TO_REFP(mj, rp);
        rp_reset(rp, nmv, (rp_fini_fun)mjson_fini);
    }

    return MJSON_GET_FUN_NAME(object)(mv, key, pe);
}

void SET_FUN_NAME(kv)(mjson_t *mj, const char *key, mjson_t *value, mjson_error_t *pe) {
    if (mj == NULL) {
        set_error(pe, MJSONE_NULL);
        return;
    }

    TO_TYPE(mj, mjson_value_t, mv);
    if (mv->type != MJSON_OBJECT) {
        mjson_value_t *nmv = mjson_ini(MJSON_OBJECT);
        if (nmv == NULL) {
            set_error(pe, MJSONE_MEM);
            return;
        }

        TO_REFP(mj, rp);
        rp_reset(rp, nmv, (rp_fini_fun)mjson_fini);
    }

    MJSON_SET_FUN_NAME(object)(mv, key, value, pe);
}

#define GET_FUN(type_t, def)                                    \
type_t GET_FUN_NAME(type_t)(mjson_t *mj, mjson_error_t *pe) {   \
    if (mj == NULL) {                                           \
        set_error(pe, MJSONE_NULL);                             \
        return def;                                             \
    }                                                           \
    TO_TYPE(mj, mjson_value_t, mv);                             \
    return MJSON_GET_FUN_NAME(type_t)(mv, pe);                  \
}

#define SET_FUN(type_t, TYPE_T)                                             \
void SET_FUN_NAME(type_t)(mjson_t *mj, type_t value, mjson_error_t *pe) {   \
    if (mj == NULL) {                                                       \
        set_error(pe, MJSONE_NULL);                                         \
        return;                                                             \
    }                                                                       \
                                                                            \
    TO_REFP(mj, rp);                                                        \
    TO_TYPE(mj, mjson_value_t, mv);                                         \
    if (mv->type == TYPE_T && rp_ref(rp) == 1) {                            \
        MJSON_SET_FUN_NAME(type_t)(mv, value, pe);                          \
    } else {                                                                \
        mjson_value_t *nmv = mjson_ini(TYPE_T);                             \
        if (nmv == NULL) {                                                  \
            set_error(pe, MJSONE_MEM);                                      \
            return;                                                         \
        }                                                                   \
        MJSON_SET_FUN_NAME(type_t)(nmv, value, pe);                         \
        rp_reset(rp, nmv, (rp_fini_fun)mjson_fini);                         \
    }                                                                       \
    set_error(pe, MJSONE_OK);                                               \
}

GET_FUN(int, 0)
SET_FUN(int, MJSON_INTEGER)

GET_FUN(double, 0.0)
SET_FUN(double, MJSON_DOUBLE)

/*
 *
 * 静态类型
 * 由于是全局变量,所以不能用基本类型的方式进行修改(无法修改)
 *
 */

int GET_FUN_NAME(bool)(mjson_t *mj, mjson_error_t *pe) {
    if (mj == NULL) {
        set_error(pe, MJSONE_NULL);
        return 0;
    }

    int b = 0;
    TO_TYPE(mj, mjson_value_t, mv);
    switch (mv->type) {
    case MJSON_TRUE:
        b = 1;
        set_error(pe, MJSONE_OK);
        break;

    case MJSON_FALSE:
        b = 1;
        set_error(pe, MJSONE_OK);
        break;

    case MJSON_OBJECT:
    case MJSON_ARRAY:
    case MJSON_STRING:
    case MJSON_DOUBLE:
        b = 1;
        set_error(pe, MJSONE_TYPE);
        break;

    case MJSON_INTEGER:
        b = MJSON_GET_FUN_NAME(int)(mv, pe);
        set_error(pe, MJSONE_OK);
        break;

    case MJSON_NULL:
    default:
        b = 0;
        set_error(pe, MJSONE_TYPE);
        break;
    }

    return b;
}

void SET_FUN_NAME(bool)(mjson_t *mj, int value, mjson_error_t *pe) {
    if (mj == NULL) {
        set_error(pe, MJSONE_NULL);
        return;
    }

    TO_REFP(mj, rp);
    if (value) {
        rp_reset(rp, mjson_ini(MJSON_TRUE), (rp_fini_fun)mjson_fini);
    } else {
        rp_reset(rp, mjson_ini(MJSON_FALSE), (rp_fini_fun)mjson_fini);
    }
    set_error(pe, MJSONE_OK);
}

int GET_FUN_NAME(null)(mjson_t *mj, mjson_error_t *pe) {
    if (mj == NULL) {
        set_error(pe, MJSONE_NULL);
        return 1;
    }

    TO_TYPE(mj, mjson_value_t, mv);
    return mv->type == MJSON_NULL;
}

void SET_FUN_NAME(null)(mjson_t *mj, mjson_error_t *pe) {
    if (mj == NULL) {
        set_error(pe, MJSONE_NULL);
        return;
    }

    TO_REFP(mj, rp);
    rp_reset(rp, mjson_ini(MJSON_NULL), (rp_fini_fun)mjson_fini);
}
