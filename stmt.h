#ifndef STMT_H
#define STMT_H

#include "expr.h"

typedef enum{
    STATEMENT_SET,
    STATEMENT_IF,
    STATEMENT_WHILE,
    STATEMENT_BREAK,
    STATEMENT_PRINT,
//    STATEMENT_DO,
    STATEMENT_BEGIN,
    STATEMENT_END
} StatementType;

typedef struct Statement Statement;

typedef enum{
    BLOCK_IF,
    BLOCK_ELSE,
    BLOCK_WHILE,
    BLOCK_DO,
    BLOCK_FOR,
    BLOCK_NONE,
    BLOCK_MAIN
} BlockType;

typedef struct{
    int numStatements;
    BlockType blockName;
    Statement* statements;
} Block;

typedef struct{
    Token pos;
} Break;

/*typedef struct{
    Expression* expression;
} ExpressionStatement;
*/
// Function

typedef struct{
    int line;
    Expression* condition;
    Block thenBranch;
    Block elseBranch;
} If;

typedef struct{
    int line;
    int argCount;
    Expression** expressions;
} Print;

typedef struct{
    char *identifer;
    Expression *initializerExpression;
} Initializer;

typedef struct{
    int line;
    int count;
    Initializer *initializers;
} Set;

typedef struct{
    int line;
    Expression* condition;
    Block body;
} While;

struct Statement{
    StatementType type;
    union{
        Block blockStatement;
        Break breakStatement;
//        ExpressionStatement expressionStatement;
        If ifStatement;
        Print printStatement;
        Set setStatement;
        While whileStatement;
    };
};

#endif
