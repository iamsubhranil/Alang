#include <stdlib.h>
#include <stdio.h>

#include "display.h"
#include "allocator.h"

typedef struct Object{
    void* pointer;
    struct Object* next;
} Object;

static Object *front = NULL, *rear = NULL;
static int allocationCount = 0, insertCount = 0, reallocationCount = 0;

static void insert(void *mem){
    insertCount++;
    if(front == NULL){
//        printf("\n[Allocator] Front was nil. Rear was %p!", rear);
        front = rear = (Object *)malloc(sizeof(Object));
        front->pointer = mem;
        front->next = NULL;
    }
    else{
//        printf("\n[Allocator] Rear->next is at %p", rear->next);
        rear->next = (Object *)malloc(sizeof(Object));
        rear->next->pointer = mem;
        rear->next->next = NULL;
        rear = rear->next;
    }
//        printf("\n[Allocator] Created object at %p holding pointer %p!", rear, mem);
//        fflush(stdout);
}

static void exchange(void *oldmem, void *newmem){
    Object *bak = front;
    while(bak != NULL){
        if(bak->pointer == oldmem){
//            printf("\n[Allocator] Exchanged %p to %p", oldmem, newmem);
//            fflush(stdout);
            bak->pointer = newmem;
            return;
        }
        bak = bak->next;
    }
    insert(newmem);
}

void* mallocate(size_t size){
    void *mem = malloc(size);
    allocationCount++;
    if(mem == NULL){
        printf(error("Unable to allocate object! Insufficient memory!"));
        exit(1);
    }
    insert(mem);
    return mem;
}

void* reallocate(void *mem, size_t size){
    void *newmem = realloc(mem, size);
    if(newmem == NULL){
        printf(error("Unable to extend! Insufficient memory!"));
        exit(1);
    }
    reallocationCount++;
    if(mem != newmem)
        exchange(mem, newmem);
//    printf("\n[Allocator] Reallocated %p to %p", mem, newmem);
//        fflush(stdout);
    return newmem;
}

int get_realloc_count(){
    return reallocationCount;
}

void memfree(void *old){
    Object *bak = front, *prev = NULL;
    while(bak != NULL){
        if(bak->pointer == old){
            if(prev == NULL){
                if(rear == front)
                    rear = front = NULL;
                else
                    front = front->next;
            }
            else{
                prev->next = bak->next;
                if(prev->next == NULL)
                    rear = prev;
            }
            free(bak->pointer);
            allocationCount--;
            insertCount--;
            free(bak);
            return;
        }
        prev = bak;
        bak = bak->next;
    }
 /*   bak = front;
    while(bak->next != NULL){
        rear = bak;
        bak = bak->next;
    }
    */
}

void memfree_all(){
//    printf("\n[Allocator] Allocated memory %d times in %d objects", allocationCount, insertCount);
    int i = 0;
    while(front != NULL){
        Object *bak = front->next;
//        printf("\n[Allocator] Freed object %p holding %p", front, front->pointer);
        free(front->pointer);
        free(front);
        front = bak;
        i++;
    }
//    printf("\n[Allocator] Freed %d objects!", insertCount);
    front = rear = NULL;
}
