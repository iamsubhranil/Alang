#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "allocator.h"
#include "callframe.h"
#include "display.h"
#include "env.h"
#include "interpreter.h"
#include "parser.h"
#include "scanner.h"
#include "values.h"

#define prsz(x) dbg("Size of " #x " : %zu bytes", sizeof(x))

void printSizes() {
	prsz(Data);
	prsz(Environment);
	prsz(Record);
	prsz(Instance);
	prsz(CallFrame);
	printf("\n");
}

int main(int argc, char **argv) {
	if(argc != 2)
		return 2;
	// printSizes();
	TokenList *tokens = scanTokens(argv[1]);
	if(hasScanErrors()) {
		err("%d errors occured while scanning. Correct them and try to run "
		    "again.",
		    hasScanErrors());
		memfree_all();
		return 1;
	}
	//    printList(tokens);

	parse(tokens);
	if(hasParseError()) {
		err("%d errors occured while parsing. Correct them and try to run "
		    "again.\n",
		    hasParseError());
		memfree_all();
		return 1;
	}
	freeList(tokens);

	// ins_print();
	interpret();

	memfree_all();
	return 0;
}
