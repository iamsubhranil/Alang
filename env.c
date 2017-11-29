#include <stdio.h>

#include "env.h"
#include "allocator.h"
#include "display.h"
#include "strings.h"
#include "interpreter.h"

Environment env_new(Environment *parent){
    Environment env;
    env.records = NULL;
    env.parent = parent;
    return env;
}

static Record* new_record(uint64_t key, Data value){
    Record *record = (Record *)mallocate(sizeof(Record));
    record->key = key;
    record->data = value;
    record->next = NULL;
    return record;
}

void env_put(uint64_t key, Data value, Environment *env){
    if(env == NULL)
        return;
    Record *top = env->records, *prev = NULL;
    value.refCount++;
    while(top!=NULL){
        if(top->key == key){
            Data old = top->data;
            old.refCount--;
            if(old.refCount == 0)
                data_free(old);
            top->data = value;
            return;
        }
        prev = top;
        top = top->next;
    }
    if(prev == NULL)
        env->records = new_record(key, value);
    else
        prev->next = new_record(key, value);
}

Data env_get(uint64_t key, Environment *env, uint8_t beSilent){
    if(env == NULL)
        return new_none();
    Record *top = env->records;
    while(top!=NULL){
        if(top->key == key)
            return top->data;
        top = top->next;
    }
    Data p = env_get(key, env->parent, beSilent);
    if(isnone(p) && !beSilent){
        printf(error("Uninitialized variable '%s'!\n"), str_get(key));
        stop();
    }
    return p;
}

void env_free(Environment env){
    uint64_t i = 0;
    Record *top = env.records;
    while(top!=NULL){
        Record *bak = top->next;
        data_free(top->data);
        memfree(top);
        top = bak;
    }
}
