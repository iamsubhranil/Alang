#include <stdio.h>
#include <stdlib.h>

#include "scanner.h"
#include "generator.h"

int main(int argc, char **argv){
    if(argc != 2)
        return 2;
    FILE *f = fopen(argv[1], "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);  //same as rewind(f);

    char *string = (char *)malloc(fsize + 1);
    fread(string, fsize, 1, f);
    fclose(f);

    string[fsize] = 0;

    initScanner(string);

    TokenList *tokens = scanTokens();

    printList(tokens);
    generate(tokens);

    freeList(tokens);

    printf("\n");
    return 0;
}
