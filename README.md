# Alang
### An algorithmic language

In the era of programs and never ending quest for the "perfect" programming languages, Alang brings something fresh and new. It is a relatively simple and easy-to-use language, which closely resembles how general algorithms are written. If you've already written some algorithms, or atleast one of them, you know how to code in Alang. It's that simple. Right now, Alang is pretty limited, but is fairly strong enough to run general problems, and it also supports routines(another funky name of a `function`).

#### Variable and datatypes

Alang is dynamically typed, meaning it manages the datatype of a variable on-the-fly. It also doesn't scold you if you put a value of typeA in a variable which previously contained a value of typeB. Alang supports integer, floating point, boolean and string datatypes. All standard arithmetic and logical operations are permitted on integer and floating point values, including % for 'modulo divison'(integer only) and ^ for exponentiation operation. String supports all logical operations except 'logical and' and 'logical or'. The only 'arithmetic' operator which can be used in between two strings is '+', which results a new string as a concatenation of the older ones. Boolean variables can only be used in a logical operation or expression.
A variable name can contain any alphanumeric character, but it must lead with an alphabet. It cannot contain '.', '#' or any other special characters. A string must be specified between ""(double quotes).
Alang also supports arrays, and an array *can* contain heterogeneous elements. Array index starts from 1 and goes upto size_of_the_array, and trying to read or write outside of this range results in a runtime error. You can shrink and/or grow arrays at runtime by redefining it, which preserves the existing elements of the array. However, if the new size is lesser than the older one, all the elements with index > size gets deleted.

#### Operator and expressions

| Symbol | Operation |
| --- | --- |
| + | Arithmetic addition |
| - | Arithmetic subtraction |
| * | Arithmetic multiplication |
| / | Arithmetic division |
| % | Arithmetic modulo remainder division |
| ^ | Arithmetic exponentiation operation |
| And | Logical and |
| Or | Logical Or |
| True | Logical true |
| False | Logical false |

#### Statements and blocks

Each alang statement must terminate with a newline. To specify a block of statements as a single unit, tabs must be used. Each tab must be either of a hardcoded character('\t'), or *4 spaces*. Use anything more or less than that and bad things will happen. To specify a block of statements, indent them in the same level.

Example:

    Statement0
    Statement1
        //Start of a block
        Statement2
        Statement3
            //Start of a nested block
            Statement4
            Statement5
            .
            .
            .
            StatementM
            //End of a nested block
        StatementN
        //End of outer block
    StatementP

#### Routines

Each Alang program can be subdivided into several routines. Routines are the temporary store of reusuable instructions, and can be "called" from anywhere in the code, even from within the routine itself. One absolutely necessary routine of an Alang program is Main, which is the entrypoint of the program. Unlike Main, each routine can have multiple arguments, and may return a value to the caller. Routines can be freely called from anywhere where writing an expression is permitted. Every routine shares the global environment, but each of them also has their own environment too, which stays alive while the routine performs, and gets automatically garbage collected when the routine exits. Routines should be defined like the following :

```
Routine MyRoutine(argument1, argument2, argument3, ...)
    Statement1
    .
    .
    .
    StatementN
    Return expression
EndRoutine
```

Every routine must be declared on the outermost indentation level, i.e. nested routines are not permitted.
It is not necessary for a routine to return a value. If a routine doesn't return something, it is assumed that the return value is `Null`. Since all routines share the global environment, any changes to the state of global variables can be tracked by other routines. To call a routine, one can either use the explicit `Call` statement, or call the routine as :

```
    Set i = MyRoutine(parameter1, parameter2, ...)
```

#### Syntax

An Alang program is a collection of statements, each of which starts with one of the given keywords :

1. Set : Assigns a value to a variable
```
    Set variable_name1 = value1 [, variable_name2 = value2 [...]]
```

2. Input : Takes an input from the standard input. You can specify what type of value to read by using either of Int or Float keywords, otherwise a string is read implicitly. You can also specify by a prompt to display by writing a string as an argument, which will be displayed to the user.
```
    Input ["prompt_string" ,] variable1[:Int|Float] [, variable2[:Int|Float] [...]]
```

3. Print : Prints a string, a variable, or an expression to the standard output.
```
    Print ["output_string", ] variable1 [, expression1 [...]]
```

4. Array : Declares an array. Array dimension needs to be specified using square braces while declaration, and it can be changed later. Though the dimension can be an arithmetic expression, but it *must* be an integer. An array can be resized by redefining it.
```
    Array array_name1[dimension_expression1] [, array_name2[dimension2] [...]]
```

5. If : Performs a conditional executions of a block of statements. Each If statement must be terminated with an EndIf statement in the same indent.
```
    If(condition0)
    [Then]
        ThenBlock0 // Runs if condition0 is True
    Else If(condition1)
    [Then]
        ThenBlock1 // Runs if condition1 is True
    .
    .
    Else
        ElseBlock // Runs if all given conditions are false
    EndIf
```

6. While : Runs a conditional loop until the condition is False. Each While statement must be terminated with an EndWhile statement in the same indent.
```
    While(condition)
    [Begin]
        Statements // Execute if condition is True
    EndWhile
```

7. Break : Breaks out of a loop abruptly. Using `Break` outside of a loop is not permitted.
```
    Break
```

8. End : Terminates present program instantly.
```
    End
```

9. Return : Returns a value from a routine.
```
    Return expression
```

10. Routine : Defines a routine.
```
    Routine ARoutine(arg1, arg2, arg3, ...)
        Block
    EndRoutine
```

11. Call : Calls a routine.
```
    Call MyRoutine(arg1, arg2, arg3, ...)
```

#### Examples

Examples can be found in `algos` folder.
