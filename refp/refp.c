#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "refp.h"

#include "ref.h"

struct refp_t {
    ref_t *r;
    void *d;
    rp_fini_fun f;
};

refp_t *rp_ini(void *data, rp_fini_fun f) {
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
        rp->f = (rp_fini_fun)free;
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

int rp_assign(refp_t *d, refp_t *s) {
    if (d == NULL || s == NULL) {
        return REFP_NULL;
    }
    assert(d->r != NULL && s->r != NULL);
    if (ref_inc(s->r) == 0) {
        return REFP_NULL;
    }

    if (ref_dec(d->r) == 0) {
        ref_fini(d->r);
        d->f(d->d);
    }

    d->r = s->r;
    d->d = s->d;
    d->f = s->f;

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

int rp_reset(refp_t *rp, void *data, rp_fini_fun f) {
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
        t.f = (rp_fini_fun)free;
    }

    if (ref_dec(rp->r) == 0) {
        ref_fini(rp->r);
        rp->f(rp->d);
    }

    rp_swap(rp, &t);

    return REFP_OK;
}

void *rp_get(refp_t *rp) {
    if (rp == NULL) {
        return NULL;
    }
    assert(rp->d != NULL);

    return rp->d;
}

size_t rp_ref(refp_t *rp) {
    if (rp == NULL) {
        return 0;
    }
    assert(rp->r != NULL);

    return ref_ref(rp->r);
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

    rp_fini_fun t_f = l->f;
    l->f = r->f;
    r->f = t_f;
}
