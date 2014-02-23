#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ref_str.h"

#include "refp/refp.h"

struct ref_str_t {
    size_t begin;
    size_t end;
    refp_t *rp;
};

ref_str_t *rs_ini_new(const char *str, size_t len) {
    if (str == NULL) {
        return NULL;
    }

    ref_str_t *rs = (ref_str_t *)calloc(1, sizeof (ref_str_t));
    if (rs == NULL) {
        return NULL;
    }

    rs->rp = rp_ini((void *)str, NULL);
    if (rs->rp == NULL) {
        free(rs);
        return NULL;
    }

    rs->end = len;
    return rs;
}

ref_str_t *rs_ini(const char *str, size_t len) {
    if (str == NULL) {
        return NULL;
    }

    if (len == 0 || len > strlen(str)) {
        len = strlen(str);
    }

    char *nstr = (char *)malloc((len + 1) * sizeof (char));
    if (nstr == NULL) {
        return NULL;
    }
    strncpy(nstr, str, len);
    nstr[len] = '\0';

    ref_str_t *rs = rs_ini_new(nstr, len);
    if (rs == NULL) {
        free(nstr);
        return NULL;
    }

    return rs;
}

void rs_fini(ref_str_t *rs) {
    if (rs == NULL) {
        return;
    }
    assert(rs->rp != NULL);

    rp_fini(rs->rp);
    rs->rp = NULL;

    free(rs);
}

ref_str_t *rs_use(ref_str_t *rs) {
    if (rs == NULL) {
        return NULL;
    }
    assert(rs->rp != NULL);

    ref_str_t *nrs = (ref_str_t *)malloc(sizeof (ref_str_t));
    if (nrs == NULL) {
        return NULL;
    }

    nrs->begin = rs->begin;
    nrs->end = rs->end;
    nrs->rp = rp_ini_copy(rs->rp);
    if (nrs->rp == NULL) {
        free(nrs);
        return NULL;
    }

    return nrs;
}

ref_str_t *rs_move(ref_str_t *rs) {
    return rs;
}

int rs_reset_new(ref_str_t *rs, const char *str, size_t len) {
    if (rs == NULL || str == NULL) {
        return -1;
    }
    assert(rs->rp != NULL);

    if (rp_reset(rs->rp, (void *)str, NULL) < 0) {
        return -1;
    }
    rs->begin = 0;
    rs->end = len;

    return 0;
}

int rs_reset(ref_str_t *rs, const char *str, size_t len) {
    if (rs == NULL || str == NULL) {
        return -1;
    }
    assert(rs->rp != NULL);

    if (len == 0 || len > strlen(str)) {
        len = strlen(str);
    }

    char *nstr = (char *)malloc((len + 1) * sizeof (char));
    if (nstr == NULL) {
        return -1;
    }
    strncpy(nstr, str, len);
    nstr[len] = '\0';

    int ret = rs_reset_new(rs, (void *)nstr, len);
    if (ret < 0) {
        free(nstr);
    }

    return ret;
}

static const ref_str_data_t null_str;

ref_str_data_t rs_get(ref_str_t *rs) {
    if (rs == NULL) {
        return null_str;
    }
    assert(rs->rp != NULL);

    ref_str_data_t d;
    d.begin = rs->begin;
    d.end = rs->end;
    d.str = (const char *)rp_get(rs->rp);

    return d;
}

int rs_set_range(ref_str_t *rs, size_t begin, size_t end) {
    if (rs == NULL) {
        return -1;
    }

    if (begin > rs->begin && begin <= rs->end) {
        rs->begin = begin;
    }
    if (end >= rs->begin && end < rs->end) {
        rs->end = end;
    }

    return 0;
}

int rs_set_begin(ref_str_t *rs, size_t begin) {
    if (rs == NULL) {
        return -1;
    }

    if (begin > rs->begin && begin <= rs->end) {
        rs->begin = begin;
    }

    return 0;
}

int rs_set_end(ref_str_t *rs, size_t end) {
    if (rs == NULL) {
        return -1;
    }

    if (end >= rs->begin && end < rs->end) {
        rs->end = end;
    }

    return 0;
}

#ifdef MJSON_DEBUG
#include <stdio.h>

void rs_debug(ref_str_t *rs) {
    if (rs == NULL) {
        printf("rs is null\n");
        return;
    }
    assert(rs->rp != NULL);

    printf("begin:%lu\tend:%lu\n", rs->begin, rs->end);
    printf("ref:%lu\tstr:%s\n", rp_ref(rs->rp), (char *)rp_get(rs->rp));
}
#endif

