#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "expr.h"

typedef struct{
    int count;
    Literal *values;
} Array;

typedef enum{
    LITERAL,
    ARRAY
} EnvType;

typedef struct Record{
    char* name;
    EnvType type;
    union{
        Literal value;
        Array arr;
    };
    struct Record* next;
} Record;

typedef struct Environment{
    Record *front;
    Record *rear;
    struct Environment *parent;
} Environment;

void env_put(char *identifer, Literal value, Environment *env);
Literal env_get(char *identifer, int line, Environment *env);
void env_arr_new(char *identifer, int line, long numElements, Environment *env);
void env_arr_put(char *identifer, long index, Literal value, Environment *env);
Literal env_arr_get(char *identifer, int line, long index, Environment *env);
Environment *env_new(Environment *parent);
void env_free(Environment *env);

#endif
