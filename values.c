#include "values.h"
#include "allocator.h"
#include "strings.h"
#include "env.h"

Data* new_int(int64_t val){
    Data* d = new_data();
    d->type = INT;
    d->refCount = 0;
    d->ivalue = val;
    return d;
}

Data* new_float(double val){
    Data* d = new_data();
    d->type = FLOAT;
    d->refCount = 0;
    d->cvalue = val;
    return d;
}

Data* new_str(const char* val){
    Data* d = new_data();
    d->type = STRING;
    d->refCount = 0;
    d->svalue = str_insert(val);
    return d;
}

Data* new_identifer(const char* val){
    Data* d = new_data();
    d->type = IDENTIFIER;
    d->refCount = 0;
    d->svalue = str_insert(val);
    return d;
}

Data* new_ins(Instance *val){
    Data* d = new_data();
    d->type = INSTANCE;
    d->refCount = 0;
    d->pvalue = val;
    return d;
}

Data* new_logical(uint8_t val){
    Data* d = new_data();
    d->type = LOGICAL;
    d->refCount = 0;
    d->ivalue = val;
    return d;
}

Data* new_null(){
    Data* d = new_data();
    d->type = NIL;
    d->refCount = 0;
    return d;
}

Data* new_array(uint64_t size){
    Data *d = new_data();
    d->type = ARR;
    d->refCount = 0;
    d->numElements = size;
    d->arr = (struct Data **)mallocate(sizeof(struct Data *)*size);
    return d;
}

void data_free(Data *d){
    if(d == NULL || d->refCount > 0)
        return;
    if(d->type == STRING){
        str_ref_decr(d->cvalue);
    }
    else if(d->type == INSTANCE){
        env_free(tenv(d->pvalue));
        memfree(d->pvalue);
    }
    else if(d->type == ARR){
        uint64_t i = 0;
        while(i < d->numElements){
            data_free((Data *)d->arr[i]);
            i++;
        }
    }
    memfree(d);
}
