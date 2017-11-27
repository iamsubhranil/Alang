#include "callframe.h"
#include "allocator.h"

static CallFrame *cf_stack = NULL;
static uint64_t index = 0, size = 0;

#define CF_INC 20

CallFrame cf_new(){
    return (CallFrame){0, 0, NULL};
}

void cf_push(CallFrame frame){
    if(index >= size){
        size += CF_INC;
        cf_stack = (CallFrame *)reallocate(cf_stack, sizeof(CallFrame)*size);
    }
    cf_stack[index++] = frame;
}

CallFrame cf_peek(){
    if(cf_stack == NULL)
        return cf_new();
    return cf_stack[index - 1];
}

CallFrame cf_pop(){
    CallFrame ret = cf_stack[--index];
    if(size-index >= CF_INC){
        size -= CF_INC;
        if(size == 0){
            memfree(cf_stack);
            cf_stack = NULL;
        }
        else
            cf_stack = (CallFrame *)reallocate(cf_stack, sizeof(CallFrame)*size);
    }
    return ret;
}

void cf_free(CallFrame frame){
    env_free(frame.env);
}
