#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "allocator.h"
#include "interpreter.h"
#include "strings.h"
#include "values.h"
#include "display.h"
#include "scanner.h"
#include "parser.h"

static uint8_t *instructions = NULL;
static uint32_t ip = 0, lastins = 0;

static FileInfo *fileInfos = NULL;

uint32_t ins_add(uint8_t ins){
    instructions = (uint8_t *)reallocate(instructions, 8*++ip);
    instructions[ip - 1] = ins;
    lastins = ip - 1;
    fileInfos = (FileInfo *)reallocate(fileInfos, sizeof(FileInfo)*ip);
    Token t = presentToken();
    fileInfos[ip - 1].fileName = str_insert(t.fileName);
    fileInfos[ip - 1].line = t.line;
    return ip - 1;
}

void ins_set(uint32_t mem, uint8_t ins){
    instructions[mem] = ins;
}

uint8_t ins_last(){
    return instructions[lastins];
}

uint32_t ins_add_val(uint32_t store){
    //    printf("\nStoring %lu at %lu : 0x", store, ip);
    instructions = (uint8_t *)reallocate(instructions, 8*(ip + 4));
    uint32_t i = 0;
    while(i < 4){
        instructions[ip + i] = (store >> ((3-i)*8)) & 0xff;
        //        printf("%x", instructions[ip + i]);
        i++;
    }
    ip += 4;

    //    ins_print();
    return ip - 4;
}

uint32_t ins_add_double(double d){
    uint8_t *bits = (uint8_t *)&d;
    instructions = (uint8_t *)reallocate(instructions, 8*(ip + sizeof(double)));
    uint32_t i = 0;
    while(i < sizeof(double)){
        instructions[ip + i] = bits[i];
        i++;
    }
    ip += 8;

    //    ins_print();
    return ip - 8;
}

void ins_set_val(uint32_t mem, uint32_t store){
    //    printf("\nReStoring %lu at %lu : 0x", store, mem);
    uint8_t i = 0;
    while(i < 4){
        instructions[mem + i] = (store >> ((3-i)*8)) & 0xff;
        //        printf("%x", instructions[mem + i]);
        i++;
    }
}

static inline uint32_t ins_get_val(uint32_t mem){
    uint32_t ret = instructions[mem];
    ret = (ret << 8) | instructions[mem + 1];
    ret = (ret << 8) | instructions[mem + 2];
    ret = (ret << 8) | instructions[mem + 3];
    return ret;
}

static inline double ins_get_double(uint32_t mem){
    double ret;
    uint8_t *bytes = (uint8_t *)&ret;
    bytes[0] = instructions[mem];
    bytes[1] = instructions[mem + 1];
    bytes[2] = instructions[mem + 2];
    bytes[3] = instructions[mem + 3];
    bytes[4] = instructions[mem + 4];
    bytes[5] = instructions[mem + 5];
    bytes[6] = instructions[mem + 6];
    bytes[7] = instructions[mem + 7];
    return ret;
}

uint8_t ins_get(uint32_t mem){
    return instructions[mem];
}

uint32_t ip_get(){
    return ip;
}

static const char* opString[] = {"pushf", "pushi", "pushl", "pushs", "pushid", "pushn",
    "add", "sub", "mul", "div", "pow", "mod",
    "gt", "gte", "lt", "lte", "eq", "neq", "and", "or",
    "set", "inputi", "inputs", "inputf", "print",
    "halt", "jump", "jift", "jiff", "call", "return",
    "array", "memref", "narray", "noop", "ncont",
    "memset", "aref", "aset", "awrite", 
    "calln", "seti", "pushidv"};

void ins_print(){
    uint32_t i = 0;
    printf("\n\nPrinting memory[ip : %" PRIu32 "]..\n", ip);
    while(i < ip){
        printf("%x ", instructions[i]);
        i++;
        if(i > 0 && (i % 8 == 0))
            printf("] [");
    }
    //    
    i = 0;
    printf("\nPrinting instructions..\n");
    while(i < ip){
        printf("ip %4" PRIu32 " : ", i);
        switch(instructions[i]){
            case PUSHF:
                printf("pushf %g", ins_get_double(++i));
                i += 7;
                break;
            case PUSHI:
                printf("pushi %" PRId32, ins_get_val(++i));
                i += 3;
                break;
            case PUSHIDV:
                printf("pushidv %s", str_get(ins_get_val(++i)));
                i += 3;
                break;
            case PUSHL:
                printf("pushl %s", ins_get_val(++i)==0?"false":"true");
                i += 3;
                break;
            case PUSHS:
                printf("pushs %s", str_get(ins_get_val(++i)));
                i += 3;
                break;
            case PUSHID:
                printf("pushid %s", str_get(ins_get_val(++i)));
                i += 3;
                break;
            case SETI:
                printf("seti %s", str_get(ins_get_val(++i)));
                i += 3;
                break;
            case JUMP:
                printf("jump %" PRIu32, ins_get_val(++i));
                i+=3;
                break;
            case JUMP_IF_TRUE:
                printf("jump_if_true %" PRIu32, ins_get_val(++i));
                i+=3;
                break;
            case JUMP_IF_FALSE:
                printf("jump_if_false %" PRIu32, ins_get_val(++i));
                i+=3;
                break;
            case CALL:
                printf("call %" PRIu32, ins_get_val(++i));
                i += 3;
                printf(" arity=%" PRIu32, ins_get_val(++i));
                i += 3;
                break;
            case CALLNATIVE:
                printf("calln %s", str_get(ins_get_val(++i)));
                i += 3;
                printf(" arity=%" PRIu32, ins_get_val(++i));
                i += 3;
                break;
            default:
                printf("%s", opString[instructions[i]]);
                break;
        }
        i++;
        printf("\n");
    }
}

#include "datastack.h"
#include <string.h>
#include <math.h>
#include "env.h"
#include "routines.h"
#include "callframe.h"
#include "io.h"
#include <time.h>
#include "native.h"
#include "fman.h"
#include <float.h>

static uint8_t run = 1;
static clock_t tmStart, tmEnd;
static uint64_t insExec = 0, counter[43] = {0};

void print_stat(){
    uint8_t i = 0;
    printf("\nInstruction    Execution Count");
    printf("\n===========    ===============");
    while(i < 43){
        if(counter[i] > 0){
            printf("\n%s", opString[i]);
            printf("\t%16" PRIu64, counter[i]);
        }
        i++;
    }
}

void stop(){
    tmEnd = clock()-tmStart;
    double tm = (double)tmEnd / CLOCKS_PER_SEC;
    //printf("\nRealloc called : %d times\n", get_realloc_count());
    dbg("[Interpreter] Execution time : %gs", tm);
    dbg("[Interpreter] Instructions executed : %" PRIu64, insExec);
    dbg("[Interpreter] Average execution speed : %gs", tm/insExec);
    print_stat();
    printf("\n");
    //heap_free();
    str_free();
    dStackFree();
    memfree(instructions);
    cs_free();
    free_all();
    exit(0);
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

static uint8_t init = 0;

void init_interpreter(){
    dataStack = NULL;
    sp = 0;
    stackSize = 0;
    dStackInit();
    init = 1;
}

FileInfo fileInfo_of(uint32_t ip){
    return fileInfos[ip];
}

void interpret(){
    if(init == 0)
        init_interpreter();

    cs_init();
    init_cache();

    CallFrame callFrame = cf_new();
    callFrame.env = env_new(NULL);
    callFrame.returnAddress = 0;
    callFrame.arity = 27;
    register_native(&callFrame.env);
    ip = 0;
    static const void *dispatchTable[] = {&&DO_PUSHF, &&DO_PUSHI, &&DO_PUSHL, &&DO_PUSHS, &&DO_PUSHID, &&DO_PUSHN,
        &&DO_ADD, &&DO_SUB, &&DO_MUL, &&DO_DIV, &&DO_POW, &&DO_MOD,
        &&DO_GT, &&DO_GTE, &&DO_LT, &&DO_LTE, &&DO_EQ, &&DO_NEQ, &&DO_AND, &&DO_OR,
        &&DO_SET, &&DO_INPUTI, &&DO_INPUTS, &&DO_INPUTF, &&DO_PRINT,
        &&DO_HALT, &&DO_JUMP, &&DO_JUMP_IF_TRUE, &&DO_JUMP_IF_FALSE, &&DO_CALL, &&DO_RETURN,
        &&DO_ARRAY, &&DO_MEMREF, &&DO_MAKE_ARRAY, &&DO_NOOP,
        &&DO_NEW_CONTAINER, &&DO_MEMSET, &&DO_ARRAYREF, &&DO_ARRAYSET, &&DO_ARRAYWRITE,
        &&DO_CALLNATIVE, &&DO_SETI, &&DO_PUSHIDV};

#define DISPATCH() {insExec++; \
    ++counter[instructions[ip+1]]; \
    goto *dispatchTable[instructions[++ip]];}
#define DISPATCH_WINC() {insExec++; \
    ++counter[instructions[ip]]; \
    goto *dispatchTable[instructions[ip]];}

    tmStart = clock();
    DISPATCH_WINC();
    while(1){
DO_PUSHF:
        dpushf(ins_get_double(++ip));
        ip += 7;
        DISPATCH();
DO_PUSHI:
        dpushi((int32_t)ins_get_val(++ip));
        ip += 3;
        DISPATCH();
DO_PUSHL:
        dpushl((int32_t)ins_get_val(++ip));
        ip += 3;
        DISPATCH();
DO_PUSHS:
             dpushsk(ins_get_val(++ip));
             //   printf("\n[Info] Pushing string [sp : %lu] %s", sp, str);
             ip += 3;
             //      Data *d;
             //      dpopv(d,callFrame);
             //     printf("\n[Info] Stored : %s", str_get(d->svalue));
         
         DISPATCH();
DO_PUSHID:
         dpushidk(ins_get_val(++ip));
         ip += 3;
         DISPATCH();
DO_PUSHIDV:
         
            dpush(env_get(ins_get_val(++ip), &callFrame.env, 0));
            ip += 3;
            DISPATCH();
         
DO_PUSHN:
         dpushn();
         DISPATCH();
DO_ADD:
         {
             Data d1, d2; dpop(d1); dpop(d2);
             if(isnum(d1) && isnum(d2)){
                 if(isfloat(d1) || isfloat(d2)){
                     double res = tnum(d1) + tnum(d2);
                     dpushf(res);
                     DISPATCH();
                 }

                 int32_t res = tint(d1) + tint(d2);

                 dpushi(res);
                 DISPATCH();
             }
             if(isstr(d1) && isstr(d2)){
                 size_t s1 = str_len(tstrk(d1)), s2 = str_len(tstrk(d2));
                 char *res = (char *)mallocate(sizeof(char) * (s1 + s2 + 1));
                 res[0] = '\0';
                 strcat(res, str_get(tstrk(d2)));
                 strcat(res, str_get(tstrk(d1)));
                 res[s1+s2] = '\0';
                 dpushs(res);
                 DISPATCH();
             }
             rerr("Bad operands for operator '+'!");
         }
DO_SUB:
         {
             Data d1, d2; dpop(d1); dpop(d2);
             if(isnum(d1) && isnum(d2)){
                 if(isfloat(d1) || isfloat(d2)){
                     double res = tnum(d2) - tnum(d1);
                     dpushf(res);
                     DISPATCH();
                 }

                 int32_t res = tint(d2) - tint(d1);
                 dpushi(res);
                 DISPATCH();
             }
             rerr("Bad operands for operator '-'!");
         }
DO_MUL:
         {
             Data d1, d2; dpop(d1); dpop(d2);
             if(isnum(d1) && isnum(d2)){
                 if(isfloat(d1) || isfloat(d2)){
                     double res = tnum(d2) * tnum(d1);
                     dpushf(res);
                     DISPATCH();
                 }

                 int32_t res = tint(d2) * tint(d1);
                 dpushi(res);
                 DISPATCH();
             }
             rerr("Bad operands for operator '-'!");
         }
DO_DIV:
         {
             Data d1, d2; dpop(d1); dpop(d2);
             if(isnum(d1) && isnum(d2)){
                 if(isfloat(d1) || isfloat(d2)){
                     double res = tnum(d2) / tnum(d1);
                     dpushf(res);
                     DISPATCH();
                 }

                 int32_t res = tint(d2) / tint(d1);
                 dpushi(res);
                 DISPATCH();
             }
             rerr("Bad operands for operator '-'!");
         }
DO_POW:
         {
             Data d1, d2; dpop(d1); dpop(d2);
             if(isint(d1) && isnum(d2)){
                 dpushf(pow(tnum(d2), tint(d1)));
                 DISPATCH();
             }
             rerr("Bad operands for operator '^'!");

         }
DO_MOD:
         {
             Data d1, d2; dpop(d1); dpop(d2);
             if(isint(d1) && isint(d2)){
                 dpushi(tint(d2) % tint(d1));
                 DISPATCH();
             }
             rerr("Bad operands for operator '%%'", ip);

         }
DO_GT:
         {
             Data d1, d2; dpop(d1); dpop(d2);
             if(isnum(d1) && isnum(d2)){
                 dpushl(tnum(d2) > tnum(d1));
                 DISPATCH();
             }
             if(isstr(d1) && isstr(d2)){
                 dpushl(str_len(tstrk(d2)) > str_len(tstrk(d1)));
                 DISPATCH();
             }
             rerr("Bad operands for operator '>'!");

         }
DO_GTE:
         {
             Data d1, d2; dpop(d1); dpop(d2);
             if(isnum(d1) && isnum(d2)){
                 dpushl(tnum(d2) >= tnum(d1));
                 DISPATCH();
             }

             if(isstr(d1) && isstr(d2)){
                 dpushl(str_len(tstrk(d2)) >= str_len(tstrk(d1)));
                 DISPATCH();
             }
             rerr("Bad operands for operator '>='!");

         }
DO_LT:
         {
             Data d1, d2; dpop(d1); dpop(d2);

             if(isnum(d1) && isnum(d2)){
                 dpushl(tnum(d2) < tnum(d1));
                 DISPATCH();
             }

             if(isstr(d1) && isstr(d2)){
                 dpushl(str_len(tstrk(d2)) < str_len(tstrk(d1)));
                 DISPATCH();
             }
             rerr("Bad operands for operator '<'!");

         }
DO_LTE:
         {
             Data d1, d2; dpop(d1); dpop(d2);

             if(isnum(d1) && isnum(d2)){
                 dpushl(tnum(d2) <= tnum(d1));
                 DISPATCH();
             }

             if(isstr(d1) && isstr(d2)){
                 dpushl(str_len(tstrk(d2)) <= str_len(tstrk(d1)));
                 DISPATCH();
             }

             rerr("Bad operands for operator '<='!");

         }
DO_EQ:
         {
             Data d1, d2; dpop(d1); dpop(d2);

             if(isnum(d1) && isnum(d2)){
                 //     printf("\nComparaing %g and %g : %d!", tnum(d2), tnum(d1), tnum(d2) == tnum(d1));
                 dpushl(fabs(tnum(d2) - tnum(d1)) <= FLT_EPSILON);
                 DISPATCH();
             }

             if(isnull(d1) || isnull(d2)){
                 dpushl(isnull(d1) && isnull(d2));
                 DISPATCH();
             }

             if(isstr(d1) && isstr(d2)){
                 dpushl(tstrk(d2) == tstrk(d1));
                 DISPATCH();
             }
             rerr("Bad operands for operator '=='!");

         }
DO_NEQ:
         {

             Data d1, d2; dpop(d1); dpop(d2);

             if(isnum(d1) && isnum(d2)){
                 dpushl(fabs(tnum(d2) - tnum(d1)) > FLT_EPSILON);
                 DISPATCH();
             }

             if(isnull(d1) || isnull(d2)){
                 dpushl(!(isnull(d1) && isnull(d2)));
                 DISPATCH();
             }

             if(isstr(d1) && isstr(d2)){
                 dpushl(tstrk(d1) != tstrk(d2));
                 DISPATCH();
             }
             rerr("Bad operands for operator '!='!");

         }
DO_AND:
         {
             Data d1, d2; dpop(d1); dpop(d2);
             if(islogical(d1) && islogical(d2)){
                 dpushl(tint(d1) && tint(d2));
                 DISPATCH();
             }

             rerr("Bad operands for operator 'And'!");

         }
DO_OR:
         {
             Data d1, d2; dpop(d1); dpop(d2);
             if(islogical(d1) && islogical(d2)){
                 dpushl(tint(d1) || tint(d2));
                 DISPATCH();
             }

             rerr("Bad operands for operator 'Or'!");


         }
DO_SET:
         {
             Data id, value;
             dpopv(value, callFrame);
             dpop(id); 
             if(isidentifer(id)){
                 env_put(tstrk(id), value, &callFrame.env);
                 DISPATCH();
             }

             rerr("Bad assignment target!");

         }
DO_SETI:
         {
             Data value;
             dpop(value);
             env_implicit_put(ins_get_val(++ip), value, &callFrame.env);
             ip += 3;
             DISPATCH();
         }
DO_INPUTI:
         {
             Data id;
             dpop(id);
             env_put(tstrk(id), getInt(), &callFrame.env);
             DISPATCH();         
         }
DO_INPUTS:
         {
             Data id;
             dpop(id);
             env_put(tstrk(id), getString(), &callFrame.env);
             DISPATCH();
         }
DO_INPUTF:
         {
             Data id;
             dpop(id);
             env_put(tstrk(id), getFloat(), &callFrame.env);
             DISPATCH();
         }
DO_PRINT:
         {
             //printf("\n[Info] Printing [sp : %lu]", sp);
             Data value;
             dpopv(value, callFrame);
             //printf("\nType : %d", (int)value->type);
             switch(value.type){
                 case FLOAT:
                     printf("%g", tfloat(value));
                     DISPATCH();
                 case INT:
                     printf("%" PRId32, tint(value));
                     DISPATCH();
                 case LOGICAL:
                     printf("%s", tint(value) == 0?"False":"True");
                     DISPATCH();
                 case NIL:
                     printf("Null");
                     DISPATCH();
                 case STRING:
                     printString(str_get(tstrk(value)));
                     DISPATCH();
                 case INSTANCE:
                     printf("<instance of %s#%" PRIu32 ">", str_get(value.pvalue->container_key),
                             value.pvalue->id);
                     DISPATCH();
                 case IDENTIFIER:
                     printf("<identifer %s>", str_get(tstrk(value)));
                     DISPATCH();
                 case ARR:
                     printf("<array of %" PRIu32 ">", value.numElements);
                     DISPATCH();
                 case NONE:
                     printf("<none>");
                     DISPATCH();
             }
         }
DO_HALT:
         stop(); 
DO_JUMP:
         {
             uint32_t ja = ins_get_val(++ip);
             ip = ja;
             DISPATCH_WINC();
         }
DO_JUMP_IF_TRUE:
         {
             Data c;
             uint32_t ja = ins_get_val(++ip);
             dpopv(c,callFrame); 
             if(islogical(c)){
                 if(tint(c)){
                     ip = ja;
                     DISPATCH_WINC();
                 }
                 ip += 3;
                 DISPATCH();
             }

             rerr("Illogical jump!");


         }
DO_JUMP_IF_FALSE:
         {
             Data c;
             int32_t ja = ins_get_val(++ip);
             dpopv(c,callFrame);
             if(islogical(c)){
                 if(!tint(c)){
                     ip = ja;
                     DISPATCH_WINC();
                 }
                 ip += 3;
                 //        printf("\nCond is true!");
                 DISPATCH();

             }

             rerr("Illogical jump!");


         }
DO_CALL:
         {
             //uint32_t numArg;
             uint32_t ja;
             ja = ins_get_val(++ip);
             ip += 3;
             //numArg = ins_get_val(++ip);
             ip += 4;

             cf_push(callFrame);
             CallFrame nf = cf_new();
             nf.env = env_new(cf_root_env());
             nf.returnAddress = ip + 1;

             callFrame = nf;

             ip = ja;
             DISPATCH_WINC();
         }
DO_CALLNATIVE:
         {
             uint32_t name = ins_get_val(++ip), i;
             ip += 3;
             uint32_t numArg = ins_get_val(++ip);
             ip += 3;
             cf_push(callFrame);
             CallFrame nf = cf_new();
             nf.env = env_new(cf_root_env());
             nf.returnAddress = ip + 1;

             Routine2 routine = routine_get(name);

             if(numArg > 0){
                 i = 0;
                 while(i < numArg){
                     Data d; dpop(d);
                     env_implicit_put(routine.arguments[routine.arity - i - 1], d, &nf.env);
                     i++;
                 }
             }

             callFrame = nf;

             dpush(handle_native(routine.name, &nf.env));
             goto DO_RETURN;
         }
DO_RETURN:
         {
             if(isins(dpeek()))
                 tins(dpeek())->refCount++; 

             ip = callFrame.returnAddress;
             //dbg("Returning to %lu", ip);
             cf_free(callFrame);
             callFrame = cf_pop();
             //dbg("ReStoring address : %lu", callFrame.returnAddress);
             DISPATCH_WINC();
         }
DO_ARRAY:
         {
             Data id, index;
             dpop(id); dpopv(index, callFrame);
             Data arr = env_get(tstrk(id), &callFrame.env, 0);
             if(!isarray(arr) && !isstr(arr)){
                 rerr("'%s' is not an array!"), str_get(tstrk(id));

             }
             if(isint(index)){
                 if(isarray(arr)){
                     if(tint(index) < 1 || tint(index) > arr.numElements){
                         rerr("Array index out of range : %" PRId32, tint(index));

                     }

                     dpush((arr.arr)[tint(index) - 1]);
                     DISPATCH();
                 }

                 if(tint(index) < 1 || (size_t)tint(index) > (str_len(tstrk(arr)) + 1)){
                     rerr("String index out of range for '%s' : %" PRId32 " [Expected <= %" PRIu32 "]", str_get(tstrk(arr)),
                             tint(index), str_len(tstrk(arr)));
                 }

                 if((size_t)tint(index) == str_len(tstrk(arr)) + 1){
                     dpushn();
                     DISPATCH();
                 }

                 char *c = (char *)mallocate(sizeof(char) * 2);
                 c[0] = tstr(arr)[tint(index) - 1];
                 c[1] = 0;
                 dpushs(c);
                 DISPATCH();

             }

             rerr("Array index must be an integer!");


         }
DO_MEMREF:
         {
             Data ins, mem;
             dpop(mem); dpopv(ins, callFrame);
             if(isins(ins)){
                 if(isidentifer(mem)){
                     dpush(env_get(tstrk(mem), tenv(ins), 0));
                     DISPATCH();
                 }
                 rerr("Bad identifer!");

             }
             rerr("Referenced value is not a container instance!");

         }
DO_MAKE_ARRAY:
         {
             Data size, id;
             dpop(id); dpopv(size, callFrame); 
             if(isint(size)){
                 if(isidentifer(id)){
                     if(env_get(tstrk(id), &callFrame.env, 1).type != NONE){
                         rerr("Variable '%s' is already defined!", str_get(tstrk(id)));

                     }
                     if(tint(size) > 0){
                         env_put(tstrk(id), new_array(tint(size)), &callFrame.env);
                         DISPATCH();
                     }

                     rerr("Array size must be positive!");


                 }

                 rerr("Expected array identifer!");


             }

             rerr("Array size must be integer!");

         }
DO_NOOP:
         {
             DISPATCH();
         }
DO_NEW_CONTAINER:
         {
             uint32_t name;
             dpopsk(name);
             dpush(new_ins(&callFrame.env, name));
             ip = callFrame.returnAddress;
             callFrame = cf_pop();
             DISPATCH_WINC();
         }
DO_MEMSET:
         {
             Data in, mem, data;
             dpopv(data, callFrame); dpop(mem); dpopv(in, callFrame);
             if(isins(in)){
                 if(isidentifer(mem)){
                     env_put(tstrk(mem), data, tenv(in));
                     DISPATCH();
                 }
                 rerr("Bad member reference!");

             }
             rerr("Referenced value is not a container instance!");

         }
DO_ARRAYREF:
         {
             Data index, iden, cont;
             dpop(iden); dpopv(index, callFrame); dpopv(cont, callFrame);
             if(isint(index)){
                 if(isins(cont)){
                     if(isidentifer(iden)){
                         Data arr = env_get(tstrk(iden), tenv(cont), 0);
                         if(isarray(arr)){
                             if(tint(index) > 0 && tint(index) <= arr.numElements){
                                 dpush((Data)arr.arr[tint(index)- 1]);
                                 DISPATCH();
                             }
                             rerr("Array index out of range : %" PRId32, tint(index));

                         }
                         if(isstr(arr)){
                             if(tint(index) < 1 || (size_t)tint(index) > (str_len(tstrk(arr)) + 1)){
                                 rerr("String index out of range : %" PRId32 " [Expected <= %" PRIu32 "]", 
                                         tint(index), str_len(tstrk(arr)));

                             }


                             if((size_t)tint(index) == str_len(tstrk(arr)) + 1){
                                 dpushn();
                                 DISPATCH();
                             }
                             char *c = (char *)mallocate(sizeof(char) * 2);
                             c[0] = tstr(arr)[tint(index) - 1];
                             c[1] = 0;
                             dpushs(c);
                             DISPATCH();

                         }

                         rerr("Referenced item '%s' is not an array!", tstr(iden));

                     }
                     rerr("Bad identifer");

                 }
                 rerr("Referenced value is not a container instance");

             }
             rerr("Array index must be an integer!");

         }
DO_ARRAYSET:
         {
             Data index, iden, cont, value;
             dpopv(value, callFrame); dpop(iden); dpopv(index, callFrame); dpopv(cont, callFrame);
             if(isint(index)){
                 if(isins(cont)){
                     if(isidentifer(iden)){
                         Data arr = env_get(tstrk(iden), tenv(cont), 0);
                         if(isarray(arr)){
                             if(tint(index) > 0 && tint(index) <= arr.numElements){
                                 arr.arr[tint(index)- 1] = value;
                                 DISPATCH();
                             }
                             rerr("Array index out of range : %" PRId32, tint(index));

                         }
                         if(isstr(arr)){
                             if(tint(index) < 1 || (size_t)tint(index) > str_len(tstrk(arr))){
                                 rerr("String index out of range : %" PRId32, tint(index));

                             }
                             if(!isstr(value)){
                                 rerr("Bad assignment to string!");

                             }
                             if(str_len(tstrk(value)) > 1){
                                 rwarn("Ignoring extra characters!");
                             }
                             char *s = strdup(tstr(arr));
                             s[tint(index) - 1] = tstr(value)[0];
                             env_put(tstrk(iden), new_str(s), tenv(cont));
                             DISPATCH();
                         }

                         rerr("Referenced item '%s' is not an array!", tstr(iden));

                     }
                     rerr("Bad identifer");

                 }
                 rerr("Referenced value is not a container instance");

             }
             rerr("Array index must be an integer!");

         }
DO_ARRAYWRITE:
         {
             Data index, iden, value;
             dpopv(value, callFrame); dpop(iden); dpopv(index, callFrame);
             if(isint(index)){
                 if(isidentifer(iden)){
                     Data arr = env_get(tstrk(iden), &callFrame.env, 0);
                     if(isarray(arr)){
                         if(tint(index) > 0 && tint(index) <= arr.numElements){
                             arr.arr[tint(index)- 1] = value;
                             DISPATCH();
                         }
                         rerr("Array index out of range : %" PRId32, tint(index));
                     }
                     if(isstr(arr)){
                         if(tint(index) < 1 || (size_t)tint(index) > str_len(tstrk(arr))){
                             rerr("String index out of range : %" PRId32, tint(index));
                         }
                         if(!isstr(value)){
                             rerr("Bad assignment to string!");
                         }
                         if(str_len(tstrk(value)) > 1){
                             rwarn("Ignoring extra characters!");
                         }
                         char *s = strdup(tstr(arr));
                         s[tint(index) - 1] = tstr(value)[0];
                         env_put(tstrk(iden), new_str(s), &callFrame.env);
                         DISPATCH();
                     }
                     rerr("Referenced item '%s' is not an array!"), tstr(iden);
                 }
                 rerr("Bad identifer");
             }
             rerr("Array index must be an integer![%s]", tstr(iden));
         }
    }
}
