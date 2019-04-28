#include <stdio.h>
#include "../foreign_interface.h"

NativeData VargsNative(NativeData args) {
    int a = native_expect_int(native_arr_get(args, 0));
    printf("From native\n int a = %d\n", a);
    NativeData varargs = native_expect_arr(native_arr_get(args, 1));
    int size = native_expect_int(native_arr_get(args, 2));
    printf("Number of varargs : %d\n", size);
    return native_fromnull();
}

NativeData PureVargsNative(NativeData args) {
    printf("From native\n");
    NativeData varargs = native_expect_arr(native_arr_get(args, 0));
    int size = native_expect_int(native_arr_get(args, 1));
    printf("Number of varargs : %d\n", size);
    for(int i = 0;i < size;i++) {
        printf("Arg #%d : ", i + 1);
        native_print(native_arr_get(varargs, i));
        printf("\n");
    }
    return native_fromnull();
}
