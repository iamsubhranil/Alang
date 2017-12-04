#include <stdio.h>
#include "display.h"

#include "routines.h"
#include "allocator.h"
#include "interpreter.h"

Routine2 *routines = NULL;
uint32_t rp = 0;

Routine2 routine_new(){
    Routine2 r;
    r.arity = 0;
    r.endAddress = 0;
    r.name = 0;
    r.startAddress = 0;
    r.isNative = 0;
    r.arguments = NULL;
    return r;
}

void routine_add_arg(Routine2 *r, const char *arg){
    r->arity++;
    r->arguments = (uint32_t *)reallocate(r->arguments, 64*r->arity);
    r->arguments[r->arity - 1] = str_insert(arg);
}

void routine_add(Routine2 r){
    routines = (Routine2 *)reallocate(routines, sizeof(Routine2)*++rp);
//    printf(debug("[Routine] Adding routine : [%s]"), str_get(r.name));
    routines[rp - 1] = r;
}
