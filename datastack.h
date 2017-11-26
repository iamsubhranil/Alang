#pragma once

#include "allocator.h"
#include "values.h"
#include "env.h"

static Data **dataStack = NULL;
static uint64_t sp = 0, stackSize = 0;

#define SPEC_INC 10

#define incr() if((stackSize - sp) < SPEC_INC){\
        stackSize = stackSize+SPEC_INC; \
        dataStack = (Data **)reallocate(dataStack, sizeof(Data *)*stackSize);}

#define decr() if((stackSize - sp) > SPEC_INC){\
        stackSize = (stackSize/SPEC_INC) * SPEC_INC; \
        dataStack = (Data **)reallocate(dataStack, sizeof(Data *)*stackSize);}

#define dpush(x) {incr(); dataStack[sp++] = x;}
#define dpushi(x) {incr(); dataStack[sp++] = new_int(x);}
#define dpushf(x) {incr(); dataStack[sp++] = new_float(x);}
#define dpushs(x) {incr(); dataStack[sp++] = new_str(x);}
#define dpushl(x) {incr(); dataStack[sp++] = new_logical(x);}
#define dpushid(x) {incr(); dataStack[sp++] = new_identifer(x);}
#define dpushins(x) {incr(); dataStack[sp++] = new_ins(x);}
#define dpushn() {incr(); dataStack[sp++] = new_null();}

#define dpeek() dataStack[sp - 1]
#define dpop(x) {x = dataStack[--sp]; decr();}
#define dpopv(x, env) {x = dataStack[--sp]; x = isidentifer(x)?env_get(x->svalue, env, 0):x; decr();}
#define dpopi(x) {x = dataStack[--sp]->ivalue; decr();}
#define dpopf(x) {x = dataStack[--sp]->cvalue; decr();}
#define dpops(x) {x = dataStack[--sp]->svalue; decr();}
#define dpopins(x) {x = dataStack[--sp]->pvalue; decr();}
