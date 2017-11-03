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

#define EPSILON 0.000000000000000001

static Literal resolveExpression(Expression* expression);

static Literal nullLiteral = {LIT_NULL, 0, {0}};

static int isNumeric(Literal l){
    return l.type == LIT_INT || l.type == LIT_DOUBLE;
}

static Literal resolveBinary(Binary expr){
    Literal left = resolveExpression(expr.left);
    Literal right = resolveExpression(expr.right);
    if(left.type == LIT_STRING && right.type == LIT_STRING && expr.op.type == TOKEN_PLUS){
        Literal ret = {LIT_STRING, left.line, {0}};
        ret.sVal = (char *)mallocate(sizeof(char) * (strlen(left.sVal) + strlen(right.sVal) + 1));
        strncpy(ret.sVal, left.sVal, strlen(left.sVal));
        strcat(ret.sVal, right.sVal);
        return ret;
    }
    else if (!isNumeric(left) || !isNumeric(right)){
        runtime_error(left.line, "Binary operation can only be done on numerical values!");
        return nullLiteral;
    }
    Literal ret = {LIT_NULL, 0, {0}};
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
                runtime_error(left.line, "\% can only be applied between two integers!");
                break;
            default:
                break;
        }
    }
    return ret;
}

static Literal resolveLogical(Logical expr){
    Literal left = resolveExpression(expr.left);
    Literal right = resolveExpression(expr.right);
    if(left.type == LIT_STRING && right.type == LIT_STRING){
        Literal ret = {LIT_LOGICAL, 0, {0}};
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
                runtime_error(left.line, "Bad logical operator between string operands!");
                break;
        }
        return ret;
    }
    else if((!isNumeric(left) && left.type != LIT_LOGICAL) 
            || (!isNumeric(right) && right.type != LIT_LOGICAL)){
        runtime_error(left.line, "Logical expression must be performed on numeric values!");
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
            if(left.type != LIT_LOGICAL || right.type != LIT_LOGICAL)
                runtime_error(left.line, "'And' can only be applied over logical expressions!");
            ret.lVal = left.lVal & right.lVal;
            break;
        case TOKEN_OR:
            if(left.type != LIT_LOGICAL || right.type != LIT_LOGICAL)
                runtime_error(left.line, "'Or' can only be applied over logical expressions!");
            ret.lVal = left.lVal | right.lVal;
            break;
        default:
            break;
    }
    return ret;
}

static Literal resolveVariable(Variable expr){
    return env_get(expr.name);
}

static Literal resolveArray(ArrayExpression ae){
    Literal index = resolveExpression(ae.index);
    if(index.type != LIT_INT)
        runtime_error(index.line, "Array index must be an integer!");
    return env_arr_get(ae.identifier, index.iVal);
}

static Literal resolveExpression(Expression* expression){
    switch(expression->type){
        case EXPR_LITERAL:
            return expression->literal;
        case EXPR_BINARY:
            return resolveBinary(expression->binary);
        case EXPR_LOGICAL:
            return resolveLogical(expression->logical);
        case EXPR_NONE:
            return nullLiteral;
        case EXPR_VARIABLE:
            return resolveVariable(expression->variable);
        case EXPR_ARRAY:
            return resolveArray(expression->arrayExpression);
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
            else
                putchar('\\');
        }
        else
            putchar(s[i]);
        i++;
    }
}

static void executePrint(Print p){
    int i = 0;
    while(i < p.argCount){
        Literal result = resolveExpression(p.expressions[i]);
        switch(result.type){
            case LIT_NULL:
                printf("null");
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
}

static void executeStatement(Statement s);

static void executeBlock(Block b){
    //debug("Executing block statement");
    int num = 0;
    while(num < b.numStatements){
        executeStatement(b.statements[num]);
        num++;
    }
}

static void executeIf(If ifs){
    //debug("Executing if statement");
    Literal cond = resolveExpression(ifs.condition);
    if(cond.type != LIT_LOGICAL){
        runtime_error(ifs.line, "Not a logical expression as condition!");
    }
    if(cond.lVal){
        executeBlock(ifs.thenBranch);
    }
    else{
        executeBlock(ifs.elseBranch);
    }
}

static void executeWhile(While w){
    //debug("Executing while statement");
    Literal cond = resolveExpression(w.condition);
    if(cond.type != LIT_LOGICAL){
        runtime_error(w.line, "Not a logical expression as condition!");
    }
    while(cond.lVal){
        executeBlock(w.body);
        cond = resolveExpression(w.condition);
    }
}

static void executeSet(Set s){
    //debug("Executing set statement");
    int i = 0;
    while(i < s.count){
        Expression *id = s.initializers[i].identifer;
        if(id->type == EXPR_VARIABLE)
            env_put(id->variable.name, resolveExpression(s.initializers[i].initializerExpression));
        else if(id->type == EXPR_ARRAY){
            Literal index = resolveExpression(id->arrayExpression.index);
            if(index.type != LIT_INT)
                runtime_error(index.line, "Array index must be an integer!");
            env_arr_put(id->arrayExpression.identifier, index.iVal, 
                    resolveExpression(s.initializers[i].initializerExpression));
        }
        else
            runtime_error(s.line, "Bad assignment target!");
        i++;
    }
}

static void executeArray(ArrayInit ai){
    int i = 0;
    while(i < ai.count){
        Expression *iden = ai.initializers[i];
        Literal init = resolveExpression(iden->arrayExpression.index);
        if(init.type != LIT_INT)
            runtime_error(ai.line, "Array dimension must be an integer!");
        env_arr_new(iden->arrayExpression.identifier, init.iVal);
        i++;
    }
}

static void executeInput(InputStatement is){
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
                            env_put(ide, getString());
                            break;
                        case INPUT_FLOAT:
                            env_put(ide, getFloat());
                            break;
                        case INPUT_INT:
                            env_put(ide, getInt());
                            break;
                    }
                }
        }
        i++;
    }
}

static void executeBreak(Break b){
    //debug("Executing break statement");
    warning("Break is a no-op!\n");
}

static void executeEnd(){
    //debug("Executing end statement");
    memfree_all();
    exit(0);
}

static void executeBegin(){
    //debug("Executing begin statement");
    warning("Begin is a no-op!\n");
}

static void executeStatement(Statement s){
    switch(s.type){
        case STATEMENT_PRINT:
            executePrint(s.printStatement);
            break;
        case STATEMENT_IF:
            executeIf(s.ifStatement);
            break;
        case STATEMENT_WHILE:
            executeWhile(s.whileStatement);
            break;
        case STATEMENT_SET:
            executeSet(s.setStatement);
            break;
        case STATEMENT_ARRAY:
            executeArray(s.arrayStatement);
            break;
        case STATEMENT_INPUT:
            return executeInput(s.inputStatement);
        case STATEMENT_BREAK:
            executeBreak(s.breakStatement);
            break;
        case STATEMENT_END:
            executeEnd();
            break;
        case STATEMENT_BEGIN:
            executeBegin();
            break;
            //       case STATEMENT_DO:
            //           executeDo(s.)
        default:
            break;
    }
}

void interpret(Block b){
    executeBlock(b);
}
