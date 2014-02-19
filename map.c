#include <stdlib.h>
#include <string.h>

#include "map.h"

typedef struct map_node_t map_node_t;

/*
 *
 * value仅仅指针而已,不保存其对象生存
 *
 */
struct map_node_t {
    map_node_t *next; /* 表示下一位,不一定在本链表中,如果跨链表,则指向下一个的头 */
    ref_str_t *key;
    const void *value;/* NULL表示是链表结束 */
    int elf;          /* 第二hash */
};

struct map_t {
    map_node_t *table;
    size_t size;
    size_t num;          /* 元素个数 */

    map_node_t *head;  /* 指向链表开始的头 */
};

/*
 *
 * 判断n是否为mm中的链表头
 *
 */
static int is_table_head(map_t *mm, map_node_t *n) {
    if (n == NULL || n->value != NULL) {
        return 0;
    }

    return n >= mm->table && n < mm->table + mm->size;
}

static size_t primes[] = {
    17,             /* 0 */
    37,             /* 1 */
    79,             /* 2 */
    163,            /* 3 */
    331,            /* 4 */
    673,            /* 5 */
    1361,           /* 6 */
    2729,           /* 7 */
    5471,           /* 8 */
    10949,          /* 9 */
    21911,          /* 10 */
    43853,          /* 11 */
    87719,          /* 12 */
    175447,         /* 13 */
    350899,         /* 14 */
    701819,         /* 15 */
    1403641,        /* 16 */
    2807303,        /* 17 */
    5614657,        /* 18 */
    11229331,       /* 19 */
    22458671,       /* 20 */
    44917381,       /* 21 */
    89834777,       /* 22 */
    179669557,      /* 23 */
    359339171,      /* 24 */
    718678369,      /* 25 */
    1437356741,     /* 26 */
    2147483647      /* 27 (largest signed int prime) */
};

static size_t up_size(size_t size) {
    size_t l = 0;
    size_t r = sizeof (primes) / sizeof (primes[0]);
    while (l < r) {
        size_t m = l + (r - l) / 2; 
        if (size <= primes[m]) {
            r = m;
        } else {
            l = m + 1;
        }
    }

    return primes[r];
}

map_t *map_ini(size_t size) {
    map_t *mm = (map_t *)calloc(1, sizeof (map_t));
    if (mm == NULL) {
        return NULL;
    }
    size = up_size(size);

    mm->size = size;
    mm->table = (map_node_t *)calloc(mm->size, sizeof (map_node_t));
    if (mm->table == NULL) {
        free(mm);
        return NULL;
    }

    return mm;
}

int map_fini(map_t *mm) {
    if (mm == NULL) {
        return MAPE_NULL;
    }

    map_node_t *h = mm->head;
    while (h != NULL) {
        map_node_t *next = h->next;
        if (!is_table_head(mm, h)) {
            rs_fini(h->key);
            free(h);
        }
        h = next;
    }

    free(mm->table);
    free(mm);

    return MAPE_OK;
}

size_t map_size(map_t *mm) {
    if (mm == NULL) {
        return 0;
    }

    return mm->size;
}

size_t map_num(map_t *mm) {
    if (mm == NULL) {
        return 0;
    }

    return mm->num;
}

static long djb_hash(const char *str, size_t begin, size_t end) {
    long hash = 5381;
    while (begin != end) {
        hash = ((hash << 5) + hash) + str[begin];

        begin++;
    }

    return hash;
}

static long elf_hash(const char *str, size_t begin, size_t end) {
    long hash = 0;
    long x = 0;
    while (begin != end) {
        hash = (hash << 4) + str[begin];
        if((x = hash & 0xF0000000L) != 0) {
            hash ^= (x >> 24);
        }
        hash &= ~x;

        begin++;
    }

    return hash;
}

/*
 *
 * 返回的prev->next有以下性质:
 * 0. 为NULL
 * 1. 无效(value==NULL)
 * 2. 为该key
 *
 */
static map_node_t *find_prev(map_t *mm, const char *key, size_t begin, size_t end) {
    int hash = djb_hash(key, begin, end) % mm->size;
    int elf = elf_hash(key, begin, end);
    map_node_t *prev = &(mm->table[hash]);
    while (prev->next != NULL && prev->next->value != NULL
        && prev->next->elf != elf) {
        prev = prev->next;
    }

    return prev;
}

static const void *map_get_raw(map_t *mm, const char *key, size_t begin, size_t end) {
    if (key == NULL || begin >= end) {
        return NULL;
    }

    map_node_t *prev = find_prev(mm, key, begin, end);

    if (prev->next != NULL && prev->next->value != NULL) {
        return prev->next->value;
    }

    return NULL;
}

const void *map_get_ref(map_t *mm, ref_str_t *key) {
    if (mm == NULL || mm->num == 0 || key == NULL) {
        return NULL;
    }

    ref_str_data_t d = rs_get(key);

    return map_get_raw(mm, d.str, d.begin, d.end);
}

const void *map_get(map_t *mm, const char *key) {
    if (mm == NULL || mm->num == 0 || key == NULL) {
        return NULL;
    }

    return map_get_raw(mm, key, 0, strlen(key));
}

/*
 *
 * value=NULL時表示删除操作
 * 允许两不同链表头相连,合并删除工作在iter函数中操作
 *
 */

int map_set_raw(map_t *mm, ref_str_t *key, const void *value, int is_ref) {
    ref_str_data_t d = rs_get(key);
    map_node_t *prev = find_prev(mm, d.str, d.begin, d.end);

    if (prev->next != NULL && prev->next->value != NULL) {
        prev->next->value = value;
        /* 非引用需要清理key */
        if (!is_ref) {
            rs_fini(key);
        }
        if (value == NULL) {
            map_node_t *cur = prev->next;
            prev->next = cur->next;
            rs_fini(cur->key);
            free(cur);
            mm->num--;
            if (mm->num == 0) {
                mm->head = NULL;
            }
            return MAPE_ERASE;
        }
        return MAPE_OVERWRITTEN;
    }

    if (value == NULL) {
        return MAPE_ERASE;
    }

    map_node_t *node = (map_node_t *)malloc(sizeof (map_node_t));
    if (is_ref) {
        node->key = rs_use(key);
    } else {
        node->key = rs_move(key);
    }
    node->value = value;
    node->elf = elf_hash(d.str, d.begin, d.end);

    node->next = prev->next;
    prev->next = node;

    /* 链表头第一个添加的元素 */
    if (is_table_head(mm, prev)) {
        node->next = mm->head;
        mm->head = prev;
    }
    mm->num++;

    return MAPE_OK;
}

int map_set_ref(map_t *mm, ref_str_t *key, const void *value) {
    if (mm == NULL) {
        return MAPE_NULL;
    }

    return map_set_raw(mm, key, value, 1);
}

int map_set(map_t *mm, const char *key, const void *value) {
    if (mm == NULL) {
        return MAPE_NULL;
    }

    ref_str_t *rs = rs_ini(key, 0);
    if (rs == NULL) {
        return MAPE_MEM;
    }

    return map_set_raw(mm, rs, value, 0);
}

static const map_iter_t null_iter;

/*
 *
 * it=NULL获取begin
 * 顺便删除相连链表头
 *
 */
map_iter_t map_iter_next(map_t *mm, map_iter_t *it) {
    map_node_t h;
    h.key = NULL;
    h.value = (const void *)0x1984dead;
    h.next = mm->head;
    map_node_t *prev = &h;

    if (it != NULL) {
        prev = it->v;
    }

    map_iter_t next_it = null_iter;

    while (prev != NULL && prev->next != NULL) {
        map_node_t *cur = prev->next;
        if (cur->value != NULL) {
            next_it.v = cur;
            return next_it;
        }

        if (is_table_head(mm, cur->next)) {
            prev->next = cur->next;
            cur->next = NULL;
            if (mm->head == cur) {
                mm->head = prev->next;
            }
        } else {
            prev = prev->next;
        }
    }

    return next_it;
}

ref_str_data_t map_iter_getk(map_iter_t *it) {
    if (it == NULL || it->v == NULL) {
        return rs_get(NULL);
    }

    return rs_get(it->v->key);
}

const void *map_iter_getv(map_iter_t *it) {
    if (it == NULL || it->v == NULL) {
        return NULL;
    }

    return it->v->value;
}

#ifdef DEBUG
#include <stdio.h>

void map_debug(map_t *mm) {
    if (mm == NULL) {
        printf("mm is null\n");
        return;
    }

    printf("size:%lu\tnum:%lu\n", mm->size, mm->num);
    printf("head: %ld\n", mm->head == NULL ? -1 : mm->head - mm->table);
    int i = 0;
    for (i = 0; i < mm->size; i++) {
        map_node_t *prev = &(mm->table[i]);
        if (prev->next == NULL) {
            printf("%d: empty\n", i);
            continue;
        }

        printf("%d: ", i);
        while (prev->next != NULL && !is_table_head(mm, prev->next)) {
            map_node_t *cur = prev->next;
            ref_str_data_t d = rs_get(cur->key);
            printf("(%s, %p) ->", d.str, cur->value);

            prev = prev->next;
        }
        if (prev->next != NULL) {
            printf("next: %ld", prev->next - mm->table);
        }
        printf("\n");
    }
}
#endif

