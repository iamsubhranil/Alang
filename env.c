#include <stdio.h>

#include "env.h"
#include "allocator.h"
#include "display.h"
#include "strings.h"
#include "interpreter.h"

Environment env_new(Environment *parent){
    return (Environment){NULL, parent};
}

static inline Record* new_record(uint32_t key, Data value){
    Record *record = (Record *)mallocate(sizeof(Record));
    record->key = key;
    record->data = value;
    record->next = NULL;
    return record;
}

static inline Environment* env_match(uint32_t key, Environment *env){
    if(env == NULL)
        return NULL;
    Record *top = env->records;
    //    printf(debug("[Env:Match] Searching in %p[parent : %p] for [%s]"), env, env->parent, str_get(key));
    while(top!=NULL){
        //        printf(debug("[Env:Match] Found [%s]"), str_get(top->key));
        if(top->key == key)
            return env;
        top = top->next;
    }
    return env_match(key, env->parent);
}

void env_implicit_put(uint32_t key, Data value, Environment *env){
    Record *top = env->records;
    if(isins(value)){
        tins(value)->refCount++;
    }
    if(top == NULL)
        env->records = new_record(key, value);
    else{
        while(top->next != NULL){
            top = top->next;
        }
        top->next = new_record(key, value);
    }
}

void env_put(uint32_t key, Data value, Environment *env){
    if(env == NULL)
        return;
//    printf(debug("[Env:Put] Putting [%s]"), str_get(key));
    Environment *match = env_match(key, env);
    if(match == NULL)
        match = env;
    Record *top = match->records, *prev = NULL;
    if(isins(value)){
        tins(value)->refCount++;
//        printf(debug("[Env:Put] Incremented refcount of [%s#%lu] to %lu"),
//                str_get(tins(value)->container_key), tins(value)->id, tins(value)->refCount);
    }
    while(top!=NULL){
        if(top->key == key){
            if(isarray(top->data)){
                printf(error("Array '%s' must be accessed using indices!"), str_get(key));
                stop();
            }
            data_free(top->data);
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

Data env_get(uint32_t key, Environment *env, uint8_t beSilent){
//    printf(debug("[Env:Get] Getting [%s]"), str_get(key));
    Environment *match = env_match(key, env);
    if(match == NULL){
        if(!beSilent){
            printf(error("Uninitialized variable '%s'!\n"), str_get(key));
            stop();
        }
        else
            return new_none();
    }
    Record *top = match->records;
    while(top!=NULL){
        if(top->key == key)
            return top->data;
        top = top->next;
    }
    return new_none();
}

void env_free(Environment env){
    Record *top = env.records;
    while(top!=NULL){
        Record *bak = top->next;
        data_free(top->data);
        memfree(top);
        top = bak;
    }
}
