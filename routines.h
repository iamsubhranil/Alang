#pragma once

#include "strings.h"

typedef struct{
    uint64_t name;
    uint64_t arity;
    uint64_t *arguments;
    uint64_t startAddress;
    uint64_t endAddress;
    uint8_t isNative;
} Routine2;

Routine2* routine_new();
void routine_add_arg(Routine2 *routine, const char *arg);
void routine_add(Routine2 *routine);
Routine2* routine_get(uint64_t name);
