#include <stdio.h>
#include <stdarg.h>
#include <inttypes.h>

#include "display.h"
#include "scanner.h"
#include "interpreter.h"
#include "strings.h"

void dbg(const char* msg, ...){
    printf(ANSI_COLOR_GREEN "\n[Debug] ");
    va_list args;
    va_start(args, msg);
    vprintf(msg, args);
    printf(ANSI_COLOR_RESET);
}

void info(const char* msg, ...){
    printf(ANSI_COLOR_BLUE "\n");
    va_list args;
    va_start(args, msg);
    vprintf(msg, args);
    printf(ANSI_COLOR_RESET);
}

void err(const char* msg, ...){
    printf(ANSI_COLOR_RED "\n[Error] ");
    va_list args;
    va_start(args, msg);
    vprintf(msg,args);    
    printf(ANSI_COLOR_RESET);
}

void warn(const char* msg, ...){
    printf(ANSI_COLOR_YELLOW "\n[Warning] ");
    va_list args;
    va_start(args, msg);
    vprintf(msg, args);
    printf("\n" ANSI_COLOR_RESET);
}

void lnerr(const char* msg, Token t, ...){
    printf(ANSI_COLOR_RED "\n[Error] <%s:%d> ", t.fileName, t.line);
    va_list args;
    va_start(args, t);
    vprintf(msg, args);
    printf(ANSI_COLOR_RESET);
}

void lnwarn(const char* msg, Token t, ...){
    printf(ANSI_COLOR_YELLOW "\n[Warning] <%s:%d> ", t.fileName, t.line);
    va_list args;
    va_start(args, t);
    vprintf(msg, args);
    printf("\n" ANSI_COLOR_RESET);
}

void lninfo(const char* msg, Token t, ...){
    printf(ANSI_COLOR_BLUE "\n[Info] <%s:%d> ", t.fileName, t.line);
    va_list args;
    va_start(args, t);
    vprintf(msg, args);
    printf("\n" ANSI_COLOR_RESET);
}

void rerr(const char* msg, ...){
    FileInfo f = fileInfo_of(ip_get());
    printf(ANSI_COLOR_RED "\n[Runtime Error] <%s:%" PRIu32 "> ", str_get(f.fileName), f.line);
    va_list args;
    va_start(args, msg);
    vprintf(msg, args);
    printf("\n" ANSI_COLOR_RESET);
    stop();
}

void rwarn(const char* msg, ...){
    FileInfo f = fileInfo_of(ip_get());
    printf(ANSI_COLOR_YELLOW "\n[Warning] <%s:%" PRIu32 "> ", str_get(f.fileName), f.line);
    va_list args;
    va_start(args, msg);
    vprintf(msg, args);
    printf("\n" ANSI_COLOR_RESET);
}
