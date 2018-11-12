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
typedef enum {
#define INSTRUCTION(x) x,
#include "instruction.h"
#undef INSTRUCTION
} OpCode;
#endif
