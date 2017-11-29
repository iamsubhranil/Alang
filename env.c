#include <stdio.h>

#include "env.h"
#include "allocator.h"
#include "display.h"
#include "strings.h"
#include "interpreter.h"

Environment env_new(Environment *parent){
    return (Environment){NULL, parent};
}

static Record* new_record(uint64_t key, Data value){
    Record *record = (Record *)mallocate(sizeof(Record));
    record->key = key;
    record->data = value;
    record->next = NULL;
    return record;
}

static Environment* env_match(uint64_t key, Environment *env){
    if(env == NULL)
        return NULL;
    Record *top = env->records;
//    printf(debug("[Env:Match] Searching in %p[parent : %p] for [%s]"), env, env->parent, str_get(key));
    while(top!=NULL){
        if(top->key == key)
            return env;
        top = top->next;
    }
    return env_match(key, env->parent);
}

void env_put(uint64_t key, Data value, Environment *env){
    if(env == NULL)
        return;
//    printf(debug("[Env:Put] Putting [%s]"), str_get(key));
    Environment *match = env_match(key, env);
    if(match == NULL)
        match = env;
    Record *top = match->records, *prev = NULL;
    while(top!=NULL){
        if(top->key == key){
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
    //printf(debug("[Env:Get] Getting [%s]"), str_get(key));
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
