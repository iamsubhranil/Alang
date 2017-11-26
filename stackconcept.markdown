## A stack based virtual machine
Alang VM will be based on Harvard Architechture, i.e. there will be two different stacks for instruction and data.
Most of the instructions will work with only top few values of the stack, and will always push their result back to the top of the stack if required.
The parser and compiler will be grouped together. i.e. the parser will parse the code and push the instructions directly to the stack, eliminating the need of AST.
After parser completes, the interpreter will take over and execute the call stack until it reaches to a HALT instruction, which will be the last instruction of `Main()` of the result of an `End` statement.
There need to be a permanent store of instructions, which will store the constants and push them to stack as needed.

## Opcodes
ADD, SUB, MUL, DIV, POW, GT, GTE, LT, LTE, EQ, NEQ, AND, OR, // *Binary operations*
MAKE_ARRAY, ARRAY_R, ARRAY_W, MEMREF_R, MEMREF_W // *Composite reference or access*
SET, PRINT, INPUT, HALT, // *Basic IO*
JUMP, JUMP_IF_TRUE, JUMP_IF_FALSE, RETURN // *Branching and conditional execution*

## Used syntax and their equivalent stack representations

Type                                            Instruction
===========================================================
Numeric Constant                    addC(dataStack, value)
String                              addS(dataStack, string)
Identifer                           addI(dataStack, iden)
Literal                             addL(dataStack, lit) // 0 for False, 1 for True, -1 for Null

Array                               addExpression() // index, addI() // identifer
Member access                       addExpression() // member, addI() // variable, addIns(MEMREF)

Routine call                        add 

Binary operator                     addExp1(), addExp2(), addIns(insStack, op)

Set                                 addExpression(), addIdentifer(), addIns(insStack, set)
Print                               addExpression(), addIns(insStack, print)
Input                               addIdentifer(), addType(), addIns(insStack, input)
End                                 addIns(insStack, halt)

##### If
If(cond is True)                    addExpression(), (int)addJumpAddress(dataStack), addIns(insStack, jump_if_false)
    ThenClause                      addBlock(), addJumpAddress(dataStack) // to skip the else block
Else If(another cond is True)       patchJump(dataStack), ifStatement()
    thenClause2
Else                                patchJump(dataStack)
    elseClause                      addBlock(), patchJump(dataStack)

** No else clause **                patchJump(dataStack), patchJump(dataStack) // either way, `if` will jump to the next statement after its then block

##### While
While(cond is True)                 addExpression(), addJumpAddress(dataStack), addIns(insStack, jump_if_false)
    Block                           addBlock(), addIns(insStack, jump_if_true), patchJump(dataStack)
EndWhile                            patchBreak()

##### Routine
Routine Name(arg1, arg2)            put(symbolTable, ip, Name), addI(arg1), addIns(set), addI(arg2), addIns(set)
    Block                           addBlock()
EndRoutine                          implicitReturn(null)

Return                              addIns(return)

##### Container
Container Name(arg1, arg2)          // same as routine
    Block                           addBlock()
EndContainer                        addIns(return)

##### Array
Array a[10]                         addExpression, addIdentifer,  addIns(make_array)

##### Break
Break                               addBreakJump()

## Binary operations

Binary addition, logical comparison
====================================
First, a value will be `peek(dataStack)`ed to determine the type of operation.
It must be either a constant or a string.
1. Constant
y = popC(dataStack)
x = popC(dataStack)
pushC(x OP y)
2. String
y = popS()
x = popS()
pushS(x OP y)

Binary subtraction, multiplication, division, exponentiation
============================================================
Only constant operations are permitted.
y = popC(dataStack)
x = popC(dataStack)
pushC(dataStack, x OP y)

## Array and member reference operations
##### Array
1. Creation
size = popC(dataStack)
identifer = popI(dataStack)
create_new_array(identifer, size, env)
2. Read
index = popC(dataStack)
identifer = popI(dataStack)
push(dataStack, env_arr_get(identifer, index))
3. Write
index = popC(dataStack)
identifer = popI(dataStack)
value = pop(dataStack)
env_arr_put(index, identifer, value)
###### Member reference
1. Read
object = popO(dataStack)
member = popI(dataStack)
push(env_con_get(object->env, member))
2. Write
object = popO(dataStack)
member = popI(dataStack)
value = pop(dataStack)
env_put(object->env, member, value)

## Set
val = pop(dataStack)
env_put(val, popI())

## Print
val = pop(dataStack)
print(val)

## Input
type = pop(dataStack)
identifer = pop(dataStack)
input(identifier, type)

## Halt
exit(0)

## Jump
add = popA(dataStack)
interpreter->ip = add

## Jump if true
val = popL(dataStack)
if(val)
    jump()
else
    popA(dataStack)

## Jump if false
val = popL(dataStack)
if(!val)
    jump()
else
    popA(dataStack)

## Return
pop(callFrame)
interpreter->ip = peek(callFrame)->ip
