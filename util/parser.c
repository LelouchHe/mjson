#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "parser.h"

/* 足够64bit整数长度 */
#define MAX_INT_LEN 32
/* 512足够大,但不一定 */
#define MAX_DOUBLE_LEN 512

/* 下标: c & 0x7F  */
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

/* 
 *
 * 需要手动将空格去掉
 * 前导0其实也应该去掉,但目前无法很容易做到(会影响后续判断)
 *
 */
const char *parser_ltrim(const char *str, size_t len) {
    if (str == NULL) {
        return NULL;
    }

    int i = 0;
    while (i < len && isspace(str[i])) {
        i++;
    }

    if (i == len) {
        return NULL;
    } else {
        return str + i;
    }
}

size_t parser_rtrim(const char *str, size_t len) {
    if (str == NULL) {
        return 0;
    }

    int i = len - 1;
    while (i >= 0 && isspace(str[i])) {
        i--;
    }
    
    return i + 1;
}

const char *parser_trim(const char *str, size_t len, size_t *nlen) {
    assert(nlen != NULL);
    if (str == NULL || len == 0) {
        *nlen = 0;
        return NULL;
    }

    const char *nstr = parser_ltrim(str, len);
    if (nstr == NULL) {
        *nlen = 0;
        return NULL;
    }

    *nlen = parser_rtrim(nstr, len - (nstr - str));

    return nstr;
}

static int m_strtol(const char *str, size_t len, int *num) {
    assert(num != NULL);
    *num = 0;

    if (str == NULL) {
        return 0;
    }

    if (len > MAX_INT_LEN) {
        len = MAX_INT_LEN;
    }

    char buf[MAX_INT_LEN + 1];
    strncpy(buf, str, len);
    buf[len] = '\0';

    long int n = 0;
    char *end = NULL;
    errno = 0;
    n = strtol(buf, &end, 0);

    /* long => int,以后考虑直接long,就没有这些了 */
    if (errno == EINVAL) {
        return 0;
    } else if (errno == ERANGE) {
        if (n == LONG_MIN) {
            *num = INT_MIN;
        } else {
            *num = INT_MAX;
        }

        return 0;
    } else if (*end != '\0') {
        *num = n;
        return 0;
    } else if (n > INT_MAX) {
        *num = INT_MAX;
        return 0;
    } else if (n < INT_MIN) {
        *num = INT_MIN;
        return 0;
    } else {
        *num = n;
        return 1;
    }
}

/* 非状态机的实现,需要注意的就是在每次递增i时,需要马上判断是否在范围内 */
static int m_strtol_order(const char *s, size_t len, int *num) {
    assert(num != NULL);
    *num = 0;

    int max_d10 = INT_MAX / 10;
    int max_m10 = INT_MAX % 10;

    int i = 0;
    while (i < len && isspace(s[i])) {
        i++;
    }
    if (i == len) {
        return 0;
    }

    int sign = 1;
    if (s[i] == '+' || s[i] == '-') {
        if (s[i] == '-') {
            sign = -1;
        }
        i++;
        if (i == len) {
            return 0;
        }
    }

    int base = 10;
    if (s[i] == '0') {
        base = 8;

        i++;
        if (i == len) {
            return 1;
        }

        if (tolower(s[i]) == 'x') {
            base = 16;
            i++;
            if (i == len) {
                return 0;
            }
        }
    }

    while (i < len) {
        int v = char_map[s[i] & 0x7F];
        if (v < 0 || v >= base) {
            break;
        }

        if (*num > max_d10
            || (*num == max_d10 && v > max_m10)) {
            if (sign == 1) {
                *num = INT_MAX;
            } else {
                if (*num == max_d10 && v == max_m10 + 1) {
                    /* INT_MIN正常,所以需要+1来保证i==len */
                    i++;
                }
                *num = INT_MIN;
            }
            break;
        }

        *num = *num * base + v;

        i++;
    }

    *num *= sign;

    return i == len;
}

/*
 * 状态机最赞
 * 唯一缺点可能是性能不高
 * 注意,前导0和空格兼容C的标准即可
 * 但实际上,前导应该是非法的json整数
 */
static int m_strtol_stat(const char *s, size_t len, int *num) {
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
            if (s[i] >= '0' && s[i] <= '7') {
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

    /* INT_MIN * -1仍然等于INT_MIN,利用了这一点trick */
    *num *= sign;
    if (is_ok && (step == 2 || step == 3)) {
        is_ok = 1;
    }

    return is_ok;
}

int parser_int(const char *str, size_t len, int *num) {
    size_t nlen = 0;
    const char *nstr = parser_trim(str, len, &nlen);
    if (nstr == NULL || nlen == 0) {
        if (num != NULL) {
            *num = 0;
        }
        return 0;
    }

    /* 为了避免警告 */
    (void)m_strtol_order;
    (void)m_strtol_stat;

    int n = 0;
    int r = m_strtol(nstr, nlen, &n);
    if (num != NULL) {
        *num = n;
    }

    return r;
}

/*
 *
 * 暂时以损失精度的方式,限定str的最长长度
 * 等到看懂strtod算法后,再进行精确计算
 *
 */
static int m_strtod(const char *str, size_t len, double *num) {
    assert(num != NULL);
    *num = 0.0;

    if (str == NULL) {
        return 0;
    }
    if (len > MAX_DOUBLE_LEN) {
        len = MAX_DOUBLE_LEN;
    }

    char buf[MAX_DOUBLE_LEN + 1];
    strncpy(buf, str, len);
    buf[len] = '\0';

    char *end = NULL;
    errno = 0;
    *num = strtod(buf, &end);

    if (*end != '\0' || errno == ERANGE) {
        return 0;
    } else {
        return 1;
    }
}

int parser_double(const char *str, size_t len, double *num) {
    size_t nlen = 0;
    const char *nstr = parser_trim(str, len, &nlen);
    if (nstr == NULL || nlen == 0) {
        if (num != NULL) {
            *num = 0.0;
        }
        return 0;
    }

    double n = 0.0;
    int r = m_strtod(nstr, nlen, &n);
    if (num != NULL) {
        *num = n;
    }

    return r;
}

int parser_quote_str(const char *str, size_t len) {
    size_t nlen = 0;
    const char *nstr = parser_trim(str, len, &nlen);
    if (nstr == NULL || nlen == 0) {
        return 0;
    }

    char quote = nstr[0];
    if (quote != '\'' && quote != '\"') {
        return 0;
    }

    size_t i = 1;
    while (i < nlen) {
        if (nstr[i] == '\\') {
            /* 跳过'\\' */
            i++;
        } else if (nstr[i] == quote) {
            /* 遇到对应的引号 */
            quote = '\0';
            i++;
            break;
        }

        i++;
    }

    return (i == len) && (quote == '\0');
}

int parser_str(const char *str, size_t len, const char *target) {
    if (target == NULL) {
        return 0;
    }

    size_t nlen = 0;
    const char *nstr = parser_trim(str, len, &nlen);
    if (nstr == NULL || nlen == 0) {
        return 0;
    }

    if (nlen != strlen(target)) {
        return 0;
    }

    return strncmp(nstr, target, nlen) == 0;
}

size_t parser_find_next(const char *str, size_t len, char c) {
    if (str == NULL || len == 0) {
        return len;
    }

    /* {}嵌套数量 */
    int object_num = 0;
    /* []嵌套数量 */
    int array_num = 0;
    /* 引号最多1个而已 */
    char quote = '\0';

    size_t i = 0;
    while (i < len) {
        if (str[i] == '{') {
            object_num++;
        } else if (str[i] == '}') {
            object_num--;
            if (object_num < 0) {
                break;
            }
        } else if (str[i] == '[') {
            array_num++;
        } else if (str[i] == ']') {
            array_num--;
            if (array_num < 0) {
                break;
            }
        } else if (str[i] == '\"' || str[i] == '\'') {
            if (quote == '\0') {
                quote = str[i];
            } else if (quote == str[i]) {
                quote = '\0';
            }
        } else if (str[i] == '\\') {
            i++;
        } else if (str[i] == c) {
            if (object_num == 0 && array_num == 0 && quote == '\0') {
                break;
            }
        }

        i++;
    }

    return i;
}
