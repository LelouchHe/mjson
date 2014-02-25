#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mjson_writer.h"

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

size_t mjson_buf_size(mjson_value_t *mv) {
    if (mv == NULL) {
        return 0;
    }

    /* 没有修改过,且有值 */
    if (!mv->is_dirty && mv->text != NULL) {
        size_t s = 0;
        if (mv->is_str) {
            s = strlen((char *)mv->text);
        } else {
            ref_str_data_t d = rs_get(mv->text);
            s = d.end - d.begin;
        }

        return s;
    }

    size_t s = 0;
    switch (mv->type) {
    case MJSON_OBJECT:
        {
            mjson_object_t *mo = (mjson_object_t *)mv;
            if (!mo->h.is_dirty) {
                /* 没有修改过,且text==NULL,表示是默认,即"{}"  */
                s = 2;
            } else {
                /* 只要修改过,就不能看text了 */
            }
        }
        break;

    case MJSON_ARRAY:
        break;

    case MJSON_STRING:
        break;

        /* int,double,true,false,null */
    default:
        if (mv->is_str) {
            s = strlen((char *)mv->text);
        } else {
            ref_str_data_t d = rs_get(mv->text);
            s = d.end - d.begin;
        }
        break;
    }

    return s;
}
