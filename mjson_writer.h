#ifndef _MJSON_WRITER_H
#define _MJSON_WRITER_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct mjson_value_t;

size_t mjson_buf_size(struct mjson_value_t *mv);
size_t mjson_write(struct mjson_value_t *mv, char buf[], size_t buf_size);

/* 返回的字符串,需要调用方处理 */
char *mjson_write_int(int value);
char *mjson_write_double(double value);

#ifdef __cplusplus
}
#endif

#endif
