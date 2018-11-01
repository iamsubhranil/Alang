#include "values.h"
#include "allocator.h"
#include "routines.h"
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

// static uint32_t id = 0;

double isInt_IntPart = 0;

Data new_array(uint32_t size) {
	Array *arr  = (Array *)obj_alloc(sizeof(Array), OBJ_ARRAY);
	arr->size   = size;
	arr->values = (Data *)malloc(sizeof(Data) * size);
	for(uint32_t i = 0; i < size; i++) arr->values[i] = new_null();
	return ARR | (uintptr_t)arr;
}

Data new_ins(Data *baseStack, Routine2 *r) {
	Instance *ins    = (Instance *)obj_alloc(sizeof(Instance), OBJ_INSTANCE);
	ins->member      = r->variables;
	ins->memberCount = r->slots;
	ins->name        = r->name;
	// Copy the values
	ins->values = (Data *)malloc(sizeof(Data) * ins->memberCount);
	memcpy(ins->values, baseStack, sizeof(Data) * ins->memberCount);
	// dbg("Creating instance %p", ins);
	// for(uint32_t i = 0;i < ins->memberCount;i++)
	//        print_value("any", ins->values[i]);
	return INSTANCE | (uintptr_t)ins;
}

static Data *get_member_ptr(Instance *ins, uint32_t mem) {
	for(size_t i = 0; i < ins->memberCount; i++) {
		if(ins->member[i] == mem)
			return &(ins->values[i]);
	}
	return NULL;
}

Data get_member(Instance *ins, uint32_t mem) {
	Data *ptr = get_member_ptr(ins, mem);
	if(ptr == NULL)
		rerr("No such member found : %s", str_get(mem));
	else
		return *ptr;
	return new_none();
}

void set_member(Instance *ins, uint32_t mem, Data value) {
	Data *ptr = get_member_ptr(ins, mem);
	if(ptr == NULL)
		rerr("No such member found : %s", str_get(mem));
	else {
		// dbg("Setting '%s' of %p to ",str_get(mem), ins);
		// print_value("any", value);
		Data oldValue = *ptr;
		if(oldValue != value) {
			// dbg("Here");
			ref_incr(value);
			ref_decr(oldValue);
			*ptr = value;
		}
	}
}

void print_bit(Data d) {
	uint8_t *bits = (uint8_t *)&d;
	size_t   i    = sizeof(d);
	printf("\tBytes : ");
	while(i > 0) printf("%x", bits[--i]);
}

void print_value(const char *expected, Data d) {
	printf("Expected : %s\n", expected);
	printf("Received : ");
	if(isfloat(d)) {
		printf("Float\tValue : %F", tfloat(d));
		// printf("\t&QNAN : ");
		// print_bit(d & QNAN);
	} else if(isint(d))
		printf("Integer\tValue : %" PRId32, tint(d));
	else if(islogical(d))
		printf("Logical\tValue : %" PRId32, tlogical(d));
	else if(isstr(d))
		printf("String\tKey : %" PRId32 "\tValue : %s", tstrk(d), tstr(d));
	else if(isidentifer(d))
		printf("Identifier\tKey : %" PRId32 "\tValue : %s", tstrk(d), tstr(d));
	else if(isnull(d))
		printf("Null");
	else if(isnone(d))
		printf("None");
	else if(isins(d))
		printf("Instance");
	else if(isarray(d)) {
		printf("Array");
		Array *arr = tarr(d);
		printf("\tSize : %zu", arr_size(arr));
	}

	print_bit(d);

	printf("\n");
}

void data_free(Data d) {
	// if(isnum(d))
	//    return;
	if(isstr(d)) {
		str_ref_decr(tstrk(d));
	} else if(isins(d)) {
		obj_ref_decr(tins(d));
	} else if(isarray(d)) {
		obj_ref_decr(tarr(d));
	}
}

size_t arr_release(void *arr) {
	Array *a = (Array *)arr;
	if(!obj_isfree()) {
		// Decrement the ref counter for all
		// values
		for(uint32_t i = 0; i < a->size; i++) {
			ref_decr(a->values[i]);
		}
	}
	// Release the array
	free(a->values);
	return sizeof(Array);
}

size_t ins_release(void *ins) {
	Instance *i = (Instance *)ins;
	if(!obj_isfree()) {
		for(uint32_t j = 0; j < i->memberCount; j++) {
			ref_decr(i->values[j]);
		}
	}
	free(i->values);
	return sizeof(Instance);
}

/*

   int main(){
   print_type("Float", new_float(9.9878938438));
   print_type("Int", new_int(9));
   print_type("Logical", new_logical(1));
   print_type("Identifier", new_identifer("Iden"));
   print_type("String", new_str("This is a string!"));
   print_type("Null", new_null());
   print_type("None", new_none());
   print_type("Array(5)", new_array(5));
   print_type("Instance", new_ins(NULL, 9));
   }
   */
