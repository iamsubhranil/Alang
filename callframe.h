#pragma once

#include <stdint.h>
#include "env.h"
#include "allocator.h"

typedef struct{
    uint16_t arity;
    uint64_t returnAddress;
    Environment env;
} CallFrame;

extern CallFrame *callStack;
extern uint32_t callFrameSize, callFramePointer;
extern Environment rootEnvironment;

CallFrame *callStack = NULL;
uint32_t callFrameSize = 0, callFramePointer = 0;
Environment rootEnvironment;

static inline void cf_push(CallFrame frame);
static inline CallFrame cf_peek();
static inline CallFrame cf_pop();
static inline void cf_free(CallFrame frame);

static inline void cf_push(CallFrame frame){
    if(callFramePointer >= callFrameSize){
        callFrameSize *= 2;
        callStack = (CallFrame *)reallocate(callStack, sizeof(CallFrame) * callFrameSize);
    }
    if(callFramePointer == 0)
        rootEnvironment = frame.env;
    callStack[callFramePointer++] = frame;
}

static inline CallFrame cf_peek(){
    return callStack[callFramePointer - 1];
}

static inline CallFrame cf_pop(){
    return callStack[--callFramePointer];
}

#define cf_new() (CallFrame){0, 0, {NULL, NULL}}
#define cf_root_env() &(rootEnvironment)
#define cf_free(frame) env_free(frame.env)

#define cs_init() { callFrameSize = 10; \
    callStack = (CallFrame *)mallocate(sizeof(CallFrame) * callFrameSize); }

#define cs_free() memfree(callStack)
