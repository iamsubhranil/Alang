#include <math.h>

#include "../foreign_interface.h"

NativeData Sin(NativeData args) {
    return native_fromfloat(sin(native_expect_float(native_arr_get(args, 0))));
}

NativeData ASin(NativeData args) {
    return native_fromfloat(asin(native_expect_float(native_arr_get(args, 0))));
}

NativeData Sinh(NativeData args) {
    return native_fromfloat(sinh(native_expect_float(native_arr_get(args, 0))));
}

NativeData Cos(NativeData args) {
    return native_fromfloat(cos(native_expect_float(native_arr_get(args, 0))));
}

NativeData ACos(NativeData args) {
    return native_fromfloat(acos(native_expect_float(native_arr_get(args, 0))));
}

NativeData Cosh(NativeData args) {
    return native_fromfloat(cosh(native_expect_float(native_arr_get(args, 0))));
}

NativeData Tan(NativeData args) {
    return native_fromfloat(tan(native_expect_float(native_arr_get(args, 0))));
}

NativeData ATan(NativeData args) {
    return native_fromfloat(atan(native_expect_float(native_arr_get(args, 0))));
}

NativeData Tanh(NativeData args) {
    return native_fromfloat(tanh(native_expect_float(native_arr_get(args, 0))));
}

NativeData Log(NativeData args) {
    return native_fromfloat(log(native_expect_float(native_arr_get(args, 0))));
}

NativeData Log10(NativeData args) {
    return native_fromfloat(log10(native_expect_float(native_arr_get(args, 0))));
}

NativeData Exp(NativeData args) {
    return native_fromfloat(exp(native_expect_float(native_arr_get(args, 0))));
}
