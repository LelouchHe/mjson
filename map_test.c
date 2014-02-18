#include <stdio.h>

#include "map.h"

int main() {
    const ref_str_t *k = NULL;
    const char *v = NULL;

    ref_str_t *keys[5];
    keys[0] = rs_ini("0", -1);
    keys[1] = rs_ini("1", -1);
    keys[2] = rs_ini("2", -1);
    keys[3] = rs_ini("3", -1);
    keys[4] = rs_ini("4", -1);

    map_t *m = map_ini(3);
    map_set_ref(m, keys[0], "0");
    v = (const char *)map_get_ref(m, keys[0]);
    printf("v:%s\n", v);

    rs_debug(keys[0]);

    map_set_ref(m, keys[0], NULL);
    v = (const char *)map_get_ref(m, keys[0]);
    printf("v:%s\n", v);

    map_set_ref(m, keys[0], NULL);

    map_set_ref(m, keys[0], "0");
    map_set_ref(m, keys[1], "1");
    map_set_ref(m, keys[2], "2");
    map_set_ref(m, keys[3], "3");
    map_set_ref(m, keys[4], "4");
    map_set_ref(m, keys[1], NULL);
    map_set_ref(m, keys[4], NULL);
    map_set_ref(m, keys[2], NULL);

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

    map_set(m, "100", "100");
    map_debug(m);

    map_fini(m);

    return 0;
}
