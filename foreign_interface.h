#ifndef FOREIGN_INTERFACE_H
#define FOREIGN_INTERFACE_H

#include <stdint.h>
#include <stdlib.h>

// It's a nantagged type, don't mess with it directly,
// or bad things will happen
typedef uint64_t NativeData;

// Query
uint8_t native_isint(NativeData d);
uint8_t native_isfloat(NativeData d);
uint8_t native_isbool(NativeData d);
uint8_t native_isstr(NativeData d);
uint8_t native_isins(NativeData d);
uint8_t native_isarr(NativeData d);
uint8_t native_isnull(NativeData d);
// Retrieve
int32_t     native_toint(NativeData d);
double      native_tofloat(NativeData d);
uint8_t     native_tobool(NativeData d);
const char *native_tostr(NativeData d);
// NativeArr   native_toarr(NativeData d);
// NativeIns   native_toins(NativeData d);
// Error report
void __native_rerr_impl(const char *msg, const char *fun, int line, ...);
void __native_rwarn_impl(const char *msg, const char *fun, int line, ...);
#define native_rerr(msg, ...) \
	__native_rerr_impl(msg, __func__, __LINE__, ##__VA_ARGS__)
#define native_rwarn(msg, ...) \
	__native_rwarn_impl(msg, __func__, __LINE__, ##__VA_ARGS__)
// Match or fail
// These functions will call rerr() if
// the types do not match
int32_t __native_expect_impl_int(NativeData d, const char *func, int line);
double  __native_expect_impl_float(NativeData d, const char *func, int line);
uint8_t __native_expect_impl_bool(NativeData d, const char *func, int line);
const char *__native_expect_impl_str(NativeData d, const char *func, int line);
NativeData  __native_expect_impl_arr(NativeData d, const char *func, int line);
NativeData  __native_expect_impl_ins(NativeData d, const char *func, int line);

#define native_expect_int(d) __native_expect_impl_int(d, __func__, __LINE__)
#define native_expect_float(d) __native_expect_impl_float(d, __func__, __LINE__)
#define native_expect_bool(d) __native_expect_impl_bool(d, __func__, __LINE__)
#define native_expect_str(d) __native_expect_impl_str(d, __func__, __LINE__)
#define native_expect_arr(d) __native_expect_impl_arr(d, __func__, __LINE__)
#define native_expect_ins(d) __native_expect_impl_ins(d, __func__, __LINE__)
// NativeData conversion functions
NativeData native_fromint(int32_t i);
NativeData native_fromfloat(double d);
NativeData native_frombool(uint8_t b);
NativeData native_fromstr(const char *str);
// NativeData native_fromarr(NativeArr na);
// NativeData native_fromins(NativeIns ni);
NativeData native_fromnull();
// native_array manipulation functions
NativeData native_arr_new(size_t size);
size_t     native_arr_size(NativeData na);
void       native_arr_set(NativeData arr, size_t pos, NativeData value);
NativeData native_arr_get(NativeData na, size_t pos);
void       native_arr_free(NativeData na);
// Instance manipulation functions
// NativeIns  native_ins_new(const char *name, ...);
// void       native_ins_set_arg(size_t pos, NativeData value);
// NativeData native_ins_get_arg(size_t pos);
void native_ins_set_member(NativeData ins, const char *name, NativeData value);
NativeData native_ins_get_member(NativeData ins, const char *name);

void native_print(NativeData nd);
#endif
