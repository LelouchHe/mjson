#ifndef _REF_STR_H
#define _REF_STR_H

#ifdef __cplusplus
extern "C" {
#endif

struct ref_str_t;
typedef struct ref_str_t ref_str_t;

/* len=0表示默认长度  */
ref_str_t *rs_ini(const char *str, size_t len);
/* 直接使用新建的str,str必须在heap上 */
ref_str_t *rs_ini_new(const char *str, size_t len);
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
 * 类似直接赋值,之后rs不能再使用(即不用释放了)
 */
ref_str_t *rs_move(ref_str_t *rs);

int rs_reset(ref_str_t *rs, const char *str, size_t len);
int rs_reset_new(ref_str_t *rs, const char *str, size_t len);

struct ref_str_data_t {
    size_t begin;
    size_t end;
    const char *str;
};
typedef struct ref_str_data_t ref_str_data_t;

ref_str_data_t rs_get(ref_str_t *rs);
int rs_set_range(ref_str_t *rs, size_t begin, size_t end);
int rs_set_begin(ref_str_t *rs, size_t begin);
int rs_set_end(ref_str_t *rs, size_t end);

#ifdef DEBUG
void rs_debug(ref_str_t *rs);
#else
#define rs_debug(x)
#endif

#ifdef __cplusplus
}
#endif

#endif
