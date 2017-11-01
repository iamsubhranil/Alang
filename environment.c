#include <string.h>

#include "expr.h"
#include "display.h"
#include "allocator.h"
#include "environment.h"

typedef struct Environment{
    char* name;
    Literal value;
    struct Environment* next;
} Environment;

static Environment *head = NULL, *rear = NULL;

static void env_new(char* identifer, Literal value){
    Environment *env = (Environment *)mallocate(sizeof(Environment));
    env->name = identifer;
    env->value = value;
    env->next = NULL;
    if(head == NULL){
        head = rear = env;
    }
    else{
        rear->next = env;
        rear = env;
    }
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
    return env->value;
}
