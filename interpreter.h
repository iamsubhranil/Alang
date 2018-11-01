#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "values.h"
#include <stdint.h>

size_t ins_add(uint8_t ins);
size_t ins_add_val(uint32_t d); // It will be either of a 32 bit heap address
                                // of a 32 bit integer
size_t ins_add_double(double d);

void ins_set(size_t mem, uint8_t ins);
void ins_set_val(size_t mem, uint32_t d); // Same reason

// uint32_t ins_get_val(uint32_t mem);
// double ins_get_double(uint32_t mem);
uint8_t ins_get(size_t mem);

size_t  ip_get();
void    ins_print();
uint8_t ins_last();

void interpreter_push(Data d);
void interpret();
void stop();
void print_stack_trace();

// Debugging

typedef struct {
	uint32_t line;
	uint32_t from; // From ip
	uint32_t to;   // To ip
	uint32_t fileName;
} FileInfo;

FileInfo fileInfo_of(uint32_t line);

// Instructions

// Load
#define PUSHF 0x00  // String => push index
#define PUSHI 0x01  // Identifer
#define PUSHL 0x02  // Int => push constant
#define PUSHS 0x03  // Float => push constant
#define PUSHID 0x04 // Logical => push 0/1
#define PUSHN 0x05  // Null => pushN
//#define PUSHU 0x28 // Unsigned => push unsigned int
// Arithmetic
#define ADD 0x06
#define SUB 0x07
#define MUL 0x08
#define DIV 0x09
#define POW 0x0a
#define MOD 0x0b
// Conditional
#define GT 0x0c
#define GTE 0x0d
#define LT 0x0e
#define LTE 0x0f
#define EQ 0x10
#define NEQ 0x11
#define AND 0x12
#define OR 0x13
// Basic IO operations
//#define SET 0x14          // Replaced by load/store slots
#define INPUTI \
	0x15 // Just reads an integer from stdin and pushes it to the stack
#define INPUTS 0x16 //               string
#define INPUTF 0x17 //               float
#define PRINT 0x18
// Control flow and branching
#define HALT 0x19
#define JUMP 0x1a
#define JUMP_IF_TRUE 0x1b
#define JUMP_IF_FALSE 0x1c
#define CALL 0x1d
#define RETURN 0x1e
// Data structures
#define ARRAYREF 0x1f   // Array access
#define MEMREF 0x20     // Takes the ID to resolve
#define MAKE_ARRAY 0x21 // Takes the slot number to store
// Noop
#define NOOP 0x22
// Container
#define NEW_CONTAINER 0x23 // Takes container name to resolve
#define MEMSET 0x24        // top <- value, top - 1 <- member id, top - 2 <- ins
//#define ARRAYREF 0x25
#define ARRAYSET 0x26
//#define ARRAYWRITE 0x27
// Native Calls
#define CALLNATIVE 0x28
// Implicit Set
//#define SETI 0x29
// Implicit push value of identifer
//#define PUSHIDV 0x2a
// Set the specified slot in (top - 1)
// with the top of the stack
#define STORE_SLOT 0x2b
// Load the value stored in the operand
// slot to the top of the stack
#define LOAD_SLOT 0x2c
// Reserve specified number of slots on
// the stack
#define RESERVE_SLOT 0x2d
// Store into global slot N
// First operand is the slot number
// Second operand is the parent number <-- Parent number is not needed,
//                                          since global environment
//                                          is always the enclosing env
//                                          of everyone
#define STORE_SLOT_GLOBAL 0x2e
// Load from global slot N
#define LOAD_SLOT_GLOBAL 0x2f
// Save store slot
// Saves the future store slot for the
// store_ call in a variable, instead
// of pushing into the stack
#define SAVE_STORE_SLOT 0x30
#endif
