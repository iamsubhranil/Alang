#include "display.h"
#include <stdio.h>

#include "allocator.h"
#include "interpreter.h"
#include "routines.h"

Routine2 *routines = NULL;
uint32_t  rp       = 0;

Routine2 routine_new() {
	Routine2 r;
	r.arity        = 0;
	r.name         = 0;
	r.startAddress = 0;
	r.isNative     = 0;
	r.variables    = NULL;
	r.slots        = 0;
	r.isVararg     = 0;
	return r;
}

void routine_add_slot(Routine2 *r, uint32_t var) {
	r->variables =
	    (uint32_t *)reallocate(r->variables, sizeof(var) * ++r->slots);
	r->variables[r->slots - 1] = var;
}

uint8_t routine_get_slot(Routine2 *r, uint32_t var) {
	for(uint8_t i = 0; i < r->slots; i++) {
		if(r->variables[i] == var)
			return i;
	}
	routine_add_slot(r, var);
	return r->slots - 1;
}

void routine_add(Routine2 r) {
	routines = (Routine2 *)reallocate(routines, sizeof(Routine2) * ++rp);
	//    printf(debug("[Routine] Adding routine : [%s]"), str_get(r.name));
	routines[rp - 1] = r;
}

void routine_free() {
	uint32_t i = 0;
	while(i < rp) {
		memfree(routines[i].variables);
		i++;
	}
	memfree(routines);
}

Routine2 *routine_get(uint32_t name) {
	uint32_t i = 0;
	while(i < rp) {
		//        printf(debug("Routine : %s"), str_get(routines[i]->name));
		if(routines[i].name == name)
			return &routines[i];
		i++;
	}
	rerr("Routine not found : '%s'!", str_get(name));
	return NULL;
}
