#ifndef _PARSER_H
#define _PARSER_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 返回bool值表示是否为待解析类型 */
int parser_int(const char *str, size_t begin, size_t end, int *num);
int parser_double(const char *str, size_t begin, size_t end, double *num);
int parser_str(const char *str, size_t begin, size_t end);
int parser_true(const char *str, size_t begin, size_t end);
int parser_false(const char *str, size_t begin, size_t end);
int parser_null(const char *str, size_t begin, size_t end);

#ifdef __cplusplus
}
#endif

#endif
