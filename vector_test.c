#include <stdio.h>

#include "vector.h"

int main() {
    vector_t *vec = vec_ini(20);
    vec_set(vec, 0, "0");

    vec_push(vec, 222);
    vec_push(vec, 223);
    const char *v = (const char *)vec_pop(vec);
    //printf("%s\n", v);

    vec_append(vec, 1111, 2222, NULL);

    int i = 0;
    for (i = 0; i < vec_num(vec); i++) {
        const void *p = (const char *)vec_get(vec, i);
        printf("%d: %p", i, p);
    }

    vec_fini(vec);
    return 0;
}
