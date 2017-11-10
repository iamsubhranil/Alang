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

typedef enum{
    LIT_DOUBLE,
    LIT_INT,
    LIT_STRING,
    LIT_LOGICAL,
    LIT_NULL
} LiteralType;

static const char *literalNames[] = {
    "LIT_DOUBLE",
    "LIT_INT",
    "LIT_STRING",
    "LIT_LOGICAL",
    "LIT_NULL"
};

typedef struct{
    int line;
    LiteralType type;
    union{
        int lVal;
        long iVal;
        double dVal;
        char *sVal;
    };
} Literal;

typedef struct{
    int line;
    Expression* left;
    Token op;
    Expression* right;
} Binary;

// Call
typedef struct{
    int line;
    char *identifer;
    int argCount;
    Expression **arguments;
} Call;

typedef struct{
    int line;
    Expression *index;
    char *identifier;
} ArrayExpression;

typedef struct{
    int line;
    Expression* left;
    Token op;
    Expression* right;
} Logical;

typedef struct{
    int line;
    char* name;
} Variable;

typedef struct{
    int line;
    Expression *containerName;
    Expression *member;
} Reference;

typedef enum{
//    EXPR_ASSIGN,
    EXPR_BINARY,
    EXPR_LOGICAL,
    EXPR_LITERAL,
    EXPR_VARIABLE,
    EXPR_ARRAY,
    EXPR_CALL,
    EXPR_REFERENCE,
    EXPR_NONE
} ExpressionType;

static const char *expressionNames[] = {
//    EXPR_ASSIGN,
    "EXPR_BINARY",
    "EXPR_LOGICAL",
    "EXPR_LITERAL",
    "EXPR_VARIABLE",
    "EXPR_ARRAY",
    "EXPR_CALL",
    "EXPR_REFERENCE",
    "EXPR_NONE"
};

typedef struct Expression{
    ExpressionType type;
    union{
//        Assign assignment;
        Binary binary;
        Logical logical;
        Literal literal;
        ArrayExpression arrayExpression;
        Variable variable;
        Call callExpression;
        Reference referenceExpression;
    };
} Expression;

#endif
