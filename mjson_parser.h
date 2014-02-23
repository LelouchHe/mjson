#ifndef _MJSON_PARSER_H
#define _MJSON_PARSER_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct mjson_value_t;
struct ref_str_t;

/*
 *
 * 从str初始化一个json
 * 返回的是root节点
 *
 */
struct mjson_value_t *mjson_ini_with_str(const char *str, size_t len);
struct mjson_value_t *mjson_ini_with_ref(struct ref_str_t *rs, int is_move);

/* 
 *
 * 解析mj的下一层节点(mj必然为组合类型)
 * 返回值表示状态
 * is_all表示是否需要递归解析
 *
 */
int mjson_parse(struct mjson_value_t *mj, int is_all);

#ifdef __cplusplus
}
#endif

#endif
