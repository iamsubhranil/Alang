#include <stdio.h>

#include "env.h"
#include "allocator.h"
#include "display.h"
#include "strings.h"
#include "interpreter.h"

Environment* env_new(Environment *parent){
    Environment* env = (Environment *)mallocate(sizeof(Environment));
    env->data = NULL;
    env->keys = NULL;
    env->keyCount = 0;
    env->parent = parent;
    return env;
}

void env_put(uint64_t key, Data* value, Environment *env){
    if(env == NULL)
        return;
    uint64_t i = 0;
    value->refCount++;
    while(i < env->keyCount){
        if(env->keys[i] == key){
            Data *old = env->data[i];
            old->refCount--;
            if(old->refCount == 0)
                data_free(old);
            env->data[i] = value;
            return;
        }
        i++;
    }
    env->keys = (uint64_t *)reallocate(env->keys, 64*++env->keyCount);
    env->keys[env->keyCount - 1] = key;
    env->data = (Data **)reallocate(env->data, sizeof(Data *)*env->keyCount);
    env->data[env->keyCount - 1] = value;
}

Data* env_get(uint64_t key, Environment *env, uint8_t beSilent){
    if(env == NULL)
        return NULL;
    uint64_t i = 0;
    while(i < env->keyCount){
        if(env->keys[i] == key)
            return env->data[i];
        i++;
    }
    Data* p = env_get(key, env->parent, beSilent);
    if(p == NULL && !beSilent){
        printf(error("Uninitialized variable '%s'!\n"), str_get(key));
        stop();
    }
    return p;
}

void env_free(Environment *env){
    uint64_t i = 0;
    while(i < env->keyCount){
        data_free(env->data[i]);
        i++;
    }
    memfree(env->data);
    memfree(env->keys);
    memfree(env);
}
