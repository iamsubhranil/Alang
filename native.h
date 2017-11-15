#ifndef NATIVE_H
#define NATIVE_H

#include "interpreter.h"
#include "environment.h"

Object handle_native(Call c, Environment *env);
void register_native(Environment *env);
void unload_all();
#endif
