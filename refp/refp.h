#ifndef _REFP_H
#define _REFP_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define REFP_SAME 1
#define REFP_OK 0
#define REFP_NULL -1
#define REFP_MEM -2

struct refp_t;
typedef struct refp_t refp_t;
typedef void (*rp_fini_fun)(void *data);

/* 不接受NULL */
refp_t *rp_ini(void *data, rp_fini_fun f);
refp_t *rp_ini_copy(refp_t *rp);

void rp_fini(refp_t *rp);

/*
 *
 * 返回<0表示没有成功
 * 此时,原值不会被修改
 *
 */
int rp_assign(refp_t *d, refp_t *s);
int rp_reset(refp_t *rp, void *data, rp_fini_fun f);

void *rp_get(refp_t *rp);
size_t rp_ref(refp_t *rp);
void rp_swap(refp_t *l, refp_t *r);

#ifdef __cplusplus
}
#endif

#endif
