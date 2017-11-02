#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stdlib.h>

void* mallocate(size_t size);
void* reallocate(void *oldmem, size_t size);
void memfree(void *mem);
void memfree_all();

#endif
