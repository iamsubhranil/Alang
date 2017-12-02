#include <stdio.h>
#include <stdarg.h>
#include <inttypes.h>

#include "display.h"
#include "scanner.h"
#include "interpreter.h"
#include "strings.h"

static void format(va_list args, const char *msg){
    int i = 0;
    while(msg[i] != 0){
        if(msg[i] == '%'){
            int j = i + 1;
            if(msg[j] == 0){
                printf("%s", "%");
                continue;
            }
            else if(msg[j] == 'd')
                printf("%d", va_arg(args, int));
            else if(msg[j] == 'f')
                printf("%f", va_arg(args, double));
            else if(msg[j] == 's')
                printf("%s", va_arg(args, char *));
            else if(msg[j] == 'x')
                printf("%x", va_arg(args, int));
            else if(msg[j] == 'g')
                printf("%g", va_arg(args, double));
            else if(msg[j] == 'l'){
                if(msg[++j] == 'd')
                    printf("%ld", va_arg(args, long));
                else if(msg[j] == 'f')
                    printf("%lf", va_arg(args, double));
                else if(msg[j] == 'u')
                    printf("%lu", va_arg(args, unsigned long));
            }
            else if(msg[j] == 'u'){
                printf("%u", va_arg(args, unsigned int)); 
            }
            else if(msg[j] == '%')
                printf("%s", "%");
            i++;
        }
        else
            printf("%c", msg[i]);
        i++;
    }
}

void lnerr(const char* msg, Token t, ...){
    printf(ANSI_COLOR_RED "\n[Error] <%s:%d> ", t.fileName, t.line);
    va_list args;
    va_start(args, t);
    format(args, msg);
    printf("\n" ANSI_COLOR_RESET);
}

void lnwarn(const char* msg, Token t, ...){
    printf(ANSI_COLOR_YELLOW "\n[Warning] <%s:%d> ", t.fileName, t.line);
    va_list args;
    va_start(args, t);
    format(args, msg);
    printf("\n" ANSI_COLOR_RESET);
}

void lninfo(const char* msg, Token t, ...){
    printf(ANSI_COLOR_BLUE "\n[Info] <%s:%d> ", t.fileName, t.line);
    va_list args;
    va_start(args, t);
    format(args, msg);
    printf("\n" ANSI_COLOR_RESET);
}

void rerr(const char* msg, ...){
    FileInfo f = info_of(ip_get());
    printf(ANSI_COLOR_RED "\n[Runtime Error] <%s:%" PRIu32 "> ", str_get(f.fileName), f.line);
    va_list args;
    va_start(args, msg);
    format(args, msg);
    printf("\n" ANSI_COLOR_RESET);
    stop();
}

void rwarn(const char* msg, ...){
    FileInfo f = info_of(ip_get());
    printf(ANSI_COLOR_YELLOW "\n[Warning] <%s:%" PRIu32 "> ", str_get(f.fileName), f.line);
    va_list args;
    va_start(args, msg);
    format(args, msg);
    printf("\n" ANSI_COLOR_RESET);
    stop();
}
