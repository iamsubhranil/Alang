#include <stdio.h>
#include <string.h>

#include "allocator.h"
#include "display.h"
#include "object.h"
#include "strings.h"

typedef struct {
	Object   obj;
	char *   value;
	size_t   length;
	uint32_t hash;
} String;

static String **strarray    = NULL;
size_t          stringCount = 0;
size_t          c           = 0;

static uint32_t hash(const char *str) {
	uint32_t hash = 5381;
	c             = 0;
	//    printf("\nHashing..\n");
	while(str[c] != 0)
		hash = ((hash << 5) + hash) + str[c++]; /* hash * 33 + c */
	//    printf("\nInput String : [%s] Hash : %lu\n", str, hash);
	return hash;
}

size_t str_insert(char *str, uint8_t isConstant) {
	uint32_t has = hash(str);
	size_t   i   = 0;
	while(i < stringCount) {
		if(strarray[i]->hash == has) {
			obj_ref_incr(strarray[i]);
			//            printf(debug("[Strings] Match found for [%s] at
			//            %lu[%s]"), str, i, strarray[i]->value);
			memfree(str);
			return i;
		}
		i++;
	}
	strarray =
	    (String **)reallocate(strarray, sizeof(String *) * ++stringCount);
	strarray[stringCount - 1] = (String *)obj_alloc(sizeof(String), OBJ_STRING);
	strarray[stringCount - 1]->hash   = has;
	strarray[stringCount - 1]->value  = str;
	strarray[stringCount - 1]->length = c;
	if(isConstant)
		obj_ref_incr(strarray[stringCount - 1]);
	//    printf(debug("[Strings] Adding [%s]"), str);
	return stringCount - 1;
}

void str_ref_incr(uint32_t index) {
	obj_ref_incr(strarray[index]);
}

size_t str_len(uint32_t index) {
	return strarray[index]->length;
}

void str_ref_decr(uint32_t index) {
	// if(stringCount == 0 || index > (stringCount - 1))
	//    return;
	obj_ref_decr(strarray[index]);
}

const char *str_get(uint32_t index) {
	if(index >= stringCount) {
		// printf("\n%u\n", index);
		return "BadIndex";
	}
	return strarray[index]->value;
}

void str_free() {
	memfree(strarray);
	stringCount = 0;
}

size_t str_release(void *s) {
	String *str = (String *)s;
	free(str->value);
	return sizeof(String);
}
