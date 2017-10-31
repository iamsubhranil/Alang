#ifndef PARSER_H
#define PARSER_H

#include "scanner.h"
#include "stmt.h"

Block parse(TokenList *list);

#endif
