#include <stdio.h>

#include "mjson.h"

int main() {
    const char *text = "{\"a\":\"b\",\"c\":[1,2,3],\"d\":{\"e\":\"f\"},\"g\":true,\"h\":null}";
    mjson_t *mj = mj_parse(text);
    printf("%s\n", mj_as_str(mj_object_get(mj, "a")));
    char buf[1024];
    mj_write(mj, buf, 1024);
    printf("%s\n", buf);
    return 0;
}
