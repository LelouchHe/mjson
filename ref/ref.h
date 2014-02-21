#ifndef _REF_H
#define _REF_H

#ifdef __cplusplus
extern "C" {
#endif

struct ref_t;
typedef struct ref_t ref_t;

ref_t *ref_ini();
void ref_fini(ref_t *r);

/*
 *
 * 返回的是修改之后的引用计数值
 *
 */
size_t ref_inc(ref_t *r);
size_t ref_dec(ref_t *r);

size_t ref_ref(ref_t *r);

#ifdef __cplusplus
}
#endif

#endif
