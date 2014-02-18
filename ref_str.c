#include <stdlib.h>
#include <string.h>

#include "ref_str.h"

struct ref_str_in_t {
    char *str;
    int ref;
};
typedef struct ref_str_in_t ref_str_in_t;

struct ref_str_t {
    int begin;
    int end;
    ref_str_in_t *rsi;
};

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

    strncpy(rsi->str, str, len);
    rsi->str[len] = '\0';
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

ref_str_t *rs_ini(const char *str, int len) {
    if (str == NULL) {
        return NULL;
    }
    if (len > strlen(str)) {
        len = strlen(str);
    }

    ref_str_t *rs = (ref_str_t *)calloc(1, sizeof (ref_str_t));
    rs->rsi = rsi_ini(str, len);
    if (rs->rsi == NULL) {
        return rs;
    }

    rs->end = len;
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

int rs_set_range(ref_str_t *rs, int begin, int end) {
    if (rs == NULL) {
        return -1;
    }

    if (begin >= rs->begin && begin <= rs->end) {
        rs->begin = begin;
    }
    if (end >= rs->begin && end <= rs->end) {
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

    printf("begin:%d\tend:%d\n", rs->begin, rs->end);
    printf("ref:%d\tstr:%s\n", rs->rsi->ref, rs->rsi->str);
}
#endif

