#pragma once

#include <stdint.h>

#include "strings.h"
#include "allocator.h"

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
    uint32_t container_key;
    uint32_t id;
    uint32_t refCount;
    void *env;
} Instance;

typedef struct Dat Data;

typedef struct{
    int32_t numElements;
    Data *data;
} Array;

struct Dat{
    Datatype type;
    //void *env;
    union{
        Array *arr;
        Instance *pvalue;
        double cvalue;
        int32_t ivalue;
        uint32_t svalue;
    };
};

#define MAX_FREE_INSTANCES 512

extern Instance *freeInstances[MAX_FREE_INSTANCES];
extern uint16_t freeInstancePointer;

#define ins_new() freeInstancePointer==0?(Instance *)mallocate(sizeof(Instance)):freeInstances[--freeInstancePointer]
#define ins_free(x) {if(freeInstancePointer < MAX_FREE_INSTANCES) \
                        freeInstances[freeInstancePointer++] = x; \
                    else \
                         memfree(x);\
                    }

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

#define ttype(x) (x.type)
#define tint(x) (x.ivalue)
#define tfloat(x) (x.cvalue)
#define tnum(x) (isint(x)?tint(x):tfloat(x))
#define tstr(x) (str_get(x.svalue))
#define tstrk(x) (x.svalue)
#define tins(x) (x.pvalue)
#define tenv(x) ((Environment *)x.pvalue->env)
#define tarr(x) (x.arr)
#define arr_size(x) (x->numElements)
#define arr_elements(x) (x->data)

#define new_data() ((Data *)mallocate(sizeof(Data)))
#define new_int(x) ((Data){.type = INT, {.ivalue = x}})
#define new_float(x) ((Data){.type = FLOAT, {.cvalue = x}})
#define new_str(x) ((Data){.type = STRING, {.svalue = str_insert(x)}})
#define new_strk(x) ((Data){.type = STRING, {.svalue = x}})
#define new_identifer(x) ((Data){.type = IDENTIFIER, {.svalue = str_insert(x)}})
#define new_identiferk(x) ((Data){.type = IDENTIFIER, {.svalue = x}})
#define new_logical(x) ((Data){.type = LOGICAL, {.ivalue = x}})
#define new_null() ((Data){.type = NIL})
#define new_none() ((Data){.type = NONE})

Data new_array(uint32_t size);
Data new_ins(void *env, uint32_t name);

//static inline void data_free(Data d);
