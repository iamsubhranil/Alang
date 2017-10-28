#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "scanner.h"
#include "display.h"
#include "parser.h"
#include "stree.h"

static char *newChar(char c){
    char *ret = (char *)malloc(sizeof(char));
    ret[0] = c;
    return ret;
}

static char *getVal(Token token){
    if(token.type == TOKEN_NEWLINE)
        return newChar('\n');
    if(token.type == TOKEN_INDENT)
        return newChar('\t');
    if(token.type == TOKEN_EOF)
        return newChar('\0');

    char *ret = (char *)malloc(sizeof(char) * (token.length + 1));
    strncpy(ret, token.start, token.length);
    return ret;
}

static int currentIndentLevel = 0, inIf = 0, inWhile = 0, inDo = 0, inFor = 0;
static TokenList *head = NULL;
static Token errorToken = {TOKEN_ERROR,"BadToken",0,-1};

typedef enum{
    BLOCK_IF,
    BLOCK_ELSE,
    BLOCK_WHILE,
    BLOCK_DO,
    BLOCK_FOR,
    BLOCK_NONE,
    BLOCK_MAIN
} BlockType;

typedef struct Compiler{
    struct Compiler *parent;
    int indentLevel;
    BlockType blockName;
} Compiler;

static Compiler* initCompiler(Compiler *parent, int indentLevel, BlockType blockName){
    Compiler *ret = (Compiler *)malloc(sizeof(Compiler));
    ret->parent = parent;
    ret->indentLevel = indentLevel;
    ret->blockName = blockName;
    return ret;
}

static TokenType peek(){
    if(head == NULL)
        return TOKEN_EOF;
    return head->value.type;
}

static Token advance(){
    if(head->next != NULL){
        head = head->next;
        //printf("\n[Info:Advance] Advancing to [%s]", tokenNames[head->value.type]);
        return head->value;
    }
    return errorToken;
}

static Token present(){
    Token present = head->value;
    advance();
    return present;
}

static int match(TokenType type){
    if(head->value.type == type){
        advance();
        return 1;
    }
    return 0;
}

static void synchronize(){
    while(peek() != TOKEN_NEWLINE && peek() != TOKEN_EOF)
        advance();
    if(peek() == TOKEN_EOF)
        error("Unexpected end of input!");
    else
        head = head->next;
}

static Token consume(TokenType type, const char* err){
    if(head->value.type == type){
        //printf("\n[Info:Consume] Consuming [%s]", tokenNames[type]);
        return advance();
    }
    else{
        //printf("\n[Error:Consume] Expected [%s], Received [%s]!", tokenNames[type], tokenNames[head->value.type]);
        line_error(head->value.line, err);
        synchronize();
    }
    return errorToken;
}

static void consumeIndent(int indentLevel){
    while(indentLevel > 0){
        consume(TOKEN_INDENT, "Expected indent!");
        indentLevel--;
    }
}

static int getNextIndent(){
    TokenList *temp = head;
    int nextIndent = 0;
    while(temp->value.type == TOKEN_INDENT){
        temp = temp->next;
        nextIndent++;
    }
    return nextIndent;
}

static int isOperator(TokenType type){
    return type == TOKEN_EQUAL || type == TOKEN_EQUAL_EQUAL 
        || type == TOKEN_LESS_EQUAL || type == TOKEN_GREATER_EQUAL
        || type == TOKEN_GREATER || type == TOKEN_LESS
        || type == TOKEN_PLUS || type == TOKEN_MINUS
        || type == TOKEN_STAR || type == TOKEN_SLASH
        || type == TOKEN_PERCEN;
}

static Expression* expression(){
    debug("Parsing expression");

    Expression* expr = (Expression *)malloc(sizeof(Expression));
    expr->literal = errorToken;
    expr->op = errorToken;
    expr->right = NULL;

    if(peek() == TOKEN_IDENTIFIER || peek() == TOKEN_NUMBER){
        expr->literal = present();
        //consume(peek(), "Nope!");
        //info("Found literal!");
        //printf("\nLiteral : %s", getVal(consume(peek(), "Consuming literal!")));
        while(isOperator(peek())){
            expr->op = present();
            expr->right = expression();
        }
    }
    else if(peek() == TOKEN_LEFT_PAREN){
        present(); // Consume )
        free(expr);
        expr = expression();
        consume(TOKEN_RIGHT_PAREN, "Expected closing paren[')'] after expression!");
    }
    else{
        expr->literal = errorToken;
        line_error(head->value.line, "Bad Expression!");
        //synchronize();
    }
    debug("Expression parsed");
    return expr;
}

static Statement statement(Compiler *compiler);

static Block newBlock(){
    Block b = {0, NULL};
    return b;
}

static void addToBlock(Block* block, Statement statement){
    block->numStatements += 1;
    block->statements = (Statement *)realloc(block->statements, sizeof(Statement) * block->numStatements);
    (block->statements)[block->numStatements-1] = statement;
}

static Block blockStatement(Compiler *compiler, BlockType name){
    debug("Parsing block statement");
    int indent = compiler->indentLevel+1;
    Compiler *blockCompiler = initCompiler(compiler, indent, name);
    Block b = newBlock();
    while(getNextIndent() == indent){
        debug("Found a block statement");
        addToBlock(&b, statement(blockCompiler));
    }
    debug("Block statement parsed");
    return b;
}

static IfStatement* fromIf(IfStatement from){
    IfStatement* ret = (IfStatement *)malloc(sizeof(IfStatement));
    ret->ifBlock = from.ifBlock;
    ret->hasElseIf = from.hasElseIf;
    if(ret->hasElseIf == 1)
        ret->elseBlock = from.elseBlock;
    else if(ret->hasElseIf == 2)
        ret->elseIfBlock = from.elseIfBlock;
    return ret;
}

static Statement ifStatement(Compiler *compiler){
    debug("Parsing if statement");
    Statement s;
    s.type = STATEMENT_IF;
    s.ifStatement.elseIfBlock = NULL;
    s.ifStatement.hasElseIf = 0;

    consume(TOKEN_LEFT_PAREN, "Conditionals must start with an openning brace['('] after If!");
    s.ifStatement.ifBlock.condition = expression();
    consume(TOKEN_RIGHT_PAREN, "Conditional must end with a closing brace[')'] after If!");
    consume(TOKEN_NEWLINE, "Expected newline after If!");

    if(getNextIndent() == compiler->indentLevel){
        consumeIndent(compiler->indentLevel);
        consume(TOKEN_THEN, "Expected Then on the same indent!");
        consume(TOKEN_NEWLINE, "Expected newline after Then!");
    }

    s.ifStatement.ifBlock.statements = blockStatement(compiler, BLOCK_IF);
    consumeIndent(compiler->indentLevel);

    while(match(TOKEN_ELSE)){
        if(match(TOKEN_IF)){
            s.ifStatement.hasElseIf = 2;
            s.ifStatement.elseIfBlock = fromIf(ifStatement(compiler).ifStatement);
        }
        else{
            consume(TOKEN_NEWLINE, "Expected newline after else");
            s.ifStatement.hasElseIf = 1;
            s.ifStatement.elseBlock = blockStatement(compiler, BLOCK_ELSE);
            consumeIndent(compiler->indentLevel);
            break;
        }
    }

    if(s.ifStatement.hasElseIf != 2){
        consume(TOKEN_ENDIF, "Expected EndIf!");
        consume(TOKEN_NEWLINE, "Expected newline after EndIf!");
    }
    debug("If statement parsed");

    return s;
}

static Statement whileStatement(Compiler* compiler){
    debug("Parsing while statement");

    Statement s;
    s.type = STATEMENT_WHILE;

    consume(TOKEN_LEFT_PAREN, "Expected left paren before conditional!");
    s.conditionalStatement.condition = expression();
    consume(TOKEN_RIGHT_PAREN, "Expected right paren after conditional!");
    consume(TOKEN_NEWLINE, "Expected newline after While!");

    if(getNextIndent() == compiler->indentLevel){
        consumeIndent(compiler->indentLevel);
        consume(TOKEN_BEGIN, "Expected Begin on same indent!");
        consume(TOKEN_NEWLINE, "Expected newline after begin!");
    }

    s.conditionalStatement.statements = blockStatement(compiler, BLOCK_WHILE);
    consumeIndent(compiler->indentLevel);
    consume(TOKEN_ENDWHILE, "Expected EndWhile on same indent!");
    consume(TOKEN_NEWLINE, "Expected newline after EndWhile!");
    debug("While statement parsed");
    return s;
}

static Statement doStatement(Compiler* compiler){
    debug("Parsing do statement");
    Statement s;
    s.type = STATEMENT_DO;
    consume(TOKEN_NEWLINE, "Expected newline after Do!");
    if(getNextIndent() == compiler->indentLevel){
        consumeIndent(compiler->indentLevel);
        consume(TOKEN_BEGIN, "Expected Begin on the same indent level after Do!");
        consume(TOKEN_NEWLINE, "Expected newline after Begin!");
    }
    s.conditionalStatement.statements = blockStatement(compiler, BLOCK_DO);
    consumeIndent(compiler->indentLevel);
    if(match(TOKEN_ENDDO)){
        consume(TOKEN_NEWLINE, "Expected newline after EndDo!");
        consumeIndent(compiler->indentLevel);
    }
    consume(TOKEN_WHILE, "Expected While after Do!");
    consume(TOKEN_LEFT_PAREN, "Expected left paren before conditional!");
    s.conditionalStatement.condition = expression();
    consume(TOKEN_RIGHT_PAREN, "Expected right parent before conditional!");
    consume(TOKEN_NEWLINE, "Expected newline after While!");
    debug("Do statement parsed");
    return s;
}

static Statement breakStatement(Compiler *compiler){
    debug("Parsing break statement");
    Statement s;
    s.type = STATEMENT_BREAK;
    s.breakStatement.scopeDepth = 0;
    int i = 0;
    while(compiler != NULL && (compiler->blockName != BLOCK_WHILE || compiler->blockName != BLOCK_DO)){
        compiler = compiler->parent;
        i++;
    }
    if(compiler == NULL)
        line_error(peek(), "Break without While or Do!");
    s.breakStatement.scopeDepth = i;
    consume(TOKEN_NEWLINE, "Expected newline after Break!");
    debug("Break statement parsed");
    return s;
}

static Statement setStatement(){
    Statement s;
    s.type = STATEMENT_SET;
    debug("Parsing set statement");
    s.expressionStatement.expression = expression();
    consume(TOKEN_NEWLINE, "Expected newline after Set statement!");
    debug("Set statement parsed");
    return s;
}

static Statement printStatement(){
    Statement s;
    s.type = STATEMENT_PRINT;
    debug("Parsing print statement");
    if(peek() == TOKEN_STRING)
        s.printStatement.print = present();
    else
        s.printStatement.print = consume(TOKEN_IDENTIFIER, "Expected string or identifer after Print!");
    consume(TOKEN_NEWLINE, "Expected newline after Print!");
    debug("Print statement parsed");
    return s;
}

static Statement beginStatement(){
    Statement s;
    s.type = STATEMENT_BEGIN;
    debug("Parsing begin statement");
    consume(TOKEN_NEWLINE, "Expected newline after Begin!");
    debug("Begin statement parsed");
    return s;
}

static Statement endStatement(){
    Statement s;
    s.type = STATEMENT_END;
    debug("End statement parsed");
    consume(TOKEN_NEWLINE, "Expected newline after End!");
    return s;
}

static Statement statement(Compiler *compiler){
    consumeIndent(compiler->indentLevel);

    if(match(TOKEN_BEGIN))
        return beginStatement();
    else if(match(TOKEN_END))
        return endStatement();
    else if(match(TOKEN_IF))
        return ifStatement(compiler);
    else if(match(TOKEN_SET))
        return setStatement();
    else if(match(TOKEN_WHILE))
        return whileStatement(compiler);
    else if(match(TOKEN_DO))
        return doStatement(compiler);
    else if(match(TOKEN_PRINT))
        return printStatement();
    else if(match(TOKEN_BREAK))
        return breakStatement(compiler);
}

Block parse(TokenList *list){
    head = list;
    Block mainBlock = newBlock();
    Compiler* root = initCompiler(NULL, 0, BLOCK_MAIN);
    while(!match(TOKEN_EOF))
        addToBlock(&mainBlock, statement(root));

    return mainBlock;
}
