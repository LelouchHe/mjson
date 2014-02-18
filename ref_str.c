#include <stdlib.h>
#include <string.h>

#include "ref_str.h"

struct ref_str_in_t {
    char *str;
    int ref;
};

typedef struct ref_str_in_t ref_str_in_t;

ref_str_in_t *rsi_ini(const char *str, int len) {
    if (str == NULL) {
        return NULL;
    }

    ref_str_in_t *rsi = (ref_str_in_t *)malloc(sizeof (ref_str_in_t));
    if (rsi == NULL) {
        return NULL;
    }
    
    rsi->str = (char *)malloc((len + 1) * sizeof (char));
    if (rsi->str == NULL) {
        free(rsi);
        return NULL;
    }

    strncpy(rsi->str, str, len + 1);
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
    }

    return 0;
}

static const char *empty_str = "";

ref_str_t rs_ini(const char *str, int len) {
    if (str == NULL) {
        str = empty_str;
    }

    ref_str_t rs;
    rs.begin = 0;
    rs.end = 0;
    rs.rsi = rsi_ini(str, len);
    if (rs.rsi == NULL) {
        return rs;
    }

    rs.end = strlen(str);
    return rs;
}

int rs_fini(ref_str_t *rs) {
    if (rs == NULL || rs->rsi == NULL) {
        return -1;
    }

    rsi_fini(rs->rsi);
    rs->rsi = NULL;

    return 0;
}

ref_str_t rs_use(ref_str_t *rs) {
    ref_str_t nrs;
    nrs.begin = 0;
    nrs.end = 0;
    nrs.rsi = NULL;

    if (rs == NULL || rs->rsi == NULL) {
        return nrs;
    }

    nrs = *rs;
    nrs.rsi->ref++;

    return nrs;
}

const char *rs_get(ref_str_t *rs) {
    if (rs == NULL || rs->rsi == NULL) {
        return NULL;
    }

    return rs->rsi->str;
}

