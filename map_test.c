#include <stdio.h>

#include "map.h"

int main() {
    map_t *m = map_ini(11);
    const char *k = NULL;
    const char *v = NULL;

    map_set(m, "1", "2");
    v = (const char *)map_get(m, "1");
    printf("v:%s\n", v);

    map_set(m, "1", NULL);
    v = (const char *)map_get(m, "1");
    printf("v:%s\n", v);

    map_debug(m);

    map_set(m, "1", "1");
    map_set(m, "2", "2");
    map_set(m, "3", "3");
    map_set(m, "4", "4");
    map_set(m, "5", "5");
    map_set(m, "6", "6");
    map_set(m, "7", "7");
    map_set(m, "8", "8");
    map_set(m, "9", "9");
    map_set(m, "10", "10");
    map_set(m, "11", "11");
    map_set(m, "12", "12");
    map_set(m, "13", "13");
    map_set(m, "14", "14");
    map_set(m, "15", "15");
    map_set(m, "5", NULL);
    map_set(m, "15", NULL);
    map_set(m, "15", NULL);
    map_debug(m);

    map_iter_t it = map_iter_next(m, NULL);
    while (it.v != NULL) {
        k = (const char *)map_iter_getk(&it);
        v = (const char *)map_iter_getv(&it);
        printf("k:%s\tv:%p\n", k, v);
        it = map_iter_next(m, &it);
    }
    map_debug(m);

    map_fini(m);

    return 0;
}
