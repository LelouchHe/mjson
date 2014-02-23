#ifndef _MJSON_PARSER_H
#define _MJSON_PARSER_H

#include <stddef.h>
#include "mjson_core.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 *
 * 从str初始化一个json
 * 返回的是root节点
 *
 */
mjson_t *mjson_ini(const char *str, size_t len);

/* 
 *
 * 解析mj的下一层节点(mj必然为组合类型)
 * 返回值表示状态
 * is_all表示是否需要递归解析
 *
 */
int mjson_parse(mjson_t *mj, int is_all);

#ifdef __cplusplus
}
#endif

#endif
