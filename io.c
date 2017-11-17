#include <stdio.h>

#include "allocator.h"
#include "expr.h"
#include "io.h"
#include "display.h"

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

Literal getString(int line){
    char *s = readString();
    Literal ret = {line, LIT_STRING, {0}};
    ret.sVal = s;
    return ret;
}

static int isNum(char c){
    return c>='0' && c<='9';
}

static int isSign(char c){
    return c == '-'|| c == '+';
}

static int isInt(char *s){
    int i = 0;
    if(isSign(s[0]))
        i++;
    while(s[i] != '\0'){
        if(!isNum(s[i]))
            return 0;
        i++;
    }
    if(isSign(s[0]) && i == 1)
        return 0;
    return 1;
}

Literal getInt(int line){
    char *s = readString();
    while(!isInt(s)){
        printf(warning("[Input Error] Not an integer : %s!\n[Re-Input] "), s);
        s = readString();
    }
    long l = 0;
    sscanf(s, "%ld", &l);
    memfree(s);
    if(s[0] == '-')
        l *= -1;
    Literal lit = {line, LIT_INT, {0}};
    lit.iVal = l;
    return lit;
}

static int isNumber(char *s){
    int i = 0, hasDot = 0;
    if(isSign(s[0]))
        i++;
    while(s[i] != '\0'){
        if(s[i] == '.'){
            if(hasDot == 0)
                hasDot = 1;
            else
                return 0;
        }
        else if(!isNum(s[i]))
            return 0;
        i++;
    }
    if(isSign(s[0]) && i == 1)
        return 0;
    return 1;
}

Literal getFloat(int line){
    char *s = readString();
    while(!isNumber(s)){
        printf(warning("[Input Error] Not a number : %s!\n[Re-Input] "), s);
        s = readString();
    }
    double d = 0;
    sscanf(s, "%lf", &d);
    memfree(s);
    if(s[0] == '-')
        d *= -1;
    Literal lit = {line, LIT_DOUBLE, {0}};
    lit.dVal = d;
    return lit;
}
