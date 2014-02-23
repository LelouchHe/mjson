#ifndef _VECTOR_H
#define _VECTOR_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define VECE_NULL -1

struct vector_t;
typedef struct vector_t vector_t;

vector_t *vec_ini(size_t size);
void vec_fini(vector_t *vec);

size_t vec_size(vector_t *vec);
size_t vec_num(vector_t *vec);

/* 没有默认值,get之后的数据有可能是随机 */
const void *vec_get(vector_t *vec, size_t offset);
/* 
 *
 * 因为vec需要连续,所以value可以为NULL,删除需要显式删除
 * 返回值是添加的个数
 * -1表示vec为NULL
 * 0表示没有添加成功
 *
 */
int vec_set(vector_t *vec, size_t offset, const void *value);
const void *vec_erase(vector_t *vec, size_t offset);

const void **vec_data(vector_t *vec);

/*
 *
 * 下面是一些便于使用的上述API的简单封装
 *
 */

/* 实现原因,只优先支持末尾操作 */
int vec_push(vector_t *vec, const void *value);
const void *vec_pop(vector_t *vec);
const void *vec_front(vector_t *vec);
const void *vec_back(vector_t *vec);

/*
 *
 * 赋值以NULL为结束符,所以不能赋值NULL
 * 需要赋值NULL的请用其他形式
 * just for fun
 *
 */
#define vec_append(vec, args...) vec_append_private(vec, ##args, NULL)
int vec_append_private(vector_t *vec, ...);

#ifdef __cplusplus
}
#endif

#endif
