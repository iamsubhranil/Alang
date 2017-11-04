#include <string.h>
#include <stdio.h>

#include "expr.h"
#include "display.h"
#include "allocator.h"
#include "environment.h"
#include "interpreter.h"

static void insert(Record *toInsert, Environment *parent){ 
    if(parent->front == NULL){
        parent->front = parent->rear = toInsert;
    }
    else{
        parent->rear->next = toInsert;
        parent->rear = toInsert;
    }
}

static void rec_new(char* identifer, Literal value, Environment *parent){
    Record *env = (Record *)mallocate(sizeof(Record));
    env->name = identifer;
    env->value = value;
    env->type = LITERAL;
    env->next = NULL;
    insert(env, parent);
}

static Record* env_match(char* identifer, Environment *env){
    if(env == NULL)
        return NULL;
    Record *bak = env->front;
    while(bak != NULL){
        if(strcmp(bak->name, identifer) == 0)
            return bak;
        bak = bak->next;
    }
    return env_match(identifer, env->parent);
}

Environment* env_new(Environment *parent){
    Environment *ret = (Environment *)mallocate(sizeof(Environment));
    ret->front = ret->rear = NULL;
    ret->parent = parent;
    return ret;
}

void env_free(Environment *env){
    while(env->front != NULL){
        Record *rec = env->front;
        Record *bak = rec->next;
        if(rec->type == ARRAY)
            memfree(rec->arr.values);
        memfree(rec);
        env->front = bak;
    }
    memfree(env);
}

void env_put(char* identifer, Literal value, Environment *env){
    Record *get = env_match(identifer, env);
    if(get == NULL)
        rec_new(identifer, value, env);
    else
        get->value = value;
}

Literal env_get(char *identifer, int line, Environment *env){
    Record *get = env_match(identifer, env);
    if(get == NULL){
        printf(runtime_error("Undefined variable %s!"), line, identifer);
        stop();
    }
    else if(get->type == ARRAY){
        printf(runtime_error("%s is an array and cannot be accessed directly!"), line, identifer);
        stop();
    }
    return get->value;
}

void env_arr_new(char *identifer, int line, long numElements, Environment *env){
    Record *match = env_match(identifer, env);
    if(match != NULL && match->type != ARRAY)
        printf(runtime_error("Variable %s is already defined!"), line, identifer);
    else if(match != NULL){
        long bak = match->arr.count;
        match->arr.count = numElements;
        match->arr.values = (Literal *)reallocate(match->arr.values, sizeof(Literal)*numElements);
        while(bak < numElements){
            match->arr.values[bak].type = LIT_INT;
            match->arr.values[bak].iVal = 0;
            bak++;
        }
        return;
    }
    Record *rec = (Record *)mallocate(sizeof(Record));
    rec->name = identifer;
    rec->type = ARRAY;
    rec->next = NULL;
    rec->arr.count = numElements;
    rec->arr.values = (Literal *)mallocate(sizeof(Literal) * numElements);
    long i = 0;
    while(i < numElements){
        rec->arr.values[i].type = LIT_INT;
        rec->arr.values[i].iVal = 0;
        i++;
    }
    insert(rec, env);
}

void env_arr_put(char *identifer, long index, Literal value, Environment *env){
    Record *get = env_match(identifer, env);
    if(get == NULL){
        printf(runtime_error("Undefined array %s!"), value.line, identifer);
        stop();
    }
    else if(get->type != ARRAY){
        printf(runtime_error("Variable %s is not an array!"), value.line, identifer);
        stop();
    }
    else if(index < 1 || get->arr.count < index){
        printf(runtime_error("Array index out of range [%ld]!"), value.line, index);
        stop();
    }

    get->arr.values[index - 1] = value;
}

Literal env_arr_get(char *identifer, int line, long index, Environment *env){ 
    Record *get = env_match(identifer, env);
    if(env == NULL){
        printf(runtime_error("Undefined array %s!"), line, identifer);
        stop();
    }
    else if(get->type != ARRAY){
        printf(runtime_error("Variable %s is not an array!"), line, identifer);
        stop();
    }
    else if(index < 1 || get->arr.count < index){
        printf(runtime_error("Array index out of range [%ld]!"), line, index);
        stop();
    }

    return get->arr.values[index - 1];
}
