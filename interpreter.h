#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "stmt.h"

void interpret(Code c);
void stop();

typedef struct Object Object;

typedef struct{
    int count;
    Object *values;
} Array;

typedef struct{
    int refCount;
    char *name;
    int fromReturn;
    int insCount;
    void *environment;
} Instance;

typedef enum{
    OBJECT_NULL,
    OBJECT_LITERAL,
    OBJECT_ARRAY,
    OBJECT_ROUTINE,
    OBJECT_CONTAINER,
    OBJECT_INSTANCE
} ObjectType;

struct Object{
    ObjectType type;
    union{
        Literal literal;
        Array arr;
        Routine routine;
        Container container;
        Instance* instance;
    };
};

static Literal nullLiteral = {0, LIT_NULL, {0}};
static Object nullObject = {OBJECT_NULL, {{0, LIT_NULL, {0}}}};

#endif
