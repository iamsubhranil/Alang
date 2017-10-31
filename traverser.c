#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "stree.h"
#include "scanner.h"
#include "display.h"

static const char* statementNames[] = {
    "Set\0",
    "If\0",
    "Repeat through\0",
    "Break\0",
    "Print\0",
    "Do\0",
    "Begin\0",
    "End\0"
};

typedef struct Traverser{
    BlockType blockOf;
    int indentLevel;
    int step;
    struct Traverser* next;
} Traverser;

static Traverser* newTraverser(BlockType block){
    Traverser* ret = (Traverser *)malloc(sizeof(Traverser));
    ret->next = NULL;
    ret->blockOf = block;
    ret->indentLevel = 0;
    ret->step = 0;
    return ret;
}

static FILE* file = NULL;
static Traverser* root = NULL;
static int segmentedStep = 0, totalSteps = 0;

static void writeToken(Token t){
    int i = 0;
    //printf("\n[Writing] ");
    while(i < t.length){
        fputc(t.start[i], file);
        //putchar(t.start[i]);
        i++;
    }
    fflush(file);
}

static void write(const char* string){
    fprintf(file, "%s", string);
    fflush(file);
}

static void space(){
    write(" ");
}

static void newline(){
    write("\n");
}

static void printProlouge(){
    Traverser* troot = root, *last = NULL;
    write("Step \0");
    totalSteps++;
    while(troot != NULL){
        if(troot->next == NULL){
            troot->step++;
            last = troot;
        }
        if(segmentedStep){
            if(troot != root) 
                write(".\0");
            fprintf(file, "%d", troot->step);
        }
        troot = troot->next;
    }
    if(!segmentedStep){
        fprintf(file, "%2d", totalSteps);
    }
    write(": \0");
    int temp = last->indentLevel;
    printf("\nIndentLevel : %d", temp);
    while(temp > 0){
        fputc_unlocked('\t', file);
        temp--;
    }
}

static void writeExpression(Expression* e){
    writeToken(e->literal);
    if(e->op.type != TOKEN_ERROR){
        space();
        writeToken(e->op);
        if(e->right != NULL){
            space();
            writeExpression(e->right);
        }
    }
}

static void resolveExpression(Statement s){
    debug("Resolving expression");
    write(statementNames[s.type]);   
    if(s.type == STATEMENT_SET){
        space();
        writeExpression(s.expressionStatement.expression);
    } 
    newline();
    debug("Expression resolved");
}

static void resolvePrint(PrintStatement p){
    debug("Resolving print");
    write(statementNames[STATEMENT_PRINT]);
    space();
    writeToken(p.print);
    newline();
    debug("Print resolved");
}

static void resolve(Statement s, Traverser *t);

static void resolveBlock(Block b, Traverser* t){
    int i = 0;
    Traverser *tra = root;
    if(t != NULL){
        tra = newTraverser(b.blockName);
        tra->indentLevel = t->indentLevel+1;
        t->next = tra;
    }
    debug("Resolving block");
    while(i < b.numStatements){
        resolve(b.statements[i], tra);
        i++;
    }
    if(t != NULL){
        free(tra);
        t->next = NULL;
    }
    debug("Block resolved");
}

static void resolveIf(IfStatement s, Traverser* t){
    debug("Resolving if");
    write(statementNames[STATEMENT_IF]);
    space();
    write("(");
    writeExpression(s.ifBlock.condition);
    write(")");
    newline();
    resolveBlock(s.ifBlock.statements, t);
    if(s.hasElseIf == 1){
        printProlouge();
        write("Else\0");
        newline();
        resolveBlock(s.elseBlock, t);
    }
    else if(s.hasElseIf == 2){
        printProlouge();
        write("Else\0");
        space();
        resolveIf(*s.elseIfBlock, t);
    }
    if(s.hasElseIf != 2){
        printProlouge();
        write("EndIf\0");
        newline();
    }
    debug("If resolved");
}

static void resolveConditional(Statement s, Traverser* t){
    debug("Resolving conditional");
    if(s.type == STATEMENT_DO){
        write("Do\0");
        newline();
        resolveBlock(s.conditionalStatement.statements, t);
        printProlouge();
        write("EndDo\nWhile\0");
        space();
        write("(");
        writeExpression(s.conditionalStatement.condition);
        write(")");
    }
    else if(s.type == STATEMENT_WHILE){
        write("While\0");
        space();
        write("(");
        writeExpression(s.conditionalStatement.condition);
        write(")");
        newline();
        resolveBlock(s.conditionalStatement.statements, t);
        printProlouge();
        write("EndWhile\0");
    }
    newline();
    debug("Conditional resolved");
}

static void resolveBreak(BreakStatement s){
    debug("Resolving break");
    write("Break\0");
    newline();
    debug("Break resolved");
}

static void resolve(Statement s, Traverser* t){
    printProlouge();
    switch(s.type){
        case STATEMENT_BEGIN:
        case STATEMENT_END:
        case STATEMENT_SET:
            resolveExpression(s);
            break;
        case STATEMENT_PRINT:
            resolvePrint(s.printStatement);
            break;
        case STATEMENT_IF:
            resolveIf(s.ifStatement, t);
            break;
        case STATEMENT_DO:
        case STATEMENT_WHILE:
            resolveConditional(s, t);
            break;
        case STATEMENT_BREAK:
            resolveBreak(s.breakStatement);
            break;
        default:
            fatal("Unknown statement!");
    }
}

void traverse(Block b, FILE* toWrite){
    file = toWrite;
    root = newTraverser(BLOCK_MAIN);
    debug("Started traversal");
    resolveBlock(b, NULL);
}
