#pragma once

#include "values.h"
#include "strings.h"

typedef struct Environment{
    uint64_t *keys;
    uint64_t keyCount;
    Data *data;
    struct Environment *parent;
} Environment;

Environment* env_new();
void env_put(uint64_t key, Data value, Environment *env);
Data env_get(uint64_t key, Environment *env, uint8_t beSilent);
void env_free(Environment *env);

