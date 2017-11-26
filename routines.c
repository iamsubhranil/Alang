#include <stdio.h>
#include "display.h"

#include "routines.h"
#include "allocator.h"
#include "interpreter.h"


static Routine2 **routines = NULL;
static uint64_t index = 0;

Routine2* routine_new(){
    Routine2 *r = (Routine2 *)mallocate(sizeof(Routine2));
    r->arity = 0;
    r->endAddress = 0;
    r->name = 0;
    r->startAddress = 0;
    r->isNative = 0;
    r->arguments = NULL;
    return r;
}

void routine_add_arg(Routine2 *r, const char *arg){
    r->arity++;
    r->arguments = (uint64_t *)reallocate(r->arguments, 64*r->arity);
    r->arguments[r->arity - 1] = str_insert(arg);
}

void routine_add(Routine2 *r){
    routines = (Routine2 **)reallocate(routines, sizeof(Routine2 *)*++index);
    routines[index - 1] = r;
}

Routine2* routine_get(uint64_t name){
    uint64_t i = 0;
    while(i < index){
//        printf(debug("Routine : %s"), str_get(routines[i]->name));
        if(routines[i]->name == name)
            return routines[i];
        i++;
    }
    printf(error("Routine not found : '%s'!\n"), str_get(name));
    stop();
    return NULL;
}
