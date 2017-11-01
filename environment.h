#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "expr.h"

void env_put(char *identifer, Literal value);
Literal env_get(char *identifer);

#endif
