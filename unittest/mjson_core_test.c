#include <stdio.h>

#include "mjson_core.h"

int main() {
    mjson_t *mj = mj_ini(MJSON_FALSE);
    printf("false: %d\n", mj_bool_get_error(mj, NULL));
    mj_bool_set_error(mj, 1, NULL);
    printf("true : %d\n", mj_bool_get_error(mj, NULL));
    mj_fini(mj);

    mjson_t *nmj = mj_ini(MJSON_INTEGER);
    printf("%d\n", mj_int_get_error(nmj, NULL));
    mj_int_set_error(nmj, 0, NULL);
    printf("%d\n", mj_int_get_error(nmj, NULL));
    mj_fini(nmj);
    return 0;
}
