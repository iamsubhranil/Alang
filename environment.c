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

static void incr_ref(Object value){ 
    if(value.type == OBJECT_INSTANCE){
        if(value.instance->fromReturn == 1){
            value.instance->fromReturn = 0;
        }
        value.instance->refCount++;
        //        printf(debug("[New rec] Incremented refcount to %d of container %s#%d for identifer %s!"),
        //                value.instance->refCount, value.instance->name, value.instance->insCount, identifer);
    }
}

static void rec_new(char* identifer, Object value, Environment *parent){
    Record *env = (Record *)mallocate(sizeof(Record));
    env->name = identifer;
    incr_ref(value);
    env->object = value;
    env->next = NULL;
    insert(env, parent);
}

void inline gc_obj(Object o){
    if(o.type == OBJECT_INSTANCE){
        Instance *ins = o.instance;
        ins->refCount--;
        if(ins->refCount <= 0 && ins->fromReturn == 0){
//            printf(debug("[Gc_Obj] Garbage collecting %s#%d!"), ins->name, ins->insCount);
            env_free((Environment *)ins->environment);
            memfree(ins);
        }
    }
}

static void inline gc_rec(Record *rec){
    gc_obj(rec->object);
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
        if(rec->object.type == OBJECT_ARRAY)
            memfree(rec->object.arr.values);
        else if(rec->object.type == OBJECT_INSTANCE){
            gc_rec(rec);
        }
        //        memfree(rec->name);
        memfree(rec);
        env->front = bak;
    }
    memfree(env);
}

void env_put(char* identifer, Object value, Environment *env){
    Record *get = env_match(identifer, env);
    if(get == NULL)
        rec_new(identifer, value, env);
    else{
        Object old = get->object;
        incr_ref(value);
        get->object = value;
        if(old.type == OBJECT_INSTANCE){
            gc_obj(old);
            //            printf(debug("[Put] Reassigning %s! Decremented refcount of %s#%d to %d!"),
            //                    identifer, get->object.instance->name, get->object.instance->insCount,
            //                    get->object.instance->refCount);
        }
    }
}

Object env_get(char *identifer, int line, Environment *env){
    Record *get = env_match(identifer, env);
    if(get == NULL){
        printf(runtime_error("Undefined variable %s!"), line, identifer);
        stop();
    }
    else if(get->object.type == OBJECT_ARRAY){
        printf(runtime_error("%s is an array and cannot be accessed directly!"), line, identifer);
        stop();
    }
    return get->object;
}

void env_arr_new(char *identifer, int line, long numElements, Environment *env){
    Record *match = env_match(identifer, env);
    if(match != NULL && match->object.type != OBJECT_ARRAY)
        printf(runtime_error("Variable %s is already defined!"), line, identifer);
    else if(match != NULL){
        long bak = match->object.arr.count;
        match->object.arr.count = numElements;
        match->object.arr.values = (Literal *)reallocate(match->object.arr.values, sizeof(Literal)*numElements);
        while(bak < numElements){
            match->object.arr.values[bak].type = LIT_INT;
            match->object.arr.values[bak].iVal = 0;
            bak++;
        }
        return;
    }
    Object o;
    o.type = OBJECT_ARRAY;
    o.arr.count = numElements;
    o.arr.values = (Literal *)mallocate(sizeof(Literal) * numElements);
    long i = 0;
    while(i < numElements){
        o.arr.values[i].type = LIT_INT;
        o.arr.values[i].iVal = 0;
        i++;
    }
    rec_new(identifer, o, env);
}

void env_arr_put(char *identifer, long index, Literal value, Environment *env){
    Record *get = env_match(identifer, env);
    if(get == NULL){
        printf(runtime_error("Undefined array %s!"), value.line, identifer);
        stop();
    }
    else if(get->object.type != OBJECT_ARRAY){
        printf(runtime_error("Variable %s is not an array!"), value.line, identifer);
        stop();
    }
    else if(index < 1 || get->object.arr.count < index){
        printf(runtime_error("Array index out of range [%ld]!"), value.line, index);
        stop();
    }

    get->object.arr.values[index - 1] = value;
}

Literal env_arr_get(char *identifer, int line, long index, Environment *env){ 
    Record *get = env_match(identifer, env);
    if(env == NULL){
        printf(runtime_error("Undefined array %s!"), line, identifer);
        stop();
    }
    else if(get->object.type != OBJECT_ARRAY){
        printf(runtime_error("Variable %s is not an array!"), line, identifer);
        stop();
    }
    else if(index < 1 || get->object.arr.count < index){
        printf(runtime_error("Array index out of range [%ld]!"), line, index);
        stop();
    }

    return get->object.arr.values[index - 1];
}

void env_routine_put(Routine r, int line, Environment *env){
    Record *match = env_match(r.name, env);
    if(match != NULL){
        if(match->object.type != OBJECT_ROUTINE){
            printf(runtime_error("Identifer %s cannot be redefined as a routine in the same scope!"), line, r.name);
        }
        else{
            printf(runtime_error("Routine %s is already defined!"), line, r.name);
        }
        stop();
    }

    Object record;
    record.type = OBJECT_ROUTINE;
    record.routine = r;

    rec_new(r.name, record, env);
}

Routine env_routine_get(char *identifer, int line, Environment *env){
    Record *match = env_match(identifer, env);
    if(match == NULL){
        if(strcmp(identifer, "Main") == 0){
            printf(error("Unable to start! Routine Main is not defined!"));
        }
        else
            printf(runtime_error("Routine %s is not defined!"), line, identifer);
        stop();
    }
    else if(match->object.type != OBJECT_ROUTINE){
        printf(runtime_error("%s is not a callable routine!"), line, identifer);
        stop();
    }
    return match->object.routine;
}

void env_container_put(Container c, int line, Environment *env){
    Record *match = env_match(c.name, env);
    if(match != NULL){
        if(match->object.type != OBJECT_CONTAINER){
            printf(runtime_error("Identifer %s cannot be redefined as a container in the same scope!"), line, c.name);
        }
        else{
            printf(runtime_error("Container %s is already defined!"), line, c.name);
        }
        stop();
    }

    Object o;
    o.type = OBJECT_CONTAINER;
    o.container = c;

    rec_new(c.name, o, env);
}

Container env_container_get(char *identifer, int line, Environment *env){
    Record *match = env_match(identifer, env);
    if(match == NULL){
        printf(runtime_error("Container %s is not defined!"), line, identifer);
        stop();
    }
    else if(match->object.type != OBJECT_CONTAINER){
        printf(runtime_error("%s is not a container!"), line, identifer);
        stop();
    }
    return match->object.container;
}
