#include <stdio.h>

#include "strings.h"
#include "allocator.h"
#include "display.h"

typedef struct{
    const char *value;
    uint32_t hash;
    uint32_t refCount;
    size_t length;
} String;

static String **strarray = NULL;
uint32_t stringCount = 0;
size_t c = 0;

static uint32_t hash(const char *str){
    uint32_t hash = 5381;
    c = 0;
//    printf("\nHashing..\n");
    while (str[c] != 0)
        hash = ((hash << 5) + hash) + str[c++]; /* hash * 33 + c */
//    printf("\nInput String : [%s] Hash : %lu\n", str, hash);
    return hash;
}

uint32_t str_insert(const char *str){
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
    strarray[stringCount - 1]->length = c;
//    printf(debug("[Strings] Adding [%s]"), str);
    return stringCount - 1;
}

void str_ref_incr(uint32_t index){
    strarray[index]->refCount++;
}

size_t str_len(uint32_t index){
    return strarray[index]->length;
}

void str_ref_decr(uint32_t index){
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

const char* str_get(uint32_t index){
    return strarray[index]->value;
}

void str_free(){
    uint64_t i = 0;
    while(i < stringCount){
        memfree(strarray[i++]);
    }
    memfree(strarray);
}
