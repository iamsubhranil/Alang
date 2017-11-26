#include "callframe.h"
#include "allocator.h"

static CallFrame **cf_stack = NULL;
static uint64_t index = 0;

CallFrame* cf_new(){
    CallFrame* ret = (CallFrame *)mallocate(sizeof(CallFrame));
    ret->arity = 0;
    ret->env = NULL;
    ret->returnAddress = 0;
    return ret;
}

void cf_push(CallFrame *frame){
    cf_stack = (CallFrame **)reallocate(cf_stack, sizeof(CallFrame *)*++index);
    cf_stack[index - 1] = frame;
}

CallFrame* cf_peek(){
    if(cf_stack == NULL)
        return NULL;
    return cf_stack[index - 1];
}

CallFrame* cf_pop(){
    CallFrame *ret = cf_stack[--index];
    if(index == 0){
        memfree(cf_stack);
        cf_stack = NULL;
    }
    else
        cf_stack = (CallFrame **)reallocate(cf_stack, sizeof(CallFrame *)*index);
    return ret;
}

void cf_free(CallFrame *frame){
    env_free(frame->env);
    memfree(frame);
}
