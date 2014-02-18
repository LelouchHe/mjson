#ifndef _REF_STR_H
#define _REF_STR_H

#ifdef __cplusplus
extern "C" {
#endif

struct ref_str_t;
typedef struct ref_str_t ref_str_t;

/* 不允许NULL赋值,会自动更新为""  */
ref_str_t *rs_ini(const char *str, int len);
int rs_fini(ref_str_t *rs);

/* 
 *
 * 需要使用返回的ref_str_t
 * 默认是全部
 * 取到指针自己设置(好危险!,不过不这样感觉使用不便)
 *
 */
ref_str_t *rs_use(ref_str_t *rs);

/* 
 *
 * 添加辅助结构,限定边界
 * 如果你非要超出去,我这里也没有办法啊
 * 需要在use之后才能用,并且在fini之后不能使用
 *
 */

struct ref_str_data_t {
    int begin;
    int end;
    const char *str;
};
typedef struct ref_str_data_t ref_str_data_t;

ref_str_data_t rs_get(ref_str_t *rs);

#ifdef DEBUG
void rs_debug(ref_str_t *rs);
#else
#define rs_debug(x)
#endif

#ifdef __cplusplus
}
#endif

#endif
