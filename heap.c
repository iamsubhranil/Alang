#include <stdint.h>
#include <stdio.h>

#include "allocator.h"
#include "values.h"
#include "heap.h"
#include "display.h"

static Data *heap = NULL;
static uint64_t hp = 0;

#define heap_check() if(address > hp){ \
        printf(error("Bad heap access : %ld"), address); \
        return 0;}

uint64_t heap_add_int(int64_t i){
    uint64_t start = 0;
    while(start < hp){
        if(isint(heap[start]) && tint(heap[start]) == i)
            return start;
        start++;
    }
    heap = (Data *)reallocate(heap, sizeof(Data)*++hp);
    heap[hp - 1] = new_int(i);
    return hp - 1;
}

uint64_t heap_add_float(double i){
    uint64_t start = 0;
    while(start < hp){
        if(isfloat(heap[start]) && tfloat(heap[start]) == i)
            return start;
        start++;
    }
    heap = (Data *)reallocate(heap, sizeof(Data)*++hp);
    heap[hp - 1] = new_float(i);
    return hp - 1;
}

uint64_t heap_add_str(uint64_t key){
    uint64_t start = 0;
    while(start < hp){
        if(isstr(heap[start]) && tstrk(heap[start]) == key)
            return start;
        start++;
    }
    heap = (Data *)reallocate(heap, sizeof(Data)*++hp);
    heap[hp - 1] = new_strk(key);
    return hp - 1; 
}

uint64_t heap_add_identifer(uint64_t key){
    uint64_t start = 0;
    while(start < hp){
        if(isidentifer(heap[start]) && tstrk(heap[start]) == key)
            return start;
        start++;
    }
    heap = (Data *)reallocate(heap, sizeof(Data)*++hp);
    heap[hp - 1] = new_identiferk(key);
    return hp - 1; 
}

uint64_t heap_add_logical(int64_t i){
    uint64_t start = 0;
    while(start < hp){
        if(islogical(heap[start]) && tint(heap[start]) == i)
            return start;
        start++;
    }
    heap = (Data *)reallocate(heap, sizeof(Data)*++hp);
    heap[hp - 1] = new_logical(i);
    return hp - 1;
}

Data heap_get_data(uint64_t address){
    if(address > hp - 1){
        printf(error("Invalid heap access : %lu"), address);
        return new_null();
    }
    return heap[address];
}

int64_t heap_get_int(uint64_t address){
    heap_check();
    return tint(heap[address]);
}

double heap_get_float(uint64_t address){
    heap_check();
    return tfloat(heap[address]);
}

uint64_t heap_get_str(uint64_t address){
    heap_check();
    return tstrk(heap[address]);
}

int64_t heap_get_logical(uint64_t address){
    heap_check();
    return tint(heap[address]);
}
