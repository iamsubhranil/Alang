#pragma once

#include <stdint.h>
#include "values.h"

uint64_t heap_add_int(int64_t val);
uint64_t heap_add_float(double val);
uint64_t heap_add_str(uint64_t val);
uint64_t heap_add_identifer(uint64_t val);
uint64_t heap_add_logical(int64_t val);

Data heap_get_data(uint64_t address);
int64_t heap_get_int(uint64_t address);
double heap_get_float(uint64_t address);
uint64_t heap_get_str(uint64_t address);
int64_t heap_get_logical(uint64_t address);
