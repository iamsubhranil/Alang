#include "callframe.h"
#include "allocator.h"

typedef struct CallStack{
    CallFrame frame;
    struct CallStack *next;
} CallStack;

static CallStack *top = NULL, *bottom = NULL;
static uint64_t index = 0, size = 0;

#define CF_INC 20

CallFrame cf_new(){
    return (CallFrame){0, 0, {NULL, NULL}};
}

void cf_push(CallFrame frame){
    CallStack *ns = (CallStack *)mallocate(sizeof(CallStack));
    ns->next = top;
    ns->frame = frame;
    if(top == NULL)
        bottom = ns;
    top = ns;
}

CallFrame cf_peek(){
    if(top == NULL)
        return cf_new();
    return top->frame;
}

CallFrame cf_pop(){
    if(top == NULL)
        return cf_new();
    CallFrame ret = top->frame;
    CallStack *bak = top;
    top = top->next;
    if(top == NULL)
        bottom = NULL;
    memfree(bak);
    return ret;
}

Environment* cf_root_env(){
    return &bottom->frame.env;
}

void cf_free(CallFrame frame){
    env_free(frame.env);
}
