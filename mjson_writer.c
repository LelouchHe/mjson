#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "mjson_writer.h"

#include "refp/refp.h"
#include "util/ref_str.h"
#include "util/map.h"
#include "util/vector.h"

#include "mjson_core.h"
#include "mjson_type.h"

#define MAX_INT_LEN 32
#define MAX_DOUBLE_LEN 64

char *mjson_write_int(int value) {
    char buf[MAX_INT_LEN + 1];
    size_t len = snprintf(buf, MAX_INT_LEN, "%d", value);
    char *text = (char *)malloc((len + 1) * sizeof (char));
    if (text == NULL) {
        return NULL;
    }
    strncpy(text, buf, len);
    text[len] = '\0';

    return text;
}

char *mjson_write_double(double value) {
    char buf[MAX_DOUBLE_LEN + 1];
    size_t len = snprintf(buf, MAX_DOUBLE_LEN, "%g", value);
    char *text = (char *)malloc((len + 1) * sizeof (char));
    if (text == NULL) {
        return NULL;
    }
    strncpy(text, buf, len);
    text[len] = '\0';

    return text;
}

size_t mjson_strlen(mjson_value_t *mv) {
    if (mv == NULL) {
        return 0;
    }

    size_t s = 0;
    switch (mv->type) {
    case MJSON_OBJECT:
        {
            mjson_object_t *mo = (mjson_object_t *)mv;
            if (mo->h.text == NULL && mo->m == NULL) {
                /* 空初始化,'{}' */
                s = 2;
            } else if (mo->m != NULL) {
                /* m的优先级要高于text */
                map_iter_t it = map_iter_next(mo->m, NULL);
                while (it.v != NULL) {
                    ref_str_data_t k = map_iter_getk(&it);
                    mjson_t *v = (mjson_t *)map_iter_getv(&it);
                    s += k.end - k.begin + 2;   /* \"k\" */
                    s++;                        /* : */
                    s += mj_strlen(v);          /* v */
                    s++;                        /* , */

                    it = map_iter_next(mo->m, &it);
                }
                if (s > 0) {
                    /* 去掉最后一个, */
                    s--;
                }
                /* {} */
                s += 2;
            } else {
                ref_str_data_t d = rs_get(mo->h.text);
                s = d.end - d.begin;
            }
        }
        break;

    case MJSON_ARRAY:
        {
            mjson_array_t *ma = (mjson_array_t *)mv;
            if (ma->h.text == NULL && ma->v == NULL) {
                s = 2;
            } else if (ma->v != NULL) {
                size_t num = vec_num(ma->v);
                size_t i = 0;
                for (i = 0; i < num; i++) {
                    mjson_t *v = (mjson_t *)vec_get(ma->v, i);
                    if (v == NULL) {
                        /* MJSON_NULL,"null" */
                        s += 4;
                    }
                    s++;
                }
                if (s > 0) {
                    s--;
                }
                s += 2;
            } else {
                ref_str_data_t d = rs_get(ma->h.text);
                s = d.end - d.begin;
            }
        }
        break;

    case MJSON_STRING:
        if (mv->text == NULL) {
            /* 空初始化,'\"\"' */
            s = 2;
        } else if (mv->is_str) {
            /* 独立字符串时,没有引号 */
            s = strlen((char *)mv->text) + 2;
        } else {
            ref_str_data_t d = rs_get(mv->text);
            s = d.end - d.begin;
        }
        break;

        /* int/double/true/false/null */
    default:
        if (mv->text == NULL) {
            /* 空初始化,'0'  */
            s = 1;
        } else if (mv->is_str) {
            s = strlen((char *)mv->text);
        } else {
            ref_str_data_t d = rs_get(mv->text);
            s = d.end - d.begin;
        }
        break;
    }

    return s;
}

/* buf的长度保证足够 */
static size_t mw_write_h(mjson_value_t *mv, char *buf) {
    assert(mv != NULL);
    assert(buf != NULL);

    size_t len = 0;
    switch (mv->type) {
    case MJSON_OBJECT:
        {
            mjson_object_t *mo = (mjson_object_t *)mv;
            len = 0;
            buf[len++] = '{';

            if (mo->m != NULL) {
                map_iter_t it = map_iter_next(mo->m, NULL);
                while (it.v != NULL) {
                    ref_str_data_t k = map_iter_getk(&it);
                    mjson_t *v = (mjson_t *)map_iter_getv(&it);
                    TO_TYPE(v, mjson_value_t, nv);

                    buf[len++] = '\"';
                    strncpy(buf + len, k.str + k.begin, k.end - k.begin);
                    len += k.end - k.begin;
                    buf[len++] = '\"';

                    buf[len++] = ':';

                    len += mw_write_h(nv, buf + len);

                    buf[len++] = ',';

                    it = map_iter_next(mo->m, &it);
                }
                if (len > 1) {
                    len--;
                }
            } else if (mo->h.text != NULL) {
                ref_str_data_t d = rs_get(mo->h.text);
                strncpy(buf + len, d.str + d.begin + 1, d.end - d.begin - 2);
                len += d.end - d.begin - 2;
            }
            buf[len++] = '}';
        }
        break;

    case MJSON_ARRAY:
        {
            mjson_array_t *ma = (mjson_array_t *)mv;
            len = 0;
            buf[len++] = '[';
            if (ma->v != NULL) {
                size_t num = vec_num(ma->v);
                size_t i = 0;
                for (i = 0; i < num; i++) {
                    mjson_t *v = (mjson_t *)vec_get(ma->v, i);
                    if (v == NULL) {
                        strncpy(buf + len, "null", 4);
                        len += 4;
                    } else {
                        TO_TYPE(v, mjson_value_t, nv);
                        len += mw_write_h(nv, buf + len);
                    }
                    buf[len++] = ',';
                }
                if (len > 1) {
                    len--;
                }
            } else if (ma->h.text != NULL) {
                ref_str_data_t d = rs_get(ma->h.text);
                strncpy(buf + len, d.str + d.begin + 1, d.end - d.begin - 2);
                len += d.end - d.begin - 2;
            }
            buf[len++] = ']';
        }
        break;

    case MJSON_STRING:
        {
            const char *str = "";
            len = 0;
            if (mv->text != NULL) {
                if (mv->is_str) {
                    str = (const char *)mv->text;
                    len = strlen(str);
                } else {
                    ref_str_data_t d = rs_get(mv->text);
                    str = d.str + d.begin;
                    len = d.end - d.begin;
                }
            }
            buf[0] = '\"';
            strncpy(buf + 1, str, len);
            buf[len + 1] = '\"';

            len += 2;
        }
        break;

        /* int/double/true/false/null  */
    default:
        {
            const char *str = "0";
            len = 1;
            if (mv->text != NULL) {
                if (mv->is_str) {
                    str = (const char *)mv->text;
                    len = strlen(str);
                } else {
                    ref_str_data_t d = rs_get(mv->text);
                    str = d.str + d.begin;
                    len = d.end - d.begin;
                }
            }
            strncpy(buf, str, len);
        }
        break;
    }

    return len;
}

size_t mjson_write(mjson_value_t *mv, char buf[], size_t buf_size) {
    if (mv == NULL) {
        return 0;
    }

    size_t len = mjson_strlen(mv);
    if (buf == NULL || len >= buf_size) {
        return len;
    }

    len = mw_write_h(mv, buf);
    buf[len] = '\0';

    return len;
}
