#include <stdio.h>
#include <stdlib.h>

#include "scanner.h"
#include "parser.h"
#include "display.h"
#include "stmt.h"
#include "allocator.h"
#include "interpreter.h"

static void p(const char* name, size_t size){
    printf("\n%s : %lu bytes", name, size);
}

static void printSize(){
    p("Expression", sizeof(Expression));
    p("Statement", sizeof(Statement));
}

int main(int argc, char **argv){
//    printSize();
    if(argc != 2)
        return 2;
    FILE *f = fopen(argv[1], "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);  //same as rewind(f);

    char *string = (char *)mallocate(fsize + 1);
    fread(string, fsize, 1, f);
    fclose(f);

    string[fsize] = 0;

    initScanner(string);

    TokenList *tokens = scanTokens();
    if(hasScanErrors()){
        printf(error("%d errors occured while scanning. Correct them and try to run again.\n"), hasScanErrors());
        memfree_all();
        return 1;
    }
//    printList(tokens);

    Code all = parse(tokens);
    if(hasParseError()){
        printf(error("%d errors occured while parsing. Correct them and try to run again.\n"), hasParseError());
        memfree_all();
        return 1;
    }

    freeList(tokens);
    interpret(all);

    memfree_all();

    printf("\n");
    return 0;
}
