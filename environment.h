#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "stmt.h"
#include "interpreter.h"

typedef struct Record{
    char *name;
    Object object;
    struct Record* next;
} Record;

typedef struct Environment{
    Record *front;
    Record *rear;
    struct Environment *parent;
} Environment;

Environment *env_new(Environment *parent);
void env_free(Environment *env);

void env_put(char *identifer, Object value, Environment *env);
Object env_get(char *identifer, int line, Environment *env);

void env_arr_new(char *identifer, int line, long numElements, Environment *env);
void env_arr_put(char *identifer, long index, Literal value, Environment *env);
Literal env_arr_get(char *identifer, int line, long index, Environment *env);

void env_routine_put(Routine r, int line, Environment *env);
Routine env_routine_get(char *identifer, int line, Environment *env);

void env_container_put(Container c, int line, Environment *env);
Container env_container_get(char *identifer, int line, Environment *env);

#endif
