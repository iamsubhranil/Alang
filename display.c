#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include "display.h"
#include "interpreter.h"
#include "scanner.h"
#include "strings.h"

void dbg(const char *msg, ...) {
	printf(ANSI_COLOR_GREEN "\n[Debug] ");
	va_list args;
	va_start(args, msg);
	vprintf(msg, args);
	printf(ANSI_COLOR_RESET);
	va_end(args);
}

void info(const char *msg, ...) {
	printf(ANSI_COLOR_BLUE "\n");
	va_list args;
	va_start(args, msg);
	vprintf(msg, args);
	printf(ANSI_COLOR_RESET);
	va_end(args);
}

void err(const char *msg, ...) {
	printf(ANSI_COLOR_RED "\n[Error] ");
	va_list args;
	va_start(args, msg);
	vprintf(msg, args);
	printf(ANSI_COLOR_RESET);
	va_end(args);
}

void warn(const char *msg, ...) {
	printf(ANSI_COLOR_YELLOW "\n[Warning] ");
	va_list args;
	va_start(args, msg);
	vprintf(msg, args);
	printf("\n" ANSI_COLOR_RESET);
	va_end(args);
}

void lnerr(const char *msg, Token t, ...) {
	printf(ANSI_COLOR_RED "\n[Error] <%s:%d> ", t.fileName, t.line);
	va_list args;
	va_start(args, t);
	vprintf(msg, args);
	printf(ANSI_COLOR_RESET);
	va_end(args);
}

void lnwarn(const char *msg, Token t, ...) {
	printf(ANSI_COLOR_YELLOW "\n[Warning] <%s:%d> ", t.fileName, t.line);
	va_list args;
	va_start(args, t);
	vprintf(msg, args);
	printf("\n" ANSI_COLOR_RESET);
	va_end(args);
}

void lninfo(const char *msg, Token t, ...) {
	printf(ANSI_COLOR_BLUE "\n[Info] <%s:%d> ", t.fileName, t.line);
	va_list args;
	va_start(args, t);
	vprintf(msg, args);
	printf("\n" ANSI_COLOR_RESET);
	va_end(args);
}

void rerr(const char *msg, ...) {
	FileInfo f = fileInfo_of(ip_get() - 1);
	printf(ANSI_COLOR_RED "\n[Runtime Error] <%s:%" PRIu32 "> ",
	       str_get(f.fileName), f.line);
	va_list args;
	va_start(args, msg);
	vprintf(msg, args);
	printf("\n" ANSI_COLOR_RESET);
	va_end(args);
	stop();
}

void rwarn(const char *msg, ...) {
	FileInfo f = fileInfo_of(ip_get() - 1);
	printf(ANSI_COLOR_YELLOW "\n[Warning] <%s:%" PRIu32 "> ",
	       str_get(f.fileName), f.line);
	va_list args;
	va_start(args, msg);
	vprintf(msg, args);
	printf("\n" ANSI_COLOR_RESET);
	va_end(args);
}
