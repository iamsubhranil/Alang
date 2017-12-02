#ifndef NATIVE_H
#define NATIVE_H

#include "interpreter.h"
#include "env.h"

Data handle_native(uint32_t name, Environment *env);
void register_native(Environment *env);
void unload_all();
#endif
