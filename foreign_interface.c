#include <stdio.h>

#include "environment.h"
#include "display.h"
#include "interpreter.h"

static int is_num(Object obj){
    return obj.literal.type == LIT_INT || obj.literal.type == LIT_DOUBLE;
}

static double get_val(Literal lit){
    return lit.type == LIT_INT ? lit.lVal : lit.dVal;
}

double get_double(char *identifer, int line, Environment *env){
    Object o = env_get(identifer, line, env);
    if(o.type != OBJECT_LITERAL || !is_num(o)){
        printf(runtime_error("Expected numeric value!"), line);
        stop();
    }
    return get_val(o.literal);
}

long get_long(char *identifer, int line, Environment *env){
    Object o = env_get(identifer, line, env);
    if(o.type != OBJECT_LITERAL || o.literal.type != LIT_INT){
        printf(runtime_error("Expected integer value!"), line);
        stop();
    }
    return o.literal.iVal;
}

char* get_string(char *identifer, int line, Environment *env){
    Object o = env_get(identifer, line, env);
    if(o.type != OBJECT_LITERAL || o.literal.type != LIT_STRING){
        printf(runtime_error("Expected string value!"), line);
        stop();
    }
    return o.literal.sVal;
}

Object fromDouble(double d){
    Object o;
    o.type = OBJECT_LITERAL;
    o.literal.type = LIT_DOUBLE;
    o.literal.dVal = d;
    return o;
}

Object fromLong(long l){
    Object o;
    o.type = OBJECT_LITERAL;
    o.literal.type = LIT_INT;
    o.literal.dVal = l;
    return o;
}

Object fromString(char *string){
    Object o;
    o.type = OBJECT_LITERAL;
    o.literal.type = LIT_STRING;
    o.literal.sVal = string;
    return o;
}
