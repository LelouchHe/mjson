#include <stdlib.h>
#include <string.h>

#include "mjson_type.h"
#include "mjson_parser.h"

int parser_int(const char *str, size_t begin, size_t end, mjson_error_t *pe) {
    if (str == NULL || begin >= end) {
        set_error(pe, MJSONE_RANGE);
        return 0;
    }


}
