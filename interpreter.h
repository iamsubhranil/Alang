#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <stdint.h>

uint32_t ins_add(uint8_t ins);
uint32_t ins_add_val(uint32_t d); // It will be either of a 32 bit heap address of a 32 bit integer
uint32_t ins_add_double(double d);

void ins_set(uint32_t mem, uint8_t ins);
void ins_set_val(uint32_t mem, uint32_t d); // Same reason

//uint32_t ins_get_val(uint32_t mem);
//double ins_get_double(uint32_t mem);
uint8_t ins_get(uint32_t mem);

uint32_t ip_get();
void ins_print();
uint8_t ins_last();

void interpret();
void stop();

// Debugging

typedef struct{
    uint32_t fileName;
    uint32_t line;
} FileInfo;

FileInfo fileInfo_of(uint32_t line);

// Instructions

// Load
#define PUSHF 0x00 // String => push index
#define PUSHI 0x01 // Identifer
#define PUSHL 0x02 // Int => push constant
#define PUSHS 0x03 // Float => push constant
#define PUSHID 0x04 // Logical => push 0/1
#define PUSHN 0x05 // Null => pushN
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
#define SET 0x14
#define INPUTI 0x15
#define INPUTS 0x16
#define INPUTF 0x17
#define PRINT 0x18
// Control flow and branching
#define HALT 0x19
#define JUMP 0x1a
#define JUMP_IF_TRUE 0x1b
#define JUMP_IF_FALSE 0x1c
#define CALL 0x1d
#define RETURN 0x1e
// Data structures
#define ARRAY 0x1f
#define MEMREF 0x20
#define MAKE_ARRAY 0x21
// Noop
#define NOOP 0x22
// Container
#define NEW_CONTAINER 0x23
#define MEMSET 0x24
#define ARRAYREF 0x25
#define ARRAYSET 0x26
#define ARRAYWRITE 0x27
// Native Calls
#define CALLNATIVE 0x28
// Implicit Set
#define SETI 0x29
// Implicit push value of identifer
#define PUSHIDV 0x2a
#endif
