# Alang
### An algorithmic language

In the era of programs and never ending quest for the "perfect" programming languages, Alang brings something fresh and new. It is a relatively simple and easy-to-use language, which closely resembles how general algorithms are written. If you've already written some algorithms, or atleast one of them, you know how to code in Alang. It's that simple. Right now, Alang is pretty limited, but is fairly strong enough to run general problems, and it also supports routines(another funky name of a `function`).

#### Variable and datatypes

Alang is dynamically typed, meaning it manages the datatype of a variable on-the-fly. It also doesn't scold you if you put a value of typeA in a variable which previously contained a value of typeB. Alang supports integer, floating point, boolean and string datatypes. All standard arithmetic and logical operations are permitted on integer and floating point values, including % for 'modulo divison'(integer only) and ^ for exponentiation operation. String supports all logical operations except 'logical and' and 'logical or'. The only 'arithmetic' operator which can be used in between two strings is '+', which results a new string as a concatenation of the older ones. Boolean variables can only be used in a logical operation or expression.
A variable name can contain any alphanumeric character, but it must lead with an alphabet. It cannot contain '.', '#' or any other special characters. A string must be specified between ""(double quotes).
Alang also supports arrays, and an array *can* contain heterogeneous elements. Array index starts from 1 and goes upto size_of_the_array, and trying to read or write outside of this range results in a runtime error. You can shrink and/or grow arrays at runtime by redefining it, which preserves the existing elements of the array. However, if the new size is lesser than the older one, all the elements with index > size gets deleted.
Alang also supports accessing letters of a string using index, and reading and writing strings in the same way is permitted.

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
Routine MyRoutine(argument1, argument2, argument3, .. , argumentN)
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
    Set i = MyRoutine(parameter1, parameter2, .. , parameterN)
```

#### Containers

Containers are packets of data, and have some distinct properties of both a routine and an array. Like an array, a container is a collection of values. Unlike array, members of a container can be accessed by name. Like a function, a container can have arguments and a block of statements. But unlike functions, those instructions cannot be reused. They are executed one time while initializing the container. Any variable declared while the execution of the block is considered as a member of the container, and can be accessed using the following syntax later on : 
```
Set i = MyContainer() 
Set i.member = value 
```
Containers differ from both C structures and OOP classes. Unlike structures, a container can have an implicit constructor. Unlike classes, a container cannot have routines.
To declare a cotainer, place the constructor between `Container` - `EndContainer` block.
```
Container MyContainer(x, y)
    Set value = x
    Set next = y
EndContainer
```
A container must be declared on the outermost indent, like routines. Alang "tries" to intelligently garbage collect all leftover containers instances when they are not in use, but may get stuck on some places. If you can find one such place, please open an issue with your full program and exact output.
Only the variables declared while executing the constructor block are considered as members. Trying to access members other than them will result in errors.

#### Syntax

An Alang program is a collection of statements, each of which starts with one of the given keywords :

1. Set : Assigns a value to a variable
```
    Set variable_name1 = value1 [, variable_name2 = value2 [...]]
```

2. Input : Takes an input from the standard input. You can specify what type of value to read by using either of Int or Float keywords, otherwise a string is read implicitly. You can also specify by a prompt to display by writing a string as an argument, which will be displayed to the user.
```
    Input variable1[:Int|Float] [, variable2[:Int|Float] [...]]
    Input container.member[:Int|Float]
    Input array[19][:Int|Float]
```

3. Print : Prints a string, a variable, or an expression to the standard output.
```
    Call Print(variable1 [, expression1 [...]])
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
    Routine ARoutine(arg1, arg2, arg3, .. , argN)
        Block
    EndRoutine
```

11. Call : Calls a routine.
```
    Call MyRoutine(arg1, arg2, arg3, .. , argN)
```

#### Native Functions

You can load and execute a function precompiled in binary from Alang.

1. First, declare the routine as `Foreign` in the Alang source.
```
Routine Foreign MyRoutine(x, y, z)
```

2. Next, add a call to `LoadLibrary` to load the library (`.so/.dll`) where your function resides. 
`LoadLibrary` will return `True` if the shared object can be successfully loaded, `False` otherwise.
```
    If(LoadLibrary("MyLibrary.so"))
        <Put the call to MyRoutine inside this>
    Else
        Print "Unable to load MyLibrary!"
    EndIf
```

3. In `mylibrary.c`, delcare the native function with the same name as in the Alang source, with the following signature
```
#include <alang/foreign_interface.h>

// Do not try to directly manipulate the NativeData
// variables. Use the provided native_* helper functions
// defined in foreign_interface.h
NativeData MyRoutine(NativeData args) {
    // args is just an array of values
    NativeData arg1 = native_arr_get(args, 0);
    int32_t val = native_expect_int(arg1);

    return native_fromint(val - 1);
}
```

4. Compile `mylibrary.c` as a shared library and link it with `-llalang`.

5. Voila, now the call to `MyRoutine` should work.

See `algos/nativefibonacci.algo` and corresponding `.c` source for more information.

Some predefined native library functions :

* `LoadLibrary` : Loads a shared object and maps it to the address space of Alang.
* `UnloadLibrary` : Unmaps a shared object from Alang.
* `Clock` : Returns the system clock, in milis. (`clock()` in `<time.h>`)
* `Int` : Converts a given float or string to `int` (by default all numbers are `float`s).
* `Sin`, `Sinh`, `ASin`, `Cos`, `Cosh`, `ACos`, `Tan`, `Tanh`, `ATan` : Represents their counterparts in `<math.h>`.

Some predefined constants :

* `Math_Pi` : `acos(-1.0)` in `<math.h>`.
* `Math_E` : `M_E` in glibc.
* `ClocksPerSecond` : `CLOCKS_PER_SEC` in `<time.h>`.

#### Variadic arguments

Routines can also take variable number of arguments using the following syntax : 
```
Routine ARoutine(a, ...)
```
The variadic argument identifier `...` must be either the last or the only argument of the routine. The variadic arguments passed while calling the routine will be stored inside an array named `Vargs`, which will be locally accessible inside the declared routine, and the number of variadic arguments will be stored in variable `Vlength`, also local to the declared routine('ARoutine' in this case). 

Native variadic routines can access the variadic arguments by first casting the penultimate argument to arr using `native_expect_arr`, which will contain the actual passed arguments, and then casting the last argument to int using `native_expect_int`, which will contain the number of such variadic arguments passed to the routine. See algos/varargnative.c for more information.

#### Examples

Examples can be found in `algos` folder.
