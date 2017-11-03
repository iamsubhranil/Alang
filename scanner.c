#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "scanner.h"
#include "display.h"
#include "allocator.h"

typedef struct {
    const char* name;
    size_t      length;
    TokenType   type;
} Keyword;

// The table of reserved words and their associated token types.
static Keyword keywords[] = {
    {"Begin",   5, TOKEN_BEGIN},
    {"End",     3, TOKEN_END},

    {"True",    4, TOKEN_TRUE},
    {"False",   5, TOKEN_FALSE},
    {"Null",    4, TOKEN_NULL},
    {"And",     3, TOKEN_AND},
    {"Or",      2, TOKEN_OR},
    {"Int",     3, TOKEN_INT},
    {"Float",   5, TOKEN_FLOAT},

    {"Set",     3, TOKEN_SET},
    {"Array",   5, TOKEN_ARRAY},
    {"Input",   5, TOKEN_INPUT},

    {"If",      2, TOKEN_IF},
    {"Then",    4, TOKEN_THEN},
    {"Else",    4, TOKEN_ELSE},
    {"EndIf",   5, TOKEN_ENDIF},

    {"For",     3, TOKEN_FOR},
    {"EndFor",  6, TOKEN_ENDFOR},

    {"While",   5, TOKEN_WHILE},
    {"Break",   5, TOKEN_BREAK},
    {"EndWhile",8, TOKEN_ENDWHILE},

    {"Do",      2, TOKEN_DO},
    {"EndDo",   5, TOKEN_ENDDO},

    {"Print",   5, TOKEN_PRINT},
    // Sentinel to mark the end of the array.
    {NULL,      0, TOKEN_EOF}
};

typedef struct {
    const char* source;
    const char* tokenStart;
    const char* current;
    int line;
} Scanner;

Scanner scanner;

void initScanner(const char* source) {
    scanner.source = source;
    scanner.tokenStart = source;
    scanner.current = source;
    scanner.line = 1;
}

// Returns true if `c` is an English letter or underscore.
static bool isAlpha(char c) {
    return (c >= 'a' && c <= 'z') ||
        (c >= 'A' && c <= 'Z') ||
        c == '_';
}

// Returns true if `c` is a digit.
static bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

// Returns true if `c` is an English letter, underscore, or digit.
static bool isAlphaNumeric(char c) {
    return isAlpha(c) || isDigit(c);
}

static bool isAtEnd() {
    return *scanner.current == '\0';
}

static char advance() {
    scanner.current++;
    return scanner.current[-1];
}

static char peek() {
    return *scanner.current;
}

static char peekNext() {
    if (isAtEnd()) return '\0';
    return scanner.current[1];
}

static bool match(char expected) {
    if (isAtEnd()) return false;
    if (*scanner.current != expected) return false;

    scanner.current++;
    return true;
}

static Token makeToken(TokenType type) {
    Token token;
    token.type = type;
    token.start = scanner.tokenStart;
    token.length = (int)(scanner.current - scanner.tokenStart);
    token.line = scanner.line;

    return token;
}

static Token errorToken(const char* message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = scanner.line;

    return token;
}

static Token identifier() {
    while (isAlphaNumeric(peek())) advance();

    TokenType type = TOKEN_IDENTIFIER;

    // See if the identifier is a reserved word.
    size_t length = scanner.current - scanner.tokenStart;
    Keyword *keyword;
    for (keyword = keywords; keyword->name != NULL; keyword++) {
        if (length == keyword->length &&
                memcmp(scanner.tokenStart, keyword->name, length) == 0) {
            type = keyword->type;
            break;
        }
    }

    return makeToken(type);
}

static Token number() {
    while (isDigit(peek())) advance();

    // Look for a fractional part.
    if (peek() == '.' && isDigit(peekNext())) {
        // Consume the "."
        advance();

        while (isDigit(peek())) advance();
    }

    return makeToken(TOKEN_NUMBER);
}

static Token string() {
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') scanner.line++;
        advance();
    }

    // Unterminated string.
    if (isAtEnd()) return errorToken("Unterminated string.");

    // The closing ".
    advance();
    return makeToken(TOKEN_STRING);
}

static Token scanToken() {

    // The next token starts with the current character.
    scanner.tokenStart = scanner.current;

    if (isAtEnd()) return makeToken(TOKEN_EOF);

    char c = advance();

    if (isAlpha(c)) return identifier();
    if (isDigit(c)) return number();

    switch (c) {
        case ' ': {
                    int count = 1;
                    while(peek() == ' ' && count < 4){
                        count++;
                        advance();
                    }
                    if(count == 4){
                        return makeToken(TOKEN_INDENT);
                    }
                    else
                        return scanToken(); // Ignore all other spaces
                 }
        case '\r':
                  if(peek() == '\n'){
                      advance();
                      scanner.line++;
                      return makeToken(TOKEN_NEWLINE);
                  }
                  return scanToken(); // Ignore \r
        case '\n':
                  scanner.line++;
                  return makeToken(TOKEN_NEWLINE);
        case '\t': return makeToken(TOKEN_INDENT);
        case '(': return makeToken(TOKEN_LEFT_PAREN);
        case ')': return makeToken(TOKEN_RIGHT_PAREN);
        case '{': return makeToken(TOKEN_LEFT_BRACE);
        case '}': return makeToken(TOKEN_RIGHT_BRACE);
        case '[': return makeToken(TOKEN_LEFT_SQUARE);
        case ']': return makeToken(TOKEN_RIGHT_SQUARE);
        case ';': return makeToken(TOKEN_SEMICOLON);
        case ':': return makeToken(TOKEN_COLON);
        case ',': return makeToken(TOKEN_COMMA);
        case '.': return makeToken(TOKEN_DOT);
        case '-': return makeToken(TOKEN_MINUS);
        case '+': return makeToken(TOKEN_PLUS);
        case '/': 
                  if(peek() == '/'){
                      while (peek() != '\n' && !isAtEnd()) advance();
                      advance(); // \n
                      scanner.line++;
                      return scanToken();
                  }
                  else if(peek() == '*'){
                      while(!(peek() == '*' && peekNext() == '/') && !isAtEnd()){
                          if(peek() == '\n')
                              scanner.line++;
                          advance();
                      }
                      if(!isAtEnd()){
                            advance(); // *
                            advance(); // /
                            advance(); // \n
                            scanner.line++;
                      }
                    return scanToken();
                  }
                  return makeToken(TOKEN_SLASH);
        case '*': return makeToken(TOKEN_STAR);
        case '%': return makeToken(TOKEN_PERCEN);
        case '^': return makeToken(TOKEN_CARET);
        case '!':
                  if (match('=')) return makeToken(TOKEN_BANG_EQUAL);
                  return makeToken(TOKEN_BANG);

        case '=':
                  if (match('=')) return makeToken(TOKEN_EQUAL_EQUAL);
                  return makeToken(TOKEN_EQUAL);

        case '<':
                  if (match('=')) return makeToken(TOKEN_LESS_EQUAL);
                  return makeToken(TOKEN_LESS);

        case '>':
                  if (match('=')) return makeToken(TOKEN_GREATER_EQUAL);
                  return makeToken(TOKEN_GREATER);

        case '"': return string();
        default:
                  line_error(scanner.line, "Unexpected character!");
                  break;
    }
}

static TokenList *newList(Token t){ 
    TokenList *now = (TokenList *)mallocate(sizeof(TokenList));
    now->value = t;
    now->next = NULL;
    return now;
}

static void insertList(TokenList **head, TokenList **prev, TokenList *toInsert){
    if(*head == NULL)
        (*head) = toInsert;
    else
        (*prev)->next = toInsert;

    (*prev) = toInsert;
}

TokenList* scanTokens(){
    TokenList *ret = NULL, *head = NULL;
    Token t;
    while((t = scanToken()).type != TOKEN_EOF){
        //info("Looping in makelist");
        insertList(&head, &ret, newList(t));
    }

    insertList(&head, &ret, newList(makeToken(TOKEN_EOF)));

    return head;
}

void printList(TokenList *list){
    if(list == NULL){
        printf("\n[Error] Empty list!");
    }
    printf("\n");
    while(list->value.type != TOKEN_EOF){
        printf("%s ", tokenNames[list->value.type]);
        if(list->value.type == TOKEN_NEWLINE)
            printf("\n") ;
        else if(list->value.type == TOKEN_ERROR){
            printf("\n[Error] %s\n", list->value.start);
        }
       list = list->next; 
    }
    printf("TOKEN_EOF\n");
}

void freeList(TokenList *list){
    while(list != NULL){
        TokenList *bak = list->next;
        free(list);
        list = bak;
    }
}
