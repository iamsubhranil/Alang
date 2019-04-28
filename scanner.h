//> Scanning on Demand not-yet
#ifndef scanner_h
#define scanner_h

typedef enum {
	TOKEN_LEFT_PAREN,
	TOKEN_RIGHT_PAREN,
	TOKEN_LEFT_BRACE,
	TOKEN_RIGHT_BRACE,
	TOKEN_LEFT_SQUARE,
	TOKEN_RIGHT_SQUARE,

	TOKEN_BANG,
	TOKEN_BANG_EQUAL,
	TOKEN_COMMA,
	TOKEN_DOT,
	TOKEN_VARARG,
	TOKEN_COLON,

	TOKEN_EQUAL,
	TOKEN_EQUAL_EQUAL,
	TOKEN_GREATER,
	TOKEN_GREATER_EQUAL,
	TOKEN_LESS,
	TOKEN_LESS_EQUAL,

	TOKEN_MINUS,
	TOKEN_PLUS,
	TOKEN_SEMICOLON,
	TOKEN_SLASH,
	TOKEN_STAR,
	TOKEN_PERCEN,
	TOKEN_CARET,

	TOKEN_IDENTIFIER,
	TOKEN_STRING,
	TOKEN_NUMBER,
	TOKEN_INDENT,
	TOKEN_NEWLINE,

	TOKEN_TRUE,
	TOKEN_FALSE,
	TOKEN_NULL,
	TOKEN_AND,
	TOKEN_OR,
	// TOKEN_INT,
	// TOKEN_FLOAT,

	TOKEN_BEGIN,
	TOKEN_END,

	TOKEN_SET,
	TOKEN_ARRAY,
	TOKEN_INPUT,

	TOKEN_IF,
	TOKEN_THEN,
	TOKEN_ELSE,
	TOKEN_ENDIF,

	TOKEN_FOR,
	TOKEN_ENDFOR,

	TOKEN_DO,
	TOKEN_ENDDO,

	TOKEN_WHILE,
	TOKEN_BREAK,
	TOKEN_ENDWHILE,

	TOKEN_ROUTINE,
	TOKEN_ENDROUTINE,
	TOKEN_CALL,
	TOKEN_RETURN,
	TOKEN_FOREIGN,

	TOKEN_CONTAINER,
	TOKEN_ENDCONTAINER,

	TOKEN_ERROR,
	TOKEN_EOF
} TokenType;

static const char *tokenNames[] = {"TOKEN_LEFT_PAREN",  "TOKEN_RIGHT_PAREN",
                                   "TOKEN_LEFT_BRACE",  "TOKEN_RIGHT_BRACE",
                                   "TOKEN_LEFT_SQUARE", "TOKEN_RIGHT_SQUARE",

                                   "TOKEN_BANG",        "TOKEN_BANG_EQUAL",
                                   "TOKEN_COMMA",       "TOKEN_DOT",
                                   "TOKEN_VARARG",      "TOKEN_COLON",

                                   "TOKEN_EQUAL",       "TOKEN_EQUAL_EQUAL",
                                   "TOKEN_GREATER",     "TOKEN_GREATER_EQUAL",
                                   "TOKEN_LESS",        "TOKEN_LESS_EQUAL",
                                   "TOKEN_MINUS",       "TOKEN_PLUS",
                                   "TOKEN_SEMICOLON",   "TOKEN_SLASH",
                                   "TOKEN_STAR",        "TOKEN_PERCEN",
                                   "TOKEN_CARET",

                                   "TOKEN_IDENTIFIER",  "TOKEN_STRING",
                                   "TOKEN_NUMBER",      "TOKEN_INDENT",
                                   "TOKEN_NEWLINE",

                                   "TOKEN_TRUE",        "TOKEN_FALSE",
                                   "TOKEN_NULL",        "TOKEN_AND",
                                   "TOKEN_OR",          "TOKEN_INT",
                                   "TOKEN_FLOAT",

                                   "TOKEN_BEGIN",       "TOKEN_END",

                                   "TOKEN_SET",         "TOKEN_ARRAY",
                                   "TOKEN_INPUT",

                                   "TOKEN_IF",          "TOKEN_THEN",
                                   "TOKEN_ELSE",        "TOKEN_ENDIF",

                                   "TOKEN_FOR",         "TOKEN_ENDFOR",

                                   "TOKEN_DO",          "TOKEN_ENDDO",

                                   "TOKEN_WHILE",       "TOKEN_BREAK",
                                   "TOKEN_ENDWHILE",

                                   "TOKEN_ROUTINE",     "TOKEN_ENDROUTINE",
                                   "TOKEN_CALL",        "TOKEN_RETURN",
                                   "TOKEN_FOREIGN",

                                   "TOKEN_CONTAINER",   "TOKEN_ENDCONTAINER",

                                   "TOKEN_ERROR",       "TOKEN_EOF"};

typedef struct {
	TokenType   type;
	const char *start;
	int         length;
	int         line;
	const char *fileName;
} Token;

typedef struct TokenList {
	Token             value;
	struct TokenList *next;
} TokenList;

TokenList *scanTokens(char *file);
void       printList(TokenList *list);
void       freeList(TokenList *list);
char *     read_all(const char *source);
int        hasScanErrors();

#endif
