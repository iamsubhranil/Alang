#include <string.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <dlfcn.h>

#include "environment.h"
#include "allocator.h"
#include "interpreter.h"
#include "display.h"
#include "foreign_interface.h"
#include "native.h"

typedef struct{
    char *name;
    void *handle;
} Library;

static Library *libraries = NULL;
static int libCount = 0;

static int hasLib(char *name){
    int i = 0;
    while(i < libCount){
        if(strcmp(libraries[i].name, name) == 0)
            return 1;
        i++;
    }
    return 0;
}

static Object load_library(int c, Environment *env, char *s){
    if(env != NULL)
        s = get_string("x", c, env);

    if(hasLib(s))
        return nullObject;

    void *lib = dlopen(s, RTLD_LAZY);
    if(!lib){
        printf(runtime_error("%s"), c, dlerror());
        stop();
    }
    libCount++;
    libraries = (Library *)reallocate(libraries, sizeof(Library) * libCount);
    libraries[libCount - 1].name = s;
    libraries[libCount - 1].handle = lib;

    return nullObject;
}

void unload_all(){
    int i = 0;
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

static Object unload_library(int c, Environment *env){
    char *s = get_string("x", c, env);

    if(!hasLib(s)){
        printf(warning("Library '%s' is not loaded!"), s);
        return nullObject;
    }
    char *err = NULL;
    if(libCount == 1){
        unload_all();
        return nullObject;
    }

    int i = 0;
    while(i < libCount){
        if(strcmp(libraries[i].name, s) == 0){
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
    return nullObject;
}

static void* get_func(char *identifer, int line){
    int i = 0;
    if(libCount == 0){
        printf(runtime_error("Unable to call %s : No libraries loaded!"), line, identifer);
        stop();
    }
    while(i < libCount){
        void *f = dlsym(libraries[i].handle, identifer);
        if(dlerror() == NULL)
            return f;
        i++;
    }
    printf(runtime_error("Foreign routine '%s' not found in loaded libraries!"), line, identifer);
    stop();
    return NULL;
}

typedef Object (*handler)(int line, Environment *env);

static Object run_native(Call c, Environment *env){
    handler fhandle = (handler)get_func(c.identifer, c.line);
    return fhandle(c.line, env);
}

Object handle_native(Call c, Environment *env){
    char *fname = c.identifer;
    int i = 0;
    if(strcmp(fname, "LoadLibrary") == 0)
        return load_library(c.line, env, NULL);
    else if(strcmp(fname, "UnloadLibrary") == 0)
        return unload_library(c.line, env);
    else
        return run_native(c, env);
}

static Routine get_routine(char *identifer, int arity){
    Routine r;
    r.isNative = 1;
    r.name = identifer;
    r.arity = arity;
    r.line = 0;
    r.arguments = NULL;

    return r;
}

static void add_argument(Routine *r, char *argName){
    r->arity++;
    r->arguments = (char **)reallocate(r->arguments, sizeof(char *) * r->arity);
    r->arguments[r->arity - 1] = argName;
}

static Routine getSingleArgRoutine(char *name){
    Routine r = get_routine(name, 0);
    add_argument(&r, "x");
    return r;
}

static void define_cons(Environment *env){
    env_put("Math_Pi", 0, fromDouble(acos(-1.0)), env);
    env_put("Math_E", 0, fromDouble(M_E), env);
}

void register_native(Environment *env){
    env_routine_put(getSingleArgRoutine("LoadLibrary"), 0, env);
    env_routine_put(getSingleArgRoutine("UnloadLibrary"), 0, env);
    define_cons(env);
    load_library(0, NULL, "./libnmath.so");
}
