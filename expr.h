#ifndef EXPR_H
#define EXPR_H

#include "scanner.h"

typedef struct Expression Expression;

/*
typedef struct{
    Token name;
    Expression* value;
} Assign;
*/
typedef struct{
    Expression* left;
    Token op;
    Expression* right;
} Binary;

// Call

typedef enum{
    LIT_NUMERIC,
    LIT_STRING,
    LIT_LOGICAL,
    LIT_NULL
} LiteralType;

typedef struct{
    LiteralType type;
    union{
        int lVal;
        double dVal;
        char* sVal;
    };
} Literal;

typedef struct{
    Expression* left;
    Token op;
    Expression* right;
} Logical;

typedef struct{
    Token name;
} Variable;

typedef enum{
//    EXPR_ASSIGN,
    EXPR_BINARY,
    EXPR_LOGICAL,
    EXPR_LITERAL,
    EXPR_VARIABLE,
    EXPR_NONE
} ExpressionType;

typedef struct Expression{
    ExpressionType type;
    union{
//        Assign assignment;
        Binary binary;
        Logical logical;
        Literal literal;
        Variable variable;
    };
} Expression;

#endif
