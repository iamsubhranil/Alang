#include <string.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <dlfcn.h>
#include <stdint.h>

#include "env.h"
#include "allocator.h"
#include "interpreter.h"
#include "display.h"
#include "foreign_interface.h"
#include "native.h"
#include "routines.h"

typedef struct{
    uint64_t name;
    void *handle;
} Library;

static Library *libraries = NULL;
static uint64_t libCount = 0;

static int hasLib(uint64_t name){
    uint64_t i = 0;
    while(i < libCount){
        if(libraries[i].name == name)
            return 1;
        i++;
    }
    return 0;
}

static Data load_library(Environment *env, uint64_t name){
    if(env != NULL){
        Data d = env_get(str_insert("x"), env, 0);
        name = tstrk(d);
    }

    if(hasLib(name))
        return new_null();

    void *lib = dlopen(str_get(name), RTLD_LAZY);
    if(!lib){
        if(env == NULL)
            printf(warning("%s"), dlerror());
        else{
            printf(error("%s"), dlerror());
            stop();
        }
    }
    libCount++;
    libraries = (Library *)reallocate(libraries, sizeof(Library) * libCount);
    libraries[libCount - 1].name = name;
    libraries[libCount - 1].handle = lib;

    return new_null();
}

void unload_all(){
    uint64_t i = 0;
    char *err = NULL;
    while(i < libCount){
        dlclose(libraries[i].handle);
        if((err = dlerror()) != NULL)
            printf(warning("%s"), err);
        i++;
    }
    memfree(libraries);
    libCount = 0;
    libraries = NULL;
}

static Data unload_library(Environment *env){
    uint64_t name = tstrk(env_get(str_insert("x"), env, 0));

    if(!hasLib(name)){
        printf(warning("Library '%s' is not loaded!"), str_get(name));
        return new_null();
    }

    char *err = NULL;
    if(libCount == 1){
        unload_all();
        return new_null();
    }

    uint64_t i = 0;
    while(i < libCount){
        if(libraries[i].name == name){
            dlclose(libraries[i].handle);
            if((err = dlerror())!=NULL){
                printf(warning("%s"), err);
            }
            libraries[i].name = libraries[libCount - 1].name;
            libraries[i].handle = libraries[libCount - 1].handle;
            break;
        }
        i++;
    }
    libCount--;
    libraries = (Library *)reallocate(libraries, sizeof(Library) * libCount);
    return new_null();
}

static void* get_func(uint64_t name){
    uint64_t i = 0;
    if(libCount == 0){
        printf(error("Unable to call %s : No libraries loaded!"), str_get(name));
        stop();
    }
    while(i < libCount){
        void *f = dlsym(libraries[i].handle, str_get(name));
        if(dlerror() == NULL)
            return f;
        i++;
    }
    printf(error("Foreign routine '%s' not found in loaded libraries!"), str_get(name));
    stop();
    return NULL;
}

typedef Data (*handler)(Environment *env);

static Data run_native(uint64_t name, Environment *env){
    handler fhandle = (handler)get_func(name);
    return fhandle(env);
}

Data handle_native(uint64_t name, Environment *env){
    if(str_insert("LoadLibrary") == name)
        return load_library(env, name);
    else if(str_insert("UnloadLibrary") == name)
        return unload_library(env);
    else
        return run_native(name, env);
}

static Routine2 get_routine(uint64_t name, int arity){
    Routine2 r;
    r.isNative = 1;
    r.name = name;
    r.arity = arity;
    r.arguments = NULL;

    return r;
}

static void add_argument(Routine2 *r, uint64_t argName){
    r->arity++;
    r->arguments = (uint64_t *)reallocate(r->arguments, 64 * r->arity);
    r->arguments[r->arity - 1] = argName;
}

static Routine2 getSingleArgRoutine(uint64_t argName){
    Routine2 r = get_routine(argName, 0);
    add_argument(&r, str_insert("x"));
    return r;
}

static void define_cons(Environment *env){
    env_put(str_insert("Math_Pi"), new_float(acos(-1.0)), env);
    env_put(str_insert("Math_E"), new_float(M_E), env);
    env_put(str_insert("ClocksPerSecond"), new_int(CLOCKS_PER_SEC), env);
}

void register_native(Environment *env){
    routine_add(getSingleArgRoutine(str_insert("LoadLibrary")));
    routine_add(getSingleArgRoutine(str_insert("UnloadLibrary")));
    define_cons(env);
    load_library(NULL, str_insert("./liblalang.so"));
}
