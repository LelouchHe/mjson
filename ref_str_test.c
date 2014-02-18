#include <stdio.h>
#include <string.h>

#include "ref_str.h"

int main() {
    ref_str_t rs = rs_ini(NULL, -1);

    ref_str_t nrs = rs_use(&rs);
    printf("%s\n", rs_get(&nrs));
    rs_fini(&nrs);
    rs_fini(&rs);
    return 0;
}
