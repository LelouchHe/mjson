#include <stdio.h>

#include "mjson_core.h"

int main() {
    mjson_t *mj = mj_ini(MJSON_NULL);
    mj_fini(mj);
    return 0;
}
