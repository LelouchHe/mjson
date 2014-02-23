#ifndef _MJSON_H
#define _MJSON_H

#include <stddef.h>

/*
 *
 * mjson核心API
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

#define MJSONE_OK 0
#define MJSONE_NULL -1
#define MJSONE_TYPE -2
#define MJSONE_MEM -3
#define MJSONE_RANGE -3
#define MJSONE_NUM -4

enum json_type_t {
    MJSON_OBJECT,
    MJSON_ARRAY,
    MJSON_STRING,
    MJSON_INTEGER,
    MJSON_DOUBLE,
    MJSON_TRUE,
    MJSON_FALSE,
    MJSON_NULL,
};

/* 完全隐藏细节 */
typedef void mjson_t;

/* 暂时未定 */
struct mjson_error_t {
    int stat;   /* 接口状态 */
};
typedef struct mjson_error_t mjson_error_t;

mjson_t *mj_ini(size_t type);
mjson_t *mj_parse(const char *str, size_t len);
void mj_fini(mjson_t *mj);

/*
int mj_read(mjson_t *mj, const char *text, size_t len);
int mj_write(mjson_t *mj, char *buf, size_t buf_size);
int mj_buf_size(mjson_t *mj);
*/

int mj_type(mjson_t *mj);
/*
int mj_check(mjson_t *mj);
size_t mj_size(mjson_t *mj);
*/

/*
void mj_erase_key(mjson_t *mj, const char *key);
void mj_erase_index(mjson_t *mj, size_t index);
*/


/*
 *
 * 错误时返回默认值(NULL/0.0/0/false),不进行特别处理
 * 存在pe,就赋值,没有的话,就忽略了
 * set时取消了返回值,避免get/set不统一,造成歧义
 *
 */

/*
 *
 * 组合类型的get有特殊之处
 * 类似map一样,get时,如果不存在,会把对应项添加进去
 * 默认的json是MJSON_NULL
 *
 */
mjson_t *mj_get_object_error(mjson_t *mj, const char *key, mjson_error_t *pe);
void mj_set_object_error(mjson_t *mj, const char *key, mjson_t *value, mjson_error_t *pe);

/*
mjson_t *mj_get_array_error(mjson_t *mj, size_t index, mjson_error_t *pe);
void mj_set_array_error(mjson_t *mj, size_t index, mjson_t *value, mjson_error_t *pe);
*/

const char *mj_get_str_error(mjson_t *mj, mjson_error_t *pe);
void mj_set_str_error(mjson_t *mj, const char *key, mjson_error_t *pe);

int mj_get_int_error(mjson_t *mj, mjson_error_t *pe);
void mj_set_int_error(mjson_t *mj, int value, mjson_error_t *pe);

double mj_get_double_error(mjson_t *mj, mjson_error_t *pe);
void mj_set_double_error(mjson_t *mj, double value, mjson_error_t *pe);

/*
 * object/array/double/true => true
 * null/false               => false
 * inteter                  => 根据整数的值
 */
int mj_get_bool_error(mjson_t *mj, mjson_error_t *pe);
void mj_set_bool_error(mjson_t *mj, int value, mjson_error_t *pe);

/*
 * 返回bool值,表示是否为MJSON_NULL
 * 默认是
 */
int mj_get_null_error(mjson_t *mj, mjson_error_t *pe);
void mj_set_null_error(mjson_t *mj, mjson_error_t *pe);

#ifdef __cplusplus
}
#endif

#endif
