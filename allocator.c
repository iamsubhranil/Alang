#include <stdlib.h>

#include "display.h"
#include "allocator.h"

typedef struct Object{
    void* pointer;
    struct Object* next;
} Object;

static Object *front = NULL, *rear = NULL;

static void insert(void *mem){ 
    if(front == NULL){
        front = rear = (Object *)malloc(sizeof(Object));
        front->pointer = mem;
        front->next = NULL;
    }
    else{
        rear->next = (Object *)malloc(sizeof(Object));
        rear->next->pointer = mem;
        rear->next->next = NULL;
        rear = rear->next;
    }
}

static void exchange(void *oldmem, void *newmem){
    Object *bak = front;
    while(bak != NULL){
        if(bak->pointer == oldmem){
            bak->pointer = newmem;
            return;
        }
        bak = bak->next;
    }
    insert(newmem);
}

void* mallocate(size_t size){
    void *mem = malloc(size);
    if(mem == NULL)
        error("Unable to allocate object! Insufficient memory!");
    insert(mem);
    return mem;
}

void* reallocate(void *mem, size_t size){
    void *newmem = realloc(mem, size);
    if(newmem == NULL)
        error("Unable to extend! Insufficient memory!");
    exchange(mem, newmem);
    return newmem;
}

void mfree(){
    while(front != NULL){
        Object *bak = front->next;
  //      free(front->pointer);
  //      free(front);
        front = bak;
    }
    front = rear = NULL;
}
