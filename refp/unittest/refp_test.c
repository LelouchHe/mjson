#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "refp.h"

void test_d(void *d) {
    printf("in test_d\n");
    free(d);
}

int main() {
    const char *name = "Lelouch_He";
    char *m = (char *)malloc(100);
    strncpy(m, name, strlen(name));
    refp_t *rp = rp_ini(m, test_d);

    printf("%s\n", (char *)rp_get(rp));

    refp_t *nrp = rp_ini_copy(rp);

    rp_assign(nrp, rp);

    rp_fini(rp);

    rp_fini(nrp);
    return 0;
}
