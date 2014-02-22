#include <ctype.h>
#include <limits.h>
#include <assert.h>

#include "parser.h"

static const int char_map[] = {
    -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, -1, -1, -1, -1, -1, -1, -1, 
    0,  1,  2,  3,  4,  5,  6,  7, 
    8,  9, -1, -1, -1, -1, -1, -1, 
    -1, 10, 11, 12, 13, 14, 15, -1, 
    -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, 10, 11, 12, 13, 14, 15, -1, 
    -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, -1, -1, -1, -1, -1, -1, -1, 
};

/* 状态机最赞 */
int m_strtol(const char *s, size_t len, int *num) {
    assert(num != NULL);
    *num = 0;

    /* 初始状态 */
    int step = 0;

    /* 不同进制 */
    int base = 10;

    /* 溢出控制 */
    int max_d10 = INT_MAX / 10;
    int max_m10 = INT_MAX % 10;

    /* 符号 */
    int sign = 1;

    int is_ok = 1;
    int i = 0;
    for (i = 0; i < len && is_ok; i++) {
        switch (step) {
        case 0:
            if (isspace(s[i])) {
                step = 0;
            } if (s[i] == '+' || s[i] == '-') {
                step = 1;
                if (s[i] == '-') {
                    sign = -1;
                }
            } else if (s[i] == '0') {
                step = 2;
            } else if (s[i] >= '1' && s[i] <= '9') {
                *num = char_map[s[i] & 0x7F];
                step = 3;
            } else {
                is_ok = 0;
            }
            break;

        case 1:
            if (s[i] == '0') {
                step = 2;
            } else if (s[i] >= '1' && s[i] <= '9') {
                *num = char_map[s[i] & 0x7F];
                step = 3;
            } else {
                is_ok = 0;
            }
            break;

        case 2:
            if (s[i] >= '1' && s[i] <= '7') {
                base = 8;
                *num = char_map[s[i] & 0x7F];
                step = 3;
            } else if (s[i] == 'x' || s[i] == 'X') {
                step = 4;
            } else {
                is_ok = 0;
            }
            break;

        case 3:
            {
                int d = char_map[s[i] & 0x7F];
                if (d >= 0 && d < base) {
                    if (*num > max_d10
                        || (*num == max_d10 && d > max_m10)) {
                        /* 有溢出可能 */
                        if (sign == 1) {
                            *num = INT_MAX;
                            is_ok = 0;
                        } else {
                            if (!(*num == max_d10 && d == max_m10 + 1)) {
                                /* 负数溢出比正数大1 */
                                is_ok = 0;
                            }
                            *num = INT_MIN;
                        }
                        break;
                    }
                    *num = *num * base + d;
                } else {
                    is_ok = 0;
                }
            }
            break;

        case 4:
            {
                base = 16;
                int d = char_map[s[i] & 0x7F];
                if (d >= 0 && d < base) {
                    *num = d;
                    step = 3;
                } else {
                    is_ok = 0;
                }
            }
            break;

        default:
            is_ok = 0;
            break;
        }
    }

    if (is_ok && (step == 2 || step == 3)) {
        *num *= sign;
        is_ok = 1;
    }

    return is_ok;
}

int parser_int(const char *str, size_t begin, size_t end, int *num) {
    if (str == NULL || begin >= end) {
        return 0;
    }

    int n = 0;
    int r = m_strtol(str + begin, end - begin, &n);
    if (num != NULL) {
        *num = n;
    }

    return r;
}
