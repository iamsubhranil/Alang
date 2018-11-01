#ifndef NATIVE_H
#define NATIVE_H

#include "interpreter.h"

Data handle_native(uint32_t name, uint32_t numargs, Data *env);
void register_natives();
void load_natives();
void unload_all();
#endif
