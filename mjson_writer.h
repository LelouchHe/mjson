#ifndef _MJSON_WRITER_H
#define _MJSON_WRITER_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct mjson_value_t;

size_t mjson_strlen(struct mjson_value_t *mv);

/* 
 *
 * 写入的是以'\0'结尾的字符串
 * 0: mv为NULL
 * >buf_size: 实际的长度(不包括结尾的'\0')
 * 余: 输出长度(不包括结尾的'\0')
 *
 */
size_t mjson_write(struct mjson_value_t *mv, char buf[], size_t buf_size);

/* 返回的字符串,需要调用方处理 */
char *mjson_write_int(int value);
char *mjson_write_double(double value);

#ifdef __cplusplus
}
#endif

#endif
