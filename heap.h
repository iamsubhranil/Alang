#pragma once

#include <stdint.h>
#include "values.h"

uint32_t heap_add_int(int32_t val);
uint32_t heap_add_float(double val);
uint32_t heap_add_str(uint32_t val);
uint32_t heap_add_identifer(uint32_t val);
uint32_t heap_add_logical(int32_t val);

Data heap_get_data(uint32_t address);
int32_t heap_get_int(uint32_t address);
double heap_get_float(uint32_t address);
uint32_t heap_get_str(uint32_t address);
int32_t heap_get_logical(uint32_t address);

void heap_free();
