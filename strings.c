#include <stdio.h>

#include "strings.h"
#include "allocator.h"
#include "display.h"

typedef struct{
    const char *value;
    uint64_t hash;
    uint64_t refCount;
} String;

static String **strarray = NULL;
uint64_t stringCount = 0;

static uint64_t hash(const char *str){
    uint64_t hash = 5381;
    int c = 0;
//    printf("\nHashing..\n");
    while (str[c] != 0)
        hash = ((hash << 5) + hash) + str[c++]; /* hash * 33 + c */
//    printf("\nInput String : [%s] Hash : %lu\n", str, hash);
    return hash;
}

uint64_t str_insert(const char *str){
    uint64_t has = hash(str);
    uint64_t i = 0;
    while(i < stringCount){
        if(strarray[i]->hash == has){
            strarray[i]->refCount++;
//            printf(debug("[Strings] Match found for [%s] at %lu[%s]"), str, i, strarray[i]->value);
            return i;
        }
        i++;
    }
    strarray = (String **)reallocate(strarray, sizeof(String *) * ++stringCount);
    strarray[stringCount - 1] = (String *)reallocate(NULL, sizeof(String));
    strarray[stringCount - 1]->hash = has;
    strarray[stringCount - 1]->value = str;
    strarray[stringCount - 1]->refCount = 1;
//    printf(debug("[Strings] Adding [%s]"), str);
    return stringCount - 1;
}

void str_ref_incr(uint64_t index){
    strarray[index]->refCount++;
}

void str_ref_decr(uint64_t index){
    strarray[index]->refCount--;
    if(strarray[index]->refCount == 0){
        String *s = strarray[index];
        strarray[index] = strarray[stringCount - 1];
        strarray = (String **)reallocate(strarray, sizeof(String *) * --stringCount);
//        printf(debug("[Strings] Freeing [%s]"), s->value);
        memfree((void *)s->value);
        memfree(s);
    }
}

const char* str_get(uint64_t index){
    return strarray[index]->value;
}
