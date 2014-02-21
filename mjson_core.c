#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "util/ref_str.h"
#include "util/map.h"
#include "util/vector.h"
#include "refp/refp.h"

#include "mjson_type.h"
#include "mjson_core.h"

#define DECLARE_VALUE_INFO(type_pre)                \
{type_pre##_ini, type_pre##_fini}

typedef mjson_value_t *(*mjson_value_ini_fun)();
typedef void (*mjson_value_fini_fun)(mjson_value_t *mv);
static struct mjson_value_info_t {
    mjson_value_ini_fun ini_f;
    mjson_value_fini_fun fini_f;
} mjson_value_infos[] = {
    DECLARE_VALUE_INFO(mjson_object),
    DECLARE_VALUE_INFO(mjson_array),
    DECLARE_VALUE_INFO(mjson_str),
    DECLARE_VALUE_INFO(mjson_int),
    DECLARE_VALUE_INFO(mjson_double),
    DECLARE_VALUE_INFO(mjson_true),
    DECLARE_VALUE_INFO(mjson_false),
    DECLARE_VALUE_INFO(mjson_null),
};

struct mjson_t {
    refp_t *rp;
};

mjson_t *mj_ini(int type) {
    if (type < MJSON_OBJECT || type > MJSON_NULL) {
        return NULL;
    }

    mjson_t *mj = (mjson_t *)malloc(sizeof (mjson_t));
    if (mj == NULL) {
        return NULL;
    }

    mjson_value_t *mv = mjson_value_infos[type].ini_f();
    if (mv == NULL) {
        free(mj);
        return NULL;
    }

    mj->rp = rp_ini(mv, (rp_fini_fun)mjson_value_infos[type].fini_f);
    if (mj->rp == NULL) {
        free(mv);
        free(mj);
        return NULL;
    }

    return mj;
}

void mj_fini(mjson_t *mj) {
    if (mj == NULL) {
        return;
    }
    assert(mj->rp != NULL);

    rp_fini(mj->rp);
    mj->rp = NULL;
    
    free(mj);
}


















