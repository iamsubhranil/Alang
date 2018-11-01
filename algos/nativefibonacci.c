#include "../foreign_interface.h"

static int32_t fib(int32_t r) {
	if(r < 2)
		return r;
	return fib(r - 1) + fib(r - 2);
}

NativeData NativeFibonacci(NativeData args) {
	return native_fromint(fib(native_expect_int(native_arr_get(args, 0))));
}
