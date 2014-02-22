#include <stdio.h>
#include <string.h>

#include "parser.h"

int main(int argc, char *argv[]) {
    const char *str = "0x1234";
    if (argc == 2) {
        str = argv[1];
    }

    int num = 0;
    int r = parser_int(str, 0, strlen(str), &num);
    printf("is_ok:%d\nnum:%d\n", r, num);
    return 0;
}
