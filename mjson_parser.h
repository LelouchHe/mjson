#ifndef _MJSON_PARSER_H
#define _MJSON_PARSER_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct mjson_error_t;

int parser_int(const char *str, size_t begin, size_t end, struct mjson_error_t *pe);

#ifdef __cplusplus
}
#endif

#endif
