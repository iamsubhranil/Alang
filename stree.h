#ifndef STREE_H
#define STREE_H

#include "scanner.h"

typedef struct Expression{
    Token literal;
    Token op;
    struct Expression* right;
} Expression;

typedef enum{
    STATEMENT_SET,
    STATEMENT_IF,
    STATEMENT_WHILE,
    STATEMENT_BREAK,
    STATEMENT_PRINT,
    STATEMENT_DO,
    STATEMENT_BEGIN,
    STATEMENT_END
} StatementType;

typedef struct Statement Statement;

typedef struct{
    int numStatements;
    Statement* statements;
} Block;

typedef struct{
    Expression* condition;
    Block statements;
} Conditional; // If, Do, While

typedef struct IfStatement{
    Conditional ifBlock;
    unsigned char hasElseIf; // 0 -> no else, no elseif : 1 -> has else, no elseif : 2 -> no else, has elseif
    union{
        struct IfStatement* elseIfBlock;
        Block elseBlock;
    };
} IfStatement;

typedef struct{
    int scopeDepth;
} BreakStatement; // Break

typedef struct{
    Expression* expression;
} ExpressionStatement; // Begin, End, Set

typedef struct{
    Token print;
} PrintStatement;

struct Statement{
    StatementType type;
    union{
        Conditional conditionalStatement;
        IfStatement ifStatement;
        BreakStatement breakStatement;
        ExpressionStatement expressionStatement;
        PrintStatement printStatement;
    };
};

#endif
