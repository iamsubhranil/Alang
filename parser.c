#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "scanner.h"
#include "display.h"
#include "parser.h"
#include "allocator.h"

#include "values.h"
#include "strings.h"
#include "interpreter.h"
#include "routines.h"
#include "heap.h"

static int inWhile = 0, inRoutine = 0;
static int he = 0;
static uint64_t *breakAddresses, breakCount = 0;
static TokenList *head = NULL;
static Token errorToken = {TOKEN_ERROR,"BadToken",0,-1};

typedef enum{
    BLOCK_IF,
    BLOCK_ELSE,
    BLOCK_WHILE,
    BLOCK_FUNC,
    BLOCK_NONE
} BlockType;

typedef struct Compiler{
    struct Compiler *parent;
    int indentLevel;
    BlockType blockName;
} Compiler;

static Compiler* initCompiler(Compiler *parent, int indentLevel, BlockType name){
    Compiler *ret = (Compiler *)mallocate(sizeof(Compiler));
    ret->parent = parent;
    ret->indentLevel = indentLevel;
    ret->blockName = name;
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

static Token consume(TokenType type, const char* err){
    //printf("\n[Info] Have %s expected %s", tokenNames[peek()], tokenNames[type]);
    if(peek() == type){
        //printf("\n[Info:Consume] Consuming [%s]", tokenNames[type]);
        return advance();
    }
    else{
        printf(line_error("%s"), presentLine(), err);
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

static void expression();

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

static int64_t longOf(const char *s){
    int64_t l;
    sscanf(s,"%" SCNu64, &l);
    return l;
}

static uint8_t insFromToken(Token t){
    switch(t.type){
        case TOKEN_CARET:
            return POW;
        case TOKEN_STAR:
            return MUL;
        case TOKEN_SLASH:
            return DIV;
        case TOKEN_PLUS:
            return ADD;
        case TOKEN_MINUS:
            return SUB;
        case TOKEN_PERCEN:
            return MOD;
        case TOKEN_EQUAL_EQUAL:
            return EQ;
        case TOKEN_BANG_EQUAL:
            return NEQ;
        case TOKEN_GREATER:
            return GT;
        case TOKEN_GREATER_EQUAL:
            return GTE;
        case TOKEN_LESS:
            return LT;
        case TOKEN_LESS_EQUAL:
            return LTE;
        case TOKEN_AND:
            return AND;
        case TOKEN_OR:
            return OR;
        default:
            return NOOP;
    }
}

static void getCall(){
    uint64_t argCount = 0;
    if(!match(TOKEN_RIGHT_PAREN)){
        do{
            expression();
            argCount++;
        } while(match(TOKEN_COMMA));
        consume(TOKEN_RIGHT_PAREN, "Expected ')' after expression!");
    }
    ins_add(PUSHI);
    ins_add_val(heap_add_int(argCount));
}

static void primary(){
    if(match(TOKEN_TRUE)){
        ins_add(PUSHL);
        ins_add_val(heap_add_logical(1));
    }
    else if(match(TOKEN_FALSE)){
        ins_add(PUSHL);
        ins_add_val(heap_add_logical(0));
    }
    else if(match(TOKEN_NULL)){
        ins_add(PUSHN);
    }
    else if(peek() == TOKEN_NUMBER){
        char *val = stringOf(advance());
        if(isDouble(val)){
            ins_add(PUSHF);
            ins_add_val(heap_add_float(doubleOf(val)));
        }
        else{
            ins_add(PUSHI);
            ins_add_val(heap_add_int(longOf(val)));
        }
    }
    else if(peek() == TOKEN_STRING){
        uint64_t st = str_insert(stringOf(advance()));
        ins_add(PUSHS);
        //    printf("\nPushing string %lu", st);
        ins_add_val(heap_add_str(st));
    }
    else if(peek() == TOKEN_IDENTIFIER){
        uint64_t st = str_insert(stringOf(advance()));
        if(match(TOKEN_LEFT_SQUARE)){
            expression(); // index
            ins_add(PUSHID);
            ins_add_val(heap_add_identifer(st));
            ins_add(ARRAY);
            consume(TOKEN_RIGHT_SQUARE, "Expected ']' after array index!");
        }
        else if(match(TOKEN_LEFT_PAREN)){
            getCall();
            ins_add(PUSHID);
            ins_add_val(heap_add_identifer(st));
            ins_add(CALL);
        }
        else{
            ins_add(PUSHID);
            ins_add_val(heap_add_identifer(st));
        }
    }
    else if(match(TOKEN_LEFT_PAREN)){
        expression();
        consume(TOKEN_RIGHT_PAREN, "Expected ')' after grouping expression!");
    }
    else{
        Token t = advance();
        printf(line_error("Incorrect expression!"), t.line);
    }
}

static void call(){
    primary();
    while(true){
        if(match(TOKEN_DOT)){
            primary();
            if(ins_last() == ARRAY)
                ins_set(ip_get() - 1, ARRAYREF);
            else
                ins_add(MEMREF);
        }
        else
            break;
    }
}

static void tothepower(){
    call();
    while(peek() == TOKEN_CARET){
        uint8_t op = insFromToken(advance());
        call();
        ins_add(op);
    }
}

static void multiplication(){
    tothepower();
    while(peek() == TOKEN_STAR || peek() == TOKEN_SLASH){
        uint8_t op = insFromToken(advance());
        tothepower();
        ins_add(op);
    }
}

static void addition(){
    multiplication();
    while(peek() == TOKEN_PLUS || peek() == TOKEN_MINUS
            || peek() == TOKEN_PERCEN){
        uint8_t op = insFromToken(advance());
        multiplication();
        ins_add(op);
    }
}

static void comparison(){
    addition();
    while(peek() == TOKEN_GREATER || peek() == TOKEN_LESS
            || peek() == TOKEN_GREATER_EQUAL || peek() == TOKEN_LESS_EQUAL){
        uint8_t op = insFromToken(advance());
        addition();
        ins_add(op);
    }
}

static void equality(){
    comparison();
    while(peek() == TOKEN_EQUAL_EQUAL || peek() == TOKEN_BANG_EQUAL){
        uint8_t op = insFromToken(advance());
        comparison();
        ins_add(op);
    }
}

static void andE(){
    equality();
    while(peek() == TOKEN_AND){
        advance();
        equality();
        ins_add(AND);
    }
}

static void orE(){
    andE();
    while(peek() == TOKEN_OR){
        advance();
        andE();
        ins_add(OR); 
    }
}

static void expression(){
    orE();
}

static void statement(Compiler *compiler);

static void blockStatement(Compiler *compiler, BlockType name){
    //debug("Parsing block statement");
    int indent = compiler->indentLevel+1;
    Compiler *blockCompiler = initCompiler(compiler, indent, name);
    while(getNextIndent() == indent){
        debug("Found a block statement");
        statement(blockCompiler);
    }
    memfree(blockCompiler);
    //debug("Block statement parsed");
}

static void ifStatement(Compiler *compiler){
    //debug("Parsing if statement");

    consume(TOKEN_LEFT_PAREN, "Conditionals must start with an openning brace['('] after If!");
    expression();
    consume(TOKEN_RIGHT_PAREN, "Conditional must end with a closing brace[')'] after If!");
    consume(TOKEN_NEWLINE, "Expected newline after If!");
    ins_add(PUSHI);
    uint64_t jmp = ins_add_val(heap_add_int(0));
    ins_add(JUMP_IF_FALSE);
    if(getNextIndent() == compiler->indentLevel){
        consumeIndent(compiler->indentLevel);
        consume(TOKEN_THEN, "Expected Then on the same indent!");
        consume(TOKEN_NEWLINE, "Expected newline after Then!");
    }

    blockStatement(compiler, BLOCK_IF);
    consumeIndent(compiler->indentLevel);
    ins_add(PUSHI);
    uint64_t skip = ins_add_val(heap_add_int(0));
    ins_add(JUMP);
    ins_set_val(jmp, heap_add_int(ip_get()));
    if(match(TOKEN_ELSE)){
        if(match(TOKEN_IF)){
            ifStatement(compiler);
        }
        else{
            consume(TOKEN_NEWLINE, "Expected newline after Else!");
            blockStatement(compiler, BLOCK_ELSE);
            consumeIndent(compiler->indentLevel);
            consume(TOKEN_ENDIF, "Expected EndIf after If!");
            consume(TOKEN_NEWLINE, "Expected newline after EndIf!");
        }
    }
    else{
        consume(TOKEN_ENDIF, "Expected EndIf after If!");
        consume(TOKEN_NEWLINE, "Expected newline after EndIf!");
    }
    ins_set_val(skip, heap_add_int(ip_get()));
    //debug("If statement parsed");
}

static void whileStatement(Compiler* compiler){
    //debug("Parsing while statement");
    consume(TOKEN_LEFT_PAREN, "Expected left paren before conditional!");
    uint64_t start = ip_get();
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expected right paren after conditional!");
    consume(TOKEN_NEWLINE, "Expected newline after While!");
    ins_add(PUSHI);
    uint64_t jmp = ins_add_val(heap_add_int(0));
    ins_add(JUMP_IF_FALSE);
    if(getNextIndent() == compiler->indentLevel){
        consumeIndent(compiler->indentLevel);
        consume(TOKEN_BEGIN, "Expected Begin on same indent!");
        consume(TOKEN_NEWLINE, "Expected newline after begin!");
    }
    inWhile++;
    blockStatement(compiler, BLOCK_WHILE);
    ins_add(PUSHI);
    ins_add_val(heap_add_int(start));
    ins_add(JUMP);
    ins_set_val(jmp, heap_add_int(ip_get()));
    consumeIndent(compiler->indentLevel);
    consume(TOKEN_ENDWHILE, "Expected EndWhile after while on same indent!");
    consume(TOKEN_NEWLINE, "Expected newline after EndWhile!");
    debug("While statement parsed");
    uint64_t i = 0;
    while(i < breakCount){
        ins_set_val(breakAddresses[i], heap_add_int(ip_get()));
        i++;
    }
    inWhile--;
    if(inWhile == 0){
        breakCount = 0;
        memfree(breakAddresses);
        breakAddresses = NULL;
    }
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

static void breakStatement(){
    //debug("Parsing break statement");
    if(inWhile == 0){
        printf(line_error("Break without While or Do!"), presentLine());
        he++;
    }
    else{
        breakAddresses = (uint64_t *)reallocate(breakAddresses, 64*++breakCount);
        ins_add(PUSHI);
        uint64_t ba = ins_add_val(heap_add_int(0));
        breakAddresses[breakCount - 1] = ba;
        ins_add(JUMP);
    }
    consume(TOKEN_NEWLINE, "Expected newline after Break!");
    //debug("Break statement parsed");
}

static void setStatement(){
    //debug("Parsing set statement");
    do{
        expression();
        uint8_t type = 0;
        if(ins_last() == MEMREF){
            type = 1;
            ins_set(ip_get() - 1, NOOP);
        }
        else if(ins_last() == ARRAY){
            type = 2;
            ins_set(ip_get() - 1, NOOP);
        }
        else if(ins_last() == ARRAYREF){
            type = 3;
            ins_set(ip_get() - 1, NOOP);
        }
        consume(TOKEN_EQUAL, "Expected '=' after identifer!");
        expression();
        
        if(type == 0)
            ins_add(SET);
        else if(type == 1)
            ins_add(MEMSET);
        else if(type == 2)
            ins_add(ARRAYWRITE);
        else if(type == 3)
            ins_add(ARRAYSET);

    } while(match(TOKEN_COMMA));
    consume(TOKEN_NEWLINE, "Expected newline after Set statement!");
    //debug("Set statement parsed");
}

static void arrayStatement(){
    //debug("Parsing array statement");

    do{
        expression();
        if(ins_last() != ARRAY){
            printf(line_error("Expected array expression!"), presentLine());
            he++;
        }
        ins_set(ip_get()-1, MAKE_ARRAY);
    } while(match(TOKEN_COMMA));
    consume(TOKEN_NEWLINE, "Expected newline after Array statement!");
    //debug("Array statement parsed");
}

static void inputStatement(){
    //debug("Parsing input statement");
    do{
        if(peek() == TOKEN_IDENTIFIER){
            uint64_t name = str_insert(stringOf(advance()));
            uint8_t op = INPUTS;
            if(match(TOKEN_COLON)){
                if(match(TOKEN_INT))
                    op = INPUTI;
                else if(match(TOKEN_FLOAT))
                    op = INPUTF;
                else{
                    printf(line_error("Bad input format specifier!"), presentLine());
                    he++;
                }
            }
            ins_add(PUSHID);
            ins_add_val(heap_add_identifer(name));
            ins_add(op);
        }
        else{
            printf(line_error("Bad input statement!"), presentLine());
            he++;
            continue;
        }
    } while(match(TOKEN_COMMA));

    consume(TOKEN_NEWLINE, "Expected newline after Input!");
}

static void printStatement(){
    //debug("Parsing print statement");
    do{
        expression();
        ins_add(PRINT);
    } while(match(TOKEN_COMMA));
    consume(TOKEN_NEWLINE, "Expected newline after Print!");
    //debug("Print statement parsed");
}

static void beginStatement(){
    //debug("Parsing begin statement");
    consume(TOKEN_NEWLINE, "Expected newline after Begin!");
    ins_add(NOOP);
    //debug("Begin statement parsed");
}

static void endStatement(){
    //debug("End statement parsed");
    ins_add(HALT);
    consume(TOKEN_NEWLINE, "Expected newline after End!");
}

static void routineStatement(Compiler *compiler){
    Routine2 routine = routine_new();

    if(compiler->indentLevel > 0){
        printf(line_error("Routines can only be declared in top level indent!"), presentLine());
        he++;
    }

    if(match(TOKEN_FOREIGN))
        routine.isNative = 1;

    routine.name = str_insert(stringOf(consume(TOKEN_IDENTIFIER, "Expected routine name!")));
    consume(TOKEN_LEFT_PAREN, "Expected '(' after routine declaration!");
    if(peek() != TOKEN_RIGHT_PAREN){
        do{
            routine_add_arg(&routine, stringOf(consume(TOKEN_IDENTIFIER, "Expected identifer as argument!")));
        } while(match(TOKEN_COMMA));
        consume(TOKEN_RIGHT_PAREN, "Expected ')' after argument declaration!");
    }
    else
        advance();
    consume(TOKEN_NEWLINE, "Expected newline after routine declaration!");

    if(routine.isNative == 0){
        routine.startAddress = ip_get();
        blockStatement(compiler, BLOCK_FUNC);
        if(ins_last() != RETURN){
            ins_add(PUSHN);
            ins_add(RETURN);
        }
        consume(TOKEN_ENDROUTINE, "Expected EndRoutine after routine definition!");
        consume(TOKEN_NEWLINE, "Expected newline after routine definition!");
    }
    routine_add(routine);
}

static void callStatement(){
    call();
    consume(TOKEN_NEWLINE, "Expected newline after routine call!");
}

static void returnStatement(){
    if(inRoutine > 0){
        printf(line_error("A routine should not return anything!"), presentLine());
        he++;
    }
    if(peek() == TOKEN_NEWLINE){
        ins_add(PUSHN);
    }
    else
        expression();
    ins_add(RETURN);
    consume(TOKEN_NEWLINE, "Expected newline after Return!");
}

static void containerStatement(Compiler *compiler){
    Routine2 routine = routine_new();

    if(compiler->indentLevel > 0){
        printf(line_error("Containers can only be declared in top level indent!"), presentLine());
        he++;
    }
    routine.isNative = 0;

    routine.name = str_insert(stringOf(consume(TOKEN_IDENTIFIER, "Expected container name!")));
    consume(TOKEN_LEFT_PAREN, "Expected '(' after container declaration!");
    if(peek() != TOKEN_RIGHT_PAREN){
        do{
            routine_add_arg(&routine, stringOf(consume(TOKEN_IDENTIFIER, "Expected identifer as argument!")));
        } while(match(TOKEN_COMMA));
        consume(TOKEN_RIGHT_PAREN, "Expected ')' after argument declaration!");
    }
    else
        advance();
    consume(TOKEN_NEWLINE, "Expected newline after container declaration!");
    routine.startAddress = ip_get();
    inRoutine++;
    blockStatement(compiler, BLOCK_FUNC);
    ins_add(PUSHID);
    ins_add_val(heap_add_int(routine.name));
    ins_add(NEW_CONTAINER);
    inRoutine--;
    consume(TOKEN_ENDCONTAINER, "Expected EndContainer on the same indent!");
    consume(TOKEN_NEWLINE, "Expected newline after EndContainer!");
    routine_add(routine);
}
/*
   static Statement noopStatement(){
   Statement s;
   s.type = STATEMENT_NOOP;
   return s;
   }
   */
static void statement(Compiler *compiler){
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
}

uint64_t lastjump = 0;

void part(Compiler *c){
    if(peek() == TOKEN_EOF)
        beginStatement();
    else if(match(TOKEN_ROUTINE)){
        routineStatement(c);
    }
    else if(match(TOKEN_SET)){
        ins_set_val(lastjump, heap_add_int(ip_get())); // Patch the last jump
        setStatement();
        ins_add(PUSHI); // Make a jump to next global statement
        lastjump = ins_add_val(heap_add_int(0));
        ins_add(JUMP);
    }
    else if(match(TOKEN_ARRAY)){
        ins_set_val(lastjump, heap_add_int(ip_get())); // Patch the last jump
        arrayStatement();
        ins_add(PUSHI); // Make a jump to next global statement
        lastjump = ins_add_val(heap_add_int(0));
        ins_add(JUMP);
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

void parse(TokenList *list){
    head = list;
    Compiler *comp = initCompiler(NULL, 0, BLOCK_NONE);
    ins_add(PUSHI);
    lastjump = ins_add_val(heap_add_int(0)); // The first location will tell the address of first instruction
    // to be executed, which will be either a global 'Set'/'Array',
    // or the routine 'Main'
    ins_add(JUMP); // Jump to the first global instruction
    while(!match(TOKEN_EOF)){
        part(comp);
    }
    routine_get(str_insert("Main")); // Check if Main is defined

    uint64_t callMain = ins_add(PUSHI); // Add an implicit call to Main()
    ins_add_val(heap_add_int(0));
    ins_add(PUSHID);
    ins_add_val(heap_add_identifer(str_insert("Main")));
    ins_add(CALL);

    ins_set_val(lastjump, heap_add_int(callMain)); // After all globals statements are
    // executed, jump to the routine 'Main'
    ins_add(HALT); // After Main returns, halt the machine

    memfree(comp);
}

int hasParseError(){
    return he;
}
