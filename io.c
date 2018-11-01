#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#include "allocator.h"
#include "display.h"
#include "io.h"
#include "values.h"

static char *readString() {
	char *ret = NULL;
	char  c   = getchar();
	int   i   = 0;
	while(c != ' ' && c != '\n') {
		i++;
		ret        = (char *)reallocate(ret, sizeof(char) * i);
		ret[i - 1] = c;
		c          = getchar();
	}
	ret    = (char *)reallocate(ret, sizeof(char) * (i + 1));
	ret[i] = '\0';
	return ret;
}

static char *readStringAll() {
	char *ret = NULL;
	char  c   = getchar();
	int   i   = 0;
	while(c != '\n') {
		i++;
		ret        = (char *)reallocate(ret, sizeof(char) * i);
		ret[i - 1] = c;
		c          = getchar();
	}
	ret    = (char *)reallocate(ret, sizeof(char) * (i + 1));
	ret[i] = '\0';
	return ret;
}

Data getString() {
	char *s = readStringAll();
	return new_str(s);
}

static int isNum(char c) {
	return c >= '0' && c <= '9';
}

static int isSign(char c) {
	return c == '-' || c == '+';
}

static int isInt(char *s) {
	int i = 0;
	if(isSign(s[0]))
		i++;
	while(s[i] != '\0') {
		if(!isNum(s[i]))
			return 0;
		i++;
	}
	if(isSign(s[0]) && i == 1)
		return 0;
	return 1;
}

Data getInt() {
	char *s = readString();
	while(!isInt(s)) {
		rwarn("[Input Error] Not an integer : %s!", s);
	int_retake:
		memfree(s);
		info("[Re-input] ");
		s = readString();
	}
	int64_t l = 0;
	sscanf(s, "%" SCNd64, &l);
	if(l > INT32_MAX) {
		rwarn("[Input Error] Integer out of range : %s > %" PRId32, s,
		      INT32_MAX);
		goto int_retake;
	} else if(l < INT32_MIN) {
		rwarn("[Input Error] Integer out of range : %s < %" PRId32, s,
		      INT32_MIN);
		goto int_retake;
	}
	memfree(s);
	return new_int(l);
}

static int isNumber(char *s) {
	int i = 0, hasDot = 0;
	if(isSign(s[0]))
		i++;
	while(s[i] != '\0') {
		if(s[i] == '.') {
			if(hasDot == 0)
				hasDot = 1;
			else
				return 0;
		} else if(!isNum(s[i]))
			return 0;
		i++;
	}
	if(isSign(s[0]) && i == 1)
		return 0;
	return 1;
}

Data getFloat() {
	char *s = readString();
	while(!isNumber(s)) {
		rwarn("[Input Error] Not a number : %s!", s);
		memfree(s);
		info("[Re-input] ");
		s = readString();
	}
	double d = 0;
	sscanf(s, "%lf", &d);
	memfree(s);
	return new_float(d);
}
