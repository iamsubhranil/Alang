#pragma once

#include "allocator.h"
#include "values.h"
#include "env.h"

static Data *dataStack = NULL;
static uint64_t sp = 0, stackSize = 0;

#define SPEC_INC 20

#define incr() if((stackSize - sp) < SPEC_INC){\
        stackSize = stackSize+SPEC_INC; \
        dataStack = (Data *)reallocate(dataStack, sizeof(Data)*stackSize);}

#define decr() if((stackSize - sp) > SPEC_INC){\
        stackSize = (stackSize/SPEC_INC) * SPEC_INC; \
        dataStack = (Data *)reallocate(dataStack, sizeof(Data)*stackSize);}

#define dpush(x) {incr(); dataStack[sp++] = x;}
#define dpushi(x) {incr(); dataStack[sp++] = new_int(x);}
#define dpushf(x) {incr(); dataStack[sp++] = new_float(x);}
#define dpushs(x) {incr(); dataStack[sp++] = new_str(x);}
#define dpushsk(x) {incr(); dataStack[sp++] = new_strk(x);}
#define dpushl(x) {incr(); dataStack[sp++] = new_logical(x);}
#define dpushid(x) {incr(); dataStack[sp++] = new_identifer(x);}
#define dpushidk(x) {incr(); dataStack[sp++] = new_identiferk(x);}
#define dpushins(x) {incr(); dataStack[sp++] = new_ins(x);}
#define dpushn() {incr(); dataStack[sp++] = new_null();}

#define binpopv() (Data d1, d2; dpopv(d1, env); dpopv(d2, env);)

#define dpeek() dataStack[sp - 1]
#define dpop(x) {x = dataStack[--sp]; decr();}
#define dpopv(x, frame) {x = dataStack[--sp]; x = isidentifer(x)?env_get(tstrk(x), &frame.env, 0):x; decr();}
#define dpopi(x) {x = tint(dataStack[--sp]); decr();}
#define dpopf(x) {x = tfloat(dataStack[--sp]); decr();}
#define dpops(x) {x = tstr(dataStack[--sp]); decr();}
#define dpopins(x) {x = tins(dataStack[--sp]); decr();}
