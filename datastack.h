#pragma once

#include "allocator.h"
#include "values.h"

#ifdef DYNAMIC_STACK
static Data *   dataStack            = NULL;
static uint32_t stackSize            = 0;
static int32_t  stackLastIncremented = 0;
#else
#define STACK_SIZE 65532
static Data     dataStack[STACK_SIZE] = {0};
static uint32_t stackSize             = STACK_SIZE;
#endif
static uint64_t sp = 0;

#define STACK_INC_CACHE \
	-10 // Preven stack decrement if it was incremented recently

#ifdef DYNAMIC_STACK

#define dStackInit()                                             \
	{                                                            \
		stackSize = 10;                                          \
		dataStack = (Data *)mallocate(sizeof(Data) * stackSize); \
	}
#define dStackFree() \
	{ memfree(dataStack); }

#define incr()                                                               \
	if(sp >= stackSize) {                                                    \
		stackSize *= 2;                                                      \
		stackLastIncremented = 0;                                            \
		dataStack = (Data *)reallocate(dataStack, sizeof(Data) * stackSize); \
	}

#else
#define dStackInit() stackSize = STACK_SIZE;
#define dStackFree()
#define incr()          \
	if(sp == stackSize) \
		rerr("Stack overflow ! %u", sp);
#endif

#define dtop() dataStack[sp - 1]
#define dtoppi(x) dataStack[sp - 1] = new_int(x);
#define dtoppf(x) dataStack[sp - 1] = new_float(x);
#define dtopps(x) dataStack[sp - 1] = new_str(x);
#define dtoppsk(x) dataStack[sp - 1] = new_strk(x);
#define dtoppl(x) dataStack[sp - 1] = new_logical(x);
#define dtoppid(x) dataStack[sp - 1] = new_identifer(x);
#define dtoppidk(x) dataStack[sp - 1] = new_identiferk(x);
#define dtoppins(x) dataStack[sp - 1] = new_ins(x);
#define dtoppn() dataStack[sp - 1] = new_null();
#define dtoppptr(x) dataStack[sp - 1] = new_ptr(x);

#define dpush(x)             \
	{                        \
		incr();              \
		dataStack[sp++] = x; \
	}
#define dpushi(x)                     \
	{                                 \
		incr();                       \
		dataStack[sp++] = new_int(x); \
	}
#define dpushf(x)                       \
	{                                   \
		incr();                         \
		dataStack[sp++] = new_float(x); \
	}
#define dpushs(x)                     \
	{                                 \
		incr();                       \
		dataStack[sp++] = new_str(x); \
	}
#define dpushsk(x)                     \
	{                                  \
		incr();                        \
		dataStack[sp++] = new_strk(x); \
	}
#define dpushl(x)                         \
	{                                     \
		incr();                           \
		dataStack[sp++] = new_logical(x); \
	}
#define dpushid(x)                          \
	{                                       \
		incr();                             \
		dataStack[sp++] = new_identifer(x); \
	}
#define dpushidk(x)                          \
	{                                        \
		incr();                              \
		dataStack[sp++] = new_identiferk(x); \
	}
#define dpushins(x)                   \
	{                                 \
		incr();                       \
		dataStack[sp++] = new_ins(x); \
	}
#define dpushn()                      \
	{                                 \
		incr();                       \
		dataStack[sp++] = new_null(); \
	}
#define dpushptr(x)                   \
	{                                 \
		incr();                       \
		dataStack[sp++] = new_ptr(x); \
	}

#define binpopv() (Data d1, d2; dpopv(d1, env); dpopv(d2, env);)

#define dpeek() dataStack[sp - 1]
#define dpop(x) \
	{ x = dataStack[--sp]; }
#define dpopi(x) \
	{ x = tint(dataStack[--sp]); }
#define dpopf(x) \
	{ x = tfloat(dataStack[--sp]); }
#define dpops(x) \
	{ x = tstr(dataStack[--sp]); }
#define dpopsk(x) \
	{ x = tstrk(dataStack[--sp]); }
#define dpopins(x) \
	{ x = tins(dataStack[--sp]); }
#define dpopptr(x) \
	{ x = tptr(dataStack[--sp]); }
