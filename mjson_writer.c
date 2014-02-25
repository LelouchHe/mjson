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

    size_t s = 0;
    switch (mv->type) {
    case MJSON_OBJECT:
        break;

    case MJSON_ARRAY:
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

    case MJSON_INTEGER:
    case MJSON_DOUBLE:
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

    case MJSON_TRUE:
    case MJSON_FALSE:
    case MJSON_NULL:
        s = strlen((char *)mv->text);
        break;

    default:
        break;
    }

    return s;
}
