#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>

#include "allocator.h"
#include "values.h"
#include "heap.h"
#include "display.h"

Data *heap = NULL;
uint32_t hp = 0;

uint32_t heap_add_int(int32_t i){
    uint32_t start = 0;
    while(start < hp){
        if(isint(heap[start]) && tint(heap[start]) == i)
            return start;
        start++;
    }
    heap = (Data *)reallocate(heap, sizeof(Data)*++hp);
    heap[hp - 1] = new_int(i);
    return hp - 1;
}

uint32_t heap_add_float(double i){
    uint32_t start = 0;
    while(start < hp){
        if(isfloat(heap[start]) && tfloat(heap[start]) == i)
            return start;
        start++;
    }
    heap = (Data *)reallocate(heap, sizeof(Data)*++hp);
    heap[hp - 1] = new_float(i);
    return hp - 1;
}

uint32_t heap_add_str(uint32_t key){
    uint32_t start = 0;
    while(start < hp){
        if(isstr(heap[start]) && tstrk(heap[start]) == key)
            return start;
        start++;
    }
    heap = (Data *)reallocate(heap, sizeof(Data)*++hp);
    heap[hp - 1] = new_strk(key);
    return hp - 1; 
}

uint32_t heap_add_identifer(uint32_t key){
    uint32_t start = 0;
    while(start < hp){
        if(isidentifer(heap[start]) && tstrk(heap[start]) == key)
            return start;
        start++;
    }
    heap = (Data *)reallocate(heap, sizeof(Data)*++hp);
    heap[hp - 1] = new_identiferk(key);
    return hp - 1; 
}

uint32_t heap_add_logical(int32_t i){
    uint32_t start = 0;
    while(start < hp){
        if(islogical(heap[start]) && tint(heap[start]) == i)
            return start;
        start++;
    }
    heap = (Data *)reallocate(heap, sizeof(Data)*++hp);
    heap[hp - 1] = new_logical(i);
    return hp - 1;
}

void heap_free(){
    memfree(heap);
}
