#include <stdio.h>
#include <limits.h>
#include <float.h>

#include "mjson_core.h"

int main() {
    mjson_t *mj = NULL;

    mj = mj_ini(MJSON_FALSE);
    printf("false: %d\n", mj_bool_get_error(mj, NULL));
    mj_bool_set_error(mj, 1, NULL);
    printf("true : %d\n", mj_bool_get_error(mj, NULL));
    mj_fini(mj);

    mj = mj_ini(MJSON_INTEGER);
    printf("%d\n", mj_int_get_error(mj, NULL));
    mj_int_set_error(mj, INT_MAX, NULL);
    printf("%d\n", mj_int_get_error(mj, NULL));
    mj_fini(mj);

    mj = mj_ini(MJSON_DOUBLE);
    printf("%g\n", mj_double_get_error(mj, NULL));
    mj_double_set_error(mj, DBL_MIN, NULL);
    printf("%g\n", mj_double_get_error(mj, NULL));
    mj_fini(mj);

    return 0;
}
