#include "mjson_core.h"

#include <stdlib.h>
#include <assert.h>

#include "util/ref_str.h"
#include "util/map.h"
#include "util/vector.h"
#include "refp/refp.h"

#include "mjson_type.h"
#include "mjson_parser.h"

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

mjson_t *mj_ini(size_t type) {
    if (type < MJSON_OBJECT || type > MJSON_NULL) {
        return NULL;
    }

    mjson_value_t *mv = mjson_value_infos[type].ini_f();
    if (mv == NULL) {
        return NULL;
    }

    refp_t *mj = rp_ini(mv, (rp_fini_fun)mjson_value_infos[type].fini_f);
    if (mj == NULL) {
        free(mv);
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

mjson_t *GET_FUN_NAME(object)(mjson_t *mj, const char *key, mjson_error_t *pe) {
    if (mj == NULL) {
        set_error(pe, MJSONE_NULL);
        return NULL;
    }

    TO_TYPE(mj, mjson_value_t, mv);
    if (mv->type != MJSON_OBJECT) {
        mjson_value_t *nmv = mjson_value_infos[MJSON_OBJECT].ini_f();
        if (nmv == NULL) {
            set_error(pe, MJSONE_MEM);
            return NULL;
        }

        TO_REFP(mj, rp);
        rp_reset(rp, nmv, (rp_fini_fun)mjson_value_infos[MJSON_TRUE].fini_f);
    }

    return MJSON_GET_FUN_NAME(object)(mv, key, pe);
}

void SET_FUN_NAME(object)(mjson_t *mj, const char *key, mjson_t *value, mjson_error_t *pe) {

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
        mjson_value_t *nmv = MJSON_INI_FUN_NAME(type_t)();                  \
        if (nmv == NULL) {                                                  \
            set_error(pe, MJSONE_MEM);                                      \
            return;                                                         \
        }                                                                   \
        MJSON_SET_FUN_NAME(type_t)(nmv, value, pe);                         \
        rp_reset(rp, nmv, (rp_fini_fun)MJSON_FINI_FUN_NAME(type_t));        \
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
        rp_reset(rp, mjson_value_infos[MJSON_TRUE].ini_f(), (rp_fini_fun)mjson_value_infos[MJSON_TRUE].fini_f);
    } else {
        rp_reset(rp, mjson_value_infos[MJSON_FALSE].ini_f(), (rp_fini_fun)mjson_value_infos[MJSON_FALSE].fini_f);
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
    rp_reset(rp, mjson_value_infos[MJSON_NULL].ini_f(), (rp_fini_fun)mjson_value_infos[MJSON_NULL].fini_f);
}
