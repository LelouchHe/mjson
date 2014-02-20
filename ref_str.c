#include <stdlib.h>
#include <string.h>

#include "ref_str.h"

struct ref_str_in_t {
    char *str;
    size_t ref;
};
typedef struct ref_str_in_t ref_str_in_t;

struct ref_str_t {
    size_t begin;
    size_t end;
    ref_str_in_t *rsi;
};

ref_str_in_t *rsi_ini(const char *str, size_t len) {
    ref_str_in_t *rsi = (ref_str_in_t *)malloc(sizeof (ref_str_in_t));
    if (rsi == NULL) {
        return NULL;
    }
    
    rsi->str = str;
    rsi->ref = 1;

    return rsi;
}

int rsi_fini(ref_str_in_t *rsi) {
    if (rsi == NULL) {
        return -1;
    }

    rsi->ref--;
    if (rsi->ref == 0) {
        free(rsi->str);
        rsi->str = NULL;
        free(rsi);
    }

    return 0;
}

ref_str_t *rs_ini_new(const char *str, size_t len) {
    if (str == NULL) {
        return NULL;
    }

    ref_str_t *rs = (ref_str_t *)calloc(1, sizeof (ref_str_t));
    if (rs == NULL) {
        return NULL;
    }

    rs->rsi = rsi_ini(str, len);
    if (rs->rsi == NULL) {
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


int rs_fini(ref_str_t *rs) {
    if (rs == NULL || rs->rsi == NULL) {
        return -1;
    }

    rsi_fini(rs->rsi);
    free(rs);

    return 0;
}

ref_str_t *rs_use(ref_str_t *rs) {
    if (rs == NULL || rs->rsi == NULL) {
        return NULL;
    }

    ref_str_t *nrs = (ref_str_t *)malloc(sizeof (ref_str_t));
    if (nrs == NULL) {
        return NULL;
    }

    nrs->begin = rs->begin;
    nrs->end = rs->end;
    nrs->rsi = rs->rsi;
    nrs->rsi->ref++;

    return nrs;
}

ref_str_t *rs_move(ref_str_t *rs) {
    return rs;
}

int rs_reset_new(ref_str_t *rs, const char *str, size_t len) {
    if (rs == NULL || rs->rsi == NULL || str == NULL) {
        return -1;
    }

    if (rs->rsi->str == str) {
        return 0;
    }

    if (rs->rsi->ref == 1) {
        free(rs->rsi->str);
        rs->rsi->str = str;
        rs->begin = 0;
        rs->end = len;
    } else {
        rs->rsi->ref--;
        rs->rsi = rsi_ini(str, len);
        if (rs->rsi == NULL) {
            return -1;
        }
    }

    return 0;
}

int rs_reset(ref_str_t *rs, const char *str, size_t len) {
    if (rs == NULL || rs->rsi == NULL || str == NULL) {
        return -1;
    }

    if (rs->rsi->str == str) {
        return 0;
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

    int ret = rs_reset_new(rs, nstr, len);
    if (ret < 0) {
        free(nstr);
    }

    return ret;
}

static const ref_str_data_t null_str;

ref_str_data_t rs_get(ref_str_t *rs) {
    if (rs == NULL || rs->rsi == NULL) {
        return null_str;
    }

    ref_str_data_t d;
    d.begin = rs->begin;
    d.end = rs->end;
    d.str = rs->rsi->str;

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

#ifdef DEBUG
#include <stdio.h>

void rs_debug(ref_str_t *rs) {
    if (rs == NULL) {
        printf("rs is null\n");
        return;
    } else if (rs->rsi == NULL) {
        printf("rs->rsi is null\n");
        return;
    }

    printf("begin:%lu\tend:%lu\n", rs->begin, rs->end);
    printf("ref:%lu\tstr:%s\n", rs->rsi->ref, rs->rsi->str);
}
#endif

