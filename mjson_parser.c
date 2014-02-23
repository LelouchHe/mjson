#include <string.h>
#include <ctype.h>
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
 *           'n' 'i' => double
 * 静态类型: 'true'  => true
 *           'false' => false
 *           'null'  => null
 *
 * 除组合类型外,其余都进行校验
 * 静态类型在mj_ini前必须校验,因为是全局变量,无法更改判断
 *
 */
static int mp_type(const char *str, size_t len) {
    if (str == NULL || len == 0) {
        return -1;
    }

    int type = -1;
    switch (str[0]) {
    case '{':
        type = MJSON_OBJECT;
        break;

    case '[':
        type = MJSON_ARRAY;
        break;

    case '\"':
    case '\'':
        if (parser_quote_str(str, len)) {
            type = MJSON_STRING;
        }
        break;

    case '+':
    case '-':
        if (parser_int(str, len, NULL)) {
            type = MJSON_INTEGER;
        } else if (parser_double(str, len, NULL)) {
            type = MJSON_DOUBLE;
        }
        break;

    case 't':
        if (parser_str(str, len, "true")) {
            type = MJSON_TRUE;
        }
        break;

    case 'f':
        if (parser_str(str, len, "false")) {
            type = MJSON_FALSE;
        }
        break;

    case 'n':
        if (parser_str(str, len, "null")) {
            type = MJSON_NULL;
        } else if (parser_double(str, len, NULL)) {
            /* nan */
            type = MJSON_DOUBLE;
        }
        break;

    case 'i':
        if (parser_double(str, len, NULL)) {
            /* inf */
            type = MJSON_DOUBLE;
        }

    default:
        break;
    }

    if (type == -1 && isdigit(str[0])) {
        if (parser_int(str, len, NULL)) {
            type = MJSON_INTEGER;
        } else if (parser_double(str, len, NULL)) {
            type = MJSON_DOUBLE;
        }
    }

    return type;
}

mjson_value_t *mjson_ini_with_ref(ref_str_t *rs, int is_move) {
    if (rs == NULL) {
        if (is_move) {
            rs_fini(rs);
        }
        return NULL;
    }

    ref_str_data_t d = rs_get(rs);
    size_t nlen = 0;
    const char *nstr = parser_trim(d.str + d.begin, d.end - d.begin, &nlen);
    if (nstr == NULL || nlen == 0) {
        if (is_move) {
            rs_fini(rs);
        }
        return NULL;
    }

    int type = mp_type(nstr, nlen);
    if (type == -1) {
        if (is_move) {
            rs_fini(rs);
        }
        return NULL;
    }

    mjson_value_t *mv = mjson_ini(type);
    if (mv == NULL) {
        if (is_move) {
            rs_fini(rs);
        }
        return NULL;
    }
    /* 静态类型直接返回 */
    if (type == MJSON_TRUE || type == MJSON_FALSE || type == MJSON_NULL) {
        if (is_move) {
            rs_fini(rs);
        }
        return mv;
    }

    if (is_move) {
        mv->text = rs_move(rs);
    } else {
        mv->text = rs_use(rs);
        if (mv->text == NULL) {
            mjson_fini(mv);
            return NULL;
        }
    }
    rs_set_range(mv->text, nstr - d.str, nstr - d.str + nlen);

    if (mjson_parse(mv, 0) < 0) {
        mjson_fini(mv);
        return NULL;
    }

    return mv;
}


mjson_value_t *mjson_ini_with_str(const char *str, size_t len) {
    size_t nlen = 0;
    const char *nstr = parser_trim(str, len, &nlen);
    if (nstr == NULL || nlen == 0) {
        return NULL;
    }

    ref_str_t *rs = rs_ini(nstr, nlen);
    if (rs == NULL) {
        return NULL;
    }

    return mjson_ini_with_ref(rs, 1);
}

static ref_str_t *mp_trim_str(const char *str, size_t len, ref_str_t *up) {
    if (str == NULL || len == 0 || up == NULL) {
        return NULL;
    }

    ref_str_data_t d = rs_get(up);

    size_t nlen = len;
    const char *nstr = parser_trim(str, len, &nlen);
    if (nstr == NULL || nlen == 0) {
        return NULL;
    }

    size_t begin = nstr - d.str;
    size_t end = begin + nlen;
    ref_str_t *rs = rs_use(up);
    if (rs == NULL) {
        return NULL;
    }

    rs_set_range(rs, begin, end);

    return rs;
}

static map_t *mp_object_ini(ref_str_t *rs) {
    if (rs == NULL) {
        return NULL;
    }

    ref_str_data_t d = rs_get(rs);
    const char *str = d.str + d.begin;
    size_t len = d.end - d.begin;
    
    if (str == NULL || len == 0 || str[0] != '{' || str[len - 1] != '}') {
        return NULL;
    }

    map_t *m = map_ini(0);
    if (m == NULL) {
        return NULL;
    }

    size_t i = 1;
    while (i < len) {
        size_t kv_end = parser_find_next(str, len, ',');
        size_t k_end = parser_find_next(str, kv_end, ':');

        ref_str_t *key = mp_trim_str(str, k_end, rs);
        if (key == NULL) {
            map_fini(m);
            return NULL;
        }

        ref_str_t *value = mp_trim_str(str + k_end + 1, kv_end - k_end - 1, rs);
        if (value == NULL) {
            rs_fini(key);

            map_fini(m);
            return NULL;
        }

        mjson_value_t *mv = mjson_ini_with_ref(value, 1);
        if (mv == NULL) {
            rs_fini(value);
            rs_fini(key);

            map_fini(m);
            return NULL;
        }

        mjson_t *mj = (mjson_t *)rp_ini(mv, (rp_fini_fun)mjson_fini);
        if (mj == NULL) {
            mjson_fini(mv);
            rs_fini(key);

            map_fini(m);
            return NULL;
        }

        if (map_set_ref(m, key, mj, 1) < 0) {
            mj_fini(mj);

            map_fini(m);
            return NULL;
        }

        i += kv_end + 1;
        str += kv_end + 1;
        len -= kv_end + 1;
    }

    return m;
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
int mjson_parse(mjson_value_t *mv, int is_all) {
    if (mv == NULL) {
        return -1;
    }
    /* 暂时只解析一层 */
    assert(is_all == 0);

    if (mv->text == NULL) {
        return -1;
    }

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
            ref_str_data_t d = mp_get_str(mv);
            if (!parser_quote_str(d.str + d.begin, d.end - d.begin)) {
                return -1;
            }
        }
        break;

    case MJSON_INTEGER:
        {
            mjson_int_t *mi = (mjson_int_t *)mv;
            if (mi->i == 0) {
                ref_str_data_t d = mp_get_str(mv);
                if (!parser_int(d.str + d.begin, d.end - d.begin, &mi->i)) {
                    return -1;
                }
            }
        }
        break;

    case MJSON_DOUBLE:
        {
            mjson_double_t *md = (mjson_double_t *)mv;
            if (md->d == 0.0) {
                ref_str_data_t d = mp_get_str(mv);
                if (!parser_double(d.str + d.begin, d.end - d.begin, &md->d)) {
                    return -1;
                }
            }
        }

    case MJSON_TRUE:
    case MJSON_FALSE:
    case MJSON_NULL:
        break;

    default:
        return -1;
    }

    return 0;
}

