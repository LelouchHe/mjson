#ifndef _MAP_H
#define _MAP_H

#ifdef __cplusplus
extern "C" {
#endif

#define MAPE_ERASE 2
#define MAPE_OVERWRITTEN 1
#define MAPE_OK 0
#define MAPE_NULL -1

struct map_t;
typedef struct map_t map_t;
struct map_node_t;

struct map_iter_t {
    struct map_node_t *v;
};
typedef struct map_iter_t map_iter_t;

map_t *map_ini(int size);
int map_fini(map_t *mm);

const void *map_get(map_t *mm, const char *key);
int map_set(map_t *mm, const char *key, const void *value);

// map_next(mm, NULL)会返回开始
// v=NULL表示为end
map_iter_t map_iter_next(map_t *mm, map_iter_t *it);
const char *map_iter_getk(map_iter_t *it);
const void *map_iter_getv(map_iter_t *it);

void map_debug(map_t *mm);


#ifdef __cplusplus
}
#endif

#endif
