#include "values.h"
#include "allocator.h"
#include "strings.h"
#include "env.h"

Data new_array(uint64_t size){
    Data d;
    d.type = ARR;
    d.refCount = 0;
    d.numElements = size;
    d.arr = mallocate(sizeof(Data)*size);
    return d;
}

void data_free(Data d){
    if(d.refCount > 0){
        d.refCount--;
        return;
    }
    if(isstr(d)){
        str_ref_decr(d.svalue);
    }
    else if(d.type == INSTANCE){
        env_free(tenv(d.pvalue));
        memfree(d.pvalue);
    }
    else if(d.type == ARR){
        uint64_t i = 0;
        Data *arr = (Data *)d.arr;
        while(i < d.numElements){
            data_free(arr[i]);
            i++;
        }
    }
}
