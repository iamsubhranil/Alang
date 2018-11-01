#ifndef PARSER_H
#define PARSER_H

#include "scanner.h"

#ifndef MAX_LOCALS
#define MAX_LOCALS 255
#endif

void  parse(TokenList *list);
void  parser_register_variable(const char *var);
int   hasParseError();
Token presentToken();

#endif
