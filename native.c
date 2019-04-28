#include <dlfcn.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "allocator.h"
#include "display.h"
//#include "env.h"
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

// Return True if the library is loaded,
// otherwise returns False
static Data LoadLibrary(Data *stack) {
	Data d = stack[0];

	if(!native_isstr(d))
		native_rerr("Expected name of the library to load!");

	uint32_t name = tstrk(d);
	if(hasLib(name))
		return new_null();

	void *lib = dlopen(str_get(name), RTLD_LAZY);
	if(!lib) {
		native_rwarn("%s", dlerror());
		return new_logical(0);
	}
	libCount++;
	libraries = (Library *)reallocate(libraries, sizeof(Library) * libCount);
	libraries[libCount - 1].name   = name;
	libraries[libCount - 1].handle = lib;

	return new_logical(1);
}

// Return True if the library is unloaded successfully,
// otherwise returns False
static Data UnloadLibrary(Data *stk) {
	if(!isstr(stk[0])) {
		native_rerr("Expected name of the library to unload!");
	}
	uint32_t name = tstrk(stk[0]);

	if(!hasLib(name)) {
		native_rwarn("Library '%s' is not loaded!", str_get(name));
		return new_logical(0);
	}

	char *err = NULL;
	if(libCount == 1) {
		unload_all();
		return new_logical(1);
	}

	uint32_t i = 0;
	while(i < libCount) {
		if(libraries[i].name == name) {
			dlclose(libraries[i].handle);
			if((err = dlerror()) != NULL) {
				rwarn("%s", err);
				return new_logical(0);
			} else {
				libraries[i].name   = libraries[libCount - 1].name;
				libraries[i].handle = libraries[libCount - 1].handle;
				libCount--;
				libraries = (Library *)reallocate(libraries,
				                                  sizeof(Library) * libCount);
				return new_logical(1);
			}
			break;
		}
		i++;
	}
	return new_null();
}

static Data Int(Data *stack) {
	Data top = stack[0];
	if(isint(top))
		return top;
	if(isfloat(top))
		return new_int((int32_t)(tfloat(top)));
	if(isstr(top)) {
		const char *str = tstr(top);
		char *      err = NULL;
		long        res = strtol(str, &err, 10);
		if(*err != 0) {
			native_rerr("Not a valid integer string : '%s'", str);
		}
		if(res > INT32_MAX || res < INT32_MIN) {
			native_rerr("Converted number is out of range : %ld", res);
		}
		return new_int((int32_t)res);
	}

	native_rerr("Given object is not convertible to integer!");
	return new_null();
}

static Data Print(Data *stack) {
    Data vararg = stack[0];
    int argc = native_expect_int(stack[1]), i = 0;
    while(argc--) {
        data_print(arr_elements(tarr(vararg))[i]);
        i++;
    }
    return new_null();
}

void unload_all() {
	// uint32_t i   = 0;
	// char *   err = NULL;
	// while(i < libCount) {
	//	dlclose(libraries[i].handle);
	//	if((err = dlerror()) != NULL)
	//		native_rwarn("%s", err);
	//	i++;
	//}
	if(libraries)
		memfree(libraries);
	libCount  = 0;
	libraries = NULL;
}

static void *GetNative(uint32_t name) {
	uint32_t i = 0;
	if(libCount == 0) {
		native_rerr("Unable to call %s : No libraries loaded!", str_get(name));
	}
	while(i < libCount) {
		void *f = dlsym(libraries[i].handle, str_get(name));
		if(dlerror() == NULL)
			return f;
		i++;
	}
	native_rerr("Foreign routine '%s' not found in loaded libraries!",
	            str_get(name));
	stop();
	return NULL;
}

typedef NativeData (*handler)(NativeData args);

static Data RunNative(uint32_t name, uint32_t numargs, Data *stack) {
	handler    fhandle = (handler)GetNative(name);
	NativeData arr     = native_arr_new(numargs);
	for(size_t i = 0; i < numargs; i++) native_arr_set(arr, i, stack[i]);
	return fhandle(arr);
}

static uint32_t loadLibrary = 0, unloadLibrary = 0, Clock = 0, toInt = 0, print = 0;

Data handle_native(uint32_t name, uint32_t numargs, Data *stack) {
	if(name == toInt)
		return Int(stack);
	if(name == loadLibrary)
		return LoadLibrary(stack);
	if(name == unloadLibrary)
		return UnloadLibrary(stack);
	if(name == Clock)
		return new_int((int32_t)clock());
	if(name == print)
	    return Print(stack);

	return RunNative(name, numargs, stack);
}

static Routine2 get_routine(uint32_t name, int arity, int isVararg) {
	Routine2 r;
	r.isNative  = 1;
	r.name      = name;
	r.arity     = arity;
	r.variables = NULL;
	r.isVararg  = isVararg;
	return r;
}

static void add_argument(Routine2 *r, uint32_t argName) {
	r->arity++;
	r->variables = (uint32_t *)reallocate(r->variables, 64 * r->arity);
	r->variables[r->arity - 1] = argName;
}

static Routine2 getSingleArgRoutine(uint32_t name) {
	Routine2 r = get_routine(name, 0, 0);
	add_argument(&r, str_insert(strdup("x"), 1));
	return r;
}

static Routine2 getZeroArgRoutine(uint32_t name, int isVararg) {
	return get_routine(name, 0 + (isVararg * 2), isVararg);
}

typedef struct {
	const char *name;
	Data        value;
} Constant;

static Constant constants[3];
static size_t   num_constants = sizeof(constants) / sizeof(Constant);

static void register_to_parser() {
	for(size_t i = 0; i < num_constants; i++)
		parser_register_variable(constants[i].name);
}

static void define_cons() {
	constants[0] = (Constant){"Math_Pi", new_float(acos(-1.0))};
	constants[1] = (Constant){"Math_E", new_float(M_E)};
	constants[2] = (Constant){"ClocksPerSecond", new_int(CLOCKS_PER_SEC)};
	register_to_parser();
}

void register_native_routines() {
	loadLibrary   = str_insert(strdup("LoadLibrary"), 1);
	unloadLibrary = str_insert(strdup("UnloadLibrary"), 1);
	Clock         = str_insert(strdup("Clock"), 1);
	toInt         = str_insert(strdup("Int"), 1);
	print         = str_insert(strdup("Print"), 1);
	routine_add(getSingleArgRoutine(loadLibrary));
	routine_add(getSingleArgRoutine(unloadLibrary));
	routine_add(getZeroArgRoutine(Clock, 0));
	routine_add(getSingleArgRoutine(toInt));
	routine_add(getZeroArgRoutine(print, 1));
}

void register_natives() {
	// double tm = clock();
	define_cons();
	register_native_routines();
	// tm = (clock() - tm)/CLOCKS_PER_SEC;
	// printf(debug("[Native] Registration took %gs"), tm);
}

void load_natives() {
	Data d = new_str(strdup("./libalang_math.so"));
	LoadLibrary(&d);
	for(size_t i = 0; i < num_constants; i++)
		interpreter_push(constants[i].value);
}
