#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "allocator.h"
#include "display.h"
#include "interpreter.h"
#include "native.h"
#include "parser.h"
#include "routines.h"
#include "scanner.h"
#include "strings.h"
#include "values.h"

static int inWhile = 0, inContainer = 0, inCall = 0, inReturn = 0, keepID = 0,
           canDeclare            = 0;
static int        he             = 0;
static uint32_t **breakAddresses = NULL, breakLevel = 0, *breakCount = NULL;
static TokenList *head       = NULL;
static Token      errorToken = {TOKEN_ERROR, "BadToken", 0, -1, NULL};

static void break_push_scope() {
	breakAddresses                 = (uint32_t **)reallocate(breakAddresses,
                                             sizeof(uint32_t *) * ++breakLevel);
	breakAddresses[breakLevel - 1] = NULL;
	breakCount = (uint32_t *)reallocate(breakCount, 32 * breakLevel);
	breakCount[breakLevel - 1] = 0;
}

static void break_add_address(uint32_t address) {
	breakAddresses[breakLevel - 1] = (uint32_t *)reallocate(
	    breakAddresses[breakLevel - 1], 32 * ++breakCount[breakLevel - 1]);
	breakAddresses[breakLevel - 1][breakCount[breakLevel - 1] - 1] = address;
}

static uint32_t *break_peek_scope() {
	if(breakLevel == 0)
		return NULL;
	return breakAddresses[breakLevel - 1];
}

static void break_pop_scope() {
	if(breakLevel == 0)
		return;
	--breakLevel;
	memfree(breakAddresses[breakLevel]);
	breakAddresses = (uint32_t **)reallocate(breakAddresses,
	                                         sizeof(uint32_t *) * breakLevel);
	breakCount     = (uint32_t *)reallocate(breakCount, 32 * breakLevel);
}

typedef struct {
	uint32_t routineName;
	uint32_t arity;
	uint32_t callAddress;
	Token    t;
} Call;

static Call *   calls       = NULL;
static uint32_t callPointer = 0;

typedef enum {
	BLOCK_IF,
	BLOCK_ELSE,
	BLOCK_WHILE,
	BLOCK_FUNC,
	BLOCK_NONE
} BlockType;

typedef struct Compiler {
	struct Compiler *parent;
	int              indentLevel;
	BlockType        blockName;
	uint32_t *       variables;
	uint32_t         varSize;
	uint8_t          ownVariables;
} Compiler;

static Compiler *presentCompiler = NULL;

static Compiler *initCompiler(Compiler *parent, int indentLevel, BlockType name,
                              uint8_t ownVariables) {
	Compiler *ret     = (Compiler *)mallocate(sizeof(Compiler));
	ret->parent       = parent;
	ret->indentLevel  = indentLevel;
	ret->blockName    = name;
	ret->variables    = NULL;
	ret->varSize      = 0;
	ret->ownVariables = ownVariables;
	return ret;
}

static uint32_t compiler_declare_variable(Compiler *compiler, uint32_t name) {
	if(compiler->varSize == MAX_LOCALS) {
		lnerr("More than %d variables in one scope in not allowed!",
		      presentToken(), MAX_LOCALS);
		he++;
	}
	compiler->varSize++;
	compiler->variables = (uint32_t *)reallocate(
	    compiler->variables, sizeof(uint32_t) * compiler->varSize);
	compiler->variables[compiler->varSize - 1] = name;
	Compiler *tmp                              = compiler;
	while(!tmp->ownVariables && tmp->parent != NULL) {
		tmp->parent->variables = tmp->variables;
		tmp                    = tmp->parent;
	}
	return compiler->varSize - 1;
}

static uint8_t compiler_has_variable(Compiler *compiler, uint32_t name) {
	// dbg("[has] Searching for %s in %p variables %p\n", str_get(name),
	// compiler, compiler->variables);
	for(uint8_t i = 0; i < compiler->varSize; i++) {
		if(compiler->variables[i] == name)
			return 1;
	}
	return 0;
}

static uint32_t compiler_get_variable(Compiler *compiler, uint32_t name) {
	// dbg("Searching for %s in %p : ", str_get(name), compiler);
	for(uint32_t i = 0; i < compiler->varSize; i++) {
		if(compiler->variables[i] == name) {
			// printf("found\n");
			return i;
		}
	}
	// printf("not found\n");
	return 256;
}

static uint32_t compiler_get_global(Compiler *compiler, uint32_t name,
                                    uint8_t isSilent) {
	// dbg("Retrieving global %s for %p parent %p\n", str_get(name), compiler,
	//    compiler->parent);
	if(compiler == NULL || compiler->parent == NULL) {
		if(!isSilent) {
			lnerr("Variable not declared in any parent scope : %s",
			      presentToken(), str_get(name), compiler);
			he++;
		} else
			return 256;
	} else if(compiler_has_variable(compiler->parent, name)) {
		return compiler_get_variable(compiler->parent, name);
	} else {
		return compiler_get_global(compiler->parent, name, isSilent);
	}
	return 256;
}

static TokenType peek() {
	if(head == NULL)
		return TOKEN_EOF;
	return head->value.type;
}

Token presentToken() {
	if(head == NULL)
		return errorToken;
	return head->value;
}

static Token advance() {
	if(head->next != NULL) {
		Token h = head->value;
		head    = head->next;
		// printf("\n[Info:Advance] Advancing to [%s]",
		// tokenNames[head->value.type]);
		return h;
	}
	return errorToken;
}

static int match(TokenType type) {
	if(peek() == type) {
		advance();
		return 1;
	}
	return 0;
}

static void synchronize() {
	while(peek() != TOKEN_NEWLINE && peek() != TOKEN_EOF) advance();
	if(peek() == TOKEN_EOF)
		lnerr("Unexpected end of input!", head->value);
	else
		head = head->next;
}

static Token consume(TokenType type, const char *err) {
	// printf("\n[Info] Have %s expected %s", tokenNames[peek()],
	// tokenNames[type]);
	if(peek() == type) {
		// printf("\n[Info:Consume] Consuming [%s]", tokenNames[type]);
		return advance();
	} else {
		lnerr(err, head->value);
		he++;
		synchronize();
	}
	return errorToken;
}

static void consumeIndent(int indentLevel) {
	while(indentLevel > 0) {
		consume(TOKEN_INDENT, "Expected indent!");
		indentLevel--;
	}
}

static int getNextIndent() {
	TokenList *temp       = head;
	int        nextIndent = 0;
	while(temp->value.type == TOKEN_INDENT) {
		temp = temp->next;
		nextIndent++;
	}
	//    printf(debug("NextIndent : %d"), nextIndent);
	return nextIndent;
}

static void expression();

static char *numericString(Token t) {
	char *s = (char *)mallocate(sizeof(char) * t.length + 1);
	strncpy(s, t.start, t.length);
	s[t.length] = '\0';
	return s;
}

static char *stringOf(Token t) {
	if(t.type == TOKEN_NUMBER || t.type == TOKEN_IDENTIFIER)
		return numericString(t);
	//    printf("%s %d\n", t.start, t.length);
	char *s = (char *)mallocate(sizeof(char) * (t.length - 1));
	int   i;
	for(i = 1; i < t.length - 1; i++) s[i - 1] = t.start[i];
	s[t.length - 2] = '\0';
	// printf("\nGiven : %s of size %d \nReturing : [%s] of size %lu", t.start,
	// t.length, s, strlen(s));
	return s;
}
/*
   static int isDouble(const char *string){
   int i = 0;
   while(string[i] != '\0'){
   if(string[i] == '.')
   return 1;
   i++;
   }
   return 0;
   }
   */
static double doubleOf(const char *s) {
	double d;
	sscanf(s, "%lf", &d);
	return d;
}

/*
   static int32_t longOf(const char *s){
   if(strlen(s) > 10){
   lnerr("Integer overflow : %s > %" PRId32, presentToken(), s, INT32_MAX);
   he++;
   }
   int64_t l;
   sscanf(s,"%" SCNd64, &l);
   if(l > INT32_MAX){
   lnerr("Integer overflow : %" PRId64 " > %" PRId32, presentToken(), l,
   INT32_MAX); he++;
   }
   return (int32_t)l;
   }
   */
static uint8_t insFromToken(Token t) {
	switch(t.type) {
		case TOKEN_CARET: return POW;
		case TOKEN_STAR: return MUL;
		case TOKEN_SLASH: return DIV;
		case TOKEN_PLUS: return ADD;
		case TOKEN_MINUS: return SUB;
		case TOKEN_PERCEN: return MOD;
		case TOKEN_EQUAL_EQUAL: return EQ;
		case TOKEN_BANG_EQUAL: return NEQ;
		case TOKEN_GREATER: return GT;
		case TOKEN_GREATER_EQUAL: return GTE;
		case TOKEN_LESS: return LT;
		case TOKEN_LESS_EQUAL: return LTE;
		case TOKEN_AND: return AND;
		case TOKEN_OR: return OR;
		default: return NOOP;
	}
}

static uint32_t getCall() {
	uint32_t argCount = 0;
	int      bak      = keepID;
	keepID            = 0;
	if(!match(TOKEN_RIGHT_PAREN)) {
		do {
			expression();
			argCount++;
		} while(match(TOKEN_COMMA));
		consume(TOKEN_RIGHT_PAREN, "Expected ')' after expression!");
	}
	keepID = bak;
	return argCount;
}

static void compiler_resolve_variable(Compiler *compiler, uint32_t st) {
	uint8_t specific = 0; // denotes whether to declare specific LOAD
	                      // instruction or not
	if(compiler_has_variable(compiler,
	                         st)) { // Check whether the variable
		                            // is declared locally
		uint32_t varid = compiler_get_variable(compiler, st);
		if(compiler->blockName == BLOCK_NONE) {
			ins_add(LOAD_SLOT_GLOBAL);
			ins_add_val(varid);
		} else {
			if(varid < 8) {
				ins_add(LOAD_SLOT_0 + varid);
				specific = 1;
			} else {
				ins_add(LOAD_SLOT);
				ins_add_val(varid);
			}
		}
	} else { // Local variable not found, let's try globally
		uint32_t slot = compiler_get_global(compiler, st, canDeclare);
		if(slot == 256) { // Even globally the variable not found
			slot = 0;
			if(canDeclare) { // Let's check if we can declare it now
				slot = compiler_declare_variable(compiler, st);
				if(compiler->blockName == BLOCK_NONE) {
					ins_add(LOAD_SLOT_GLOBAL);
				} else if(slot < 8) {
					ins_add(LOAD_SLOT_0 + slot);
					specific = 1;
				} else
					ins_add(LOAD_SLOT);
			}
		} else // Variable found globally
			ins_add(LOAD_SLOT_GLOBAL);
		if(!specific)
			ins_add_val(slot);
	}
}

static void primary() {
	// If it is a reference, the referee must be
	// an identifier
	if(keepID && peek() != TOKEN_IDENTIFIER) {
		lnerr("Bad reference!", presentToken());
		he++;
	}

	if(match(TOKEN_TRUE)) {
		ins_add(PUSHL);
		ins_add_val(1);
	} else if(match(TOKEN_FALSE)) {
		ins_add(PUSHL);
		ins_add_val(0);
	} else if(match(TOKEN_NULL)) {
		ins_add(PUSHN);
	} else if(peek() == TOKEN_NUMBER) {
		char *val = stringOf(advance());
		// if(isDouble(val)){
		ins_add(PUSHF);
		ins_add_double(doubleOf(val));
		//  }
		//  else{
		//      ins_add(PUSHI);
		//      ins_add_val(longOf(val));
		//  }
		memfree(val);
	} else if(peek() == TOKEN_STRING) {
		uint32_t st = str_insert(stringOf(advance()), 1);
		ins_add(PUSHS);
		//    printf("\nPushing string %lu", st);
		ins_add_val(st);
	} else if(peek() == TOKEN_IDENTIFIER) {
		uint32_t st = str_insert(stringOf(advance()), 1);
		if(match(TOKEN_LEFT_PAREN)) {
			calls = (Call *)reallocate(calls, sizeof(Call) * ++callPointer);
			calls[callPointer - 1].t = presentToken();
			uint32_t tmp             = callPointer;
			inCall++;
			uint32_t arity = getCall();
			inCall--;
			ins_add(CALL);

			calls[tmp - 1].callAddress = ins_add_val(0);

			ins_add_val(arity);

			calls[tmp - 1].arity       = arity;
			calls[tmp - 1].routineName = st;
		} else {
			if(keepID == 0) {
				compiler_resolve_variable(presentCompiler, st);
			} else {
				// dbg("String inserted %s at %" PRIu32, str_get(st), st);
				ins_add(MEMREF);
				ins_add_val(st);
				// ins_print();
			}
			if(match(TOKEN_LEFT_SQUARE)) {
				int bak = keepID;
				keepID  = 0;
				expression(); // index
				keepID = bak;
				consume(TOKEN_RIGHT_SQUARE, "Expected ']' after array index!");
				ins_add(ARRAYREF);
			}
		}
	} else if(match(TOKEN_LEFT_PAREN)) {
		int bak = keepID;
		keepID  = 0;
		expression();
		keepID = bak;
		consume(TOKEN_RIGHT_PAREN, "Expected ')' after grouping expression!");
	} else {
		Token t = advance();
		lnerr("Incorrect expression!", t);
	}
}

static void call() {
	primary();
	while(1) {
		if(match(TOKEN_DOT)) {
			keepID++;
			primary();
			keepID--;
		} else
			break;
	}
}

static void tothepower() {
	call();
	while(peek() == TOKEN_CARET) {
		uint8_t op = insFromToken(advance());
		call();
		ins_add(op);
	}
}

static void multiplication() {
	tothepower();
	while(peek() == TOKEN_STAR || peek() == TOKEN_SLASH) {
		uint8_t op = insFromToken(advance());
		tothepower();
		ins_add(op);
	}
}

static void addition() {
	multiplication();
	while(peek() == TOKEN_PLUS || peek() == TOKEN_MINUS ||
	      peek() == TOKEN_PERCEN) {
		uint8_t op = insFromToken(advance());
		multiplication();
		ins_add(op);
	}
}

static void comparison() {
	addition();
	while(peek() == TOKEN_GREATER || peek() == TOKEN_LESS ||
	      peek() == TOKEN_GREATER_EQUAL || peek() == TOKEN_LESS_EQUAL) {
		uint8_t op = insFromToken(advance());
		addition();
		ins_add(op);
	}
}

static void equality() {
	comparison();
	while(peek() == TOKEN_EQUAL_EQUAL || peek() == TOKEN_BANG_EQUAL) {
		uint8_t op = insFromToken(advance());
		comparison();
		ins_add(op);
	}
}

static void andE() {
	equality();
	while(peek() == TOKEN_AND) {
		advance();
		equality();
		ins_add(AND);
	}
}

static void orE() {
	andE();
	while(peek() == TOKEN_OR) {
		advance();
		andE();
		ins_add(OR);
	}
}

static void expression() {
	orE();
}

static void statement(Compiler *compiler);

static void blockStatement(Compiler *compiler, BlockType name,
                           uint8_t ownVariables) {
	// debug("Parsing block statement");
	int       indent = compiler->indentLevel + 1;
	Compiler *blockCompiler =
	    initCompiler(compiler, indent, name, ownVariables);
	if(!ownVariables) {
		blockCompiler->variables = compiler->variables;
		blockCompiler->varSize   = compiler->varSize;
	}

	while(getNextIndent() == indent) {
		// dbg("Found a block statement");
		statement(blockCompiler);
	}

	if(!ownVariables) {
		compiler->variables = blockCompiler->variables;
		compiler->varSize   = blockCompiler->varSize;
	} else {
		memfree(blockCompiler->variables);
	}

	memfree(blockCompiler);

	presentCompiler = compiler;
	// debug("Block statement parsed");
}

static void blockStatement_compiler(Compiler *compiler, BlockType name) {
	// debug("Parsing block statement");
	int indent = compiler->indentLevel;
	while(getNextIndent() == indent) {
		// dbg("Found a block statement");
		statement(compiler);
	}
	// memfree(compiler);
	// debug("Block statement parsed");
}

static void ifStatement(Compiler *compiler) {
	// debug("Parsing if statement");

	consume(TOKEN_LEFT_PAREN,
	        "Conditionals must start with an openning brace['('] after If!");
	expression();
	consume(TOKEN_RIGHT_PAREN,
	        "Conditional must end with a closing brace[')'] after If!");
	consume(TOKEN_NEWLINE, "Expected newline after If!");
	ins_add(JUMP_IF_FALSE);
	uint32_t jmp = ins_add_val(0);
	if(getNextIndent() == compiler->indentLevel) {
		consumeIndent(compiler->indentLevel);
		consume(TOKEN_THEN, "Expected Then on the same indent!");
		consume(TOKEN_NEWLINE, "Expected newline after Then!");
	}

	blockStatement(compiler, BLOCK_IF, 0);
	consumeIndent(compiler->indentLevel);
	ins_add(JUMP);
	uint32_t skip = ins_add_val(0);
	ins_set_val(jmp, ip_get());
	if(match(TOKEN_ELSE)) {
		if(match(TOKEN_IF)) {
			ifStatement(compiler);
		} else {
			consume(TOKEN_NEWLINE, "Expected newline after Else!");
			blockStatement(compiler, BLOCK_ELSE, 0);
			consumeIndent(compiler->indentLevel);
			consume(TOKEN_ENDIF, "Expected EndIf after If!");
			consume(TOKEN_NEWLINE, "Expected newline after EndIf!");
		}
	} else {
		consume(TOKEN_ENDIF, "Expected EndIf after If!");
		consume(TOKEN_NEWLINE, "Expected newline after EndIf!");
	}
	ins_set_val(skip, ip_get());
	// debug("If statement parsed");
}

static void whileStatement(Compiler *compiler) {
	// debug("Parsing while statement");
	consume(TOKEN_LEFT_PAREN, "Expected left paren before conditional!");
	uint32_t start = ip_get();
	expression();
	consume(TOKEN_RIGHT_PAREN, "Expected right paren after conditional!");
	consume(TOKEN_NEWLINE, "Expected newline after While!");
	ins_add(JUMP_IF_FALSE);
	uint32_t jmp = ins_add_val(0);
	if(getNextIndent() == compiler->indentLevel) {
		consumeIndent(compiler->indentLevel);
		consume(TOKEN_BEGIN, "Expected Begin on same indent!");
		consume(TOKEN_NEWLINE, "Expected newline after begin!");
	}
	inWhile++;

	break_push_scope();

	blockStatement(compiler, BLOCK_WHILE, 0);

	ins_add(JUMP);
	ins_add_val(start);
	ins_set_val(jmp, ip_get());
	consumeIndent(compiler->indentLevel);
	consume(TOKEN_ENDWHILE, "Expected EndWhile after while on same indent!");
	consume(TOKEN_NEWLINE, "Expected newline after EndWhile!");
	// dbg("While statement parsed");
	uint32_t *bks = break_peek_scope();
	if(bks != NULL) {
		while(breakCount[breakLevel - 1] > 0)
			ins_set_val(bks[--breakCount[breakLevel - 1]], ip_get());
	}

	break_pop_scope();

	inWhile--;
}

// Not used now
/*static Statement doStatement(Compiler* compiler){
  debug("Parsing do statement");
  Statement s;
  s.type = STATEMENT_DO;
  consume(TOKEN_NEWLINE, "Expected newline after Do!");
  if(getNextIndent() == compiler->indentLevel){
  consumeIndent(compiler->indentLevel);
  consume(TOKEN_BEGIN, "Expected Begin on the same indent level after Do!");
  consume(TOKEN_NEWLINE, "Expected newline after Begin!");
  }
  s.conditionalStatement.statements = blockStatement(compiler, BLOCK_DO);
  consumeIndent(compiler->indentLevel);
  if(match(TOKEN_ENDDO)){
  consume(TOKEN_NEWLINE, "Expected newline after EndDo!");
  consumeIndent(compiler->indentLevel);
  }
  consume(TOKEN_WHILE, "Expected While after Do!");
  consume(TOKEN_LEFT_PAREN, "Expected left paren before conditional!");
  s.conditionalStatement.condition = expression();
  consume(TOKEN_RIGHT_PAREN, "Expected right parent before conditional!");
  consume(TOKEN_NEWLINE, "Expected newline after While!");
  debug("Do statement parsed");
  return s;
  }*/

static void breakStatement() {
	// debug("Parsing break statement");
	if(inWhile == 0) {
		lnerr("Break without While or Do!", presentToken());
		he++;
	} else {
		ins_add(JUMP);
		break_add_address(ins_add_val(0));
	}
	consume(TOKEN_NEWLINE, "Expected newline after Break!");
	// debug("Break statement parsed");
}

static uint8_t generate_store() {
	uint8_t replace_ins = NOOP;
	uint8_t ins         = ins_last();
	switch(ins) {
		case ARRAYREF:
			ins_set(ip_get() - 1, NOOP);
			replace_ins = ARRAYSET;
			break;
		case MEMREF:
			ins_set(ip_get() - 5, PUSHI);
			replace_ins = MEMSET;
			break;
		case LOAD_SLOT:
			ins_set(ip_get() - 5, SAVE_STORE_SLOT);
			replace_ins = STORE_SLOT;
			break;
		case LOAD_SLOT_0:
		case LOAD_SLOT_1:
		case LOAD_SLOT_2:
		case LOAD_SLOT_3:
		case LOAD_SLOT_4:
		case LOAD_SLOT_5:
		case LOAD_SLOT_6:
		case LOAD_SLOT_7:
			ins_set(ip_get() - 1, NOOP);
			replace_ins = STORE_SLOT_0 + (ins - LOAD_SLOT_0);
			break;
		case LOAD_SLOT_GLOBAL:
			ins_set(ip_get() - 5, SAVE_STORE_SLOT);
			replace_ins = STORE_SLOT_GLOBAL;
			break;
		default:
			// printf("\n%d\n", ins_last());
			// ins_print();
			lnerr("Bad assignment target!", presentToken());
			he++;
			break;
	}
	return replace_ins;
}

static void setStatement() {
	// debug("Parsing set statement");
	do {
		// keepID++;
		canDeclare = 1;
		expression();
		canDeclare = 0;
		// keepID--;
		uint8_t replace_ins = generate_store();
		consume(TOKEN_EQUAL, "Expected '=' after identifer!");
		expression();

		ins_add(replace_ins);

	} while(match(TOKEN_COMMA));
	consume(TOKEN_NEWLINE, "Expected newline after Set statement!");
	// debug("Set statement parsed");
}

static void arrayStatement() {
	// debug("Parsing array statement");

	do {
		Token iden =
		    consume(TOKEN_IDENTIFIER, "Expected identifier after 'Array!'");
		uint32_t name = str_insert(stringOf(iden), 1);
		if(compiler_has_variable(presentCompiler, name)) {
			lnerr("Variable %s has already been declared!", iden,
			      str_get(name));
			he++;
		} else {
			uint32_t id = compiler_declare_variable(presentCompiler, name);
			consume(TOKEN_LEFT_SQUARE, "Expected '[' after identifier!");
			expression();
			consume(TOKEN_RIGHT_SQUARE,
			        "Expected ']' after array declaration!");
			ins_add(
			    MAKE_ARRAY); // MAKE_ARRAY is passed the slot number to store
			ins_add_val(id);
		}
	} while(match(TOKEN_COMMA));
	consume(TOKEN_NEWLINE, "Expected newline after Array statement!");
	// debug("Array statement parsed");
}

static void inputStatement() {
	// debug("Parsing input statement");
	do {
		canDeclare++;
		expression();
		canDeclare--;
		uint8_t store = generate_store();
		uint8_t op    = INPUTS;
		if(match(TOKEN_COLON)) {
			Token t = advance();
			if(t.type == TOKEN_IDENTIFIER && strncmp(t.start, "Int", 3) == 0)
				op = INPUTI;
			else if(t.type == TOKEN_IDENTIFIER &&
			        strncmp(t.start, "Float", 5) == 0)
				op = INPUTF;
			else {
				lnerr("Bad input format specifier!", presentToken());
				he++;
			}
		}
		ins_add(op);
		ins_add(store);
	} while(match(TOKEN_COMMA));

	consume(TOKEN_NEWLINE, "Expected newline after Input!");
}

static void printStatement() {
	// debug("Parsing print statement");
	do {
		expression();
		ins_add(PRINT);
	} while(match(TOKEN_COMMA));
	consume(TOKEN_NEWLINE, "Expected newline after Print!");
	// debug("Print statement parsed");
}

static void beginStatement() {
	// debug("Parsing begin statement");
	consume(TOKEN_NEWLINE, "Expected newline after Begin!");
	ins_add(NOOP);
	// debug("Begin statement parsed");
}

static void endStatement() {
	// debug("End statement parsed");
	ins_add(HALT);
	consume(TOKEN_NEWLINE, "Expected newline after End!");
}

static void routineStatement(Compiler *compiler) {
	Routine2 routine = routine_new();

	if(compiler->indentLevel > 0) {
		lnerr("Routines can only be declared in top level indent!",
		      presentToken());
		he++;
	}

	if(match(TOKEN_FOREIGN))
		routine.isNative = 1;

	routine.name = str_insert(
	    stringOf(consume(TOKEN_IDENTIFIER, "Expected routine name!")), 1);

	// dbg("Routine started %s\n", str_get(routine.name));
	uint32_t *args = NULL, argp = 0;

	consume(TOKEN_LEFT_PAREN, "Expected '(' after routine declaration!");
	Compiler *comp =
	    initCompiler(compiler, compiler->indentLevel + 1, BLOCK_FUNC, 1);
	if(peek() != TOKEN_RIGHT_PAREN) {
		do {
			args = (uint32_t *)reallocate(args, sizeof(uint32_t) * ++argp);
			args[argp - 1] =
			    str_insert(stringOf(consume(TOKEN_IDENTIFIER,
			                                "Expected identifer as argument!")),
			               1);

			routine.arity++;

			if(routine.isNative)
				routine_add_slot(&routine, args[argp - 1]);
			compiler_declare_variable(comp, args[argp - 1]);
		} while(match(TOKEN_COMMA));

		consume(TOKEN_RIGHT_PAREN, "Expected ')' after argument declaration!");

		// This is no longer required. The CALL opcode itself handles this
		/*
		   if(!routine.isNative) {
		   while(argp > 0) {
		   if(routine.startAddress == 0)
		   routine.startAddress = ins_add(SETI);   // LOAD_SLOT
		   else
		   ins_add(SETI);                          // LOAD_SLOT
		   ins_add_val(args[--argp]);
		   }
		   }*/
		memfree(args);
	} else
		advance();

	// Reserve slot for locals
	uint32_t slotpatch = 0;
	if(!routine.isNative) {
		routine.startAddress = ins_add(RESERVE_SLOT);
		slotpatch            = ins_add_val(0);
	}

	consume(TOKEN_NEWLINE, "Expected newline after routine declaration!");

	if(routine.isNative == 0) {
		if(routine.startAddress == 0)
			routine.startAddress = ip_get();
		blockStatement_compiler(comp, BLOCK_FUNC);
		// Patch the RESERVE_SLOT
		// dbg("comp->varsize = %d", comp->varSize);
		// Ignore routine arguments, as they are in the stack
		// and automatically reserved by CALL
		ins_set_val(slotpatch, comp->varSize - routine.arity);
		// Copy the variable name and slots
		routine.variables = comp->variables;
		routine.slots     = comp->varSize;
		// Explicitly free the memory
		memfree(comp);

		if(ins_last() != RETURN) {
			ins_add(PUSHN);
			ins_add(RETURN);
		}
		consume(TOKEN_ENDROUTINE,
		        "Expected EndRoutine after routine definition!");
		consume(TOKEN_NEWLINE, "Expected newline after routine definition!");
	}
	routine_add(routine);
	// dbg("Routine complete\n");
}

static void callStatement() {
	call();
	// Since the return value's gonna go to waste,
	// pop it off from the stack
	ins_add(POP);
	consume(TOKEN_NEWLINE, "Expected newline after routine call!");
}

static void returnStatement() {
	if(inContainer > 0) {
		lnerr("A routine should not return anything!", presentToken());
		he++;
	}
	if(peek() == TOKEN_NEWLINE) {
		ins_add(PUSHN);
	} else {
		inReturn++;
		expression();
		inReturn--;
	}
	ins_add(RETURN);
	consume(TOKEN_NEWLINE, "Expected newline after Return!");
}

static void containerStatement(Compiler *compiler) {
	Routine2 routine = routine_new();

	if(compiler->indentLevel > 0) {
		lnerr("Containers can only be declared in top level indent!",
		      presentToken());
		he++;
	}
	routine.isNative = 0;

	routine.name = str_insert(
	    stringOf(consume(TOKEN_IDENTIFIER, "Expected container name!")), 1);

	uint32_t *args = NULL, argp = 0;

	consume(TOKEN_LEFT_PAREN, "Expected '(' after container declaration!");
	Compiler *comp =
	    initCompiler(compiler, compiler->indentLevel + 1, BLOCK_FUNC, 1);
	if(peek() != TOKEN_RIGHT_PAREN) {
		do {
			args = (uint32_t *)reallocate(args, sizeof(uint32_t) * ++argp);
			args[argp - 1] =
			    str_insert(stringOf(consume(TOKEN_IDENTIFIER,
			                                "Expected identifer as argument!")),
			               1);

			// routine_add_slot(&routine, args[argp - 1]);
			compiler_declare_variable(comp, args[argp - 1]);
			routine.arity++;
		} while(match(TOKEN_COMMA));
		consume(TOKEN_RIGHT_PAREN, "Expected ')' after argument declaration!");

		// No need
		/*
		while(argp > 0) {
		    if(routine.startAddress == 0)
		        routine.startAddress = ins_add(SETI);
		    else
		        ins_add(SETI);
		    ins_add_val(args[--argp]);
		}
		*/

		memfree(args);
	} else
		advance();
	consume(TOKEN_NEWLINE, "Expected newline after container declaration!");

	// Reserve slot for locals
	routine.startAddress = ins_add(RESERVE_SLOT);
	uint32_t slotpatch   = ins_add_val(0);

	if(routine.startAddress == 0)
		routine.startAddress = ip_get();

	inContainer++;
	blockStatement_compiler(comp, BLOCK_FUNC);
	// Copy the variable array and slots
	routine.variables = comp->variables;
	routine.slots     = comp->varSize;

	// Ignore container arguments, as they are in the stack
	// and automatically reserved by CALL
	ins_set_val(slotpatch, comp->varSize - routine.arity);

	// Explicitly free the memory
	memfree(comp);

	ins_add(NEW_CONTAINER);
	ins_add_val(routine.name);
	inContainer--;
	consume(TOKEN_ENDCONTAINER, "Expected EndContainer on the same indent!");
	consume(TOKEN_NEWLINE, "Expected newline after EndContainer!");
	routine_add(routine);
}
/*
   static Statement noopStatement(){
   Statement s;
   s.type = STATEMENT_NOOP;
   return s;
   }
   */
static void statement(Compiler *compiler) {
	consumeIndent(compiler->indentLevel);
	presentCompiler = compiler;
	// dbg("Reassigning compiler to %p", compiler);
	if(match(TOKEN_BEGIN))
		return beginStatement();
	else if(match(TOKEN_END))
		return endStatement();
	else if(match(TOKEN_IF))
		return ifStatement(compiler);
	else if(match(TOKEN_SET))
		return setStatement();
	else if(match(TOKEN_ARRAY))
		return arrayStatement();
	else if(match(TOKEN_INPUT))
		return inputStatement();
	else if(match(TOKEN_WHILE))
		return whileStatement(compiler);
	// else if(match(TOKEN_DO))
	//     return doStatement(compiler);
	else if(match(TOKEN_PRINT))
		return printStatement();
	else if(match(TOKEN_BREAK))
		return breakStatement();
	else if(match(TOKEN_ROUTINE))
		return routineStatement(compiler);
	else if(match(TOKEN_CALL))
		return callStatement();
	else if(match(TOKEN_RETURN))
		return returnStatement();
	else {
		lnerr("Bad statement %s!", head->value, tokenNames[peek()]);
		advance();
	}
}

uint32_t lastjump = 0;

void part(Compiler *c) {
	presentCompiler = c;
	if(peek() == TOKEN_EOF)
		beginStatement();
	else if(match(TOKEN_ROUTINE)) {
		routineStatement(c);
	} else if(match(TOKEN_SET)) {
		ins_set_val(lastjump, ip_get()); // Patch the last jump
		setStatement();
		ins_add(JUMP);
		lastjump = ins_add_val(0);
	} else if(match(TOKEN_ARRAY)) {
		ins_set_val(lastjump, ip_get()); // Patch the last jump
		arrayStatement();
		ins_add(JUMP);
		lastjump = ins_add_val(0);
	} else if(match(TOKEN_CONTAINER)) {
		containerStatement(c);
	} else if(match(TOKEN_NEWLINE))
		return;
	else {
		lnerr("Bad top level statement %s!", head->value, tokenNames[peek()]);
		he++;
		statement(c);
		return part(c);
	}
}

static void patchRoutines() {
	uint32_t i = 0;
	while(i < callPointer) {
		Call c = calls[i];
		// dbg("i : %" PRIu32 " calladdr : %" PRIu32 " name : %s arity : %"
		// PRIu32,
		//        i, c.callAddress, str_get(c.routineName), c.arity);
		Routine2 *r = routine_get(c.routineName);
		if(r->arity != c.arity) {
			lnerr("Arity mismatch for routine '%s' : Expected %" PRIu32
			      " Received %" PRIu32,
			      c.t, str_get(r->name), r->arity, c.arity);
			he++;
			i++;
			continue;
		}
		if(r->isNative) {
			ins_set(c.callAddress - 1, CALLNATIVE);
			ins_set_val(c.callAddress, r->name);
		} else
			ins_set_val(c.callAddress, r->startAddress);
		i++;
	}
	memfree(calls);
}

static uint32_t const_count = 0;

void parser_register_variable(const char *var) {
	const_count++;
	compiler_declare_variable(presentCompiler, str_insert(strdup(var), 1));
}

void parse(TokenList *list) {
	head = list;
	// heap_init();
	// Reserve slots for globals
	ins_add(RESERVE_SLOT);
	uint32_t  globals = ins_add_val(0);
	Compiler *comp    = initCompiler(NULL, 0, BLOCK_NONE, 1);
	presentCompiler   = comp;
	register_natives();
	ins_add(JUMP); // Jump to the first global instruction
	lastjump = ins_add_val(
	    0); // The first location will tell the address of first instruction
	// to be executed, which will be either a global 'Set'/'Array',
	// or the routine 'Main'
	while(!match(TOKEN_EOF)) {
		part(comp);
	}
	Routine2 *r =
	    routine_get(str_insert(strdup("Main"), 1)); // Check if Main is defined

	uint32_t callMain = ins_add(CALL);
	ins_add_val(r->startAddress);
	ins_add_val(0);

	ins_set_val(lastjump, callMain); // After all globals statements are
	// executed, jump to the routine 'Main'
	ins_add(HALT); // After Main returns, halt the machine

	// Patch the global storage
	ins_set_val(globals, comp->varSize - const_count);

	// register_native_routines();

	patchRoutines();
	memfree(comp);
}

int hasParseError() {
	return he;
}
