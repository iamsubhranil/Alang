#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include "display.h"
#include "interpreter.h"
#include "scanner.h"
#include "strings.h"

void dbg(const char *msg, ...) {
	printf(ANSI_COLOR_GREEN "[Debug] ");
	printf(ANSI_COLOR_RESET);
	va_list args;
	va_start(args, msg);
	vprintf(msg, args);
	va_end(args);
	printf("\n");
}

void info(const char *msg, ...) {
	printf(ANSI_COLOR_BLUE "[Info] ");
	printf(ANSI_COLOR_RESET);
	va_list args;
	va_start(args, msg);
	vprintf(msg, args);
	va_end(args);
	printf("\n");
}

void err(const char *msg, ...) {
	printf(ANSI_COLOR_RED "[Error] ");
	printf(ANSI_COLOR_RESET);
	va_list args;
	va_start(args, msg);
	vprintf(msg, args);
	va_end(args);
	printf("\n");
}

void warn(const char *msg, ...) {
	printf(ANSI_COLOR_YELLOW "[Warning] ");
	printf(ANSI_COLOR_RESET);
	va_list args;
	va_start(args, msg);
	vprintf(msg, args);
	va_end(args);
	printf("\n");
}

void lnerr(const char *msg, Token t, ...) {
	printf(ANSI_COLOR_RED "[Error] " ANSI_COLOR_RESET ANSI_FONT_BOLD
	                      "<%s:%d> " ANSI_COLOR_RESET,
	       t.fileName, t.line);
	va_list args;
	va_start(args, t);
	vprintf(msg, args);
	va_end(args);
	printf("\n");
}

void lnwarn(const char *msg, Token t, ...) {
	printf(ANSI_COLOR_YELLOW "[Warning] " ANSI_COLOR_RESET ANSI_FONT_BOLD
	                         "<%s:%d> " ANSI_COLOR_RESET,
	       t.fileName, t.line);
	va_list args;
	va_start(args, t);
	vprintf(msg, args);
	va_end(args);
	printf("\n");
}

void lninfo(const char *msg, Token t, ...) {
	printf(ANSI_COLOR_BLUE "[Info] " ANSI_COLOR_RESET ANSI_FONT_BOLD
	                       "<%s:%d> " ANSI_COLOR_RESET,
	       t.fileName, t.line);
	va_list args;
	va_start(args, t);
	vprintf(msg, args);
	va_end(args);
	printf("\n");
}

int rerr(const char *msg, ...) {
	FileInfo f = fileInfo_of(ip_get() - 1);
	printf(ANSI_COLOR_RED "\n[Runtime Error] " ANSI_COLOR_RESET ANSI_FONT_BOLD
	                      "<%s:%" PRIu32 "> " ANSI_COLOR_RESET,
	       str_get(f.fileName), f.line);
	va_list args;
	va_start(args, msg);
	vprintf(msg, args);
	va_end(args);
	printf("\n");
	print_stack_trace();
	stop();
	return 1;
}

void rwarn(const char *msg, ...) {
	FileInfo f = fileInfo_of(ip_get() - 1);
	printf(ANSI_COLOR_YELLOW "\n[Warning] " ANSI_COLOR_RESET ANSI_FONT_BOLD
	                         "<%s:%" PRIu32 "> " ANSI_COLOR_RESET,
	       str_get(f.fileName), f.line);
	va_list args;
	va_start(args, msg);
	vprintf(msg, args);
	va_end(args);
	printf("\n");
}
