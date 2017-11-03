#ifndef STMT_H
#define STMT_H

#include "expr.h"

typedef enum{
    STATEMENT_SET,
    STATEMENT_ARRAY,
    STATEMENT_INPUT,
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
    Expression *identifer;
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

typedef struct{
    int line;
    int count;
    Expression** initializers;
} ArrayInit;

typedef enum{
    INPUT_ANY,
    INPUT_INT,
    INPUT_FLOAT
} InputDataType;

typedef enum{
    INPUT_PROMPT,
    INPUT_IDENTIFER
} InputType;

typedef struct{ 
    InputType type;
    union{
        char *prompt;
        struct{
            InputDataType datatype;
            char *identifer;
        };
    };
} Input;

typedef struct{
    int line;
    int count;
    Input *inputs;
} InputStatement;

struct Statement{
    StatementType type;
    union{
        Block blockStatement;
        Break breakStatement;
//        ExpressionStatement expressionStatement;
        If ifStatement;
        Print printStatement;
        Set setStatement;
        ArrayInit arrayStatement;
        InputStatement inputStatement;
        While whileStatement;
    };
};

#endif
