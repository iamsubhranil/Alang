#include <stdint.h>
#include <stdio.h>

#include "allocator.h"
#include "values.h"
#include "heap.h"

static Data **heap = NULL;
static uint64_t hp = 0;

#define heap_check() if(address > hp){ \
        printf("[Error: Bad heap access : %ld] ", address); \
        return 0;}

uint64_t heap_add_int(int64_t i){
    uint64_t start = 0;
    while(start < hp){
        if(heap[start]->type == INT && heap[start]->ivalue == i)
            return start;
        start++;
    }
    heap = (Data **)reallocate(heap, sizeof(Data *)*++hp);
    heap[hp - 1] = new_int(i);
    return hp - 1;
}

uint64_t heap_add_float(double i){
    uint64_t start = 0;
    while(start < hp){
        if(heap[start]->type == FLOAT && heap[start]->cvalue == i)
            return start;
        start++;
    }
    heap = (Data **)reallocate(heap, sizeof(Data *)*++hp);
    heap[hp - 1] = new_float(i);
    return hp - 1;
}

uint64_t heap_add_str(uint64_t key){
    uint64_t start = 0;
    while(start < hp){
        if(heap[start]->type == STRING && heap[start]->svalue == key)
            return start;
        start++;
    }
    heap = (Data **)reallocate(heap, sizeof(Data *)*++hp);
    heap[hp - 1] = new_data();
    heap[hp - 1]->refCount = 0;
    heap[hp - 1]->type = STRING;
    heap[hp - 1]->svalue = key;
    return hp - 1; 
}

uint64_t heap_add_identifer(uint64_t key){
    uint64_t start = 0;
    while(start < hp){
        if(heap[start]->type == IDENTIFIER && heap[start]->svalue == key)
            return start;
        start++;
    }
    heap = (Data **)reallocate(heap, sizeof(Data *)*++hp);
    heap[hp - 1] = new_data();
    heap[hp - 1]->refCount = 0;
    heap[hp - 1]->type = IDENTIFIER;
    heap[hp - 1]->svalue = key;
    return hp - 1; 
}

uint64_t heap_add_logical(int64_t i){
    uint64_t start = 0;
    while(start < hp){
        if(heap[start]->type == LOGICAL && heap[start]->ivalue == i)
            return start;
        start++;
    }
    heap = (Data **)reallocate(heap, sizeof(Data *)*++hp);
    heap[hp - 1] = new_logical(i);
    return hp - 1;
}

Data* heap_get_data(uint64_t address){
    heap_check();
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
    return heap[address]->svalue;
}

int64_t heap_get_logical(uint64_t address){
    heap_check();
    return tint(heap[address]);
}
