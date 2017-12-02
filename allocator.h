#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stdlib.h>

#define USE_ALLOCATE_API

#ifdef USE_ALLOCATE_API

void* mallocate(size_t size);
void* reallocate(void *oldmem, size_t size);
void memfree(void *mem);
void memfree_all();

#else

#define mallocate(x) malloc(x)
#define reallocate(x, y) realloc(x, y)
#define memfree(x) free(x)
#define memfree_all() {}

#endif

#endif
