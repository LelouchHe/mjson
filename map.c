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
    void *value;      /* NULL表示是链表结束 */
    int elf;          /* 第二hash */
};

struct map_t {
    map_node_t *table;
    int size;

    map_node_t head;  /* 指向链表开始的头 */
};


map_t *map_ini(int size) {
    map_t *mm = (map_t *)malloc(sizeof (map_t));
    if (mm == NULL) {
        return NULL;
    }
    mm->size = size;
    mm.head.next = NULL;
    mm.head.value = NULL;
    m->table = (map_node_t *)calloc(mm->size, sizeof (map_node_t));
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

    map_node_t *h = mm->head->next;
    while (h != NULL) {
        map_node_t *next = h->next;
        free(h);
        h = next;
    }

    free(mm->table);
    free(mm);

    return MAPE_OK;
}

static long djb_hash(const char *str) {
    long hash = 5381;
    for(int i = 0; str[i] != '\0'; i++) {
        hash = ((hash << 5) + hash) + str[i];
    }

    return hash;
}

static long elf_hash(const char *str) {
    long hash = 0;
    long x = 0;
    for(int i = 0; str[i] != '\0'; i++) {
        hash = (hash << 4) + str[i];
        if((x = hash & 0xF0000000L) != 0) {
            hash ^= (x >> 24);
        }
        hash &= ~x;
    }

    return hash;
}

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

void *map_get(map_t *mm, const char *key) {
    if (mm == NULL) {
        return NULL;
    }

    map_node_t *prev = find_prev(mm, key);

    if (prev->next != NULL && prev->next->value != NULL) {
        return prev->next->value;
    }

    return NULL;
}

// TODO:
// value=NULL: 删除元素
int map_set(map_t *mm, const char *key, void *value) {
    if (mm == NULL) {
        return MAPE_NULL;
    }

    map_node_t *prev = find_prev(mm, key);

    if (prev->next != NULL && prev->next->value != NULL) {
        prev->next->val = value;
        return MAPE_OVERWRITTEN;
    }

    map_node_t *node = (map_node_t *)malloc(sizeof (map_node_t));
    node->value = value;
    node->elf = elf_hash(key);

    node->next = prev->next;
    prev->next = node;

    /* 链表头第一个添加的元素 */
    if (prev->value == NULL) {
        node->next = mm->head.next;
        mm->head.next = prev;
    }

    return MAPE_OK;
}
