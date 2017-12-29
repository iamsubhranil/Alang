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
#include "native.h"

static int inWhile = 0, inContainer = 0, inCall = 0, inReturn = 0, inRef = 0, keepID = 0;
static int he = 0;
static uint32_t **breakAddresses = NULL, breakLevel = 0, *breakCount = NULL;
static TokenList *head = NULL;
static Token errorToken = {TOKEN_ERROR,"BadToken",0,-1,NULL};

static void break_push_scope(){
    breakAddresses = (uint32_t **)reallocate(breakAddresses, sizeof(uint32_t *) * ++breakLevel);
    breakAddresses[breakLevel - 1] = NULL;
    breakCount = (uint32_t *)reallocate(breakCount, 32*breakLevel);
    breakCount[breakLevel - 1] = 0;
}

static void break_add_address(uint32_t address){
    breakAddresses[breakLevel - 1] = (uint32_t *)reallocate(breakAddresses[breakLevel - 1], 32 * ++breakCount[breakLevel - 1]);
    breakAddresses[breakLevel - 1][breakCount[breakLevel - 1] - 1] = address;
}

static uint32_t* break_peek_scope(){
    if(breakLevel == 0)
        return NULL;
    return breakAddresses[breakLevel - 1];
}

static void break_pop_scope(){
    if(breakLevel == 0)
        return;
    --breakLevel;
    memfree(breakAddresses[breakLevel]);
    breakAddresses = (uint32_t **)reallocate(breakAddresses, sizeof(uint32_t *) * breakLevel);
    breakCount = (uint32_t *)reallocate(breakCount, 32*breakLevel);
}

typedef struct{
    uint32_t routineName;
    uint32_t arity;
    uint32_t callAddress;
    Token t;
} Call;

static Call *calls = NULL;
static uint32_t callPointer = 0;

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

Token presentToken(){
    if(head == NULL)
        return errorToken;
    return head->value;
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
        lnerr("Unexpected end of input!", head->value);
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
        lnerr(err, head->value);
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

static int32_t longOf(const char *s){
    if(strlen(s) > 10){
        lnerr("Integer overflow : %s > %" PRId32, presentToken(), s, INT32_MAX);
        he++;
    }
    int64_t l;
    sscanf(s,"%" SCNd64, &l);
    if(l > INT32_MAX){
        lnerr("Integer overflow : %" PRId64 " > %" PRId32, presentToken(), l, INT32_MAX);
        he++;
    }
    return (int32_t)l;
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

static uint32_t getCall(){
    uint32_t argCount = 0;
    int bak = keepID;
    keepID = 0;
    if(!match(TOKEN_RIGHT_PAREN)){
        do{
            expression();
            argCount++;
        } while(match(TOKEN_COMMA));
        consume(TOKEN_RIGHT_PAREN, "Expected ')' after expression!");
    }
    keepID = bak;
    return argCount;
}

static void primary(){
    if(match(TOKEN_TRUE)){
        ins_add(PUSHL);
        ins_add_val(1);
    }
    else if(match(TOKEN_FALSE)){
        ins_add(PUSHL);
        ins_add_val(0);
    }
    else if(match(TOKEN_NULL)){
        ins_add(PUSHN);
    }
    else if(peek() == TOKEN_NUMBER){
        char *val = stringOf(advance());
        if(isDouble(val)){
            ins_add(PUSHF);
            ins_add_double(doubleOf(val));
        }
        else{
            ins_add(PUSHI);
            ins_add_val(longOf(val));
        }
        memfree(val);
    }
    else if(peek() == TOKEN_STRING){
        uint32_t st = str_insert(stringOf(advance()));
        ins_add(PUSHS);
        //    printf("\nPushing string %lu", st);
        ins_add_val(st);
    }
    else if(peek() == TOKEN_IDENTIFIER){
        uint32_t st = str_insert(stringOf(advance()));
        if(match(TOKEN_LEFT_SQUARE)){
            int bak = keepID;
            keepID = 0;
            expression(); // index
            keepID = bak;
            ins_add(PUSHID);
            ins_add_val(st);
            ins_add(ARRAY);
            consume(TOKEN_RIGHT_SQUARE, "Expected ']' after array index!");
        }
        else if(match(TOKEN_LEFT_PAREN)){
            calls = (Call *)reallocate(calls, sizeof(Call) * ++callPointer);
            calls[callPointer - 1].t = presentToken();
            uint32_t tmp = callPointer;
            inCall++;
            uint32_t arity = getCall();
            inCall--;
            ins_add(CALL);

            calls[tmp - 1].callAddress = ins_add_val(0);

            ins_add_val(arity);

            calls[tmp - 1].arity = arity;
            calls[tmp - 1].routineName = st;
        }
        else{
            if(keepID == 0){
                ins_add(PUSHIDV);
            }
            else
                ins_add(PUSHID);
            ins_add_val(st);
        }
    }
    else if(match(TOKEN_LEFT_PAREN)){
        int bak = keepID;
        keepID = 0;
        expression();
        keepID = bak;
        consume(TOKEN_RIGHT_PAREN, "Expected ')' after grouping expression!");
    }
    else{
        Token t = advance();
        lnerr("Incorrect expression!", t);
    }
}

static void call(){
    primary();
    while(1){
        if(match(TOKEN_DOT)){
            keepID++;
            primary();
            keepID--;
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
        //dbg("Found a block statement");
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
    ins_add(JUMP_IF_FALSE);
    uint32_t jmp = ins_add_val(0);
    if(getNextIndent() == compiler->indentLevel){
        consumeIndent(compiler->indentLevel);
        consume(TOKEN_THEN, "Expected Then on the same indent!");
        consume(TOKEN_NEWLINE, "Expected newline after Then!");
    }

    blockStatement(compiler, BLOCK_IF);
    consumeIndent(compiler->indentLevel);
    ins_add(JUMP);
    uint32_t skip = ins_add_val(0);
    ins_set_val(jmp, ip_get());
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
    ins_set_val(skip, ip_get());
    //debug("If statement parsed");
}

static void whileStatement(Compiler* compiler){
    //debug("Parsing while statement");
    consume(TOKEN_LEFT_PAREN, "Expected left paren before conditional!");
    uint32_t start = ip_get();
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expected right paren after conditional!");
    consume(TOKEN_NEWLINE, "Expected newline after While!");
    ins_add(JUMP_IF_FALSE);
    uint32_t jmp = ins_add_val(0);
    if(getNextIndent() == compiler->indentLevel){
        consumeIndent(compiler->indentLevel);
        consume(TOKEN_BEGIN, "Expected Begin on same indent!");
        consume(TOKEN_NEWLINE, "Expected newline after begin!");
    }
    inWhile++;

    break_push_scope();

    blockStatement(compiler, BLOCK_WHILE);
    ins_add(JUMP);
    ins_add_val(start);
    ins_set_val(jmp, ip_get());
    consumeIndent(compiler->indentLevel);
    consume(TOKEN_ENDWHILE, "Expected EndWhile after while on same indent!");
    consume(TOKEN_NEWLINE, "Expected newline after EndWhile!");
    //dbg("While statement parsed");
    uint32_t *bks = break_peek_scope();
    if(bks != NULL){
        while(breakCount[breakLevel - 1] > 0)
           ins_set_val(bks[--breakCount[breakLevel - 1]], ip_get());
    }

    break_pop_scope();
 
    inWhile--;
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
        lnerr("Break without While or Do!", presentToken());
        he++;
    }
    else{
        ins_add(JUMP);
        break_add_address(ins_add_val(0));
    }
    consume(TOKEN_NEWLINE, "Expected newline after Break!");
    //debug("Break statement parsed");
}

static void setStatement(){
    //debug("Parsing set statement");
    do{
        keepID++;
        expression();
        keepID--;
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
        else
            ins_add(ARRAYSET);

    } while(match(TOKEN_COMMA));
    consume(TOKEN_NEWLINE, "Expected newline after Set statement!");
    //debug("Set statement parsed");
}

static void arrayStatement(){
    //debug("Parsing array statement");

    do{
        keepID++;
        expression();
        keepID--;
        if(ins_last() != ARRAY){
            lnerr("Expected array expression!", presentToken());
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
            uint32_t name = str_insert(stringOf(advance()));
            uint8_t op = INPUTS;
            if(match(TOKEN_COLON)){
                if(match(TOKEN_INT))
                    op = INPUTI;
                else if(match(TOKEN_FLOAT))
                    op = INPUTF;
                else{
                    lnerr("Bad input format specifier!", presentToken());
                    he++;
                }
            }
            ins_add(PUSHID);
            ins_add_val(name);
            ins_add(op);
        }
        else{
            lnerr("Bad input statement!", presentToken());
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
        lnerr("Routines can only be declared in top level indent!", presentToken());
        he++;
    }

    if(match(TOKEN_FOREIGN))
        routine.isNative = 1;

    routine.name = str_insert(stringOf(consume(TOKEN_IDENTIFIER, "Expected routine name!")));

    uint32_t *args = NULL, argp = 0;

    consume(TOKEN_LEFT_PAREN, "Expected '(' after routine declaration!");
    if(peek() != TOKEN_RIGHT_PAREN){
        do{
            args = (uint32_t *)reallocate(args, sizeof(uint32_t) * ++argp);
            args[argp - 1] = str_insert(stringOf(consume(TOKEN_IDENTIFIER, "Expected identifer as argument!")));
            if(routine.isNative) 
                routine_add_arg(&routine, args[argp - 1]);
            else
                routine.arity++;
        } while(match(TOKEN_COMMA));

        consume(TOKEN_RIGHT_PAREN, "Expected ')' after argument declaration!");

        if(!routine.isNative){
            while(argp > 0){
                if(routine.startAddress == 0)
                    routine.startAddress = ins_add(SETI);
                else
                    ins_add(SETI);
                ins_add_val(args[--argp]);
            }
        }
        memfree(args);
    }
    else
        advance();
    consume(TOKEN_NEWLINE, "Expected newline after routine declaration!");

    if(routine.isNative == 0){
        if(routine.startAddress == 0)
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
    if(inContainer > 0){
        lnerr("A routine should not return anything!", presentToken());
        he++;
    }
    if(peek() == TOKEN_NEWLINE){
        ins_add(PUSHN);
    }
    else{
        inReturn++;
        expression();
        inReturn--;
    }
    ins_add(RETURN);
    consume(TOKEN_NEWLINE, "Expected newline after Return!");
}

static void containerStatement(Compiler *compiler){
    Routine2 routine = routine_new();

    if(compiler->indentLevel > 0){
        lnerr("Containers can only be declared in top level indent!", presentToken());
        he++;
    }
    routine.isNative = 0;

    routine.name = str_insert(stringOf(consume(TOKEN_IDENTIFIER, "Expected container name!")));

    uint32_t *args = NULL, argp = 0;

    consume(TOKEN_LEFT_PAREN, "Expected '(' after container declaration!");
    if(peek() != TOKEN_RIGHT_PAREN){
        do{
            args = (uint32_t *)reallocate(args, sizeof(uint32_t) * ++argp);
            args[argp - 1] = str_insert(stringOf(consume(TOKEN_IDENTIFIER, "Expected identifer as argument!")));

            routine_add_arg(&routine, args[argp - 1]);
        } while(match(TOKEN_COMMA));
        consume(TOKEN_RIGHT_PAREN, "Expected ')' after argument declaration!");

        while(argp > 0){
            if(routine.startAddress == 0)
                routine.startAddress = ins_add(SETI);
            else
                ins_add(SETI);
            ins_add_val(args[--argp]);
        }

        memfree(args);
    }
    else
        advance();
    consume(TOKEN_NEWLINE, "Expected newline after container declaration!");

    if(routine.startAddress == 0)
        routine.startAddress = ip_get();

    inContainer++;
    blockStatement(compiler, BLOCK_FUNC);
    ins_add(PUSHID);
    ins_add_val(routine.name);
    ins_add(NEW_CONTAINER);
    inContainer--;
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
        lnerr("Bad statement %s!", head->value, tokenNames[peek()]);
        advance();
    }
}

uint32_t lastjump = 0;

void part(Compiler *c){
    if(peek() == TOKEN_EOF)
        beginStatement();
    else if(match(TOKEN_ROUTINE)){
        routineStatement(c);
    }
    else if(match(TOKEN_SET)){
        ins_set_val(lastjump, ip_get()); // Patch the last jump
        setStatement();
        ins_add(JUMP);
        lastjump = ins_add_val(0);
    }
    else if(match(TOKEN_ARRAY)){
        ins_set_val(lastjump, ip_get()); // Patch the last jump
        arrayStatement();
        ins_add(JUMP);
        lastjump = ins_add_val(0);
    }
    else if(match(TOKEN_CONTAINER)){
        containerStatement(c);
    }
    else if(match(TOKEN_NEWLINE))
        return;
    else{
        lnerr("Bad top level statement %s!", head->value, tokenNames[peek()]);
        he++;
        statement(c);
        return part(c);
    }
}

static void patchRoutines(){
    uint32_t i = 0;
    while(i < callPointer){
        Call c = calls[i];
        //dbg("i : %" PRIu32 " calladdr : %" PRIu32 " name : %s arity : %" PRIu32,
        //        i, c.callAddress, str_get(c.routineName), c.arity);
        Routine2 r = routine_get(c.routineName);
        if(r.arity != c.arity){
            lnerr("Arity mismatch for routine '%s' : Expected %" PRIu32 " Received %" PRIu32,
                    c.t, str_get(r.name), r.arity, c.arity);
            he++;
            i++;
            continue;
        }
        if(r.isNative){
            ins_set(c.callAddress - 1, CALLNATIVE);
            ins_set_val(c.callAddress, r.name);
        }
        else
            ins_set_val(c.callAddress, r.startAddress);
        i++;
    }
    memfree(calls);
}

void parse(TokenList *list){
    head = list;
    //heap_init();
    Compiler *comp = initCompiler(NULL, 0, BLOCK_NONE);
    ins_add(JUMP); // Jump to the first global instruction
    lastjump = ins_add_val(0); // The first location will tell the address of first instruction
    // to be executed, which will be either a global 'Set'/'Array',
    // or the routine 'Main'
    while(!match(TOKEN_EOF)){
        part(comp);
    }
    Routine2 r = routine_get(str_insert(strdup("Main"))); // Check if Main is defined

    uint32_t callMain = ins_add(CALL);
    ins_add_val(r.startAddress);
    ins_add_val(0);

    ins_set_val(lastjump, callMain); // After all globals statements are
    // executed, jump to the routine 'Main'
    ins_add(HALT); // After Main returns, halt the machine

    register_native_routines(); 

    patchRoutines();
    memfree(comp);
}

int hasParseError(){
    return he;
}
