#pragma once

#include <stdint.h>

#include "env.h"

typedef struct{
    uint16_t arity;
    uint64_t returnAddress;
    Environment env;
} CallFrame;

typedef struct CallStack{
    CallFrame frame;
    struct CallStack *next;
} CallStack;

extern CallStack *csTop, *csBottom;

CallStack *csTop = NULL, *csBottom = NULL;

static inline void cf_push(CallFrame frame);
static inline CallFrame cf_peek();
static inline CallFrame cf_pop();
static inline void cf_free(CallFrame frame);

#define CF_INC 20

#define cf_new() (CallFrame){0, 0, {NULL, NULL}}

static inline void cf_push(CallFrame frame){
    CallStack *ns = (CallStack *)mallocate(sizeof(CallStack));
    ns->next = csTop;
    ns->frame = frame;
    if(csTop == NULL)
        csBottom = ns;
    csTop = ns;
}

static inline CallFrame cf_peek(){
    if(csTop == NULL)
        return cf_new();
    return csTop->frame;
}

static inline CallFrame cf_pop(){
    if(csTop == NULL)
        return cf_new();
    CallFrame ret = csTop->frame;
    CallStack *bak = csTop;
    csTop = csTop->next;
    if(csTop == NULL)
        csBottom = NULL;
    memfree(bak);
    return ret;
}

#define cf_root_env() &csBottom->frame.env
#define cf_free(frame) env_free(frame.env)
