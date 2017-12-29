#pragma once

// From https://github.com/munificent/wren/blob/master/src/vm/wren_value.h
// at 9661a5b

#include <stdint.h>
#include <math.h>
#include "display.h"
//#include "allocator.h"
#include "strings.h"
#include <string.h>

typedef uint64_t Data;

// A mask that selects the sign bit.
#define SIGN_BIT ((uint64_t)1 << 63)
// The bits that must be set to indicate a quiet NaN.
#define QNAN ((uint64_t)0x7ffc000000000000)
// If the NaN bits are set, it's not a number.
#define isfloat(value) ((value & QNAN) != QNAN)
//#define tfloat(value) ((double)value)
static inline double tfloat(Data value){
    double x;
    memcpy(&x, &value, sizeof(double));
    return x;
}
// Masking
// We need to specify various types that Alang already supports and works with
// It is done by masking bits 48-50
// 0 11111111111 11[type]--------------------------------------------
// 0 indicates it is not a pointer
// All 1 exponent indicates NAN, highest 1 in mantissa indicates QNAN
// All integer values will be stored in the lower order 32 bits
// i.e. : IntMask : 0x00000000ffffffff
#define MASK 0xffffffff
// Types :
// 1 : Integer (int32_t) => 0 [111 1111 1111] [1 100 1--------------- [value]] => 0x7ffc800000000000
#define INT 0x7ffc800000000000
#define isint(value) ((value >> 47) == 0x0fff9)
#define isnum(value) (isfloat(value) || isint(value))
#define tint(value) ((int32_t)(value & MASK))
#define tnum(value) (isfloat(value)?tfloat(value):tint(value))
// 2 : String (uint32_t) => 0 [111 1111 1111] [1 101 0------------- [value]] => 0x7ffd00000000000
#define STRING 0x7ffd000000000000
#define isstr(value) ((value >> 47) == 0x0fffa)
#define tstrk(value) ((uint32_t)(value & MASK))
#define tstr(value) (str_get(tstrk(value)))
// 3 : Identifer (uint32_t) => 0 [111 1111 1111] [1 101 1------------- [value]] => 0x7ffd800000000000
#define IDENTIFIER 0x7ffd800000000000
#define isidentifer(value) ((value >> 47) == 0x0fffb)
#define tiden(value) ((uint32_t)(value & MASK))
// 4 : Logical (int32_t) => 0 [111 1111 1111] [1 110 0-------------- [value]] => 0x7ffe00000000000
#define LOGICAL 0x7ffe000000000000
#define islogical(value) ((value >> 47) == 0x0fffc)
#define tlogical(value) ((uint32_t)((uint32_t)value & MASK))
// 5 : Null => 0 [111 1111 1111] [1 110 1------------- [value]] => 0x7ffe800000000000
#define NIL 0x7ffe800000000000
#define isnull(value) ((value >> 47) == 0x0fffd)
// 6 : None => 0 [111 1111 1111] [1 111 0-------------- [value]] => 0x7fff00000000000
#define NONE 0x7fff000000000000
#define isnone(value) ((value >> 47) == 0x0fffe)

static Data inline new_float(double x){
    Data d;
    memcpy(&d, &x, sizeof(Data));
    return d;
}
//#define new_float(value) (value)
static Data inline new_int(int32_t i){
    Data d = 0;
    memcpy(&d, &i, sizeof(i));
    return INT|d;
}
//#define new_int(value) ((INT | value))
static Data inline new_strk(uint32_t value){
    return STRING|value;
}
//#define new_strk(value) ((STRING | value))
static Data inline new_str(const char *str){
    return STRING | str_insert(str);
}
//#define new_str(value) ((STRING | str_insert(value)))
static Data inline new_identiferk(uint32_t value){
    return IDENTIFIER|value;
}
//#define new_identiferk(value) ((IDENTIFIER | value))
static Data inline new_identifer(const char *value){
    return IDENTIFIER | str_insert(value);
}
//#define new_identifer(value) ((IDENTIFIER | str_insert(value)))
static Data inline new_logical(int32_t value){
    return LOGICAL|value;
}
//#define new_logical(value) ((LOGICAL | value))
#define new_null() (NIL)
#define new_none() (NONE)


typedef struct{
    uint32_t container_key;
    uint32_t id;
    uint32_t refCount;
    void *env;
} Instance;

typedef struct{
    Data *arr;
    int32_t numElements;
} Array;

// Two types of pointers are used : Instances and Arrays
// An object pointer is a NaN with a set sign bit.
#define is_pointer(value) (((value) & (QNAN | SIGN_BIT)) == (QNAN | SIGN_BIT))
// 1. Instance : 1 [111 1111 1111] 1 110 -------------- => 0xfffe000000000000
#define INSTANCE 0xfffe000000000000
#define PMASK 0xffffffffffff
#define isins(value) ((value & INSTANCE) == INSTANCE)
#define tins(value) ((Instance *)((uintptr_t)value & PMASK))
#define tenv(value) ((Environment *)tins(value)->env)
#define MAX_FREE_INSTANCES 512

extern Instance *freeInstances[MAX_FREE_INSTANCES];
extern uint16_t freeInstancePointer;

#define ins_new() freeInstancePointer==0?(Instance *)mallocate(sizeof(Instance)):freeInstances[--freeInstancePointer]
#define ins_free(x) {if(freeInstancePointer < MAX_FREE_INSTANCES) \
                        freeInstances[freeInstancePointer++] = x; \
                    else \
                         memfree(x);\
                    }
// 2. Array : 1 [111 1111 1111] 1 101 ---------------- => 0xfffd000000000000
#define ARR 0xfffd000000000000
#define isarray(value) ((value & ARR) == ARR)
#define tarr(value) ((Array *)((uintptr_t)value & PMASK))
#define arr_size(value) (value->numElements)
#define arr_elements(value) (value->arr)

Data new_array(int32_t size);

Data new_ins(void *env, uint32_t container_key);

#define ttype(value) (is_pointer(value)?value & 0xffff000000000000 : value & 0xfffff00000000000)
