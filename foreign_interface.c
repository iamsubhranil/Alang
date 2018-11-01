#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "foreign_interface.h"
#include "interpreter.h"
#include "strings.h"

double get_double(const char *identifer, Environment *env) {
	Data o = env_get(str_insert(strdup(identifer)), env, 0);
	if(!isfloat(o)) {
		rerr("Expected numeric value!");
	}
	return tfloat(o);
}

int32_t get_int(const char *identifer, Environment *env) {
	Data o = env_get(str_insert(strdup(identifer)), env, 0);
	if(!isint(o)) {
		rerr("Expected integer value!");
	}
	return tint(o);
}

char *get_string(const char *identifer, Environment *env) {
	Data o = env_get(str_insert(strdup(identifer)), env, 0);
	if(!isstr(o)) {
		err("Expected integer value!");
	}
	return strdup(tstr(o));
}
