#include <stdlib.h>
#include <string.h>

#include "ref_str.h"
#include "map.h"
#include "vector.h"

#include "mjson_core.h"

struct mjson_t {
    int type;
    int is_dirty;
    ref_str_t *text;
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
