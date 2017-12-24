#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include "values.h"
#include "allocator.h"
#include "env.h"

static uint32_t id = 0;

Data new_array(int32_t size){
    Array *arr = (Array *)mallocate(sizeof(Array));
    arr->numElements = size;
    arr->arr = (Data *)mallocate(sizeof(Data) * size);
    return ARR | (uintptr_t)arr;
}

Data new_ins(void *env, uint32_t key){
    Instance *ins = ins_new();
    ins->container_key = key;
    ins->env = ienv_new();
    Environment *n = (Environment *)ins->env;
    Environment *o = (Environment *)env;
    n->parent = o->parent;
    n->records = o->records;
    ins->refCount = 0;
    ins->id = id++;
    return INSTANCE| (uintptr_t)ins;
}

void print_bit(Data d){
    uint8_t *bits = (uint8_t *)&d;
    size_t i = sizeof(d);
    printf("\tBytes : ");
    while(i > 0)
        printf("%x", bits[--i]);
}

void print_type(const char *expected, Data d){
    printf("\nExpected : %s", expected);
    printf("\nReceived : ");
    if(isfloat(d)){
        printf("Float\tValue : %F", tfloat(d));
        //printf("\t&QNAN : ");
        //print_bit(d & QNAN);
    }
    else if(isint(d))
        printf("Integer\tValue : %" PRId32, tint(d));
    else if(islogical(d))
        printf("Logical\tValue : %" PRId32, tlogical(d));
    else if(isstr(d))
        printf("String\tKey : %" PRId32 "\tValue : %s", tstrk(d), tstr(d));
    else if(isidentifer(d))
        printf("Identifier\tKey : %" PRId32 "\tValue : %s", tstrk(d), tstr(d));
    else if(isnull(d))
        printf("Null");
    else if(isnone(d))
        printf("None");
    else if(isins(d))
        printf("Instance");
    else if(isarray(d)){
        printf("Array");
        Array *arr = tarr(d);
        printf("\tSize : %" PRId32, arr_size(arr));
    }
    
    print_bit(d);

    printf("\n");
}

/*

int main(){
    print_type("Float", new_float(9.9878938438));
    print_type("Int", new_int(9));
    print_type("Logical", new_logical(1));
    print_type("Identifier", new_identifer("Iden"));
    print_type("String", new_str("This is a string!"));
    print_type("Null", new_null());
    print_type("None", new_none());
    print_type("Array(5)", new_array(5));
    print_type("Instance", new_ins(NULL, 9));
}
*/
