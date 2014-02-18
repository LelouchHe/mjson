#include <stdlib.h>

#include "map.h"

typedef struct map_node_t map_node_t;

/*
 *
 * value仅仅指针而已,不保存其对象生存
 *
 */
struct map_node_t {
    map_node_t *next; /* 表示下一位,不一定在本链表中,如果跨链表,则指向下一个的头 */
    const char *key;
    const void *value;/* NULL表示是链表结束 */
    int elf;          /* 第二hash */
};

struct map_t {
    map_node_t *table;
    int size;
    int num;          /* 元素个数 */

    map_node_t *head;  /* 指向链表开始的头 */
};

/*
 *
 * 判断n是否为mm中的链表头
 *
 */
static int is_table_head(map_t *mm, map_node_t *n) {
    if (n->value != NULL) {
        return 0;
    }

    return n >= mm->table && n < mm->table + mm->size;
}

/*
 *
 * 判断n指向的链表是否只有一个元素
 *
 */
static int has_only_one(map_t *mm, map_node_t *n) {
    if (!is_table_head(mm, n) || n->next == NULL) {
        return 0;
    }

    map_node_t *cur = n->next;
    if (cur->next == NULL || is_table_head(mm, cur->next)) {
        return 1;
    }

    return 0;
}

map_t *map_ini(int size) {
    map_t *mm = (map_t *)calloc(1, sizeof (map_t));
    if (mm == NULL) {
        return NULL;
    }
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
            free(h);
        }
        h = next;
    }

    free(mm->table);
    free(mm);

    return MAPE_OK;
}

static long djb_hash(const char *str) {
    long hash = 5381;
    int i = 0;
    while (str[i] != '\0') {
        hash = ((hash << 5) + hash) + str[i];

        i++;
    }

    return hash;
}

static long elf_hash(const char *str) {
    long hash = 0;
    long x = 0;
    int i = 0;
    while (str[i] != '\0') {
        hash = (hash << 4) + str[i];
        if((x = hash & 0xF0000000L) != 0) {
            hash ^= (x >> 24);
        }
        hash &= ~x;

        i++;
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
static map_node_t *find_prev(map_t *mm, const char *key) {
    int hash = djb_hash(key) % mm->size;
    int elf = elf_hash(key);
    map_node_t *prev = &(mm->table[hash]);
    while (prev->next != NULL && prev->next->value != NULL
        && prev->next->elf != elf) {
        prev = prev->next;
    }

    return prev;
}

const void *map_get(map_t *mm, const char *key) {
    if (mm == NULL || mm->num == 0) {
        return NULL;
    }

    map_node_t *prev = find_prev(mm, key);

    if (prev->next != NULL && prev->next->value != NULL) {
        return prev->next->value;
    }

    return NULL;
}

/*
 *
 * value=NULL時表示删除操作
 * 只有当node在链表内部才会直接删除
 * 否则,当处于链表头且只有一个元素時,需要特别处理head,所以暂缓删除
 *
 */
int map_set(map_t *mm, const char *key, const void *value) {
    if (mm == NULL) {
        return MAPE_NULL;
    }

    map_node_t *prev = find_prev(mm, key);

    if (prev->next != NULL && prev->next->value != NULL) {
        prev->next->value = value;
        if (value == NULL) {
            if (!has_only_one(mm, prev)) {
                map_node_t *cur = prev->next;
                prev->next = cur->next;
                free(cur);
            }
            mm->num--;
            return MAPE_ERASE;
        }
        return MAPE_OVERWRITTEN;
    }

    if (value == NULL) {
        return MAPE_ERASE;
    }

    map_node_t *node = (map_node_t *)malloc(sizeof (map_node_t));
    node->key = key;
    node->value = value;
    node->elf = elf_hash(node->key);

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

/*
 *
 * 删除map_set時没有删除的项
 * 每个链表最多一项
 *
 */
map_iter_t map_iter_next(map_t *mm, map_iter_t *it) {
    map_node_t *prev = mm->head;
    if (it != NULL) {
        prev = it->v;
    }

    map_iter_t next_it;
    next_it.v = NULL;

    while (prev != NULL && prev->next != NULL) {
        map_node_t *cur = prev->next;
        if (cur->value != NULL) {
            next_it.v = cur;
            return next_it;
        }

        if (is_table_head(mm, cur)) {
            prev = cur;
        } else {
            prev->next = cur->next;
            free(cur);
        }
    }

    return next_it;
}

const char *map_iter_getk(map_iter_t *it) {
    if (it == NULL || it->v == NULL) {
        return NULL;
    }

    return it->v->key;
}

const void *map_iter_getv(map_iter_t *it) {
    if (it == NULL || it->v == NULL) {
        return NULL;
    }

    return it->v->value;
}

#ifndef DEBUG
void map_debug(map_t *mm) {
}
#else
#include <stdio.h>

void map_debug(map_t *mm) {
    if (mm == NULL) {
        printf("mm is null\n");
        return;
    }

    printf("size:%d\tnum:%d\n", mm->size, mm->num);
    printf("head: %d\n", mm->head - mm->table);
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
            printf("(%s, %p) ->", cur->key, cur->value);

            prev = prev->next;
        }
        if (prev->next != NULL) {
            printf("next: %d", prev->next - mm->table);
        }
        printf("\n");
    }
}
#endif

