#pragma once

#include "display.h"
#include "strings.h"

typedef struct {
	uint32_t  name;
	uint32_t  arity;
	uint32_t *variables;
	uint32_t  startAddress;
	uint8_t   isNative;
	uint8_t   slots;
} Routine2;

extern Routine2 *routines;
extern uint32_t  rp;

Routine2  routine_new();
void      routine_add_slot(Routine2 *routine, uint32_t var);
uint8_t   routine_has_slot(Routine2 *routine, uint32_t var);
uint8_t   routine_get_slot(Routine2 *routine, uint32_t var);
void      routine_add(Routine2 routine);
void      routine_free();
Routine2 *routine_get(uint32_t name);
