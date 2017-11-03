#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "expr.h"

void env_put(char *identifer, Literal value);
Literal env_get(char *identifer);
void env_arr_new(char *identifer, long numElements);
void env_arr_put(char *identifer, long index, Literal value);
Literal env_arr_get(char *identifer, long index);

#endif
