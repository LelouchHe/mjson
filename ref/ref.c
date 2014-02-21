#include <stdlib.h>
#include <assert.h>

#include "ref.h"

struct ref_t {
    size_t r;
};

ref_t *ref_ini() {
    ref_t *r = (ref_t *)malloc(sizeof (ref_t));
    if (r == NULL) {
        return NULL;
    }
    r->r = 1;
    return r;
}

void ref_fini(ref_t *r) {
    assert(r == NULL || r->r == 0);
    free(r);
}

size_t ref_inc(ref_t *r) {
    if (r == NULL) {
        return 0;
    }

    /* =0表示正在删除,此时状态不可控 */
    if (r->r > 0) {
        r->r++;
    }

    return r->r;
}

size_t ref_dec(ref_t *r) {
    if (r == NULL) {
        return 0;
    }
    assert(r->r > 0);

    r->r--;
    return r->r;
}
