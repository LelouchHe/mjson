#include <stdlib.h>
#include <string.h>

#include "ref_str.h"
#include "map.h"
#include "vector.h"

#include "mjson_core.h"

#define MJSON_INI(type_t, TYPE)                             \
    do {                                                    \
        type_t *o = (type_t *)calloc(1, sizeof (type_t));   \
        if (o == NULL) {                                    \
            return NULL;                                    \
        }                                                   \
        o->head.type = TYPE;                                \
        return &o->head;                                    \
    } while(0)

struct mjson_t {
    int type;
    int is_dirty;
    ref_str_t *text; /* 在改写或as_string变成独立的字符串 */
};

typedef struct mjson_object_t {
    mjson_t head;
    map_t *m;
} mjson_object_t;

typedef struct mjson_array_t {
    mjson_t head;
    vector_t *v;
} mjson_array_t;

typedef struct mjson_str_t {
    mjson_t head;
} mjson_str_t;

typedef struct mjson_int_t {
    mjson_t head;
    int i;
} mjson_int_t;

typedef struct mjson_double_t {
    mjson_t head;
    double d;
} mjson_int_t;

typedef struct mjson_bool_t {
    mjson_t head;
    int b;
} mjson_bool_t;

typedef struct mjson_null_t {
    mjson_t head;
} mjson_null_t;


mjson_t *mj_ini(int type) {
    if (type < 0 || type > MJSON_NULL) {
        type = MJSON_NULL;
    }
    switch (type) {
        case MJSON_OBJECT:
            MJSON_INI(mjson_object_t, type);
            break;
    }
}
int mj_fini(mjson_t *mj);
