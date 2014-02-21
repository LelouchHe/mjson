#ifndef _MJSON_H
#define _MJSON_H

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
#define MJSONE_TYPE_ERROR -2
#define MJSONE_MEM -3

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
    int stat;   /* 接口状态,bool */
};
typedef struct mjson_error_t mjson_error_t;

mjson_t *mj_ini(size_t type);
void mj_fini(mjson_t *mj);

/*
int mj_read(mjson_t *mj, const char *text, size_t len);
int mj_write(mjson_t *mj, char *buf, size_t buf_size);
int mj_buf_size(mjson_t *mj);
*/

int mj_type(mjson_t *mj);
/*
int mj_check(mjson_t *mj);
*/

/*
mjson_t *mj_object_get(mjson_t *mj, const char *key);
int mj_object_set(mjson_t *mj, const char *key, mjson_t *value);
int mj_object_erase(mjson_t *mj, const char *key);
int mj_object_size(mjson_t *mj);

mjson_t *mj_array_get(mjson_t *mj, size_t index);
int mj_array_set(mjson_t *mj, size_t index, mjson_t *value);
int mj_array_erase(mjson_t *mj, size_t index);
int mj_array_size(mjson_t *mj);
*/

/*
 *
 * 错误时返回默认值(NULL/0/0/false),不进行特别处理
 * 存在pe,就赋值,没有的话,就忽略了
 * set时取消了返回值,避免get/set不统一,造成歧义
 *
 */
const char *mj_str_get_error(mjson_t *mj, mjson_error_t *pe);
void mj_str_set_error(mjson_t *mj, const char *key, mjson_error_t *pe);

int mj_int_get_error(mjson_t *mj, mjson_error_t *pe);
void mj_int_set_error(mjson_t *mj, int value, mjson_error_t *pe);

double mj_double_get_error(mjson_t *mj, mjson_error_t *pe);
void mj_double_set_error(mjson_t *mj, double value, mjson_error_t *pe);

int mj_bool_get_error(mjson_t *mj, mjson_error_t *pe);
void mj_bool_set_error(mjson_t *mj, int value, mjson_error_t *pe);


#ifdef __cplusplus
}
#endif

#endif
