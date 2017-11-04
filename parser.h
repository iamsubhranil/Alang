#ifndef PARSER_H
#define PARSER_H

#include "scanner.h"
#include "stmt.h"

Code parse(TokenList *list);
int hasParseError();

#endif
