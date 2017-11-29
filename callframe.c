#include "callframe.h"
#include "allocator.h"

typedef struct CallStack{
    CallFrame frame;
    struct CallStack *next;
} CallStack;

static CallStack *top = NULL;
static uint64_t index = 0, size = 0;


#define CF_INC 20

CallFrame cf_new(){
    return (CallFrame){0, 0, NULL};
}

void cf_push(CallFrame frame){
    CallStack *ns = (CallStack *)mallocate(sizeof(CallStack));
    ns->next = top;
    ns->frame = frame;
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
    memfree(bak);
    return ret;
}

void cf_free(CallFrame frame){
    env_free(frame.env);
}
