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


static Object resolveExpression(Expression* expression, Environment *env);
static Object executeBlock(Block b, Environment *env);

static Object nullObject = {OBJECT_NULL, {{0, LIT_NULL, {0}}}};
static int instanceCount = 0;
static Environment *globalEnv = NULL;

static int brk = 0, ret = 0;

static int isNumeric(Literal l){
    return l.type == LIT_INT || l.type == LIT_DOUBLE;
}

static Literal resolveLiteral(Expression *expression, int line, Environment *env){
    Object o = resolveExpression(expression, env);
    if(o.type != OBJECT_LITERAL){
        printf(runtime_error("Expected literal, Received %d!"), line, o.type);
        stop();
    }
    return o.literal;
}

static Object fromLiteral(Literal l){
    Object o = {OBJECT_LITERAL, {l}};
    return o;
}

static Object resolveBinary(Binary expr, Environment *env){
    Literal left = resolveLiteral(expr.left, expr.line, env);
    Literal right = resolveLiteral(expr.right, expr.line, env);
    //   printf("\n[Binary] Got %s and %s for operator %s", literalNames[left.type], literalNames[right.type], tokenNames[expr.op.type]);
    if(left.type == LIT_STRING && right.type == LIT_STRING && expr.op.type == TOKEN_PLUS){
        Literal ret = {expr.line, LIT_STRING, {0}};
        ret.sVal = (char *)mallocate(sizeof(char) * (strlen(left.sVal) + strlen(right.sVal) + 1));
        strncpy(ret.sVal, left.sVal, strlen(left.sVal));
        strcat(ret.sVal, right.sVal);
        return fromLiteral(ret);
    }
    else if (!isNumeric(left) || !isNumeric(right)){
        printf(runtime_error("Binary operation can only be done on numerical values!"), expr.line);
        stop();
        return nullObject;
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
    return fromLiteral(ret);
}

static Object compareInstance(Logical expr, Environment *env){
    Object a = resolveExpression(expr.left, env);
    Object b = resolveExpression(expr.right, env);
    if(a.type == OBJECT_INSTANCE && b.type == OBJECT_INSTANCE){
        Literal ret = {expr.line, LIT_LOGICAL, {0}};
        switch(expr.op.type){
            case TOKEN_EQUAL_EQUAL:
                ret.lVal = a.instance == b.instance;
                break;
            case TOKEN_BANG_EQUAL:
                ret.lVal = a.instance != b.instance;
                break;
            default:
                printf(runtime_error("Can't compare container instances!"), expr.line);
                stop();
                break;
        }
        return fromLiteral(ret);
    }
    else if((a.type == OBJECT_INSTANCE && b.type == OBJECT_LITERAL)
            || (a.type == OBJECT_LITERAL && b.type == OBJECT_INSTANCE)){
        Literal lit = a.type == OBJECT_LITERAL ? a.literal : b.literal;
        Instance* o = a.type == OBJECT_INSTANCE ? a.instance : b.instance;
        if(lit.type != LIT_NULL){
            printf(runtime_error("Unable to compare between literal and instances!"), expr.line);
            stop();
        }
        Literal ret = {expr.line, LIT_LOGICAL, {0}};
        switch(expr.op.type){
            case TOKEN_EQUAL_EQUAL: 
                ret.lVal = o == NULL;
                break;
            case TOKEN_BANG_EQUAL:
                ret.lVal = o != NULL;
                break;
            default:
                printf(runtime_error("Null can't be compared!"), expr.line);
                stop();
                break;
        }
        return fromLiteral(ret);
    }
    return nullObject;
}

static Object resolveLogical(Logical expr, Environment *env){
    Object r = compareInstance(expr, env);
    if(r.type != OBJECT_NULL)
        return r;
    Literal left = resolveLiteral(expr.left, expr.line, env);
    Literal right = resolveLiteral(expr.right, expr.line, env);
    //    printf("\n[Logical] Got %s and %s for operator %s", literalNames[left.type], literalNames[right.type], tokenNames[expr.op.type]);
    if(left.type == LIT_NULL || right.type == LIT_NULL){
        Literal ret = {expr.line, LIT_LOGICAL, {0}};
        switch(expr.op.type){
            case TOKEN_EQUAL_EQUAL:
                ret.lVal = left.type == LIT_NULL && right.type == LIT_NULL;
                break;
            case TOKEN_BANG_EQUAL:
                ret.lVal = left.type != LIT_NULL || right.type != LIT_NULL;
                break;
            default:
                printf(runtime_error("Unable to compare Null!"), expr.line);
                break;
        }
        return fromLiteral(ret);
    }
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
        return fromLiteral(ret);
    }
    else if((!isNumeric(left) && left.type != LIT_LOGICAL) 
            || (!isNumeric(right) && right.type != LIT_LOGICAL)){
        printf(runtime_error("Bad operand for logical operator!"), expr.line);
        stop();
        return nullObject;
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
    return fromLiteral(ret);
}

static Object resolveVariable(Variable expr, Environment *env){
    return env_get(expr.name, expr.line, env);
}

static Object resolveArray(ArrayExpression ae, Environment *env){
    Literal index = resolveLiteral(ae.index, ae.line, env);
    if(index.type != LIT_INT){
        printf(runtime_error("Array index must be an integer!"), ae.line);
        stop();
    }
    Object get = env_get(ae.identifier, ae.line, env);
    if(get.type == OBJECT_LITERAL && get.literal.type == LIT_STRING){ 
        char *s = get.literal.sVal;
        long le = strlen(s);
        long in = index.lVal;
        if(in < 1 || in > (le+1)){
            printf(runtime_error("String index out of range [%ld]!"), ae.line, in);
            stop();
        }
        if(in == le+1)
            return nullObject;
        char c = s[in - 1];
        char *cs = (char *)mallocate(sizeof(char) * 2);
        cs[0] = c;
        cs[1] = '\0';
        Literal l;
        l.type =  LIT_STRING;
        l.sVal = cs;
        return fromLiteral(l);
    }
    return env_arr_get(ae.identifier, ae.line, index.iVal, env);
}

static Object resolveRoutineCall(Call c, Environment *env){
    Routine r = env_routine_get(c.identifer, c.line, globalEnv);
    //printf("\nResolving call to %s", c.identifer);
    if(r.arity != c.argCount){
        printf(runtime_error("Argument count mismatch for routine %s! Expected : %d Received %d!"), 
                c.line, c.identifer, r.arity, c.argCount);
        stop();
        return nullObject;
    }
    Environment *routineEnv = env_new(globalEnv);
    int i = 0;
    // printf("\n[Call] Executing %s Arity : %d\n", r.name, r.arity);
    while(i < r.arity){
        //   printf(debug("Argument %s"), r.arguments[i]);
        env_put(r.arguments[i], c.line, resolveExpression(c.arguments[i], env), routineEnv);
        i++;
    }
    // printf("\n[Call] Executing %s\n", r.name);
    Object obj = executeBlock(r.code, routineEnv);
    if(ret)
        ret = 0;
    env_free(routineEnv);
    return obj;
}

static Object resolveContainerCall(Call c, Environment *env){
    Container r = env_container_get(c.identifer, c.line, globalEnv);
    //printf("\nResolving call to %s", c.identifer); 
    if(r.arity != c.argCount){
        printf(runtime_error("Argument count mismatch for container %s! Expected : %d Received %d!"), 
                c.line, c.identifer, r.arity, c.argCount);
        stop();
        return nullObject;
    }
    Environment *containerEnv = env_new(globalEnv);
    int i = 0;
    // printf("\n[Call] Executing container %s\n", r.name);
    while(i < r.arity){
        env_put(r.arguments[i], c.line, resolveExpression(c.arguments[i], env), containerEnv);
        i++;
    }
    // printf("\n[Call] Executing %s\n", r.name);
    executeBlock(r.constructor, containerEnv);
    Object o;
    o.type = OBJECT_INSTANCE;
    o.instance = (Instance *)mallocate(sizeof(Instance));
    o.instance->name = r.name;
    o.instance->environment = containerEnv;
    o.instance->refCount = 0;
    o.instance->insCount = ++instanceCount;
    o.instance->fromReturn = 0;
    return o;
}

static Object resolveCall(Call c, Environment *env){
    Object callee = env_get(c.identifer, c.line, env);
    if(callee.type == OBJECT_ROUTINE)
        return resolveRoutineCall(c, env);
    else
        return resolveContainerCall(c, env);
}

static Object resolveReference(Reference ref, Environment *env){
    Object o = resolveExpression(ref.containerName, env);
    if(o.type != OBJECT_INSTANCE){
        printf(runtime_error("Invalid member reference!"), ref.line);
        stop();
    }
    return resolveExpression(ref.member, (Environment *)o.instance->environment);
}

static Object resolveExpression(Expression* expression, Environment *env){
    //   printf("\nSolving expression : %s", expressionNames[expression->type]);
    switch(expression->type){
        case EXPR_LITERAL:
            return fromLiteral(expression->literal);
        case EXPR_BINARY:
            return resolveBinary(expression->binary, env);
        case EXPR_LOGICAL:
            return resolveLogical(expression->logical, env);
        case EXPR_NONE:
            return nullObject;
        case EXPR_VARIABLE:
            return resolveVariable(expression->variable, env);
        case EXPR_ARRAY:
            return resolveArray(expression->arrayExpression, env);
        case EXPR_CALL:
            return resolveCall(expression->callExpression, env);
        case EXPR_REFERENCE:
            return resolveReference(expression->referenceExpression, env);
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

static void printLiteral(Literal result){
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
}

static void printObject(Object o){ 
    switch(o.type){
        case OBJECT_ARRAY:
            printf("<array of %d>", o.arr.count);
            break;
        case OBJECT_CONTAINER:
            printf("<container %s>", o.container.name);
            break;
        case OBJECT_INSTANCE:
            printf("<instance of container %s>", o.instance->name);
            break;
        case OBJECT_ROUTINE:
            printf("<routine %s>", o.routine.name);
            break;
        case OBJECT_NULL:
            printf("Null");
            break;
        case OBJECT_LITERAL:
            printLiteral(o.literal);
            break;
    }
}

static Object executePrint(Print p, Environment *env){
    int i = 0;
    while(i < p.argCount){
        Object o = resolveExpression(p.expressions[i], env);
        printObject(o);
        i = i+1;
    }
    return nullObject;
}

static Object executeStatement(Statement s, Environment *env);

static Object executeBlock(Block b, Environment *env){
    //debug("Executing block statement");
    int num = 0;
    Object retl = nullObject;
    while(num < b.numStatements){
        retl = executeStatement(b.statements[num], env);
        if(brk || ret)
            break;
        num++;
    }
    return retl;
}

static Object executeIf(If ifs, Environment *env){
    //debug("Executing if statement");
    Literal cond = resolveLiteral(ifs.condition, ifs.line, env);
    if(cond.type != LIT_LOGICAL){
        printf(runtime_error("Not a logical expression as condition!"), ifs.line);
        stop();
        return nullObject;
    }
    if(cond.lVal){
        return executeBlock(ifs.thenBranch, env);
    }
    else{
        return executeBlock(ifs.elseBranch, env);
    }
}

static Object executeWhile(While w, Environment *env){
    //debug("Executing while statement");
    Literal cond = resolveLiteral(w.condition, w.line, env);
    if(cond.type != LIT_LOGICAL){
        printf(runtime_error("Not a logical expression as condition!"), w.line);
        stop();
        return nullObject;
    }
    Object retl = nullObject;
    while(cond.lVal){
        retl = executeBlock(w.body, env);
        if(brk){
            brk = 0;
            break;
        }
        if(ret)
            return retl;
        cond = resolveLiteral(w.condition, w.line, env);
    }
    return nullObject;
}

static void write_array(Expression *id, Expression *initializerExpression, Environment *resEnv, 
        Environment *writeEnv, int line){
    Literal index = resolveLiteral(id->arrayExpression.index, line, resEnv);
    if(index.type != LIT_INT){
        printf(runtime_error("Array index must be an integer!"), line);
        stop();
    }
    Object get = env_get(id->arrayExpression.identifier, line, writeEnv);
    if(get.type == OBJECT_LITERAL && get.literal.type == LIT_STRING){
        Literal rep = resolveLiteral(initializerExpression, line, resEnv);
        if(index.lVal < 1){
            printf(runtime_error("String index must be positive!"), line);
            stop();
        }
        if(rep.type != LIT_STRING){
            printf(runtime_error("Bad assignment to a string!"), line);
            stop();
        }
        if(strlen(rep.sVal) > 1)
            printf(warning("[Line %d] Ignoring extra characters while assignment!"), line);
        get.literal.sVal[index.lVal - 1] = rep.sVal[0];
    }
    else
        env_arr_put(id->arrayExpression.identifier, line, index.iVal, 
                resolveExpression(initializerExpression, resEnv), writeEnv);
}

static void write_ref(Expression *id, Expression *init, Environment *resEnv, 
        Environment *writeEnv, int line){ 
    Object ref = resolveExpression(id->referenceExpression.containerName, writeEnv);
    if(ref.type != OBJECT_INSTANCE){
        printf(runtime_error("Referenced item is not an instance of a container!"), line);
        stop();
    }
    Set s;
    Expression *mem = id->referenceExpression.member;
    if(mem->type == EXPR_ARRAY){
        write_array(mem, init, resEnv, (Environment *)ref.instance->environment, line);
    }
    else if(mem->type == EXPR_VARIABLE){
        Object value = resolveExpression(init, resEnv);
        env_put(mem->variable.name, s.line, value, (Environment *)ref.instance->environment);
    }
    else if(mem->type == EXPR_REFERENCE){
        write_ref(mem, init, resEnv, (Environment *)ref.instance->environment, line);
    }
    else{
        printf(runtime_error("Bad member access for container type '%s'"), 
                s.line, ref.instance->name);
        stop();
    }
}

static Object executeSet(Set s, Environment *env){
    //debug("Executing set statement");
    int i = 0;
    while(i < s.count){
        Expression *id = s.initializers[i].identifer;
        Expression *init = s.initializers[i].initializerExpression;
        if(id->type == EXPR_VARIABLE)
            env_put(id->variable.name, s.line, resolveExpression(init, env), env);
        else if(id->type == EXPR_ARRAY){
            write_array(id, init, env, env, s.line);
        }
        else if(id->type == EXPR_REFERENCE){
            write_ref(id, init, env, env, s.line);
        }
        else{
            printf(runtime_error("Bad assignment target!"), s.line);
            stop();
            return nullObject;
        }
        i++;
    }
    return nullObject;
}

static Object executeArray(ArrayInit ai, Environment *env){
    int i = 0;
    while(i < ai.count){
        Expression *iden = ai.initializers[i];
        Literal init = resolveLiteral(iden->arrayExpression.index, ai.line, env);
        if(init.type != LIT_INT){
            printf(runtime_error("Array dimension must be an integer!"), ai.line);
            stop();
            return nullObject;
        }
        env_arr_new(iden->arrayExpression.identifier, ai.line, init.iVal, env);
        i++;
    }
    return nullObject;
}

static Object executeInput(InputStatement is, Environment *env){
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
                            env_put(ide, is.line, fromLiteral(getString(is.line)), env);
                            break;
                        case INPUT_FLOAT:
                            env_put(ide, is.line, fromLiteral(getFloat(is.line)), env);
                            break;
                        case INPUT_INT:
                            env_put(ide, is.line, fromLiteral(getInt(is.line)), env);
                            break;
                    }
                }
        }
        i++;
    }
    return nullObject;
}

static Object executeBreak(){
    //debug("Executing break statement");
    brk = 1;
    return nullObject;
}

static Object executeEnd(){
    //debug("Executing end statement");
    memfree_all();
    exit(0);
    return nullObject;
}

static Object executeBegin(){
    //debug("Executing begin statement");
    warning("Begin is a no-op!\n");
    return nullObject;
}

static Object registerRoutine(Routine r){
    env_routine_put(r, r.line, globalEnv);
    return nullObject;
}

static Object registerContainer(Container c){
    env_container_put(c, c.line, globalEnv);
    return nullObject;
}

static Object executeCall(CallStatement cs, Environment *env){
    if(cs.callee->type != EXPR_CALL){
        printf(runtime_error("Expected call expression!"), cs.line);
        stop();
    }
    else{
        Object o = resolveCall(cs.callee->callExpression, env);
        if(o.type != OBJECT_NULL)
            printf(warning("[Line %d] Ignoring return value!"), cs.line);
        if(o.type == OBJECT_INSTANCE){
            gc_obj(o);
        }
    }
    return nullObject;
}

static Object executeReturn(ReturnStatement rs, Environment *env){
    Object retl = nullObject;
    if(rs.value != NULL)
        retl = resolveExpression(rs.value,  env);
    //    printf(debug("Returing object of type %d"), retl.type);
    if(retl.type == OBJECT_INSTANCE){
        retl.instance->fromReturn = 1;
        //        printf(debug("Incrementing ref to %d of %s#%d\n"), retl.instance->refCount,
        //                retl.instance->name, retl.instance->insCount);
    }
    ret = 1;
    return retl;
}

static Object executeStatement(Statement s, Environment *env){
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
        case STATEMENT_CONTAINER:
            return registerContainer(s.container);
        case STATEMENT_CALL:
            return executeCall(s.callStatement, env);
        case STATEMENT_NOOP:
            break;
        case STATEMENT_RETURN:
            return executeReturn(s.returnStatement, env);
        default:
            break;
    }
    return nullObject;
}

void interpret(Code c){
    int i = 0;
    globalEnv = env_new(NULL);
    while(i < c.count){
        executeStatement(c.parts[i], globalEnv);
        i++;
    }
    Call call;
    call.argCount = 0;
    call.identifer = strdup("Main");
    call.arguments = NULL;
    resolveCall(call, globalEnv);
    env_free(globalEnv);
}

void stop(){
    printf("\n");
    memfree_all();
    exit(1);
}
