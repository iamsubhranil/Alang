#ifndef FOREIGN_INTERFACE_H
#define FOREIGN_INTERFACE_H

#include <stdint.h>

#include "display.h"
#include "env.h"
#include "values.h"

double  get_double(const char *identifer, Environment *env);
int32_t get_int(const char *identifer, Environment *env);
char *  get_string(const char *identifer, Environment *env);

#endif
