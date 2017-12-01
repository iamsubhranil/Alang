#include <stdio.h>
#include <stdlib.h>

#include "scanner.h"
#include "parser.h"
#include "display.h"
#include "allocator.h"
#include "preprocessor.h"
#include "interpreter.h"

static void p(const char* name, size_t size){
    printf("\n%s : %lu bytes", name, size);
}

int main(int argc, char **argv){
//    printSize();
    if(argc != 2)
        return 2;
    FILE *f = fopen(argv[1], "rb");
    if(f == NULL){
        printf(error("Unable to open file %s!"), argv[1]);
        return 1;
    }

    char *string = read_all(f);

    string = preprocess(string);
    
//    printf(debug("Final source : \n%s\n"), string);

    initScanner(string);

    TokenList *tokens = scanTokens();
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
    
//    ins_print();
    freeList(tokens);
//    printf(debug("Before interpreting, realloc called : %d"), get_realloc_count());
    
    interpret();

    memfree_all();
    
//    printf(debug("After interpreting, realloc called : %d times\n"), get_realloc_count());
    return 0;
}
