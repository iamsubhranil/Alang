#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "stmt.h"
#include "expr.h"
#include "display.h"
#include "environment.h"
#include "allocator.h"
#include "io.h"
#include "interpreter.h"

#define EPSILON 0.0000000000000000000000001

static Literal resolveExpression(Expression* expression, Environment *env);
static Literal executeBlock(Block b, Environment *env);

static Literal nullLiteral = {0, LIT_NULL, {0}};

static Routine *routines = NULL, badRoutine = {-1, NULL, -1, NULL, NULL, {}};
static int routineCount = 0;
static Environment *globalEnv = NULL;

static int brk = 0, ret = 0, hasError = 0;

static Routine getRoutine(const char *identifer){
    int i = 0;
    while(i < routineCount){
        if(strcmp(routines[i].name, identifer) == 0 )
            return routines[i];
        i++;
    }
    return badRoutine;
}

static void addRoutine(Routine r){
//    printf("\n[AddRoutine] Registering %s\n", r.name);
    routineCount++;
    routines = (Routine *)reallocate(routines, sizeof(Routine)*routineCount);
    routines[routineCount - 1] = r;
}

static int isNumeric(Literal l){
    return l.type == LIT_INT || l.type == LIT_DOUBLE;
}

static Literal resolveBinary(Binary expr, Environment *env){
    Literal left = resolveExpression(expr.left, env);
    Literal right = resolveExpression(expr.right, env);
 //   printf("\n[Binary] Got %s and %s for operator %s", literalNames[left.type], literalNames[right.type], tokenNames[expr.op.type]);
    if(left.type == LIT_STRING && right.type == LIT_STRING && expr.op.type == TOKEN_PLUS){
        Literal ret = {expr.line, LIT_STRING, {0}};
        ret.sVal = (char *)mallocate(sizeof(char) * (strlen(left.sVal) + strlen(right.sVal) + 1));
        strncpy(ret.sVal, left.sVal, strlen(left.sVal));
        strcat(ret.sVal, right.sVal);
        return ret;
    }
    else if (!isNumeric(left) || !isNumeric(right)){
        printf(runtime_error("Binary operation can only be done on numerical values!"), expr.line);
        stop();
        return nullLiteral;
    }
    Literal ret = {expr.line, LIT_NULL, {0}};
    ret.line = left.line;
    if(left.type == LIT_INT && right.type == LIT_INT){
        ret.type = LIT_INT;
        switch(expr.op.type){
            case TOKEN_PLUS:
                ret.iVal = left.iVal + right.iVal;
                break;
            case TOKEN_MINUS:
                ret.iVal = left.iVal - right.iVal;
                break;
            case TOKEN_STAR:
                ret.iVal = left.iVal * right.iVal;
                break;
            case TOKEN_SLASH:
                ret.iVal = left.iVal / right.iVal;
                break;
            case TOKEN_CARET:
                ret.iVal = pow(left.iVal, right.iVal);
                break;
            case TOKEN_PERCEN:
                ret.iVal = left.iVal % right.iVal;
                break;
            default:
                break;
        }
    }
    else{
        ret.type = LIT_DOUBLE;
        double a = left.type == LIT_INT?left.iVal:left.dVal;
        double b = right.type == LIT_INT?right.iVal:right.dVal;
        switch(expr.op.type){
            case TOKEN_PLUS:
                ret.dVal = a + b;
                break;
            case TOKEN_MINUS:
                ret.dVal = a - b;
                break;
            case TOKEN_STAR:
                ret.dVal = a * b;
                break;
            case TOKEN_SLASH:
                ret.dVal = a / b;
                break;
            case TOKEN_CARET:
                ret.dVal = pow(a, b);
                break;
            case TOKEN_PERCEN:
                printf(runtime_error("%% can only be applied between two integers!"), expr.line);
                stop();
                break;
            default:
                break;
        }
    }
    return ret;
}

static Literal resolveLogical(Logical expr, Environment *env){
    Literal left = resolveExpression(expr.left, env);
    Literal right = resolveExpression(expr.right, env);
//    printf("\n[Logical] Got %s and %s for operator %s", literalNames[left.type], literalNames[right.type], tokenNames[expr.op.type]);
    if(left.type == LIT_STRING && right.type == LIT_STRING){
        Literal ret = {expr.line, LIT_LOGICAL, {0}};
        ret.line = left.line;
        switch(expr.op.type){
            case TOKEN_GREATER:
                ret.lVal = strlen(left.sVal) > strlen(right.sVal);
                break;
            case TOKEN_GREATER_EQUAL:
                ret.lVal = strlen(left.sVal) >= strlen(right.sVal);
                break;
            case TOKEN_LESS:
                ret.lVal = strlen(left.sVal) < strlen(right.sVal);
                break;
            case TOKEN_LESS_EQUAL:
                ret.lVal = strlen(left.sVal) <= strlen(right.sVal);
                break;
            case TOKEN_EQUAL_EQUAL:
                ret.lVal = strcmp(left.sVal, right.sVal) == 0?1:0;
                break;
            case TOKEN_BANG_EQUAL:
                ret.lVal = strcmp(left.sVal, right.sVal) == 0?0:1;
                break;
            default:
                printf(runtime_error("Bad logical operator between string operands!"), expr.line);
                stop();
                break;
        }
        return ret;
    }
    else if((!isNumeric(left) && left.type != LIT_LOGICAL) 
            || (!isNumeric(right) && right.type != LIT_LOGICAL)){
        printf(runtime_error("Logical expression must be performed on numeric values!"), expr.line);
        stop();
        return nullLiteral;
    }
    Literal ret;
    ret.type = LIT_LOGICAL;
    ret.line = left.line;
    double a = left.type == LIT_INT?left.iVal:left.dVal;
    double b = right.type == LIT_INT?right.iVal:right.dVal;
    switch(expr.op.type){
        case TOKEN_GREATER:
            ret.lVal = a > b;
            break;
        case TOKEN_GREATER_EQUAL:
            ret.lVal = a >= b;
            break;
        case TOKEN_LESS:
            ret.lVal = a < b;
            break;
        case TOKEN_LESS_EQUAL:
            ret.lVal = a <= b;
            break;
        case TOKEN_EQUAL_EQUAL:
            ret.lVal = fabs(a - b) <= EPSILON;
            break;
        case TOKEN_BANG_EQUAL:
            ret.lVal = fabs(a - b) > EPSILON;
            break;
        case TOKEN_AND:
            if(left.type != LIT_LOGICAL || right.type != LIT_LOGICAL){
                printf(runtime_error("'And' can only be applied over logical expressions!"), expr.line);
                stop();
            }
            ret.lVal = left.lVal & right.lVal;
            break;
        case TOKEN_OR:
            if(left.type != LIT_LOGICAL || right.type != LIT_LOGICAL){
                printf(runtime_error("'Or' can only be applied over logical expressions!"), expr.line);
                stop();
            }
            ret.lVal = left.lVal | right.lVal;
            break;
        default:
            break;
    }
    return ret;
}

static Literal resolveVariable(Variable expr, Environment *env){
    return env_get(expr.name, expr.line, env);
}

static Literal resolveArray(ArrayExpression ae, Environment *env){
    Literal index = resolveExpression(ae.index, env);
    if(index.type != LIT_INT){
        printf(runtime_error("Array index must be an integer!"), ae.line);
        stop();
    }
    return env_arr_get(ae.identifier, ae.line, index.iVal, env);
}

static Literal resolveCall(Call c, Environment *env){
    Routine r = getRoutine(c.identifer);
    //printf("\nResolving call to %s", c.identifer);
    if(r.arity == -1){
        if(strcmp(c.identifer, "Main") == 0){
            printf(error("Unable to start! Routine Main is not defined!"));
            stop();
        }
        else{
            printf(runtime_error("Routine %s is not defined!"), c.line, c.identifer);
            stop();
        }
        return nullLiteral;
    }
    if(r.arity != c.argCount){
        printf(runtime_error("Argument count mismatch for routine %s! Expected : %d Received %d!"), 
                c.line, c.identifer, r.arity, c.argCount);
        stop();
        return nullLiteral;
    }
    Environment *routineEnv = env_new(globalEnv);
    int i = 0;
    //    printf("\n[Call] Executing %s\n", r.name);
    while(i < r.arity){
        env_put(r.arguments[i], resolveExpression(c.arguments[i], env), routineEnv);
        i++;
    }
   // printf("\n[Call] Executing %s\n", r.name);
    Literal retl = executeBlock(r.code, routineEnv);
    if(ret)
        ret = 0;
    env_free(routineEnv);
    return retl;
}

static Literal resolveExpression(Expression* expression, Environment *env){
 //   printf("\nSolving expression : %s", expressionNames[expression->type]);
    if(hasError)
        return nullLiteral;
    switch(expression->type){
        case EXPR_LITERAL:
            return expression->literal;
        case EXPR_BINARY:
            return resolveBinary(expression->binary, env);
        case EXPR_LOGICAL:
            return resolveLogical(expression->logical, env);
        case EXPR_NONE:
            return nullLiteral;
        case EXPR_VARIABLE:
            return resolveVariable(expression->variable, env);
        case EXPR_ARRAY:
            return resolveArray(expression->arrayExpression, env);
        case EXPR_CALL:
            return resolveCall(expression->callExpression, env);
    }
}

static void printString(const char *s){
    int i = 0, len = strlen(s);
    //printf("\nPrinting : %s", s);
    while(i < len){
        if(s[i] == '\\' && i < (len - 1)){
            if(s[i+1] == 'n'){
                putchar('\n');
                i++;
            }
            else if(s[i+1] == 't'){
                putchar('\t');
                i++;
            }
            else if(s[i+1] == '"'){
                putchar('"');
                i++;
            }
            else
                putchar('\\');
        }
        else
            putchar(s[i]);
        i++;
    }
}

static Literal executePrint(Print p, Environment *env){
    int i = 0;
    while(i < p.argCount){
        Literal result = resolveExpression(p.expressions[i], env);
        switch(result.type){
            case LIT_NULL:
                printf("Null");
                break;
            case LIT_LOGICAL:
                printf("%s", result.lVal == 0 ? "False" : "True");
                break;
            case LIT_DOUBLE:
                printf("%g", result.dVal);
                break;
            case LIT_INT:
                printf("%ld", result.iVal);
                break;
            case LIT_STRING:
                printString(result.sVal);
                break;
        }
        i = i+1;
    }
    return nullLiteral;
}

static Literal executeStatement(Statement s, Environment *env);

static Literal executeBlock(Block b, Environment *env){
    //debug("Executing block statement");
    int num = 0;
    Literal retl = nullLiteral;
    while(num < b.numStatements){
        retl = executeStatement(b.statements[num], env);
        if(brk || ret)
            break;
        num++;
    }
    return retl;
}

static Literal executeIf(If ifs, Environment *env){
    //debug("Executing if statement");
    Literal cond = resolveExpression(ifs.condition, env);
    if(cond.type != LIT_LOGICAL){
        printf(runtime_error("Not a logical expression as condition!"), ifs.line);
        stop();
        return nullLiteral;
    }
    if(cond.lVal){
        return executeBlock(ifs.thenBranch, env);
    }
    else{
        return executeBlock(ifs.elseBranch, env);
    }
}

static Literal executeWhile(While w, Environment *env){
    //debug("Executing while statement");
    Literal cond = resolveExpression(w.condition, env);
    if(cond.type != LIT_LOGICAL){
        printf(runtime_error("Not a logical expression as condition!"), w.line);
        stop();
        return nullLiteral;
    }
    Literal retl = nullLiteral;
    while(cond.lVal){
        retl = executeBlock(w.body, env);
        if(brk){
            brk = 0;
            break;
        }
        if(ret)
            return retl;
        cond = resolveExpression(w.condition, env);
    }
    return nullLiteral;
}

static Literal executeSet(Set s, Environment *env){
    //debug("Executing set statement");
    int i = 0;
    while(i < s.count){
        Expression *id = s.initializers[i].identifer;
        if(id->type == EXPR_VARIABLE)
            env_put(id->variable.name, resolveExpression(s.initializers[i].initializerExpression, env), env);
        else if(id->type == EXPR_ARRAY){
            Literal index = resolveExpression(id->arrayExpression.index, env);
            if(index.type != LIT_INT){
                printf(runtime_error("Array index must be an integer!"), s.line);
                stop();
                return nullLiteral;
            }
            env_arr_put(id->arrayExpression.identifier, index.iVal, 
                    resolveExpression(s.initializers[i].initializerExpression, env), env);
        }
        else{
            printf(runtime_error("Bad assignment target!"), s.line);
            stop();
            return nullLiteral;
        }
        i++;
    }
    return nullLiteral;
}

static Literal executeArray(ArrayInit ai, Environment *env){
    int i = 0;
    while(i < ai.count){
        Expression *iden = ai.initializers[i];
        Literal init = resolveExpression(iden->arrayExpression.index, env);
        if(init.type != LIT_INT){
            printf(runtime_error("Array dimension must be an integer!"), ai.line);
            stop();
            return nullLiteral;
        }
        env_arr_new(iden->arrayExpression.identifier, ai.line, init.iVal, env);
        i++;
    }
    return nullLiteral;
}

static Literal executeInput(InputStatement is, Environment *env){
    int i = 0;
    while(i < is.count){
        Input in = is.inputs[i];
        switch(in.type){
            case INPUT_PROMPT:
                printString(in.prompt);
                break;
            case INPUT_IDENTIFER:
                {
                    char *ide = in.identifer;
                    switch(in.datatype){
                        case INPUT_ANY:
                            env_put(ide, getString(is.line), env);
                            break;
                        case INPUT_FLOAT:
                            env_put(ide, getFloat(is.line), env);
                            break;
                        case INPUT_INT:
                            env_put(ide, getInt(is.line), env);
                            break;
                    }
                }
        }
        i++;
    }
    return nullLiteral;
}

static Literal executeBreak(){
    //debug("Executing break statement");
    brk = 1;
    return nullLiteral;
}

static Literal executeEnd(){
    //debug("Executing end statement");
    memfree_all();
    exit(0);
    return nullLiteral;
}

static Literal executeBegin(){
    //debug("Executing begin statement");
    warning("Begin is a no-op!\n");
    return nullLiteral;
}

static Literal registerRoutine(Routine r){
    addRoutine(r);
    return nullLiteral;
}

static Literal executeCall(CallStatement cs, Environment *env){
    if(cs.callee->type != EXPR_CALL){
        printf(runtime_error("Expected call expression!"), cs.line);
        stop();
        return nullLiteral;
    }
    else
        return resolveCall(cs.callee->callExpression, env);
}

static Literal executeReturn(ReturnStatement rs, Environment *env){
    Literal retl = resolveExpression(rs.value, env);
    ret = 1;
    return retl;
}

static Literal executeStatement(Statement s, Environment *env){
    if(hasError)
        return nullLiteral;
    switch(s.type){
        case STATEMENT_PRINT:
            return executePrint(s.printStatement, env);
        case STATEMENT_IF:
            return executeIf(s.ifStatement, env);
        case STATEMENT_WHILE:
            return executeWhile(s.whileStatement, env);
        case STATEMENT_SET:
            return executeSet(s.setStatement, env);
        case STATEMENT_ARRAY:
            return executeArray(s.arrayStatement, env);
        case STATEMENT_INPUT:
            return executeInput(s.inputStatement, env);
        case STATEMENT_BREAK:
            return executeBreak();
        case STATEMENT_END:
            return executeEnd();
        case STATEMENT_BEGIN:
            return executeBegin();
            //       case STATEMENT_DO:
            //           executeDo(s.)
        case STATEMENT_ROUTINE:
            return registerRoutine(s.routine);
        case STATEMENT_CALL:
            return executeCall(s.callStatement, env);
        case STATEMENT_NOOP:
            break;
        case STATEMENT_RETURN:
            return executeReturn(s.returnStatement, env);
        default:
            break;
    }
}

void interpret(Code c){
    int i = 0;
    globalEnv = env_new(NULL);
    while(i < c.count && !hasError){
        executeStatement(c.parts[i], globalEnv);
        i++;
    }
    Call call;
    call.argCount = 0;
    call.identifer = strdup("Main");
    call.arguments = NULL;
    resolveCall(call, globalEnv);
}

void stop(){
    printf("\n");
    memfree_all();
    exit(1);
}
