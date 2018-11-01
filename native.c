#include <dlfcn.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "allocator.h"
#include "display.h"
#include "env.h"
#include "foreign_interface.h"
#include "interpreter.h"
#include "native.h"
#include "routines.h"

typedef struct {
	uint32_t name;
	void *   handle;
} Library;

static Library *libraries = NULL;
static uint32_t libCount  = 0;

static int hasLib(uint32_t name) {
	uint32_t i = 0;
	while(i < libCount) {
		if(libraries[i].name == name)
			return 1;
		i++;
	}
	return 0;
}

static Data load_library(Environment *env, uint32_t name) {
	if(env != NULL) {
		Data d = env_get(str_insert(strdup("x")), env, 0);
		name   = tstrk(d);
	}

	if(hasLib(name))
		return new_null();

	void *lib = dlopen(str_get(name), RTLD_LAZY);
	if(!lib) {
		if(env == NULL)
			warn("%s", dlerror());
		else {
			rerr("%s", dlerror());
		}
		return new_null();
	}
	libCount++;
	libraries = (Library *)reallocate(libraries, sizeof(Library) * libCount);
	libraries[libCount - 1].name   = name;
	libraries[libCount - 1].handle = lib;

	return new_null();
}

void unload_all() {
	uint32_t i   = 0;
	char *   err = NULL;
	while(i < libCount) {
		dlclose(libraries[i].handle);
		if((err = dlerror()) != NULL)
			rwarn("%s", err);
		i++;
	}
	memfree(libraries);
	libCount  = 0;
	libraries = NULL;
}

static Data unload_library(Environment *env) {
	uint32_t name = tstrk(env_get(str_insert("x"), env, 0));

	if(!hasLib(name)) {
		rwarn("Library '%s' is not loaded!", str_get(name));
		return new_null();
	}

	char *err = NULL;
	if(libCount == 1) {
		unload_all();
		return new_null();
	}

	uint32_t i = 0;
	while(i < libCount) {
		if(libraries[i].name == name) {
			dlclose(libraries[i].handle);
			if((err = dlerror()) != NULL) {
				rwarn("%s", err);
			}
			libraries[i].name   = libraries[libCount - 1].name;
			libraries[i].handle = libraries[libCount - 1].handle;
			break;
		}
		i++;
	}
	libCount--;
	libraries = (Library *)reallocate(libraries, sizeof(Library) * libCount);
	return new_null();
}

static void *get_func(uint32_t name) {
	uint32_t i = 0;
	if(libCount == 0) {
		rerr("Unable to call %s : No libraries loaded!", str_get(name));
	}
	while(i < libCount) {
		void *f = dlsym(libraries[i].handle, str_get(name));
		if(dlerror() == NULL)
			return f;
		i++;
	}
	rerr("Foreign routine '%s' not found in loaded libraries!", str_get(name));
	stop();
	return NULL;
}

typedef Data (*handler)(Environment *env);

static Data run_native(uint32_t name, Environment *env) {
	handler fhandle = (handler)get_func(name);
	return fhandle(env);
}

static uint32_t LoadLibrary = 0, UnloadLibrary = 0, Clock = 0;

Data handle_native(uint32_t name, Environment *env) {
	if(LoadLibrary == name)
		return load_library(env, name);
	else if(UnloadLibrary == name)
		return unload_library(env);
	else if(Clock == name)
		return new_int((int32_t)clock());
	else
		return run_native(name, env);
}

static Routine2 get_routine(uint32_t name, int arity) {
	Routine2 r;
	r.isNative  = 1;
	r.name      = name;
	r.arity     = arity;
	r.arguments = NULL;

	return r;
}

static void add_argument(Routine2 *r, uint32_t argName) {
	r->arity++;
	r->arguments = (uint32_t *)reallocate(r->arguments, 64 * r->arity);
	r->arguments[r->arity - 1] = argName;
}

static Routine2 getSingleArgRoutine(uint32_t name) {
	Routine2 r = get_routine(name, 0);
	add_argument(&r, str_insert(strdup("x")));
	return r;
}

static Routine2 getZeroArgRoutine(uint32_t name) {
	return get_routine(name, 0);
}

static void define_cons(Environment *env) {
	env_put(str_insert(strdup("Math_Pi")), new_float(acos(-1.0)), env);
	env_put(str_insert(strdup("Math_E")), new_float(M_E), env);
	env_put(str_insert(strdup("ClocksPerSecond")), new_int(CLOCKS_PER_SEC),
	        env);
}

void register_native_routines() {
	LoadLibrary   = str_insert(strdup("LoadLibrary"));
	UnloadLibrary = str_insert(strdup("UnloadLibrary"));
	Clock         = str_insert(strdup("Clock"));
	routine_add(getSingleArgRoutine(LoadLibrary));
	routine_add(getSingleArgRoutine(UnloadLibrary));
	routine_add(getZeroArgRoutine(Clock));
}

void register_native(Environment *env) {
	// double tm = clock();
	define_cons(env);
	load_library(NULL, str_insert(strdup("./liblalang.so")));
	// tm = (clock() - tm)/CLOCKS_PER_SEC;
	// printf(debug("[Native] Registration took %gs"), tm);
}
