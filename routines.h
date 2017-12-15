#pragma once

#include "strings.h"
#include "display.h"

typedef struct{
    uint32_t name;
    uint32_t arity;
    uint32_t *arguments;
    uint32_t startAddress;
    uint8_t isNative;
} Routine2;

extern Routine2 *routines;
extern uint32_t rp;

Routine2 routine_new();
void routine_add_arg(Routine2 *routine, uint32_t arg);
void routine_add(Routine2 routine);
static inline Routine2 routine_get(uint32_t name);

static inline Routine2 routine_get(uint32_t name){
    uint32_t i = 0;
    while(i < rp){
//        printf(debug("Routine : %s"), str_get(routines[i]->name));
        if(routines[i].name == name)
            return routines[i];
        i++;
    }
    rerr("Routine not found : '%s'!\n", str_get(name));
    return (Routine2){0,0, NULL, 0, 0, };
}
