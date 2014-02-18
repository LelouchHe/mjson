#include <stdio.h>
#include <string.h>

#include "ref_str.h"

int main() {
    ref_str_t *rs = rs_ini("what the fuck", 5);

    ref_str_t *nrs = rs_use(rs);
    ref_str_data_t d = rs_get(nrs);
    printf("%s\n", d.str ? d.str : "(null)");
    rs_fini(nrs);
    rs_fini(rs);
    return 0;
}
