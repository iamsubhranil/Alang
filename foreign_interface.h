#ifndef FOREIGN_INTERFACE_H
#define FOREIGN_INTERFACE_H

#include "interpreter.h"
#include "environment.h"
#include "display.h"

double get_double(char *identifer, int line, Environment *env);
long get_long(char *identifer, int line, Environment *env);
char* get_string(char *identifer, int line, Environment *env);

Object fromDouble(double d);
Object fromLong(long l);
Object fromString(char *strng);

#endif
