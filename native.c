#include <string.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#include "stmt.h"
#include "environment.h"
#include "allocator.h"
#include "interpreter.h"
#include "display.h"

typedef Object (*handler)(Call c, Environment *env);

Literal fromInt(int i){
    Literal l;
    l.type = LIT_INT;
    l.lVal = i;
    return l;
}

static Object handle_time(Call c, Environment *env){
   time_t tim = time(NULL); 
   Object ret;
   ret.type = OBJECT_LITERAL;
   ret.literal.type = LIT_INT;
   ret.literal.iVal = tim;
//   printf("\nReturning %ld\n", tim);
   return ret;
}

static Object handle_clock(Call c, Environment *env){
    clock_t cl = clock();

    Object ret;
    ret.type = OBJECT_LITERAL;
    ret.literal.type = LIT_INT;
    ret.literal.iVal = cl;
    if(env->parent != NULL){
        Object cps;
        cps.type = OBJECT_LITERAL;
        cps.literal.type = LIT_INT;
        cps.literal.iVal = CLOCKS_PER_SEC;
        env_put(strdup("ClocksPerSecond"), c.line, cps, env->parent);
    }
//    printf("\nReturing %ld\n", cl);
    return ret;
}

static Object constant(double val){ 
    Object o;
    o.type = OBJECT_LITERAL;
    o.literal.type = LIT_DOUBLE;
    o.literal.dVal = val;
    return o;
}

static Object handle_sin(Call c, Environment *env){
    Object x = env_get("x", c.line, env);
    if(x.type != OBJECT_LITERAL || (x.literal.type != LIT_INT && x.literal.type != LIT_DOUBLE)){
        printf(runtime_error("Expected numeric value for Sin!"), c.line);
        stop();
    }
    double d = x.literal.type == LIT_INT ? x.literal.iVal : x.literal.dVal;

    return constant(sin(d));
}

static Object handle_sinh(Call c, Environment *env){
    Object x = env_get("x", c.line, env);
    if(x.type != OBJECT_LITERAL || (x.literal.type != LIT_INT && x.literal.type != LIT_DOUBLE)){
        printf(runtime_error("Expected numeric value for Sinh!"), c.line);
        stop();
    }
    double d = x.literal.type == LIT_INT ? x.literal.iVal : x.literal.dVal;

    return constant(sinh(d));
}

static Object handle_cos(Call c, Environment *env){
    Object x = env_get("x", c.line, env);
    if(x.type != OBJECT_LITERAL || (x.literal.type != LIT_INT && x.literal.type != LIT_DOUBLE)){
        printf(runtime_error("Expected numeric value for Cos!"), c.line);
        stop();
    }
    double d = x.literal.type == LIT_INT ? x.literal.iVal : x.literal.dVal;

    return constant(cos(d));
}

static Object handle_cosh(Call c, Environment *env){
    Object x = env_get("x", c.line, env);
    if(x.type != OBJECT_LITERAL || (x.literal.type != LIT_INT && x.literal.type != LIT_DOUBLE)){
        printf(runtime_error("Expected numeric value for Cosh!"), c.line);
        stop();
    }
    double d = x.literal.type == LIT_INT ? x.literal.iVal : x.literal.dVal;

    return constant(cosh(d));
}

static Object handle_tan(Call c, Environment *env){
    Object x = env_get("x", c.line, env);
    if(x.type != OBJECT_LITERAL || (x.literal.type != LIT_INT && x.literal.type != LIT_DOUBLE)){
        printf(runtime_error("Expected numeric value for Tan!"), c.line);
        stop();
    }
    double d = x.literal.type == LIT_INT ? x.literal.iVal : x.literal.dVal;

    return constant(tan(d));
}

static Object handle_tanh(Call c, Environment *env){
    Object x = env_get("x", c.line, env);
    if(x.type != OBJECT_LITERAL || (x.literal.type != LIT_INT && x.literal.type != LIT_DOUBLE)){
        printf(runtime_error("Expected numeric value for Tanh!"), c.line);
        stop();
    }
    double d = x.literal.type == LIT_INT ? x.literal.iVal : x.literal.dVal;

    return constant(tanh(d));
}

static Object handle_log(Call c, Environment *env){
    Object x = env_get("x", c.line, env);
    if(x.type != OBJECT_LITERAL || (x.literal.type != LIT_INT && x.literal.type != LIT_DOUBLE)){
        printf(runtime_error("Expected numeric value for Log!"), c.line);
        stop();
    }
    double d = x.literal.type == LIT_INT ? x.literal.iVal : x.literal.dVal;

    return constant(log(d));
}

static Object handle_log10(Call c, Environment *env){
    Object x = env_get("x", c.line, env);
    if(x.type != OBJECT_LITERAL || (x.literal.type != LIT_INT && x.literal.type != LIT_DOUBLE)){
        printf(runtime_error("Expected numeric value for Log10!"), c.line);
        stop();
    }
    double d = x.literal.type == LIT_INT ? x.literal.iVal : x.literal.dVal;

    return constant(log10(d));
}

static handler handlers[10] = {handle_time, handle_clock,
            handle_sin, handle_sinh, handle_cos, handle_cosh, handle_tan, handle_tanh,
            handle_log, handle_log10};

static const char *indices[] = {"GetTime", "Clock", "Sin", "Sinh", "Cos", "Cosh", "Tan", "Tanh", "Log", "Log10"};

Object handle_native(Call c, Environment *env){
   char *fname = c.identifer;
   int i = 0;
   while(i < sizeof(indices)){
        if(strcmp(indices[i], fname) == 0)
            return handlers[i](c, env);
        i++;
   }

    return nullObject;
}

static Routine get_routine(char *identifer, int arity){
    Routine r;
    r.isNative = 1;
    r.name = identifer;
    r.arity = arity;
    
    return r;
}

static void add_argument(Routine *r, char *argName){
    r->arity++;
    r->arguments = (char **)reallocate(r->arguments, sizeof(char *) * r->arity);
    r->arguments[r->arity - 1] = argName;
}

static Routine getSingleArgRoutine(char *name){
    Routine r = get_routine(name, 0);
    add_argument(&r, "x");
    return r;
}

static void define_cons(Environment *env){
    env_put("Math_Pi", 0, constant(acos(-1.0)), env);
    env_put("Math_E", 0, constant(M_E), env);
}

void register_native(Environment *env){
    env_routine_put(get_routine(strdup("GetTime"), 0), 0, env);
    env_routine_put(get_routine(strdup("Clock"), 0), 0, env);
    env_routine_put(getSingleArgRoutine("Sin"), 0, env);
    env_routine_put(getSingleArgRoutine("Sinh"), 0, env);
    env_routine_put(getSingleArgRoutine("Cos"), 0, env);
    env_routine_put(getSingleArgRoutine("Cosh"), 0, env);
    env_routine_put(getSingleArgRoutine("Tan"), 0, env);
    env_routine_put(getSingleArgRoutine("Tanh"), 0, env);
    env_routine_put(getSingleArgRoutine("Log"), 0, env);
    env_routine_put(getSingleArgRoutine("Log10"), 0, env);
    define_cons(env);
}
