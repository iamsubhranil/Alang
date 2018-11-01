#include "object.h"
#include "values.h"
#include <assert.h>
// The object queue
static Object  root  = {0, NULL, OBJ_ROOT};
static Object *front = &root, *end = &root;
static size_t  allocated = 0;

#ifndef GC_THRESHOLD_BYTES
#define GC_THRESHOLD_BYTES 1024 * 100 // Number of bytes of data to be
#endif                                // allocated before kicking the gc in
// default : 10 MiB

static void collect(Object *obj) {
	/*
	dbg("Collecting garbage of %p\n", obj);
	 Object *o = (Object*)obj;
	 if(o->type == OBJ_INSTANCE){
	     Instance *ins = (Instance*)obj;
	    for(uint32_t i = 0;i < ins->memberCount;i++)
	        print_value("any", ins->values[i]);
	 } else if(o->type == OBJ_STRING) {
	    dbg("str\n");
	 }
	 dbg("\n");
	 */
	switch(obj->type) { // Release any specially allocated memory
		case OBJ_ARRAY: allocated -= arr_release((void *)obj); break;
		case OBJ_STRING: allocated -= str_release((void *)obj); break;
		case OBJ_INSTANCE: allocated -= ins_release((void *)obj); break;
		default: break;
	}
}

static void obj_gc() {
	Object *parent = front;
	for(Object *obj = front->next; obj != NULL; parent = obj, obj = obj->next) {
		if(obj->refCount == 0) { // Collect
			collect(obj);
			// Update parent
			parent->next = obj->next;
			// Free the object
			// dbg("Freeing %p\n", obj);
			free(obj);
			// Repoint obj to the previous object
			// since it is going to be updated
			// in the loop
			obj = parent;
			// Check if list is ended
			// if(obj == NULL)
			//	break;
		}
	}
	// dbg("After collection, allocated %u bytes\n", allocated);
	end       = parent;
	end->next = NULL;
}

void *obj_alloc(size_t size, ObjectType type) {
	if(allocated > GC_THRESHOLD_BYTES)
		obj_gc();
	Object *obj   = (Object *)malloc(size);
	obj->next     = NULL;
	obj->type     = type;
	obj->refCount = 0;
	// Add it to the queue
	end->next = obj;
	end       = obj;

	allocated += size;
	return obj;
}

void obj_ref_decr(void *obj) {
	// uint32_t oldref = ((Object *)obj)->refCount;
	/*dbg("%u\n", ((Object*)obj)->type);
	if(((Object*)obj)->type == OBJ_INSTANCE){
	    dbg("Instance\n");
	    Instance *ins = (Instance*)obj;
	   for(uint32_t i = 0;i < ins->memberCount;i++)
	       print_value("any", ins->values[i]);
	} */
	// assert(oldref > 0);
	// dbg("ref_decr : %p oldref %u newrref %u\n", obj, oldref, oldref - 1);
	//		dbg("Decrementing ref %p\n", obj);
	((Object *)obj)->refCount--;
	// if(((Object*)obj)->refCount == 0) {
	//    collect((Object*)obj);
	//    free(obj);
	//}
}

void obj_ref_incr(void *obj) {
	// uint32_t oldref = ((Object *)obj)->refCount;
	// dbg("ref_incr : %p oldref %u newrref %u\n", obj, oldref, oldref + 1);
	((Object *)obj)->refCount++;
}

static uint8_t isfree = 0;

void obj_free() {
	isfree = 1;
	for(Object *f = front->next; f != NULL; f = f->next) f->refCount = 0;
	obj_gc();
	isfree = 0;
}

uint8_t obj_isfree() {
	return isfree;
}
