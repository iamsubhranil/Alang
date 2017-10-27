#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "scanner.h"
#include "display.h"
#include "generator.h"

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
        || type == TOKEN_STAR || type == TOKEN_SLASH;
}

static void expression(){
    debug("Parsing expression");
    if(peek() == TOKEN_IDENTIFIER || peek() == TOKEN_NUMBER){
        consume(peek(), "Consuming literal!");
        //info("Found literal!");
        //printf("\nLiteral : %s", getVal(consume(peek(), "Consuming literal!")));
        while(isOperator(peek())){
            consume(peek(), "Expected operator!");
            expression();
        }
    }
    else if(peek() == TOKEN_LEFT_PAREN){
        consume(TOKEN_LEFT_PAREN, "Consuming left paren..");
        expression();
        consume(TOKEN_RIGHT_PAREN, "Expected closing paren[')'] after expression!");
    }
    else{
        line_error(head->value.line, "Bad Expression!");
        //synchronize();
    }
    debug("Expression parsed");
}

static void statement(Compiler *compiler);

static void blockStatement(Compiler *compiler, BlockType name){
    debug("Parsing block statement");
    int indent = compiler->indentLevel+1;
    Compiler *blockCompiler = initCompiler(compiler, indent, name);
    while(getNextIndent() == indent){
        debug("Found a block statement");
        statement(blockCompiler);
    }
    debug("Block statement parsed");
}

static void ifStatement(Compiler *compiler){
    debug("Parsing if statement");
    consume(TOKEN_LEFT_PAREN, "Conditionals must start with an openning brace['('] after If!");
    expression();
    consume(TOKEN_RIGHT_PAREN, "Conditional must end with a closing brace[')'] after If!");
    consume(TOKEN_NEWLINE, "Expected newline after If!");
    if(getNextIndent() == compiler->indentLevel){
        consumeIndent(compiler->indentLevel);
        consume(TOKEN_THEN, "Expected Then on the same indent!");
        consume(TOKEN_NEWLINE, "Expected newline after Then!");
    }
    blockStatement(compiler, BLOCK_IF);
    consumeIndent(compiler->indentLevel);
    while(match(TOKEN_ELSE)){
        if(match(TOKEN_IF))
            ifStatement(compiler);
        else{
            consume(TOKEN_NEWLINE, "Expected newline after else");
            blockStatement(compiler, BLOCK_ELSE);
            consumeIndent(compiler->indentLevel);
            break;
        }
    }
    consume(TOKEN_ENDIF, "Expected EndIf!");
    consume(TOKEN_NEWLINE, "Expected newline after EndIf!");
    debug("If statement parsed");
}

static void whileStatement(Compiler* compiler){
    debug("Parsing while statement");
    consume(TOKEN_LEFT_PAREN, "Expected left paren before conditional!");
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expected right paren after conditional!");
    consume(TOKEN_NEWLINE, "Expected newline after While!");
    if(getNextIndent() == compiler->indentLevel){
        consumeIndent(compiler->indentLevel);
        consume(TOKEN_BEGIN, "Expected Begin on same indent!");
        consume(TOKEN_NEWLINE, "Expected newline after begin!");
    }
    blockStatement(compiler, BLOCK_WHILE);
    consumeIndent(compiler->indentLevel);
    consume(TOKEN_ENDWHILE, "Expected EndWhile on same indent!");
    consume(TOKEN_NEWLINE, "Expected newline after EndWhile!");
    debug("While statement parsed");
}

static void doStatement(Compiler* compiler){
    debug("Parsing do statement");
    consume(TOKEN_NEWLINE, "Expected newline after Do!");
    if(getNextIndent() == compiler->indentLevel){
        consumeIndent(compiler->indentLevel);
        consume(TOKEN_BEGIN, "Expected Begin on the same indent level after Do!");
        consume(TOKEN_NEWLINE, "Expected newline after Begin!");
    }
    blockStatement(compiler, BLOCK_DO);
    consumeIndent(compiler->indentLevel);
    if(match(TOKEN_ENDDO)){
        consume(TOKEN_NEWLINE, "Expected newline after EndDo!");
        consumeIndent(compiler->indentLevel);
    }
    consume(TOKEN_WHILE, "Expected While after Do!");
    consume(TOKEN_LEFT_PAREN, "Expected left paren before conditional!");
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expected right parent before conditional!");
    consume(TOKEN_NEWLINE, "Expected newline after While!");
    debug("Do statement parsed");
}

static void breakStatement(Compiler *compiler){
    debug("Parsing break statement");
    consume(TOKEN_NEWLINE, "Expected newline after Break!");
    debug("Break statement parsed");
}

static void setStatement(){
    debug("Parsing set statement");
    expression();
    consume(TOKEN_NEWLINE, "Expected newline after Set statement!");
    debug("Set statement parsed");
}

static void printStatement(){
    debug("Parsing print statement");
    while(match(TOKEN_STRING) || match(TOKEN_IDENTIFIER));
    consume(TOKEN_NEWLINE, "Expected newline after Print!");
    debug("Print statement parsed");
}

static void beginStatement(){
    debug("Parsing begin statement");
    consume(TOKEN_NEWLINE, "Expected newline after Begin!");
    debug("Begin statement parsed");
}

static void endStatement(){
    debug("End statement parsed");
    //consume(TOKEN_NEWLINE, "Expected newline after End!");
}

static void statement(Compiler *compiler){
    consumeIndent(compiler->indentLevel);

    if(match(TOKEN_BEGIN))
        beginStatement();
    else if(match(TOKEN_END))
        endStatement();
    else if(match(TOKEN_IF))
        ifStatement(compiler);
    else if(match(TOKEN_SET))
        setStatement();
    else if(match(TOKEN_WHILE))
        whileStatement(compiler);
    else if(match(TOKEN_DO))
        doStatement(compiler);
    else if(match(TOKEN_PRINT))
        printStatement();
    else if(match(TOKEN_BREAK))
        breakStatement(compiler);
}

void generate(TokenList *list){
    head = list;
    Compiler* root = initCompiler(NULL, 0, BLOCK_MAIN);
    while(!match(TOKEN_EOF))
        statement(root);
}
