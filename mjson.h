#ifndef _MJSON_EXT_H
#define _MJSON_EXT_H

#include "mjson_core.h"

/*
 *
 * mjson辅助API
 * 是core的封装
 * 为了提供便利可用的接口
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

/*
 *
 * json的构建
 *
 */

mjson_t *mj_parse(const char *text);
mjson_t *mj_parse_check(const char *text);

mjson_t *mj_object();
mjson_t *mj_array();
mjson_t *mj_str(const char *value);
mjson_t *mj_int(int value);
mjson_t *mj_double(double value);
mjson_t *mj_bool(int value);
mjson_t *mj_null();

/*
 *
 * 赋值,不考虑类型匹配
 *
 */
int mj_assign(mjson_t *d, mjson_t *s);

/*
 *
 * json的转换
 *
 */
const char *mj_as_str(mjson_t *mj);
int mj_as_int(mjson_t *mj);
double ms_as_double(mjson_t *mj);
int mj_as_bool(mjson_t *mj);

/*
 *
 * 再次封装
 *
 */
int mj_size(mjson_t *mj);
mjson_iter_t *mj_begin(mjson_t *mj); 
mjson_iter_t *mj_end(mjson_t *mj);
mjson_iter_t *mj_next(mjson_t *mj, mjson_iter_t *it);

/*
 *
 * fmt格式说明:
 * p: mjson结构
 * s: 字符串
 * i: 整数
 * d: 浮点数
 * b: bool
 * n: null
 *
 */
mjson_t *mj_pack(const char *fmt, ...);


#ifdef __cplusplus
}
#endif

#endif
