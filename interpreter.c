#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>

#include "allocator.h"
#include "interpreter.h"
#include "strings.h"
#include "values.h"
#include "heap.h"

static uint8_t *instructions = NULL;
static uint64_t ip = 0;

uint64_t ins_add(uint8_t ins){
    instructions = (uint8_t *)reallocate(instructions, 8*++ip);
    instructions[ip - 1] = ins;
    return ip - 1;
}

void ins_set(uint64_t mem, uint8_t ins){
    instructions[mem] = ins;
}

uint64_t ins_add_val(uint64_t store){
//    printf("\nStoring %lu at %lu : 0x", store, ip);
    instructions = (uint8_t *)reallocate(instructions, 8*(ip + 8));
    uint64_t i = 0;
    while(i < 8){
        instructions[ip + i] = (store >> ((7-i)*8)) & 0xff;
//        printf("%x", instructions[ip + i]);
        i++;
    }
    ip += 8;
    ins_get_val(ip - 8);

    //    ins_print();
    return ip - 8;
}

void ins_set_val(uint64_t mem, uint64_t store){
//    printf("\nReStoring %lu at %lu : 0x", store, mem);
    uint64_t i = 0;
    while(i < 8){
        instructions[mem + i] = (store >> ((7-i)*8)) & 0xff;
//        printf("%x", instructions[mem + i]);
        i++;
    }
    ins_get_val(mem);
}

uint64_t ins_get_val(uint64_t mem){
    uint64_t ret = instructions[mem];
    uint64_t i = 1;
    //    printf("[Bytes : 0x%x", instructions[mem]);
    while(i < 8){
        //        printf("%x", instructions[mem + i]);
        ret = (ret << 8) | instructions[mem + i++];
    }
    //    printf(" , Returing %g from %lu] ", (double)ret, mem);
    return ret;
}

uint8_t ins_get(uint64_t mem){
    return instructions[mem];
}

uint64_t ip_get(){
    return ip;
}

void ins_print(){
    uint64_t i = 0;
    printf("\n\nPrinting memory[ip : %lu]..\n", ip);
    while(i < ip){
        printf("%x ", instructions[i]);
        i++;
        if(i > 0 && (i % 8 == 0))
            printf("] [");
    }
    //    stop();
    i = 0;
    printf("\nPrinting instructions..\n");
    while(i < ip){
        printf("ip %lu : ", i);
        switch(instructions[i]){
            case PUSHF:
                printf("pushf %g", heap_get_float(ins_get_val(++i)));
                i += 7;
                break;
            case PUSHI:
                printf("pushi %" PRId64, heap_get_int(ins_get_val(++i)));
                i += 7;
                break;
            case PUSHL:
                printf("pushl %s", heap_get_logical(ins_get_val(++i))==0?"false":"true");
                i += 7;
                break;
            case PUSHS:
                printf("pushs %s", str_get(heap_get_str(ins_get_val(++i))));
                i += 7;
                break;
            case PUSHID:
                printf("pushid %s", str_get(heap_get_str(ins_get_val(++i))));
                i += 7;
                break;
            case PUSHN:
                printf("pushn");
                break;
            case ADD:
                printf("add");
                break;
            case SUB:
                printf("sub");
                break;
            case MUL:
                printf("mul");
                break;
            case DIV:
                printf("div");
                break;
            case POW:
                printf("pow");
                break;
            case MOD:
                printf("mod");
                break;
            case GT:
                printf("gt");
                break;
            case GTE:
                printf("gte");
                break;
            case LT:
                printf("lt");
                break;
            case LTE:
                printf("lte");
                break;
            case EQ:
                printf("eq");
                break;
            case NEQ:
                printf("neq");
                break;
            case AND:
                printf("and");
                break;
            case OR:
                printf("or");
                break;
            case SET:
                printf("set");
                break;
            case INPUTI:
                printf("inputi");
                break;
            case INPUTS:
                printf("inputs");
                break;
            case INPUTF:
                printf("inputf");
                break;
            case PRINT:
                printf("print");
                break;
            case HALT:
                printf("halt");
                break;
            case JUMP:
                printf("jump");
                break;
            case JUMP_IF_TRUE:
                printf("jump_if_true");
                break;
            case JUMP_IF_FALSE:
                printf("jump_if_false");
                break;
            case CALL:
                printf("call");
                break;
            case RETURN:
                printf("return");
                break;
            case ARRAY:
                printf("array");
                break;
            case MEMREF:
                printf("memref");
                break;
            case MAKE_ARRAY:
                printf("make_array");
                break;
            case NOOP:
                printf("noop");
                break;
            default:
                printf("[Error] Unknown opcode 0x%x", instructions[i]);
                break;
        }
        i++;
        printf("\n");
    }
}

#include "datastack.h"
#include <string.h>
#include "display.h"
#include <math.h>
#include "env.h"
#include "routines.h"
#include "callframe.h"
#include "io.h"
#include <time.h>

static uint8_t run = 1;
static clock_t tmStart, tmEnd;

void stop(){
    tmEnd = clock();
    //printf("\nRealloc called : %d times\n", get_realloc_count());
    printf(debug("[Interpreter] Execution time : %gms\n"), (double)(tmEnd - tmStart)/CLOCKS_PER_SEC);
    memfree_all();
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

void interpret(){
    dataStack = NULL;
    sp = 0;
    stackSize = 0;
    dStackInit();

    CallFrame callFrame = cf_new();
    callFrame.env = env_new(NULL);
    callFrame.returnAddress = 0;
    callFrame.arity = 27;
    ip = routine_get(str_insert("Main")).startAddress;
    tmStart = clock();
    static const void *dispatchTable[35] = {&&DO_PUSHF, &&DO_PUSHI, &&DO_PUSHL, &&DO_PUSHS, &&DO_PUSHID, &&DO_PUSHN,
                                    &&DO_ADD, &&DO_SUB, &&DO_MUL, &&DO_DIV, &&DO_POW, &&DO_MOD,
                                    &&DO_GT, &&DO_GTE, &&DO_LT, &&DO_LTE, &&DO_EQ, &&DO_NEQ, &&DO_AND, &&DO_OR,
                                    &&DO_SET, &&DO_INPUTI, &&DO_INPUTS, &&DO_INPUTF, &&DO_PRINT,
                                    &&DO_HALT, &&DO_JUMP, &&DO_JUMP_IF_TRUE, &&DO_JUMP_IF_FALSE, &&DO_CALL, &&DO_RETURN,
                                    &&DO_ARRAY, &&DO_MEMREF, &&DO_MAKE_ARRAY, &&DO_NOOP};
    #define DISPATCH() {goto *dispatchTable[instructions[++ip]];}
    #define DISPATCH_WINC() {goto *dispatchTable[instructions[ip]];}

    DISPATCH_WINC();
    while(1){
            DO_PUSHF:
                dpushf(heap_get_float(ins_get_val(++ip)));
                ip += 7;
                DISPATCH();
            DO_PUSHI:
                dpushi(heap_get_int(ins_get_val(++ip)));
                ip += 7;
                DISPATCH();
            DO_PUSHL:
                dpushl(heap_get_logical(ins_get_val(++ip)));
                ip += 7;
                DISPATCH();
            DO_PUSHS:{
                           dpushsk(heap_get_str(ins_get_val(++ip)));
                           //   printf("\n[Info] Pushing string [sp : %lu] %s", sp, str);
                           ip += 7;
                           //      Data *d;
                           //      dpopv(d,callFrame);
                           //     printf("\n[Info] Stored : %s", str_get(d->svalue));
                       }
                       DISPATCH();
            DO_PUSHID:
                       
                       dpushidk(heap_get_str(ins_get_val(++ip)));
                       ip += 7;
                       DISPATCH();
            DO_PUSHN:
                       dpushn();
                       DISPATCH();
            DO_ADD:
                       {
                           Data d1, d2; dpopv(d1, callFrame); dpopv(d2, callFrame);
                           if(isstr(d1) && isstr(d2)){
                               size_t s1 = strlen(str_get(tstrk(d1))), s2 = (strlen(str_get(tstrk(d2))));
                               char *res = (char *)mallocate(sizeof(char) * (s1 + s2 + 1));
                               res[0] = '\0';
                               strcat(res, str_get(tstrk(d2)));
                               strcat(res, str_get(tstrk(d1)));
                               res[s1+s2] = '\0';
                               dpushs(res);
                               DISPATCH();
                           }
                           if(isnum(d1) && isnum(d2)){
                               uint8_t resFloat = 0;
                               if(isfloat(d1) || isfloat(d2)){
                                   double res = tnum(d1) + tnum(d2);
                                   dpushf(res);
                                   DISPATCH();
                               }

                                   int64_t res = tint(d1) + tint(d2);
                                   dpushi(res);
                               DISPATCH();
                           }
                               error("Bad operands for operator '+'!");
                               stop();
                       }
            DO_SUB:
                       {
                           Data d1, d2; dpopv(d1, callFrame); dpopv(d2, callFrame);
                           if(isnum(d1) && isnum(d2)){
                               if(isfloat(d1) || isfloat(d2)){
                                   double res = tnum(d2) - tnum(d1);
                                   dpushf(res);
                                   DISPATCH();
                               }

                                int64_t res = tint(d2) - tint(d1);
                                dpushi(res);
                                DISPATCH();
                           }
                               error("Bad operands for operator '-'!");
                               stop();
                       }
            DO_MUL:
                       {
                           Data d1, d2; dpopv(d1, callFrame); dpopv(d2, callFrame);
                           if(isnum(d1) && isnum(d2)){
                               if(isfloat(d1) || isfloat(d2)){
                                   double res = tnum(d2) * tnum(d1);
                                   dpushf(res);
                                   DISPATCH();
                               }
                               
                                   int64_t res = tint(d2) * tint(d1);
                                   dpushi(res);
                               DISPATCH();
                           }
                               error("Bad operands for operator '*'!");
                               stop();
                        }
            DO_DIV:
                       {
                           Data d1, d2; dpopv(d1, callFrame); dpopv(d2, callFrame);
                           if(isnum(d1) && isnum(d2)){
                               if(isfloat(d1) || isfloat(d2)){
                                   double res = tnum(d2) / tnum(d1);
                                   dpushf(res);
                                   DISPATCH();
                               }
                                   int64_t res = tint(d2) / tint(d1);
                                   dpushi(res);
                               DISPATCH();
                           }
                               error("Bad operands for operator '*'!");
                               stop();
                       }
            DO_POW:
                       {
                           Data d1, d2; dpopv(d1, callFrame); dpopv(d2, callFrame);
                           if(isint(d1) && isnum(d2)){
                               dpushf(pow(tnum(d2), tint(d1)));
                               DISPATCH();
                           }
                               error("Bad operands for operator '^'!");
                               stop();
                       }
            DO_MOD:
                       {
                           Data d1, d2; dpopv(d1, callFrame); dpopv(d2, callFrame);
                           if(isint(d1) && isint(d2)){
                               dpushi(tint(d2) % tint(d1));
                               DISPATCH();
                           }
                               error("Bad operands for operator '%'!");
                               stop();
                       }
            DO_GT:
                       {
                           Data d1, d2; dpopv(d1, callFrame); dpopv(d2, callFrame);
                           if(isstr(d1) && isstr(d2)){
                               dpushl(strlen(str_get(tstrk(d2))) > strlen(str_get(tstrk(d1))));
                               DISPATCH();
                           }
                           
                           if(isnum(d1) && isnum(d2)){
                               dpushl(tnum(d2) > tnum(d1));
                               DISPATCH();
                           }
                               error("Bad operands for operator '>'!");
                               stop();
                       }
            DO_GTE:
                       {
                           Data d1, d2; dpopv(d1, callFrame); dpopv(d2, callFrame);
                           if(isstr(d1) && isstr(d2)){
                               dpushl(strlen(str_get(tstrk(d2))) >= strlen(str_get(tstrk(d1))));
                               DISPATCH();
                           }
                            
                           if(isnum(d1) && isnum(d2)){
                               dpushl(tnum(d2) >= tnum(d1));
                               DISPATCH();
                           }
                               error("Bad operands for operator '>='!");
                               stop();
                       }
            DO_LT:
                       {
                           Data d1, d2; dpopv(d1, callFrame); dpopv(d2, callFrame);
                           if(isstr(d1) && isstr(d2)){
                               dpushl(strlen(str_get(tstrk(d2))) < strlen(str_get(tstrk(d1))));
                               DISPATCH();
                           }
                           
                           if(isnum(d1) && isnum(d2)){
                               dpushl(tnum(d2) < tnum(d1));
                               DISPATCH();
                           }
                               error("Bad operands for operator '<'!");
                               stop();
                       }
            DO_LTE:
                       {
                           Data d1, d2; dpopv(d1, callFrame); dpopv(d2, callFrame);
                           if(isstr(d1) && isstr(d2)){
                               dpushl(strlen(str_get(tstrk(d2))) <= strlen(str_get(tstrk(d1))));
                               DISPATCH();
                           }
                           
                           if(isnum(d1) && isnum(d2)){
                               dpushl(tnum(d2) <= tnum(d1));
                                DISPATCH();
                           }
                           
                               error("Bad operands for operator '<='!");
                               stop();
                       }
            DO_EQ:
                       {
                           Data d1, d2; dpopv(d1, callFrame); dpopv(d2, callFrame);
                           if(isstr(d1) && isstr(d2)){
                               dpushl(tstrk(d2) == tstrk(d1));
                               DISPATCH();
                           }
                           
                           if(isnum(d1) && isnum(d2)){
                               //     printf("\nComparaing %g and %g : %d!", tnum(d2), tnum(d1), tnum(d2) == tnum(d1));
                               dpushl(tnum(d2) == tnum(d1));
                               DISPATCH();
                           }
                               error("Bad operands for operator '=='!");
                               stop();
                       }
            DO_NEQ:
                       {
                           
                           Data d1, d2; dpopv(d1, callFrame); dpopv(d2, callFrame);
                           if(isstr(d1) && isstr(d2)){
                               dpushl(tstrk(d1) != tstrk(d2));
                               DISPATCH();
                           }
                           
                           if(isnum(d1) && isnum(d2)){
                               dpushl(tnum(d2) != tnum(d1));
                               DISPATCH();
                           }
                               error("Bad operands for operator '!='!");
                               stop();
                       }
            DO_AND:
                       {
                           Data d1, d2; dpopv(d1, callFrame); dpopv(d2, callFrame);
                           if(islogical(d1) && islogical(d2)){
                               dpushl(tint(d1) && tint(d2));
                               DISPATCH();
                           }

                               error("Bad operands for operator 'And'!");
                               stop();
                       }
            DO_OR:
                       {
                           Data d1, d2; dpopv(d1, callFrame); dpopv(d2, callFrame);
                           if(islogical(d1) && islogical(d2)){
                               dpushl(tint(d1) || tint(d2));
                               DISPATCH();
                           }
                        
                               error("Bad operands for operator 'Or'!");
                               stop();
                           
                       }
            DO_SET:
                       {
                           Data id, value;
                           dpopv(value, callFrame);
                           dpop(id); 
                           if(isidentifer(id)){
                               Data var = env_get(tstrk(id), &callFrame.env, 1);
                               if(isnull(var) || !isarray(var)){
                                   env_put(tstrk(id), value, &callFrame.env);
                                   DISPATCH();
                               }
                               
                                   Data index;
                                   dpopv(index, callFrame);
                                   if(isint(index)){
                                       if(tint(index) < 1 || tint(index) > var.numElements){
                                           printf(error("Array index out of range : %" PRId64), tint(index));
                                           stop();
                                       }
                                       
                                           Data *d = (Data *)var.arr;
                                           d[tint(index) - 1] = (Data)value;
                                           DISPATCH();
                                       
                                   }
                                   
                                       printf(error("Array index must be an integer!"));
                                       stop();
                                   
                               
                           }
                           
                               printf(error("Bad assignment target!"));
                               stop();
                           
                       }
            DO_INPUTI:
                       {
                           Data id;
                           dpop(id);
                           if(isidentifer(id)){
                               env_put(tstrk(id), getInt(), &callFrame.env);
                               DISPATCH();
                           }
                           
                               printf(error("Bad input target!"));
                               stop();
                
                       }
            DO_INPUTS:
                       {
                           Data id;
                           dpop(id);
                           if(isidentifer(id)){
                               env_put(tstrk(id), getString(), &callFrame.env);
                               DISPATCH();
                           }
                           
                               printf(error("Bad input target!"));
                               stop();
                        
                       }
            DO_INPUTF:
                       {
                           Data id;
                           dpop(id);
                           if(isidentifer(id)){
                               env_put(tstrk(id), getFloat(), &callFrame.env);
                                DISPATCH();
                           }
                           
                               printf(error("Bad input target!"));
                               stop();
                           
                           
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
                                   printf("%" PRId64, tint(value));
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
                                   printf("<instance of %s#%"PRIu64">", str_get(value.pvalue->container_key),
                                           value.pvalue->id);
                                   DISPATCH();
                               case IDENTIFIER:
                                   printf("<identifer %s>", str_get(tstrk(value)));
                                   DISPATCH();
                               case ARR:
                                   printf("<array of %" PRIu64 ">", value.numElements);
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
                           uint64_t ja;
                           dpopi(ja);
                           ip = ja;
                           DISPATCH_WINC();
                       }
            DO_JUMP_IF_TRUE:
                       {
                           Data c;
                           int64_t ja;
                           dpopi(ja);  dpopv(c,callFrame); 
                           if(islogical(c)){
                               if(tint(c)){
                                   ip = ja;
                                   DISPATCH_WINC();
                               }
                                    DISPATCH();
                           }
                           
                               error("Illogical jump!");
                               stop();

                       }
            DO_JUMP_IF_FALSE:
                       {
                           Data c;
                           int64_t ja;
                           dpopi(ja); dpopv(c,callFrame);
                           if(islogical(c)){
                               if(!tint(c)){
                                   ip = ja;
                                   DISPATCH_WINC();
                               }
                               
                                   //        printf("\nCond is true!");
                                   DISPATCH();
                               
                           }
                        
                               error("Illogical jump!");
                               stop();
                           
                       }
            DO_CALL:
                       {
                           int64_t numArg, i = 1;
                           Data r;
                           dpop(r); dpopi(numArg);
                           Routine2 routine = routine_get(tstrk(r));
                           if(routine.arity != numArg){
                               error("Argument count mismatch!");
                               stop();
                           }
                           CallFrame nf = cf_new();
                           nf.env = env_new(NULL);
                           nf.returnAddress = ip + 1;
                           i = 0;
                           while(i < routine.arity){
                       //               printf(debug("Defining %s!"), str_get(routine.arguments[i]));
                               Data d; dpopv(d, callFrame);
                               env_put(routine.arguments[i], d, &nf.env);
                               i++;
                           }
                           cf_push(callFrame);
                           ip = routine.startAddress;
                           callFrame = nf;
                           DISPATCH_WINC();
                       }
            DO_RETURN:
                       {
                           ip = callFrame.returnAddress;
                           if(ip == 0){
                           //    printf(debug("No parent frame to return!"));
                               stop();
                           }
                        //   printf(debug("Returning to %lu"), ip);
                           cf_free(callFrame);
                           callFrame = cf_pop();
                       //    printf("\nReStoring address : %lu", callFrame.returnAddress);
                           DISPATCH_WINC();
                       }
            DO_ARRAY:
                       {
                           Data id, index;
                           dpop(id); dpopv(index, callFrame);
                           Data arr = env_get(tstrk(id), &callFrame.env, 0);
                           if(!isarray(arr)){
                               printf(error("'%s' is not an array!"), str_get(tstrk(id)));
                               stop();
                           }
                           if(isint(index)){
                               if(tint(index) < 1 || tint(index) > arr.numElements){
                                   printf(error("Array index out of range : %" PRId64), tint(index));
                                   stop();
                               }
                            
                                   dpush(((Data *)arr.arr)[tint(index) - 1]);
                                    DISPATCH();
                           }
                           
                               printf(error("Array index must be an integer!"));
                               stop();
                           
                       }
            DO_MEMREF:
                       printf("memref(noop)");
                       DISPATCH();
            DO_MAKE_ARRAY:
                       {
                           Data size, id;
                           dpop(id); dpopv(size, callFrame); 
                           if(isint(size)){
                               if(isidentifer(id)){
                                   if(env_get(tstrk(id), &callFrame.env, 1).type != NONE){
                                       printf(error("Variable '%s' is already defined!"), str_get(tstrk(id)));
                                       stop();
                                   }
                                   if(tint(size) > 0){
                                       env_put(tstrk(id), new_array(tint(size)), &callFrame.env);
                                       DISPATCH();
                                   }
                                   
                                       printf(error("Array size must be positive!"));
                                       stop();
                                   
                               }
                               
                                   printf(error("Expected array identifer!"));
                                   stop();
                               
                           }
                        
                               printf(error("Array size must be integer!"));
                            stop();
                       }
            DO_NOOP:
                       DISPATCH();
    }
}
