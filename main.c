#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

#include "scanner.h"
#include "parser.h"
#include "display.h"
#include "allocator.h"
#include "interpreter.h"

int main(int argc, char **argv){
    if(argc != 2)
        return 2;
    TokenList *tokens = scanTokens(argv[1]);
    if(hasScanErrors()){
        printf(error("%d errors occured while scanning. Correct them and try to run again.\n"), hasScanErrors());
        memfree_all();
        return 1;
    }
//    printList(tokens);

    parse(tokens);
    if(hasParseError()){
        printf(error("%d errors occured while parsing. Correct them and try to run again.\n"), hasParseError());
        memfree_all();
        return 1;
    }
    
    //ins_print();
    freeList(tokens);
//    printf(debug("Before interpreting, realloc called : %d"), get_realloc_count());
    
    interpret();

    memfree_all();
    
//    printf(debug("After interpreting, realloc called : %d times\n"), get_realloc_count());
    return 0;
}
