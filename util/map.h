#ifndef _MAP_H
#define _MAP_H

#include "ref_str.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAPE_ERASE 2
#define MAPE_OVERWRITTEN 1
#define MAPE_OK 0
#define MAPE_NULL -1
#define MAPE_MEM -2

struct map_t;
typedef struct map_t map_t;
struct map_node_t;

struct map_iter_t {
    struct map_node_t *v;
};
typedef struct map_iter_t map_iter_t;

map_t *map_ini(size_t size);
void map_fini(map_t *mm);

size_t map_size(map_t *mm);
size_t map_num(map_t *mm);

const void *map_get_ref(map_t *mm, ref_str_t *key);
/* value=NULL表示删除 */
int map_set_ref(map_t *mm, ref_str_t *key, const void *value);

const void *map_get(map_t *mm, const char *key);
/* value=NULL表示删除 */
int map_set(map_t *mm, const char *key, const void *value);

/* 
 *
 * map_next(mm, NULL)会返回开始
 * v=NULL表示为end
 *
 */
map_iter_t map_iter_next(map_t *mm, map_iter_t *it);
ref_str_data_t map_iter_getk(map_iter_t *it);
const void *map_iter_getv(map_iter_t *it);

/*
 *
 * 辅助API
 * 基本就是上面的封装,便于使用
 *
 */

#ifdef MJSON_DEBUG
void map_debug(map_t *mm);
#else
#define map_debug(x) 
#endif


#ifdef __cplusplus
}
#endif

#endif
