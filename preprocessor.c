#include <stdio.h>
#include <string.h>

#include "allocator.h"
#include "display.h"

#include "preprocessor.h"

static char **inclusionTable = NULL;
static int count = 0;

static char* readline(const char *source, int *pointer){
    char *line = NULL;
    int i = 0;
    while(!(source[*pointer] == '\r' || source[*pointer] == '\n')){
        i++;
        line = (char *)reallocate(line, sizeof(char) * i);
        line[i - 1] = source[*pointer];
        *pointer = *pointer + 1;
    }
    if(source[*pointer] == '\r')
        *pointer = *pointer + 1;
    *pointer = *pointer + 1;
    
    i += 2;
    line = (char *)reallocate(line, sizeof(char) * i);
    line[i - 1] = '\0';
    line[i - 2] = '\n';

    return line;
}

static int startsWith(const char *source, const char *predicate){
    int slen = strlen(source), plen = strlen(predicate), i = 0;
    if(slen < plen)
        return 0;
    
    while(i < plen){
        if(source[i] != predicate[i])
            return 0;
        i++;
    }

    return 1;
}

static char* str_part(const char* source, int start){
    int i = 0;
    char *part = NULL;
    while(source[start + i] != '\n' && source[start + i] != '\r' && source[start + i] != '\0'){
//        printf(debug("Current char : [%c] i : %d"), source[start+i], i);
        part = (char *)reallocate(part, sizeof(char) * (i + 1));
        part[i] = source[start + i];
        i++;
    }

    i++;
    part = (char *)reallocate(part, sizeof(char) * i);
    part[i - 1] = '\0';

//    printf(debug("Parted for inclusion : [%s]"), part);

    return part;
}

char* read_all(FILE* source){ 
    fseek(source, 0, SEEK_END);
    long fsize = ftell(source);
    fseek(source, 0, SEEK_SET);

    char *string = (char *)mallocate(fsize + 1);
    fread(string, fsize, 1, source);
    fclose(source);

    string[fsize] = 0;

    return string;
}

static void add_to_table(char *source){
    count++;
    inclusionTable = (char **)reallocate(inclusionTable, sizeof(char *)*count);
    inclusionTable[count - 1] = source;
}

static int has_in_table(const char *source){
    int i = 0;
    while(i < count){
        if(strcmp(inclusionTable[i], source) == 0)
            return 1;
        i++;
    }
    return 0;
}

static char* str_append(const char *front, const char *back){
    char *new_str = (char *)mallocate(strlen(front)+strlen(back)+2);
    new_str[0] = '\0';   // ensures the memory is an empty string
    strcat(new_str, front);
    strcat(new_str, back);
    return new_str;
}

static int isEmpty(const char *source){
    int i = 0, j = strlen(source);
    while(i < j && (source[i] == ' ' || source[i] == '\t'))
        i++;
    if(i == j || source[i] == '\r' || source[i] == '\n')
        return 1;
    else if(i < (j - 1) && source[i] == '/' && source[i + 1] == '/'){
        return 1;
    }
    return 0;
}

static int is_start_of_mc(const char *source){
    int i = 0, j = strlen(source);
    while(i < j && (source[i] == ' ' || source[i] == '\t'))
        i++;
    if(i < (j - 1) && source[i] == '/' && source[i+1] == '*')
        return 1;
    return 0;
}

static int is_end_of_mc(const char *source){
    int j = strlen(source);
    if(j > 2 && source[j - 2] == '/' && source[j - 3] == '*')
        return 1;
    return 0;
}

char* preprocess(char *source){
    int i = 0, lcount = 0, hasInclude = 0;
    char *result = (char *)mallocate(sizeof(char) * 1), *bak;
    result[0] = '\0';
    while(source[i] != '\0'){
        char *line = readline(source, &i);
     //   printf(debug("Line : [%s]"), line);
        lcount++;
        if(startsWith(line, "Include")){
            char *toInclude = str_part(line, 8); // Include
            if(has_in_table(toInclude)){
                printf(debug("[Preprocessor] Skipping duplicate inclusion of %s"), toInclude);
                continue;
            }
            FILE *include = fopen(toInclude, "rb");
            if(include == NULL)
                printf(warning("[Line %d] Unable to open file for inclusion : %s"), lcount, toInclude);
            else{
                hasInclude++;
                char *content = read_all(include);
                add_to_table(toInclude);
                bak = result;
                result = str_append(result, content);
                memfree(content);
                memfree(bak);
            }
        }
        else if(!isEmpty(line)){
            if(is_start_of_mc(line)){
                while(!is_end_of_mc(line)){
                    memfree(line);
                    line = readline(source, &i);
                }
            }
            else{
                bak = result;
                result = str_append(result, line);
                memfree(bak);
            }
        }
        memfree(line);
    }
    if(hasInclude){
        bak = result;
        result = preprocess(result);
        memfree(bak);
    }


    return result;
}
