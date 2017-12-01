#include <stdio.h>

#include "values.h"
#include "allocator.h"
#include "strings.h"
#include "env.h"
#include "display.h"

static uint64_t id = 0;

Data new_array(uint64_t size){
    Data d;
    d.type = ARR;
    d.refCount = 0;
    d.numElements = size;
    d.arr = (Data *)mallocate(sizeof(Data)*size);
    uint64_t i = 0;
    while(i < size)
        d.arr[i++] = new_null();
    return d;
}

Data new_ins(void *env, uint64_t name){
    Data d;
    d.type = INSTANCE;
    d.pvalue = (Instance *)mallocate(sizeof(Instance));
    d.pvalue->container_key = name;
    d.pvalue->env = (Environment *)mallocate(sizeof(Environment));
    ((Environment *)d.pvalue->env)->parent = ((Environment *)env)->parent;
    ((Environment *)d.pvalue->env)->records = ((Environment *)env)->records;
    d.pvalue->id = id++;
    d.pvalue->refCount = 0;
//    printf(debug("[NewIns] Created [%s#%lu]"), str_get(name), (id - 1));
    return d;
}

void data_free(Data d){
    //if(d.refCount > 0){
    //    d.refCount--;
    //    return;
    //}
    if(isstr(d)){
        str_ref_decr(d.svalue);
    }
    else if(isins(d)){
        tins(d)->refCount--;
        if(tins(d)->refCount > 0){
          //  printf(debug("[DataFree] Refcount of [%s#%lu] is %lu"), str_get(tins(d)->container_key),
          //          tins(d)->id, tins(d)->refCount);
        }
        else{
          //  printf(debug("[DataFree] Refcount of [%s#%lu] is %lu"), str_get(tins(d)->container_key),
          //          tins(d)->id, tins(d)->refCount);
            env_free(*tenv(d));
            memfree(tenv(d));
            memfree(tins(d));
        }
    }
    else if(isarray(d)){
        int64_t i = 0;
        Data *arr = (Data *)d.arr;
        while(i < d.numElements){
            data_free(arr[i]);
            i++;
        }
    }
}
