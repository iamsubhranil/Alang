#pragma once

#include <stdint.h>
#include <stdlib.h>

typedef enum { OBJ_ROOT, OBJ_STRING, OBJ_INSTANCE, OBJ_ARRAY } ObjectType;

typedef struct Object {
	size_t         refCount;
	struct Object *next;
	ObjectType     type;
} Object;

#define objtype(x) (((x)->obj->type))
#define isobjinstance(x) ((objtype((x)) == OBJ_INSTANCE))
#define isobjstr(x) ((objtype((x)) == OBJ_STRING))
#define isobjarr(x) ((objtype((x)) == OBJ_ARRAY))

// Allocates an object of the specified size
void *obj_alloc(size_t size, ObjectType type);
void  obj_ref_decr(void *obj);
void  obj_ref_incr(void *obj);
void  obj_free();
// Marks whether this gc call resulted from obj_free
// This distinction is needed since  obj_free frees all
// object anyway, re-freeing the members of instances
// or arrays leads to a bad situation
uint8_t obj_isfree();
