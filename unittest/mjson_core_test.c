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
    if (mj_type(mj) == MJSON_OBJECT) {
        mjson_t *v = mj_get_kv_error(mj, "a", NULL);
        printf("type: %d\nint:%d\n", mj_type(v), mj_get_int_error(v, NULL));

        printf("buf_size:%lu\n", mj_strlen(mj));

        const char *data = "+nan";
        mj_set_kv_error(mj, "a", mj_parse(data, strlen(data)), NULL);

        v = mj_get_kv_error(mj, "a", NULL);
        printf("type: %d\ndouble:%g\n", mj_type(v), mj_get_double_error(v, NULL));
    }

    printf("check:%d\n", mj_check(mj));
    mj_fini(mj);

    mj = mj_ini(MJSON_OBJECT);

    mj_set_kv_error(mj, "a", mj_ini(MJSON_TRUE), NULL);
    mj_set_kv_error(mj, "b", mj_ini(MJSON_TRUE), NULL);

    mjson_t *arr = mj_ini(MJSON_ARRAY);
    mj_set_iv_error(arr, 3, mj_ini(MJSON_FALSE), NULL);
    mj_set_iv_error(arr, 5, NULL, NULL);

    mj_set_kv_error(mj, "c", arr, NULL);
    mj_set_kv_error(mj, "a", NULL, NULL);

    char buf[1024];
    mj_write(mj, buf, 1024);
    printf("%s\n", buf);

    mj_fini(mj);

    return 0;
}
