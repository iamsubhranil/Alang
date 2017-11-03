#include <stdio.h>

#include "allocator.h"
#include "expr.h"
#include "io.h"

static char* readString(){
    char *ret = NULL;
    char c = getchar();
    int i = 0;
    while(c != ' ' && c != '\n'){
        i++;
        ret = (char *)reallocate(ret, sizeof(char)*i);
        ret[i - 1] = c;
        c = getchar();
    }
    ret = (char *)reallocate(ret, sizeof(char) * (i + 1));
    ret[i] = '\0';
    return ret;
}

Literal getString(){
    char *s = readString();
    Literal ret = {LIT_STRING, 0, {0}};
    ret.sVal = s;
    return ret;
}

Literal getInt(){
    char *s = readString();
    long l = 0;
    sscanf(s, "%ld", &l);
    memfree(s);
    Literal lit = {LIT_INT, 0, {0}};
    lit.iVal = l;
    return lit;
}

Literal getFloat(){
    char *s = readString();
    double d = 0;
    sscanf(s, "%lf", &d);
    memfree(s);
    Literal lit = {LIT_DOUBLE, 0, {0}};
    lit.dVal = d;
    return lit;
}
