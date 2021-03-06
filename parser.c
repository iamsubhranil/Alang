#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "scanner.h"
#include "display.h"
#include "parser.h"
#include "expr.h"
#include "stmt.h"
#include "allocator.h"

static int inWhile = 0;
static int he = 0;
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
        Token h = head->value;
        head = head->next;
        //printf("\n[Info:Advance] Advancing to [%s]", tokenNames[head->value.type]);
        return h;
    }
    return errorToken;
}

/*
   static Token advance(){
   Token present = head->value;
   advance();
   return present;
   }
   */

static int presentLine(){
    return head->value.line;
}

static int match(TokenType type){
    if(peek() == type){
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

static void printToken(Token t){
    int i = 0;
    while(i < t.length)
        printf("%c", t.start[i++]);
}

static Token consume(TokenType type, const char* err){
    //printf("\n[Info] Have %s expected %s", tokenNames[peek()], tokenNames[type]);
    if(peek() == type){
        //printf("\n[Info:Consume] Consuming [%s]", tokenNames[type]);
        return advance();
    }
    else{
        printf(line_error("%s Got %s"), presentLine(), err, tokenNames[peek()]);
        if(peek() == TOKEN_NUMBER || peek() == TOKEN_IDENTIFIER || peek() == TOKEN_STRING){
            printf("(");
            printToken(advance());
            printf(")");
        }
        printf("!");
        he++;
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
    //    printf(debug("NextIndent : %d"), nextIndent);
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

static char* numericString(Token t){
    char* s = (char *)mallocate(sizeof(char) * t.length + 1);
    strncpy(s, t.start, t.length);
    s[t.length] = '\0';
    return s;
}

static char* stringOf(Token t){
    if(t.type == TOKEN_NUMBER || t.type == TOKEN_IDENTIFIER)
        return numericString(t);
    //    printf("%s %d\n", t.start, t.length);
    char* s = (char *)mallocate(sizeof(char) * (t.length-1));
    int i;
    for(i = 1;i < t.length-1;i++)
        s[i-1] = t.start[i];
    s[t.length - 2] = '\0';
    // printf("\nGiven : %s of size %d \nReturing : [%s] of size %lu", t.start, t.length, s, strlen(s));
    return s;
}

static int isDouble(const char *string){
    int i = 0;
    while(string[i] != '\0'){
        if(string[i] == '.')
            return 1;
        i++;
    }
    return 0;
}

static double doubleOf(const char *s){
    double d;
    sscanf(s, "%lf", &d);
    return d;
}

static long longOf(const char *s){
    long l;
    sscanf(s, "%ld", &l);
    return l;
}

static Expression* primary(){
    Expression* expr = newExpression();
    if(match(TOKEN_TRUE)){
        expr->type = EXPR_LITERAL;
        expr->literal.line = presentLine();
        expr->literal.type = LIT_LOGICAL;
        expr->literal.lVal = 1;
    }
    else if(match(TOKEN_FALSE)){
        expr->type = EXPR_LITERAL;
        expr->literal.line = presentLine();
        expr->literal.type = LIT_LOGICAL;
        expr->literal.lVal = 0;
    }
    else if(match(TOKEN_NULL)){
        expr->type = EXPR_LITERAL;
        expr->literal.line = presentLine();
        expr->literal.type = LIT_NULL;
    }
    else if(peek() == TOKEN_MINUS){ // desugaring -x to 0 - x
        expr->type = EXPR_BINARY;
        expr->binary.left = newExpression();
        expr->binary.left->type = EXPR_LITERAL;
        expr->binary.left->literal.type = LIT_INT;
        expr->binary.left->literal.dVal = 0;
        expr->binary.op = advance();
        expr->binary.right = expression();
    }
    else if(peek() == TOKEN_NUMBER){
        expr->type = EXPR_LITERAL;
        expr->literal.line = presentLine();
        char *val = stringOf(advance());
        if(isDouble(val)){
            expr->literal.type = LIT_DOUBLE;
            expr->literal.dVal = doubleOf(val);
        }
        else{
            expr->literal.type = LIT_INT;
            expr->literal.iVal = longOf(val);
        }
    }
    else if(peek() == TOKEN_STRING){
        expr->type = EXPR_LITERAL;
        expr->literal.line = presentLine();
        expr->literal.type = LIT_STRING;
        expr->literal.sVal = stringOf(advance());
    }
    else if(peek() == TOKEN_IDENTIFIER){
        char *name = stringOf(advance());
        if(match(TOKEN_LEFT_SQUARE)){
            expr->type = EXPR_ARRAY;
            expr->arrayExpression.identifier = name;
            expr->arrayExpression.line = presentLine();
            expr->arrayExpression.index = expression();
            consume(TOKEN_RIGHT_SQUARE, "Expected ']' after array index!");
        }
        else{
            expr->type = EXPR_VARIABLE;
            expr->variable.line = presentLine();
            expr->variable.name = name;
        }
    }
    else if(match(TOKEN_LEFT_PAREN)){
        memfree(expr);
        expr = expression();
        consume(TOKEN_RIGHT_PAREN, "Expected '(' after expression.");
    }
    else{
        Token t = advance();
        printf(line_error("Incorrect expression!"), t.line);
    }
    return expr;
}

static Expression* getCall(Expression* expr){
    Expression *call = newExpression();
    call->type = EXPR_CALL;
    call->callExpression.identifer = expr->variable.name;
    call->callExpression.argCount = 0;
    call->callExpression.arguments = NULL;
    call->callExpression.line = presentLine();
    if(match(TOKEN_RIGHT_PAREN))
        return call;
    do{
        Expression *argument = expression();
        call->callExpression.argCount++;
        call->callExpression.arguments = (Expression **)reallocate(call->callExpression.arguments, 
                sizeof(Expression *)*call->callExpression.argCount);
        call->callExpression.arguments[call->callExpression.argCount - 1] = argument;
    } while(match(TOKEN_COMMA));
    consume(TOKEN_RIGHT_PAREN, "Expected '(' after expression!");
    return call;
}

static Expression* call(){
    Expression* expr = primary();
    while(true){
        if(match(TOKEN_LEFT_PAREN)){
            expr  = getCall(expr);
        }
        else if(match(TOKEN_DOT)){
            Expression *ex = newExpression();
            ex->type = EXPR_REFERENCE;
            ex->referenceExpression.line = presentLine();
            ex->referenceExpression.containerName = expr;
            ex->referenceExpression.member = call();
            expr = ex;
        }
        else
            break;
    }
    return expr;
}

static Expression* tothepower(){
    Expression* expr = call();
    while(peek() == TOKEN_CARET){
        Expression* multi = newExpression();
        multi->type = EXPR_BINARY;
        multi->binary.line = presentLine();
        multi->binary.op = advance();
        multi->binary.left = expr;
        multi->binary.right = tothepower();
        expr = multi;
    }
    return expr;
}

static Expression* multiplication(){
    Expression* expr = tothepower();
    while(peek() == TOKEN_STAR || peek() == TOKEN_SLASH){
        Expression* multi = newExpression();
        multi->type = EXPR_BINARY;
        multi->binary.line = presentLine();
        multi->binary.op = advance();
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
        multi->binary.line = presentLine();
        multi->binary.op = advance();
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
        multi->type = EXPR_LOGICAL;
        multi->logical.line = presentLine();
        multi->logical.op = advance();
        multi->logical.left = expr;
        multi->logical.right = addition();
        expr = multi;
    }
    return expr;
}

static Expression* equality(){
    Expression* expr = comparison();
    while(peek() == TOKEN_EQUAL_EQUAL || peek() == TOKEN_BANG_EQUAL){
        Expression* multi = newExpression();
        multi->type = EXPR_LOGICAL;
        multi->logical.line = presentLine();
        multi->logical.op = advance();
        multi->logical.left = expr;
        multi->logical.right = comparison();
        expr = multi;
    }
    return expr;
}

static Expression* andE(){
    Expression* expr = equality();
    while(peek() == TOKEN_AND){
        Expression* logic = newExpression();
        logic->type = EXPR_LOGICAL;
        logic->logical.line = presentLine();
        logic->logical.op = advance();
        logic->logical.left = expr;
        logic->logical.right = equality();
        expr = logic;
    }
    return expr;
}

static Expression* orE(){
    Expression* expr = andE();
    while(peek() == TOKEN_OR){
        Expression* logic = newExpression();
        logic->type = EXPR_LOGICAL;
        logic->logical.line = presentLine();
        logic->logical.op = advance();
        logic->logical.left = expr;
        logic->logical.right = andE();
        expr = logic;
    }
    return expr;
}

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
    block->statements[block->numStatements-1] = statement;
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
    memfree(blockCompiler);
    debug("Block statement parsed");
    return b;
}

static Statement ifStatement(Compiler *compiler){
    debug("Parsing if statement");
    Statement s;
    s.type = STATEMENT_IF;
    s.ifStatement.line = presentLine();
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
    inWhile++;
    consume(TOKEN_LEFT_PAREN, "Expected left paren before conditional!");
    s.whileStatement.line = presentLine();
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
    inWhile--;
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

static Statement breakStatement(){
    debug("Parsing break statement");
    Statement s;
    s.type = STATEMENT_BREAK;
    s.breakStatement.pos = head->value;
    if(inWhile == 0){
        printf(line_error("Break without While or Do!"), presentLine());
        he++;
    }
    consume(TOKEN_NEWLINE, "Expected newline after Break!");
    debug("Break statement parsed");
    return s;
}

static Statement setStatement(){
    Statement s;
    s.type = STATEMENT_SET;
    debug("Parsing set statement");
    s.setStatement.line = presentLine();
    s.setStatement.count = 0;
    s.setStatement.initializers = NULL;
    do{
        s.setStatement.count++;
        s.setStatement.initializers = (Initializer *)reallocate(s.setStatement.initializers, sizeof(Initializer) * s.setStatement.count);
        s.setStatement.initializers[s.setStatement.count - 1].identifer = expression();
        consume(TOKEN_EQUAL, "Expected '=' after identifer!");
        s.setStatement.initializers[s.setStatement.count - 1].initializerExpression = expression();
    } while(match(TOKEN_COMMA));
    consume(TOKEN_NEWLINE, "Expected newline after Set statement!");
    debug("Set statement parsed");
    return s;
}

static Statement arrayStatement(){
    Statement s;
    s.type = STATEMENT_ARRAY;
    debug("Parsing array statement");
    s.arrayStatement.line = presentLine();
    s.arrayStatement.count = 0;
    s.arrayStatement.initializers = NULL;

    do{
        s.arrayStatement.count++;
        s.arrayStatement.initializers = (Expression **)reallocate(s.arrayStatement.initializers, 
                sizeof(Expression *) * s.arrayStatement.count);
        s.arrayStatement.initializers[s.arrayStatement.count - 1] = expression();
        if(s.arrayStatement.initializers[s.arrayStatement.count - 1]->type != EXPR_ARRAY){
            printf(line_error("Expected array expression!"), s.arrayStatement.line);
            he++;
        }
    } while(match(TOKEN_COMMA));
    consume(TOKEN_NEWLINE, "Expected newline after Array statement!");
    debug("Array statement parsed");
    return s;
}

static Statement inputStatement(){
    Statement s;
    s.type = STATEMENT_INPUT;
    debug("Parsing input statement");
    s.inputStatement.line = presentLine();
    s.inputStatement.count = 0;
    s.inputStatement.inputs = 0;
    do{
        s.inputStatement.count++;
        s.inputStatement.inputs = (Input *)reallocate(s.inputStatement.inputs, sizeof(Input) * s.inputStatement.count);
        Input i;
        if(peek() == TOKEN_STRING){
            i.type = INPUT_PROMPT;
            i.prompt = stringOf(advance());
        }
        else if(peek() == TOKEN_IDENTIFIER){
            i.type = INPUT_IDENTIFER;
            i.identifer = stringOf(advance());
            i.datatype = INPUT_ANY;
            if(match(TOKEN_COLON)){
                if(match(TOKEN_INT))
                    i.datatype = INPUT_INT;
                else if(match(TOKEN_FLOAT))
                    i.datatype = INPUT_FLOAT;
                else{
                    printf(line_error("Bad input format specifier!"), s.inputStatement.line);
                    he++;
                }
            }
        }
        else{
            printf(line_error("Bad input statement!"), s.inputStatement.line);
            he++;
            continue;
        }
        s.inputStatement.inputs[s.inputStatement.count - 1] = i;
    } while(match(TOKEN_COMMA));

    consume(TOKEN_NEWLINE, "Expected newline after Input!");
    return s;
}

static Statement printStatement(){
    Statement s;
    s.type = STATEMENT_PRINT;
    debug("Parsing print statement");

    int count = 1;
    Expression** exps = (Expression **)mallocate(sizeof(Expression *));
    exps[0] = expression();
    s.printStatement.line = presentLine();
    while(match(TOKEN_COMMA)){
        count++;
        exps = (Expression **)reallocate(exps, sizeof(Expression *)*count);
        exps[count - 1] = expression();
    }
    s.printStatement.argCount = count;
    s.printStatement.expressions = exps;
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

static Statement routineStatement(Compiler *compiler){
    Statement s;
    s.type = STATEMENT_ROUTINE;
    s.routine.line = presentLine();
    s.routine.arity = 0;
    s.routine.arguments = NULL;   
    s.routine.name = NULL;
    s.routine.isNative = 0;

    if(compiler->indentLevel > 0){
        printf(line_error("Routines can only be declared in top level indent!"), presentLine());
        he++;
    }

    if(match(TOKEN_FOREIGN))
        s.routine.isNative = 1;
    
    s.routine.name = stringOf(consume(TOKEN_IDENTIFIER, "Expected routine name!"));
    consume(TOKEN_LEFT_PAREN, "Expected '(' after routine declaration!");
    if(peek() != TOKEN_RIGHT_PAREN){
        do{
            s.routine.arity++;
            s.routine.arguments = (char **)reallocate(s.routine.arguments, sizeof(char *) * s.routine.arity);
            s.routine.arguments[s.routine.arity - 1] = stringOf(consume(TOKEN_IDENTIFIER, "Expected identifer as argument!"));
        } while(match(TOKEN_COMMA));
        consume(TOKEN_RIGHT_PAREN, "Expected ')' after argument declaration!");
    }
    else
        advance();
    consume(TOKEN_NEWLINE, "Expected newline after routine declaration!");

    if(s.routine.isNative == 0){
        s.routine.code = blockStatement(compiler, BLOCK_FUNC);

        consume(TOKEN_ENDROUTINE, "Expected EndRoutine after routine definition!");
        consume(TOKEN_NEWLINE, "Expected newline after routine definition!");
    }
    return s;
}

static Statement callStatement(){
    Statement s;
    s.type = STATEMENT_CALL;
    s.callStatement.callee = NULL;
    s.callStatement.line = presentLine();
    s.callStatement.callee = call();
    consume(TOKEN_NEWLINE, "Expected newline after routine call!");
    return s;
}

static Statement returnStatement(){
    Statement s;
    s.type = STATEMENT_RETURN;
    s.returnStatement.line = presentLine();
    if(peek() == TOKEN_NEWLINE)
        s.returnStatement.value = NULL;
    else
        s.returnStatement.value = expression();
    consume(TOKEN_NEWLINE, "Expected newline after Return!");
    return s;
}

static Statement containerStatement(Compiler *c){
    Statement s;
    s.type = STATEMENT_CONTAINER;
    s.container.name = stringOf(head->value);
    s.container.line = presentLine();
    s.container.arity = 0;
    consume(TOKEN_IDENTIFIER, "Expected container identifer!");
    consume(TOKEN_LEFT_PAREN, "Expected '(' after container name");
    while(!match(TOKEN_RIGHT_PAREN) && !match(TOKEN_EOF)){
        s.container.arity++;
        s.container.arguments = (char **)mallocate(sizeof(char *) * s.container.arity);
        s.container.arguments[s.container.arity - 1] = stringOf(consume(TOKEN_IDENTIFIER, "Expected identifer as argument!"));
    }
    consume(TOKEN_NEWLINE, "Expected newline after container declaration!");
    s.container.constructor = blockStatement(c, BLOCK_FUNC) ;
    consumeIndent(c->indentLevel);
    consume(TOKEN_ENDCONTAINER, "Expected EndContainer on the same indent!");
    consume(TOKEN_NEWLINE, "Expected newline after EndContainer!");
    return s;
}

static Statement noopStatement(){
    Statement s;
    s.type = STATEMENT_NOOP;
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
    else if(match(TOKEN_ARRAY))
        return arrayStatement();
    else if(match(TOKEN_INPUT))
        return inputStatement();
    else if(match(TOKEN_WHILE))
        return whileStatement(compiler);
    // else if(match(TOKEN_DO))
    //     return doStatement(compiler);
    else if(match(TOKEN_PRINT))
        return printStatement();
    else if(match(TOKEN_BREAK))
        return breakStatement();
    else if(match(TOKEN_ROUTINE))
        return routineStatement(compiler);
    else if(match(TOKEN_CALL))
        return callStatement();
    else if(match(TOKEN_RETURN))
        return returnStatement();
    else{
        printf(line_error("Bad statement %s!"), presentLine(), tokenNames[peek()]);
        advance();
    }
    return noopStatement();
}

Statement part(Compiler *c){
    if(peek() == TOKEN_EOF)
        return noopStatement();
    else if(match(TOKEN_ROUTINE)){
        return routineStatement(c);
    }
    else if(match(TOKEN_SET)){
        return setStatement();
    }
    else if(match(TOKEN_ARRAY)){
        return arrayStatement();
    }
    else if(match(TOKEN_CONTAINER)){
        return containerStatement(c);
    }
    else{
        printf(line_error("Bad top level statement %s!"), presentLine(), tokenNames[peek()]);
        he++;
        statement(c);
        return part(c);
    }
}

Code parse(TokenList *list){
    Code c = {0, NULL};
    head = list;
    Compiler *comp = initCompiler(NULL, 0, BLOCK_NONE);
    while(!match(TOKEN_EOF)){
        c.count++;
        c.parts = (Statement *)reallocate(c.parts, sizeof(Statement) * c.count);
        c.parts[c.count - 1] = part(comp);
    }
    memfree(comp);
    return c;
}

int hasParseError(){
    return he;
}
