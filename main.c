#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

#include "scanner.h"
#include "parser.h"
#include "display.h"
#include "allocator.h"
#include "interpreter.h"
#include "values.h"
#include "env.h"
#include "callframe.h"

#define prsz(x) dbg("Size of " #x " : %zu bytes", sizeof(x))

void printSizes(){
    prsz(Data);
    prsz(Environment);
    prsz(Record);
    prsz(Instance);
    prsz(CallFrame);
    printf("\n");
}

int main(int argc, char **argv){
    if(argc != 2)
        return 2;
    printSizes();
    TokenList *tokens = scanTokens(argv[1]);
    if(hasScanErrors()){
        err("%d errors occured while scanning. Correct them and try to run again.", hasScanErrors());
        memfree_all();
        return 1;
    }
//    printList(tokens);
    
    parse(tokens);
    if(hasParseError()){
        err("%d errors occured while parsing. Correct them and try to run again.\n", hasParseError());
        memfree_all();
        return 1;
    }
    
    //ins_print();
    freeList(tokens);
    interpret();

    memfree_all();
    return 0;
}
