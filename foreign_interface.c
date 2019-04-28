#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "display.h"
#include "foreign_interface.h"
#include "interpreter.h"
#include "strings.h"
#include "values.h"

static inline uint8_t isnins(NativeData d) {
	return isins(d);
}

static inline uint8_t isnarr(NativeData d) {
	return isarray(d);
}

uint8_t native_isint(NativeData d) {
	return isint(d);
}

uint8_t native_isfloat(NativeData d) {
	return isfloat(d);
}

uint8_t native_isbool(NativeData d) {
	return islogical(d);
}

uint8_t native_isstr(NativeData d) {
	return isstr(d);
}

uint8_t native_isnull(NativeData d) {
	return isnull(d);
}

uint8_t native_isins(NativeData d) {
	return isnins(d);
}

uint8_t native_isarr(NativeData d) {
	return isnarr(d);
}

int32_t native_toint(NativeData d) {
	return tint(d);
}

double native_tofloat(NativeData d) {
	return tfloat(d);
}

uint8_t native_tobool(NativeData d) {
	return tlogical(d);
}

const char *native_tostr(NativeData d) {
	return tstr(d);
}

void __native_rerr_impl(const char *msg, const char *fun, int line, ...) {
	printf(ANSI_COLOR_RED "\n[Native Error] " ANSI_COLOR_RESET ANSI_FONT_BOLD
	                      "<%s:%d> " ANSI_COLOR_RESET,
	       fun, line);
	va_list args;
	va_start(args, line);
	vprintf(msg, args);
	va_end(args);
	printf("\n");
	print_stack_trace();
	stop();
}

void __native_rwarn_impl(const char *msg, const char *fun, int line, ...) {
	printf(ANSI_COLOR_YELLOW "[Native Warning] " ANSI_COLOR_RESET ANSI_FONT_BOLD
	                         "<%s:%d> " ANSI_COLOR_RESET,
	       fun, line);
	va_list args;
	va_start(args, line);
	vprintf(msg, args);
	va_end(args);
	printf("\n");
}

int32_t __native_expect_impl_int(NativeData d, const char *func, int line) {
	if(native_isint(d))
		return native_toint(d);
	else
		__native_rerr_impl("Expected "
		                   "integer"
		                   "!",
		                   func, line);
	return 1;
}

double __native_expect_impl_float(NativeData d, const char *func, int line) {
	if(native_isfloat(d))
		return native_tofloat(d);
	else
		__native_rerr_impl("Expected "
		                   "float"
		                   "!",
		                   func, line);
	return 1;
}

uint8_t __native_expect_impl_bool(NativeData d, const char *func, int line) {
	if(native_isbool(d))
		return native_tobool(d);
	else
		__native_rerr_impl("Expected "
		                   "boolean"
		                   "!",
		                   func, line);
	return 1;
}

const char *__native_expect_impl_str(NativeData d, const char *func, int line) {
	if(native_isstr(d))
		return native_tostr(d);
	else
		__native_rerr_impl("Expected "
		                   "string"
		                   "!",
		                   func, line);
	return "No";
}

NativeData __native_expect_impl_arr(NativeData d, const char *func, int line) {
	if(native_isarr(d))
		return d;
	else
		__native_rerr_impl("Expected "
		                   "array"
		                   "!",
		                   func, line);
	return 0;
}

NativeData __native_expect_impl_ins(NativeData d, const char *func, int line) {
	if(native_isins(d))
		return d;
	else
		__native_rerr_impl("Expected "
		                   "instance"
		                   "!",
		                   func, line);
	return 0;
}

NativeData native_fromint(int32_t i) {
	return new_int(i);
}

NativeData native_fromfloat(double d) {
	return new_float(d);
}

NativeData native_frombool(uint8_t b) {
	return new_logical(b);
}

NativeData native_fromstr(const char *str) {
	return new_str(strdup(str));
}

NativeData native_fromnull() {
	return new_null();
}

NativeData native_arr_new(size_t size) {
	Data arr = new_array(size);
	return arr;
}

size_t native_arr_size(NativeData na) {
	return arr_size(tarr(na));
}

void native_arr_set(NativeData na, size_t pos, NativeData value) {
	arr_elements(tarr(na))[pos] = value;
}

NativeData native_arr_get(NativeData na, size_t pos) {
	return arr_elements(tarr(na))[pos];
}

void native_arr_free(NativeData na) {
	obj_ref_decr(tarr(na));
}

void native_ins_set_member(NativeData value, const char *name,
                           NativeData data) {
	set_member(tins(value), str_insert(strdup(name), 0), data);
}

NativeData native_ins_get_member(NativeData ins, const char *name) {
	Data ret = get_member(tins(ins), str_insert(strdup(name), 0));
	return ret;
}

void native_print(NativeData d) {
	data_print(d);
}
