#include <string.h>

#include "expr.h"
#include "display.h"
#include "allocator.h"
#include "environment.h"

typedef struct{
    int count;
    Literal *values;
} Array;

typedef enum{
    LITERAL,
    ARRAY
} EnvType;

typedef struct Environment{
    char* name;
    EnvType type;
    union{
        Literal value;
        Array arr;
    };
    struct Environment* next;
} Environment;

static Environment *head = NULL, *rear = NULL;

static void insert(Environment *env){ 
    if(head == NULL){
        head = rear = env;
    }
    else{
        rear->next = env;
        rear = env;
    }
}

static void env_new(char* identifer, Literal value){
    Environment *env = (Environment *)mallocate(sizeof(Environment));
    env->name = identifer;
    env->value = value;
    env->type = LITERAL;
    env->next = NULL;
    insert(env);
}

static Environment* env_match(char* identifer){
    Environment *bak = head;
    while(bak != NULL){
        if(strcmp(bak->name, identifer) == 0)
            return bak;
        bak = bak->next;
    }
    return NULL;
}

void env_put(char* identifer, Literal value){
    Environment *get = env_match(identifer);
    if(get == NULL)
        env_new(identifer, value);
    else
        get->value = value;
}

Literal env_get(char *identifer){
    Environment *env = env_match(identifer);
    if(env == NULL)
        error("Undefined variable!");
    else if(env->type == ARRAY)
        error("An array must be accessed using index!");
    return env->value;
}

void env_arr_new(char *identifer, long numElements){
    if(env_match(identifer) != NULL)
        error("Variable is already defined!");
    Environment *env = (Environment *)mallocate(sizeof(Environment));
    env->name = identifer;
    env->type = ARRAY;
    env->next = NULL;
    env->arr.count = numElements;
    env->arr.values = (Literal *)mallocate(sizeof(Literal) * numElements);
    long i = 0;
    while(i < numElements){
        env->arr.values[i].type = LIT_INT;
        env->arr.values[i].iVal = 0;
        i++;
    }
    insert(env);
}

void env_arr_put(char *identifer, long index, Literal value){
    Environment *env = env_match(identifer);
    if(env == NULL)
        runtime_error(value.line, "Undefined array!");
    else if(env->type != ARRAY)
        runtime_error(value.line, "Accessed variable is not an array!");
    else if(index < 1 || env->arr.count < index)
        runtime_error(value.line, "Array index out of range!");

    env->arr.values[index - 1] = value;
}

Literal env_arr_get(char *identifer, long index){ 
    Environment *env = env_match(identifer);
    if(env == NULL)
        error("Undefined array!");
    else if(env->type != ARRAY)
        error("Accessed variable is not an array!");
    else if(index < 1 || env->arr.count < index)
        error("Array index out of range!");

    return env->arr.values[index - 1];
}
