#include <stdio.h>
#include <stdlib.h>

#include "display.h"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

static int hasError = 0;

int hadError(){
    return hasError;
}

void fatal(const char* msg){
    printf("\n" ANSI_COLOR_RED "[Fatal] %s" ANSI_COLOR_RESET, msg);
    exit(1);
}

void error(const char* msg){
    printf("\n" ANSI_COLOR_RED "[Error] %s" ANSI_COLOR_RESET, msg);
    exit(1);
}

void line_error(int line, const char* msg){
    printf("\n" ANSI_COLOR_RED "[Error] [Line:%d] %s" ANSI_COLOR_RESET, line, msg);
    hasError = 1;
}

void warning(const char* msg){
    printf("\n" ANSI_COLOR_YELLOW "[Warning] %s" ANSI_COLOR_RESET, msg);
}

void info(const char* msg){
    printf("\n" ANSI_COLOR_BLUE "[Info] %s" ANSI_COLOR_RESET, msg);
}

void debug(const char* msg){
    printf("\n" ANSI_COLOR_GREEN "[Debug] %s" ANSI_COLOR_RESET, msg);
}
