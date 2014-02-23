#include <stdio.h>
#include <limits.h>
#include <float.h>
#include <string.h>

#include "mjson_core.h"

int main(int argc, char *argv[]) {
    mjson_t *mj = NULL;
    mjson_error_t e;

    mj = mj_ini(MJSON_INTEGER);
    printf("%d\n", mj_get_int_error(mj, &e));
    mj_set_int_error(mj, INT_MAX, &e);
    printf("%d\n", mj_get_int_error(mj, &e));
    mj_fini(mj);

    mj = mj_ini(MJSON_STRING);
    printf("%d\n", mj_get_int_error(mj, &e));
    mj_set_int_error(mj, INT_MIN, &e);
    printf("%d\n", mj_get_int_error(mj, &e));
    mj_fini(mj);

    mj = mj_ini(MJSON_STRING);
    printf("%d\n", mj_get_bool_error(mj, &e));
    mj_set_int_error(mj, 12345, &e);
    printf("%d\n", mj_get_bool_error(mj, &e));
    mj_fini(mj);

    const char *str = "12345";
    if (argc == 2) {
        str = argv[1];
    }

    mj = mj_parse(str, strlen(str));
    printf("type: %d\n", mj_type(mj));
    printf("int: %d\n", mj_get_int_error(mj, NULL));
    mj_fini(mj);

    return 0;
}
