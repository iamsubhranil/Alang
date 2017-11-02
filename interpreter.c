#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "stmt.h"
#include "expr.h"
#include "display.h"
#include "environment.h"

#define EPSILON 0.000000000000000001

static Literal resolveExpression(Expression* expression);

static Literal nullLiteral = {LIT_NULL, 0, {0}};

static Literal resolveBinary(Binary expr){
    Literal left = resolveExpression(expr.left);
    Literal right = resolveExpression(expr.right);
    if(left.type != LIT_NUMERIC || right.type != LIT_NUMERIC){
        runtime_error(left.line, "Binary operation can only be done on numerical values!");
        return nullLiteral;
    }
    Literal ret = {LIT_NULL, 0, {0}};
    ret.line = left.line;
    ret.type = LIT_NUMERIC;
    switch(expr.op.type){
        case TOKEN_PLUS:
            ret.dVal = left.dVal + right.dVal;
            break;
        case TOKEN_MINUS:
            ret.dVal = left.dVal - right.dVal;
            break;
        case TOKEN_STAR:
            ret.dVal = left.dVal * right.dVal;
            break;
        case TOKEN_SLASH:
            ret.dVal = left.dVal / right.dVal;
            break;
        case TOKEN_CARET:
            ret.dVal = pow(left.dVal, right.dVal);
            break;
        case TOKEN_PERCEN:
            ret.dVal = (int)left.dVal % (int)right.dVal;
            break;
        default:
            break;
    }
    return ret;
}

static Literal resolveLogical(Logical expr){
    Literal left = resolveExpression(expr.left);
    Literal right = resolveExpression(expr.right);
    if((left.type != LIT_NUMERIC && left.type != LIT_LOGICAL) 
            || (right.type != LIT_NUMERIC && right.type != LIT_LOGICAL)){
        runtime_error(left.line, "Logical expression must be performed on numeric values!");
        return nullLiteral;
    }
    Literal ret;
    ret.type = LIT_LOGICAL;
    ret.line = left.line;
    switch(expr.op.type){
        case TOKEN_GREATER:
            ret.lVal = left.dVal > right.dVal;
            break;
        case TOKEN_GREATER_EQUAL:
            ret.lVal = left.dVal >= right.dVal;
            break;
        case TOKEN_LESS:
            ret.lVal = left.dVal < right.dVal;
            break;
        case TOKEN_LESS_EQUAL:
            ret.lVal = left.dVal <= right.dVal;
            break;
        case TOKEN_EQUAL_EQUAL:
            ret.lVal = fabs(left.dVal - right.dVal) <= EPSILON;
            break;
        case TOKEN_BANG_EQUAL:
            ret.lVal = fabs(left.dVal - right.dVal) > EPSILON;
            break;
        case TOKEN_AND:
            ret.lVal = left.lVal & right.lVal;
            break;
        case TOKEN_OR:
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
    }
}

static void printString(const char *s){
    int i = 0, len = strlen(s);
    while(i < len){
        if(s[i] == '\\' && i < (len - 1)){
            if(s[i+1] == 'n'){
                putchar('\n');
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
                printf("%s", result.lVal == 0 ? "false" : "true");
                break;
            case LIT_NUMERIC:
                printf("%g", result.dVal);
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
        env_put(s.initializers[i].identifer, resolveExpression(s.initializers[i].initializerExpression));
        i++;
    }
}

static void executeBreak(Break b){
    //debug("Executing break statement");
    warning("Break is a no-op!\n");
}

static void executeEnd(){
    //debug("Executing end statement");
    printf("\n");
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
