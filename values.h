#pragma once

#include <stdint.h>

#include "strings.h"

typedef enum{
    INT,
    FLOAT,
    STRING,
    LOGICAL,
    NIL,
    INSTANCE,
    IDENTIFIER,
    ARR,
    NONE
} Datatype;

typedef struct{
    uint64_t container_key;
    uint64_t id;
    uint64_t refCount;
    void *env;
} Instance;

typedef struct Data{
    int refCount;
    Datatype type;
    union{
        Instance *pvalue;
        struct{
            struct Data *arr;
            uint64_t numElements;
        };
        double cvalue;
        int64_t ivalue;
        uint64_t svalue;
    };
} Data;

#define isint(x) (x.type == INT)
#define isfloat(x) (x.type == FLOAT)
#define isnum(x) (isint(x) || isfloat(x))
#define isstr(x) (x.type == STRING)
#define isins(x) (x.type == INSTANCE)
#define islogical(x) (x.type == LOGICAL)
#define isnull(x) (x.type == NIL)
#define isidentifer(x) (x.type == IDENTIFIER)
#define isarray(x) (x.type == ARR)
#define isnone(x) (x.type == NONE)

#define tint(x) (x.ivalue)
#define tfloat(x) (x.cvalue)
#define tnum(x) (isint(x)?tint(x):tfloat(x))
#define tstr(x) (str_get(x.svalue))
#define tstrk(x) (x.svalue)
#define tins(x) (x.pvalue)
#define tenv(x) ((Environment *)x.pvalue->env)

#define new_data() ((Data *)mallocate(sizeof(Data)))
#define new_int(x) ((Data){0, INT, {.ivalue = x}})
#define new_float(x) ((Data){0, FLOAT, {.cvalue = x}})
#define new_str(x) ((Data){0, STRING, {.svalue = str_insert(x)}})
#define new_strk(x) ((Data){0, STRING, {.svalue = x}})
#define new_identifer(x) ((Data){0, IDENTIFIER, {.svalue = str_insert(x)}})
#define new_identiferk(x) ((Data){0, IDENTIFIER, {.svalue = x}})
#define new_logical(x) ((Data){0, LOGICAL, {.ivalue = x}})
#define new_null() ((Data){0, NIL, {NULL}})
#define new_none() ((Data){0, NONE, {NULL}})
Data new_array(uint64_t size);
Data new_ins(void *env, uint64_t name);
void data_free(Data d);
