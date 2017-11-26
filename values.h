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
    ARR
} Datatype;

typedef struct{
    uint64_t container_key;
    uint64_t id;
    void *env;
} Instance;

typedef struct{
    int refCount;
    Datatype type;
    union{
        Instance *pvalue;
        struct{
            struct Data **arr;
            uint64_t numElements;
        };
        double cvalue;
        int64_t ivalue;
        uint64_t svalue;
    };
} Data;

#define isint(x) (x->type == INT)
#define isfloat(x) (x->type == FLOAT)
#define isnum(x) (isint(x) || isfloat(x))
#define isstr(x) (x->type == STRING)
#define isins(x) (x->type == INSTANCE)
#define islogical(x) (x->type == LOGICAL)
#define isnull(x) (x->type == NIL)
#define isidentifer(x) (x->type == IDENTIFIER)
#define isarray(x) (x->type == ARR)

#define tint(x) (x->ivalue)
#define tfloat(x) (x->cvalue)
#define tnum(x) (isint(x)?tint(x):tfloat(x))
#define tstr(x) (str_get(x->svalue))
#define tins(x) ((Instance *)x->pvalue)
#define tenv(x) ((Environment *)x->env)

#define new_data() ((Data *)mallocate(sizeof(Data)))
Data* new_int(int64_t val);
Data* new_float(double val);
Data* new_str(const char *val);
Data* new_identifer(const char *val);
Data* new_ins(Instance *val);
Data* new_logical(uint8_t val);
Data* new_null();
Data* new_array(uint64_t size);

void data_free(Data *d);
