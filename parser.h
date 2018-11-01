#ifndef PARSER_H
#define PARSER_H

#include "scanner.h"

void  parse(TokenList *list);
int   hasParseError();
Token presentToken();

#endif
