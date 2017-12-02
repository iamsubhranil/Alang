#pragma once

#include "values.h"
#include "strings.h"

typedef struct Record{
    uint32_t key;
    Data data;
    struct Record *next;
} Record;

typedef struct Environment{
    Record *records;
    struct Environment *parent;
} Environment;

Environment env_new(Environment *parent);
void env_put(uint32_t key, Data value, Environment *env);
void env_implicit_put(uint32_t key, Data value, Environment *env);
Data env_get(uint32_t key, Environment *env, uint8_t beSilent);
void env_free(Environment env);

