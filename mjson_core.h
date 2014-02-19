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

enum json_type_t {
    JSON_OBJECT,
    JSON_ARRAY,
    JSON_STRING,
    JSON_INTEGER,
    JSON_DOUBLE,
    JSON_TRUE,
    JSON_FALSE,
    JSON_NULL,
};

struct mjson_t;
typedef struct mjson_t mjson_t;

mjson_t *mj_ini(int type);
int mj_fini(mjson_t *mj);

int mj_read(mjson_t *mj, const char *text, int len);
int mj_write(mjson_t *mj, char *buf, int buf_size);
int mj_buf_size(mjson_t *mj);

int mj_type(mjson_t *mj);
int mj_check(mjson_t *mj);

mjson_t *mj_object_get(mjson_t *mj, const char *key);
int mj_object_set(mjson_t *mj, const char *key, mjson_t *value);
int mj_object_erase(mjson_t *mj, const char *key);
int mj_object_size(mjson_t *mj);

mjson_t *mj_array_get(mjson_t *mj, int index);
int mj_array_set(mjson_t *mj, int index, mjson_t *value);
int mj_array_erase(mjson_t *mj, int index);
int mj_array_size(mjson_t *mj);

const char *mj_str_get(mjson_t *mj);
int mj_str_set(mjson_t *mj, const char *key);

int mj_int_get(mjson_t *mj);
int mj_int_set(mjson_t *mj, int value);

double mj_double_get(mjson_t *mj);
int mj_double_set(mjson_t *mj, double value);

int mj_bool_get(mjson_t *mj);
int mj_bool_set(mjson_t *mj, int value);


#ifdef __cplusplus
}
#endif

#endif
