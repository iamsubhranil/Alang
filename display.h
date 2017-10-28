#ifndef DISPLAY_H
#define DISPLAY_H

void fatal(const char* msg);
void error(const char* msg);
void warning(const char* msg);
void info(const char* msg);
void debug(const char* msg);
void line_error(int line, const char* msg);
int hadError();

#endif
