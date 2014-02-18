#ifndef _REF_STR_H
#define _REF_STR_H

#ifdef __cplusplus
extern "C" {
#endif

struct ref_str_in_t;

/*
 *
 * 目的是作为局部变量來使用
 * 否则ref_str_t本身的生存期也需要考虑
 *
 */
struct ref_str_t {
    int begin;
    int end;
    struct ref_str_in_t *rsi;
};
typedef struct ref_str_t ref_str_t;

/* 不允许NULL赋值,会自动更新为""  */
ref_str_t rs_ini(const char *str, int len);
int rs_fini(ref_str_t *rs);

/* 
 *
 * 需要使用返回的ref_str_t
 * 默认是全部
 * 取到指针自己设置(好危险!,不过不这样感觉使用不便)
 *
 */
ref_str_t rs_use(ref_str_t *rs);

/* 
 *
 * '\0'结尾 需要自己begin/end限制
 * 相信你了,程序员
 */
const char *rs_get(ref_str_t *rs);

#ifdef __cplusplus
}
#endif

#endif
