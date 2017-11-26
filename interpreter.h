#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <stdint.h>

uint64_t ins_add(uint8_t ins);
uint64_t ins_add_val(uint64_t d);

void ins_set(uint64_t mem, uint8_t ins);
void ins_set_val(uint64_t mem, uint64_t d);

uint64_t ins_get_val(uint64_t mem);
uint8_t ins_get(uint64_t mem);

uint64_t ip_get();
void ins_print();

void interpret();
void stop();
// Instructions

// Load
#define PUSHS 0x01 // String => push index
#define PUSHID 0x02 // Identifer
#define PUSHI 0x03 // Int => push constant
#define PUSHF 0x04 // Float => push constant
#define PUSHL 0x05 // Logical => push 0/1
#define PUSHN 0x06 // Null => pushN
// Arithmetic
#define ADD 0x11
#define SUB 0x12
#define MUL 0x13
#define DIV 0x14
#define POW 0x15
#define MOD 0x16
// Conditional
#define GT 0x21
#define GTE 0x22
#define LT 0x23
#define LTE 0x24
#define EQ 0x25
#define NEQ 0x26
#define AND 0x27
#define OR 0x28
// Basic IO operations
#define SET 0x31
#define INPUTI 0x32
#define INPUTF 0x33
#define INPUTS 0x34
#define PRINT 0x35
#define HALT 0x36
// Control flow and branching
#define JUMP 0x41
#define JUMP_IF_TRUE 0x42
#define JUMP_IF_FALSE 0x43
#define CALL 0x44
#define RETURN 0x45
// Data structures
#define ARRAY 0x51
#define MEMREF 0x52
#define MAKE_ARRAY 0x53
// Noop
#define NOOP 0xFF
#endif
