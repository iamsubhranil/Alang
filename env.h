#pragma once

#include "values.h"
#include "strings.h"

typedef struct Record{
    uint32_t key;
    Data data;
    struct Record *next;
} Record;

typedef struct Environment{
    Record *records;
    struct Environment *parent;
} Environment;

static inline void env_put(uint32_t key, Data value, Environment *env);
static inline void env_implicit_put(uint32_t key, Data value, Environment *env);
static inline Data env_get(uint32_t key, Environment *env, uint8_t beSilent);
static inline void env_free(Environment env);

static inline void data_free(Data d);

#include "allocator.h"
#include "display.h"
#include "strings.h"
#include "interpreter.h"

#define env_new(parent) (Environment){NULL, parent}

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

static inline void env_implicit_put(uint32_t key, Data value, Environment *env){
    Record *top = env->records;
    if(isins(value)){
        tins(value)->refCount++;
    }
    else if(isstr(value))
        str_ref_incr(tstrk(value));

    if(top == NULL)
        env->records = new_record(key, value);
    else{
        while(top->next != NULL){
            top = top->next;
        }
        top->next = new_record(key, value);
    }
}

static void env_put(uint32_t key, Data value, Environment *env){
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
    else if(isstr(value))
        str_ref_incr(tstrk(value));

    while(top!=NULL){
        if(top->key == key){
            if(isarray(top->data)){
                rerr("Array '%s' must be accessed using indices!", str_get(key));
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

static inline Data env_get(uint32_t key, Environment *env, uint8_t beSilent){
    //    printf(debug("[Env:Get] Getting [%s]"), str_get(key));
    Environment *match = env_match(key, env);
    if(match == NULL){
        if(!beSilent){
            rerr("Uninitialized variable '%s'!", str_get(key));
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

static inline void env_free(Environment env){
    Record *top = env.records;
    while(top!=NULL){
        Record *bak = top->next;
        data_free(top->data);
        memfree(top);
        top = bak;
    }
}

static inline void data_free(Data d){
    //if(d.refCount > 0){
    //    d.refCount--;
    //    return;
    //}
    if(isstr(d)){
        str_ref_decr(d.svalue);
    }
    else if(isins(d)){
        tins(d)->refCount--;
        if(tins(d)->refCount < 1){
            env_free(*tenv(d));
            memfree(tenv(d));
            memfree(tins(d));
        }
    }
    else if(isarray(d)){
        uint32_t i = 0;
        Data *arr = d.arr;
        while(i < d.numElements){
            data_free(arr[i]);
            i++;
        }
    }
}
