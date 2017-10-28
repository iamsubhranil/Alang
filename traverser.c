#include <stdio.h>
#include <string.h>

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

static FILE* file = NULL;

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

static void resolve(Statement s);

static void resolveBlock(Block b){
    int i = 0;
    debug("Resolving block");
    while(i < b.numStatements){
        resolve(b.statements[i]);
        i++;
    }
    debug("Block resolved");
}

static void resolveIf(IfStatement s){
    debug("Resolving if");
    write(statementNames[STATEMENT_IF]);
    space();
    write("(");
    writeExpression(s.ifBlock.condition);
    write(")");
    newline();
    resolveBlock(s.ifBlock.statements);
    if(s.hasElseIf == 1){
        write("Else\0");
        newline();
        resolveBlock(s.elseBlock);
    }
    else if(s.hasElseIf == 2){
        write("Else\0");
        space();
        resolveIf(*s.elseIfBlock);
    }
    if(s.hasElseIf != 2){
        write("EndIf\0");
        newline();
    }
    debug("If resolved");
}

static void resolveConditional(Statement s){
    debug("Resolving conditional");
    if(s.type == STATEMENT_DO){
        write("Do\0");
        newline();
        resolveBlock(s.conditionalStatement.statements);
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
        resolveBlock(s.conditionalStatement.statements);
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

static void resolve(Statement s){
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
            resolveIf(s.ifStatement);
            break;
        case STATEMENT_DO:
        case STATEMENT_WHILE:
            resolveConditional(s);
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
    debug("Started traversal");
    resolveBlock(b);
}
