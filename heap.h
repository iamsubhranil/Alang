#pragma once

#include <stdint.h>
#include <inttypes.h>
#include "display.h"
#include "values.h"

extern Data *heap;
extern uint32_t hp;

uint32_t heap_add_int(int32_t val);
uint32_t heap_add_float(double val);
uint32_t heap_add_str(uint32_t val);
uint32_t heap_add_identifer(uint32_t val);
uint32_t heap_add_logical(int32_t val);

//static inline Data heap_get_data(uint32_t address);
static inline int32_t heap_get_int(uint32_t address);
static inline double heap_get_float(uint32_t address);
static inline uint32_t heap_get_str(uint32_t address);
static inline int32_t heap_get_logical(uint32_t address);

void heap_free();

#define heap_init() {heap = NULL; hp = 0;}

#define heap_check() if(address > hp) \
        { \
        rerr("Bad heap access : %" PRIu32, address); \
        return 0;} \

static inline int32_t heap_get_int(uint32_t address){
    heap_check();
    return tint(heap[address]);
}

static inline double heap_get_float(uint32_t address){
    heap_check();
    return tfloat(heap[address]);
}

static inline uint32_t heap_get_str(uint32_t address){
    heap_check();
    return tstrk(heap[address]);
}

static inline int32_t heap_get_logical(uint32_t address){
    heap_check();
    return tint(heap[address]);
}
