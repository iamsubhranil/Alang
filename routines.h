#pragma once

#include "strings.h"

typedef struct{
    uint32_t name;
    uint32_t arity;
    uint32_t *arguments;
    uint32_t startAddress;
    uint32_t endAddress;
    uint8_t isNative;
} Routine2;

Routine2 routine_new();
void routine_add_arg(Routine2 *routine, const char *arg);
void routine_add(Routine2 routine);
Routine2 routine_get(uint32_t name);
