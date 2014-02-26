#include <stdio.h>

#include "map.h"

int main() {
    const char *v = NULL;

    ref_str_t *keys[5];
    keys[0] = rs_ini("0", 0);
    keys[1] = rs_ini("1", 0);
    keys[2] = rs_ini("2", 0);
    keys[3] = rs_ini("3", 0);
    keys[4] = rs_ini("4", 0);

    map_t *m = map_ini(3);
    map_set_ref(m, keys[0], "0", 0);
    v = (const char *)map_get_ref(m, keys[0]);
    printf("v:%s\n", v);

    rs_debug(keys[0]);

    map_set_ref(m, keys[0], NULL, 0);
    v = (const char *)map_get_ref(m, keys[0]);
    printf("v:%s\n", v);

    map_set_ref(m, keys[0], NULL, 0);

    map_set_ref(m, keys[0], "0", 0);
    map_set_ref(m, keys[1], "1", 0);
    map_set_ref(m, keys[2], "2", 0);
    map_set_ref(m, keys[3], "3", 0);
    map_set_ref(m, keys[4], "4", 0);
    map_set_ref(m, keys[1], NULL, 0);
    map_set_ref(m, keys[4], NULL, 0);
    map_set_ref(m, keys[2], NULL, 0);

    map_iter_t it = map_iter_next(m, NULL);
    while (it.v != NULL) {
        ref_str_data_t d = map_iter_getk(&it);
        v = (const char *)map_iter_getv(&it);
        printf("k:%s\tv:%p\n", d.str, v);

        it = map_iter_next(m, &it);
    }

    rs_debug(keys[0]);

    rs_fini(keys[0]);
    rs_fini(keys[1]);
    rs_fini(keys[2]);
    rs_fini(keys[3]);
    rs_fini(keys[4]);

    printf("-------\n");

    map_set(m, "100", 3, "100");
    map_debug(m);

    map_fini(m);

    return 0;
}
