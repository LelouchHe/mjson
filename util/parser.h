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
 *
 * 辅助函数
 *
 */

/* trim前导空格 */
const char *parser_ltrim(const char *str, size_t len);

#ifdef __cplusplus
}
#endif

#endif
