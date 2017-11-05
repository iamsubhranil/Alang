#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "stmt.h"

void interpret(Code c);
void stop();

typedef struct{
    int count;
    Literal *values;
} Array;

typedef struct{
   char *name;
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

typedef struct{
    ObjectType type;
    union{
        Literal literal;
        Array arr;
        Routine routine;
        Container container;
        Instance instance;
    };
} Object;

#endif
