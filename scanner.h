//> Scanning on Demand not-yet
#ifndef scanner_h
#define scanner_h

typedef char bool;
#define true 1
#define false 0

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

  TOKEN_BEGIN,
  TOKEN_END,

  TOKEN_SET,
  TOKEN_ARRAY,

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

  TOKEN_PRINT,

  TOKEN_ERROR,
  TOKEN_EOF
} TokenType;

static const char* tokenNames[] = {
  "TOKEN_LEFT_PAREN",
  "TOKEN_RIGHT_PAREN",
  "TOKEN_LEFT_BRACE",
  "TOKEN_RIGHT_BRACE",
  "TOKEN_LEFT_SQUARE",
  "TOKEN_RIGHT_SQUARE",

  "TOKEN_BANG",
  "TOKEN_BANG_EQUAL",
  "TOKEN_COMMA",
  "TOKEN_DOT",
  "TOKEN_EQUAL",
  "TOKEN_EQUAL_EQUAL",
  "TOKEN_GREATER",
  "TOKEN_GREATER_EQUAL",
  "TOKEN_LESS",
  "TOKEN_LESS_EQUAL",
  "TOKEN_MINUS",
  "TOKEN_PLUS",
  "TOKEN_SEMICOLON",
  "TOKEN_SLASH",
  "TOKEN_STAR",
  "TOKEN_PERCEN",
  "TOKEN_CARET",

  "TOKEN_IDENTIFIER",
  "TOKEN_STRING",
  "TOKEN_NUMBER",
  "TOKEN_INDENT",
  "TOKEN_NEWLINE",
    
  "TOKEN_TRUE",
  "TOKEN_FALSE",
  "TOKEN_NULL",
  "TOKEN_AND",
  "TOKEN_OR",

  "TOKEN_BEGIN",
  "TOKEN_END",

  "TOKEN_SET",
  "TOKEN_ARRAY",

  "TOKEN_IF",
  "TOKEN_THEN",
  "TOKEN_ELSE",
  "TOKEN_ENDIF",

  "TOKEN_FOR",
  "TOKEN_ENDFOR",

  "TOKEN_DO",
  "TOKEN_ENDDO",

  "TOKEN_WHILE",
  "TOKEN_BREAK",
  "TOKEN_ENDWHILE",

  "TOKEN_PRINT",

  "TOKEN_ERROR",
  "TOKEN_EOF"
};

typedef struct {
  TokenType type;
  const char* start;
  int length;
  int line;
} Token;

typedef struct TokenList{
    Token value;
    struct TokenList *next;
} TokenList;

void initScanner(const char* source);

TokenList* scanTokens();
void printList(TokenList *list);
void freeList(TokenList *list);

#endif
