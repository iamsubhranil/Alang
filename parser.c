#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "scanner.h"
#include "display.h"
#include "parser.h"
#include "expr.h"
#include "stmt.h"
#include "allocator.c"

static int currentIndentLevel = 0, inIf = 0, inWhile = 0, inDo = 0, inFor = 0;
static TokenList *head = NULL;
static Token errorToken = {TOKEN_ERROR,"BadToken",0,-1};

typedef struct Compiler{
    struct Compiler *parent;
    int indentLevel;
    BlockType blockName;
} Compiler;

static Compiler* initCompiler(Compiler *parent, int indentLevel, BlockType blockName){
    Compiler *ret = (Compiler *)mallocate(sizeof(Compiler));
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
        printf("\n[Error:Consume] Expected [%s], Received [%s]!", tokenNames[type], tokenNames[head->value.type]);
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

static Expression* newExpression(){
    Expression* expr = (Expression *)mallocate(sizeof(Expression));
    expr->type = EXPR_NONE;
    return expr;
}

static Expression* expression();

static char* stringOf(Token t){
    char* s = (char *)mallocate(sizeof(char) * t.length);
    strncpy(s, t.start, t.length);
    return s;
}

static double doubleOf(Token t){
    char *s = stringOf(t);
    double d;
    sscanf(s, "%lf", &d);
    return d;
}

static Expression* primary(){
    Expression* expr = newExpression();
    if(match(TOKEN_TRUE)){
        expr->type = EXPR_LITERAL;
        expr->literal.type = LIT_LOGICAL;
        expr->literal.lVal = 1;
    }
    else if(match(TOKEN_FALSE)){
        expr->type = EXPR_LITERAL;
        expr->literal.type = LIT_LOGICAL;
        expr->literal.lVal = 0;
    }
    else if(match(TOKEN_NULL)){
        expr->type = EXPR_LITERAL;
        expr->literal.type = LIT_NULL;
    }
    else if(peek() == TOKEN_NUMBER){
        expr->type = EXPR_LITERAL;
        expr->literal.type = LIT_NUMERIC;
        expr->literal.dVal = doubleOf(present());
    }
    else if(peek() == TOKEN_STRING){
        expr->type = EXPR_LITERAL;
        expr->literal.type = LIT_STRING;
        expr->literal.sVal = stringOf(present());
    }
    else if(peek() == TOKEN_IDENTIFIER){
        expr->type = EXPR_VARIABLE;
        expr->variable.name = present();
    }
    else if(match(TOKEN_LEFT_PAREN)){
        //free(expr);
        expr = expression();
        consume(TOKEN_RIGHT_PAREN, "Expected '(' after expression.");
    }
    else{
        Token t = present();
        line_error(t.line, "Incorrect expression!");
        printf("\n[Token] %s ", tokenNames[t.type]);
    }
    return expr;
}

static Expression* tothepower(){
    Expression* expr = primary();
    while(peek() == TOKEN_CARET){
        Expression* multi = newExpression();
        multi->type = EXPR_BINARY;
        multi->logical.op = present();
        multi->binary.left = expr;
        multi->binary.right = primary();
        expr = multi;
    }
    return expr;
}

static Expression* multiplication(){
    Expression* expr = tothepower();
    while(peek() == TOKEN_STAR || peek() == TOKEN_SLASH){
        Expression* multi = newExpression();
        multi->type = EXPR_BINARY;
        multi->logical.op = present();
        multi->binary.left = expr;
        multi->binary.right = tothepower();
        expr = multi;
    }
    return expr;
}

static Expression* addition(){
    Expression* expr = multiplication();
    while(peek() == TOKEN_PLUS || peek() == TOKEN_MINUS
            || peek() == TOKEN_PERCEN){
        Expression* multi = newExpression();
        multi->type = EXPR_BINARY;
        multi->logical.op = present();
        multi->binary.left = expr;
        multi->binary.right = multiplication();
        expr = multi;
    }
    return expr;
}

static Expression* comparison(){
    Expression* expr = addition();
    while(peek() == TOKEN_GREATER || peek() == TOKEN_LESS
            || peek() == TOKEN_GREATER_EQUAL || peek() == TOKEN_LESS_EQUAL){
        Expression* multi = newExpression();
        multi->type = EXPR_BINARY;
        multi->logical.op = present();
        multi->binary.left = expr;
        multi->binary.right = addition();
        expr = multi;
    }
    return expr;
}

static Expression* equality(){
    Expression* expr = comparison();
    while(peek() == TOKEN_EQUAL_EQUAL || peek() == TOKEN_BANG_EQUAL){
        Expression* multi = newExpression();
        multi->type = EXPR_BINARY;
        multi->logical.op = present();
        multi->binary.left = expr;
        multi->binary.right = comparison();
        expr = multi;
    }
    return expr;
}

static Expression* andE(){
    Expression* expr = equality();
    while(peek() == TOKEN_AND){
        Expression* logic = newExpression();
        logic->type = EXPR_LOGICAL;
        logic->logical.op = present();
        logic->logical.left = expr;
        logic->logical.right = equality();
        expr = logic;
    }
    return expr;
}

static Expression* orE(){
    Expression* expr = andE();
    while(peek() == TOKEN_AND){
        Expression* logic = newExpression();
        logic->type = EXPR_LOGICAL;
        logic->logical.op = present();
        logic->logical.left = expr;
        logic->logical.right = andE();
        expr = logic;
    }
    return expr;
}

/*
   static Expression* assignment(){
   Expression* expr = orE();

   if(peek() == TOKEN_EQUAL){
   Token t = present();
   Expression* value = assignment();
   if(expr->type == EXPR_VARIABLE){
   Expression* ex = newExpression();
   ex->type = EXPR_ASSIGN;
   ex->assignment.name = expr->variable.name;
   ex->assignment.value = value;
   }
   else
   line_error(t.line, "Invalid assignment target!");
   }

   return expr;
   }
   */
static Expression* expression(){
    return orE();
}

static Statement statement(Compiler *compiler);

static Block newBlock(){
    Block b = {0, BLOCK_NONE, NULL};
    return b;
}

static void addToBlock(Block* block, Statement statement){
    block->numStatements += 1;
    block->statements = (Statement *)reallocate(block->statements, sizeof(Statement) * block->numStatements);
    (block->statements)[block->numStatements-1] = statement;
}

static Block blockStatement(Compiler *compiler, BlockType name){
    debug("Parsing block statement");
    int indent = compiler->indentLevel+1;
    Compiler *blockCompiler = initCompiler(compiler, indent, name);
    Block b = newBlock();
    b.blockName = name;
    while(getNextIndent() == indent){
        debug("Found a block statement");
        addToBlock(&b, statement(blockCompiler));
    }
    free(blockCompiler);
    debug("Block statement parsed");
    return b;
}

static Statement ifStatement(Compiler *compiler){
    debug("Parsing if statement");
    Statement s;
    s.type = STATEMENT_IF;
    s.ifStatement.thenBranch.numStatements = 0;
    s.ifStatement.elseBranch.numStatements = 0;

    consume(TOKEN_LEFT_PAREN, "Conditionals must start with an openning brace['('] after If!");
    s.ifStatement.condition = expression();
    consume(TOKEN_RIGHT_PAREN, "Conditional must end with a closing brace[')'] after If!");
    consume(TOKEN_NEWLINE, "Expected newline after If!");

    if(getNextIndent() == compiler->indentLevel){
        consumeIndent(compiler->indentLevel);
        consume(TOKEN_THEN, "Expected Then on the same indent!");
        consume(TOKEN_NEWLINE, "Expected newline after Then!");
    }

    s.ifStatement.thenBranch = blockStatement(compiler, BLOCK_IF);
    consumeIndent(compiler->indentLevel);

    if(match(TOKEN_ELSE)){
        if(match(TOKEN_IF)){
            Block elseifBlock = {0, BLOCK_ELSE, NULL};
            addToBlock(&elseifBlock, ifStatement(compiler));
            s.ifStatement.elseBranch = elseifBlock;
        }
        else{
            consume(TOKEN_NEWLINE, "Expected newline after Else!");
            s.ifStatement.elseBranch = blockStatement(compiler, BLOCK_ELSE);
            consumeIndent(compiler->indentLevel);
            consume(TOKEN_ENDIF, "Expected EndIf after If!");
            consume(TOKEN_NEWLINE, "Expected newline after EndIf!");
        }
    }
    else{
        consume(TOKEN_ENDIF, "Expected EndIf after If!");
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
    s.whileStatement.condition = expression();
    consume(TOKEN_RIGHT_PAREN, "Expected right paren after conditional!");
    consume(TOKEN_NEWLINE, "Expected newline after While!");

    if(getNextIndent() == compiler->indentLevel){
        consumeIndent(compiler->indentLevel);
        consume(TOKEN_BEGIN, "Expected Begin on same indent!");
        consume(TOKEN_NEWLINE, "Expected newline after begin!");
    }

    s.whileStatement.body = blockStatement(compiler, BLOCK_WHILE);
    consumeIndent(compiler->indentLevel);
    consume(TOKEN_ENDWHILE, "Expected EndWhile after while on same indent!");
    consume(TOKEN_NEWLINE, "Expected newline after EndWhile!");
    debug("While statement parsed");
    return s;
}

// Not used now
/*static Statement doStatement(Compiler* compiler){
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
  }*/

static Statement breakStatement(Compiler *compiler){
    debug("Parsing break statement");
    Statement s;
    s.type = STATEMENT_BREAK;
    s.breakStatement.pos = head->value;
    int i = 0;
    while(compiler != NULL && (compiler->blockName != BLOCK_WHILE || compiler->blockName != BLOCK_DO)){
        compiler = compiler->parent;
        i++;
    }
    if(compiler == NULL)
        line_error(peek(), "Break without While or Do!");
    consume(TOKEN_NEWLINE, "Expected newline after Break!");
    debug("Break statement parsed");
    return s;
}

static Statement setStatement(){
    Statement s;
    s.type = STATEMENT_SET;
    debug("Parsing set statement");
    s.setStatement.name = consume(TOKEN_IDENTIFIER, "Expected identifer after Set!");
    consume(TOKEN_EQUAL, "Expected equals after identifer in Set!");
    s.setStatement.initializer = expression();
    consume(TOKEN_NEWLINE, "Expected newline after Set statement!");
    debug("Set statement parsed");
    return s;
}

static Statement printStatement(){
    Statement s;
    s.type = STATEMENT_PRINT;
    debug("Parsing print statement");
    s.printStatement.argCount = 1;
    s.printStatement.expressions = expression();
    // while(match(TOKEN_COMMA)){
    //     
    // }
    // if(peek() == TOKEN_STRING)
    //     s.printStatement.print = present();
    // else
    //     s.printStatement.print = consume(TOKEN_IDENTIFIER, "Expected string or identifer after Print!");
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
    // else if(match(TOKEN_DO))
    //     return doStatement(compiler);
    else if(match(TOKEN_PRINT))
        return printStatement();
    else if(match(TOKEN_BREAK))
        return breakStatement(compiler);
    else
        line_error(present().line, "Bad statement!");
}

Block parse(TokenList *list){
    head = list;
    Block mainBlock = newBlock();
    mainBlock.blockName = BLOCK_MAIN;
    Compiler* root = initCompiler(NULL, 0, BLOCK_MAIN);
    while(!match(TOKEN_EOF))
        addToBlock(&mainBlock, statement(root));

    return mainBlock;
}
