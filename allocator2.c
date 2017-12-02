#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "display.h"
#include "allocator.h"

void **allocatedMem = NULL;
static uint32_t pointer = 0, totalSize = 0, i = 0;

void* mallocate(size_t size){
    void* mem = malloc(size);
    if(mem == NULL){
        printf(error("Insufficient memory!"));
        exit(1);
    }
    if(allocatedMem == NULL){
        allocatedMem = (void **)malloc(sizeof(void *) * 10);
        totalSize = 10;
    }
    else if(pointer == totalSize){
        totalSize *= 2;
        allocatedMem = (void **)realloc(allocatedMem, sizeof(void *)*totalSize);
    }
    allocatedMem[pointer++] = mem;
    return mem;
}

void* reallocate(void *p, size_t size){
    void* mem = realloc(p, size);
    if(mem == NULL){
        printf(error("Insufficient memory!"));
        exit(1);
    }
    if(mem == p)
        return mem;
    i = 0;
    while(i < pointer){
        if(allocatedMem[i] == p){
            allocatedMem[i] = mem;
            break;
        }
        i++;
    }
    return mem;
}

void memfree(void *p){
    i = 0;
    while(i < pointer){
        if(allocatedMem[i] == p){
            allocatedMem[i] = allocatedMem[pointer - 1];
            if(pointer < (totalSize/2)){
                totalSize /= 2;
                allocatedMem = (void **)realloc(allocatedMem, sizeof(void *)*totalSize);
            }
            pointer--;
            free(p);
            break;
        }
        i++;
    }
}

void memfree_all(){
    i = 0;
    while(i < pointer){
        free(allocatedMem[i++]);
    }
    free(allocatedMem);
}
