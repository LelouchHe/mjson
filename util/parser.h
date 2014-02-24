#ifndef _PARSER_H
#define _PARSER_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 返回bool值表示是否为待解析类型 */

/* 8/10/16进制,前导0/空格不算(当然,前导0可能会导致8进制) */
int parser_int(const char *str, size_t len, int *num);

/* 10/16进制, nan/inf */
int parser_double(const char *str, size_t len, double *num);

/* 剩下的不需要返回值,因为返回值可以说明一切 */

/* "xxx"格式,单双引号均可 */
int parser_quote_str(const char *str, size_t len);

/* 字符串 */
int parser_str(const char *str, size_t len, const char *target);

/*
 * 寻找下一个顶层的c,即跳过嵌套结构和转义
 * c不能是{}[]"'\
 * 调用方需要确认返回结果
 * r = len: 没有找到
 * r < len && str[r] == c: 找到
 * r < len && str[r] != c: r位置出现匹配错误
 */
size_t parser_find_next(const char *str, size_t begin, size_t end, char c);

/*
 *
 * 辅助函数
 *
 */

/* trim空格 */
const char *parser_ltrim(const char *str, size_t len);
size_t parser_rtrim(const char *str, size_t len);
const char *parser_trim(const char *str, size_t len, size_t *nlen);

#ifdef __cplusplus
}
#endif

#endif
