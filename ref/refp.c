#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ref.h"
#include "refp.h"

struct refp_t {
    ref_t *r;
    void *d;
    refp_fini_fun f;
};

refp_t *rp_ini(void *data, refp_fini_fun f) {
    if (data == NULL) {
        return NULL;
    }

    refp_t *rp = (refp_t *)malloc(sizeof (refp_t));
    if (rp == NULL) {
        return NULL;
    }
    rp->r = ref_ini();
    rp->d = data;
    rp->f = f;
    if (rp->f == NULL) {
        rp->f = (refp_fini_fun)free;
    }

    return rp;
}

refp_t *rp_ini_copy(refp_t *rp) {
    if (rp == NULL) {
        return NULL;
    }
    assert(rp->r != NULL);
    if (ref_inc(rp->r) == 0) {
        return NULL;
    }

    refp_t *nrp = (refp_t *)malloc(sizeof (refp_t));
    if (nrp == NULL) {
        return NULL;
    }
    nrp->r = rp->r;
    nrp->d = rp->d;
    nrp->f = rp->f;

    return nrp;
}

int rp_assign(refp_t *l, refp_t *r) {
    if (l == NULL || r == NULL) {
        return REFP_NULL;
    }
    assert(l->r != NULL && r->r != NULL);
    if (ref_inc(r->r) == 0) {
        return REFP_NULL;
    }

    if (ref_dec(l->r) == 0) {
        ref_fini(l->r);
        l->f(l->d);
    }

    l->r = r->r;
    l->d = r->d;
    l->f = r->f;

    return REFP_OK;
}

void rp_fini(refp_t *rp) {
    if (rp == NULL) {
        return;
    }
    assert(rp->r != NULL);

    if (ref_dec(rp->r) == 0) {
        ref_fini(rp->r);
        rp->f(rp->d);
    }
    /* 保证如果会访问,肯定1.assert;2.core */
    memset((void *)rp, 0, sizeof (refp_t));
    free(rp);
}

void *rp_get(refp_t *rp) {
    if (rp == NULL) {
        return NULL;
    }
    assert(rp->d != NULL);

    return rp->d;
}

int rp_reset(refp_t *rp, void *data, refp_fini_fun f) {
    if (rp == NULL || data == NULL) {
        return REFP_NULL;
    }
    assert(rp->r != NULL && rp->d != NULL);
    if (rp->d == data) {
        return REFP_SAME;
    }

    refp_t t;
    t.r = ref_ini();
    if (t.r == NULL) {
        return REFP_MEM;
    }
    t.d = data;
    t.f = f;
    if (t.f == NULL) {
        t.f = (refp_fini_fun)free;
    }

    if (ref_dec(rp->r) == 0) {
        ref_fini(rp->r);
        rp->f(rp->d);
    }

    rp_swap(rp, &t);

    return REFP_OK;
}

void rp_swap(refp_t *l, refp_t *r) {
    if (l == NULL || r == NULL) {
        return;
    }
    assert(l->r != NULL && r->r != NULL);

    ref_t *t_r = l->r;
    l->r = r->r;
    r->r = t_r;

    void *t_d = l->d;
    l->d = r->d;
    r->d = t_d;

    refp_fini_fun t_f = l->f;
    l->f = r->f;
    r->f = t_f;
}
