#include <string.h>
#include <assert.h>


#include "mjson_parser.h"

#include "util/ref_str.h"
#include "util/map.h"
#include "util/vector.h"
#include "util/parser.h"
#include "refp/refp.h"

#include "mjson_type.h"

/*
 *
 * 组合类型: '{' => object 
 *           '[' => array
 * 基本类型: '"' ''' => str
 *           '+' '-' => num 进一步判断
 * 静态类型: 'true'  => true
 *           'false' => false
 *           'null'  => null
 *
 * 除组合类型外,其余都不进行校验
 * 静态类型在mj_ini前必须校验,因为是全局变量,无法更改判断
 *
 */
static int mp_type(const char *str, size_t len) {
    return MJSON_INTEGER;
}

mjson_t *mjson_ini(const char *str, size_t len) {
    if (str == NULL || len == 0) {
        return NULL;
    }

    int type = mp_type(str, len);
    if (type == -1) {
        return NULL;
    }

    mjson_t *mj = mj_ini(type);
    if (mj == NULL) {
        return NULL;
    }
    TO_TYPE(mj, mjson_value_t, mv);
    mv->text = rs_ini(str, len);
    if (mv->text == NULL) {
        mj_fini(mj);
        return NULL;
    }

    if (mjson_parse(mj, 0) < 0) {
        mj_fini(mj);
        return NULL;
    }

    return mj;
}

static map_t *mp_object_ini(ref_str_t *rs) {
    return NULL;
}

static vector_t *mp_array_ini(ref_str_t *rs) {
    return NULL;
}

static ref_str_data_t mp_get_str(mjson_value_t *mv) {
    assert(mv != NULL);

    ref_str_data_t d;
    if (mv->is_str) {
        d.str = (const char *)mv->text;
        d.begin = 0;
        d.end = strlen(d.str);
    } else {
        d = rs_get(mv->text);
    }

    return d;
}

/* 返回值表示状态 */
int mjson_parse(mjson_t *mj, int is_all) {
    if (mj == NULL) {
        return -1;
    }
    /* 暂时只解析一层 */
    assert(is_all == 0);

    TO_TYPE(mj, mjson_value_t, mv);

    switch (mv->type) {
    case MJSON_OBJECT:
        {
            mjson_object_t *mo = (mjson_object_t *)mv;
            if (mo->m == NULL) {
                mo->m = mp_object_ini(mo->h.text);
                if (mo->m == NULL) {
                    return -1;
                }
            }
        }
        break;

    case MJSON_ARRAY:
        {
            mjson_array_t *ma = (mjson_array_t *)mv;
            if (ma->v == NULL) {
                ma->v = mp_array_ini(ma->h.text);
                if (ma->v == NULL) {
                    return -1;
                }
            }
        }
        break;

    case MJSON_STRING:
        {
            mjson_str_t *ms = (mjson_str_t *)mv;
            ref_str_data_t d = mp_get_str(mv);
            if (!parser_quote_str(d.str + d.begin, d.end - d.begin)) {
                return -1;
            }
        }
        break;

    case MJSON_INTEGER:
        {
            mjson_int_t *mi = (mjson_int_t *)mv;
            ref_str_data_t d = mp_get_str(mv);
            if (!parser_int(d.str + d.begin, d.end - d.begin, &mi->i)) {
                return -1;
            }
        }
        break;

    case MJSON_DOUBLE:
        {
            mjson_double_t *md = (mjson_double_t *)mv;
            ref_str_data_t d = mp_get_str(mv);
            if (!parser_double(d.str + d.begin, d.end - d.begin, &md->d)) {
                return -1;
            }
        }

    case MJSON_TRUE:
        {
            mjson_bool_t *mb = (mjson_bool_t *)mv;
            ref_str_data_t d = mp_get_str(mv);
            if (!parser_str(d.str + d.begin, d.end - d.begin, "true")) {
                return -1;
            }
        }
    }

    return 0;
}

























