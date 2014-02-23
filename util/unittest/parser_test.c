#include <stdio.h>
#include <string.h>
#include <float.h>

#include "parser.h"

int main(int argc, char *argv[]) {
    const char *str = "0x1234";
    int r = 0;
    if (argc == 2) {
        str = argv[1];
    }

    int len = strlen(str);
    printf("str\t: %s\n\n", str);

    int i = 0;
    r = parser_int(str, len, &i);
    printf("is_ok\t: %d\nint\t: %d\n\n", r, i);

    double d = 0.0;
    r = parser_double(str, len, &d);
    printf("is_ok\t: %d\ndouble\t: %g\n\n", r, d);

    r = parser_quote_str(str, len);
    printf("is_ok\t: %d\n\n", r);

    printf("next: %lu\n", parser_find_next(str, len, ','));

    return 0;
}
