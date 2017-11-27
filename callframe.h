#pragma once

#include <stdint.h>

#include "env.h"

typedef struct{
    uint16_t arity;
    uint64_t returnAddress;
    Environment env;
} CallFrame;

CallFrame cf_new();
void cf_push(CallFrame frame);
CallFrame cf_peek();
CallFrame cf_pop();
void cf_free(CallFrame frame);
