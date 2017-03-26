
# Assignment 3: Low-Level Virtual Machine

Your goal for this assignment is to implement a bytecode compiler and interpreter for the MITScript Virtual Machine. Your bytecode compiler will take as input your parsed MITScript AST and generate a low-level bytecode-style intermediate representation. Given that intermediate representation, your interpreter will execute the bytecode to produce the program's results.

This assignment also includes a written portion that each member of the group will submit separately.

**Deadline**: April 7th, 2017

## MITScript Virtual Machine (by Example)

The MITScript VM provides a low-level, stack-based machine abstraction of the execution of a program. 

As discussed in lecture, virtual machines provide a syntax-independent and platform-independent (e.g. x86) mechanism to specify a program representation and execution model. As a strong distinction between your bytecode interpreter from assignment #2, your representation here is *flat* in that the body of a function is a straight-line sequence of simple non-recursive operations.

The MITScript VM abstracts an MITScript program into a single function that 1) contains multiple nested functions (each of which correspond to the creation of a closure) and 2) has a list of instructions that when executed augment an *operand stack* and the heap. 

We illustrate the basic structures of an MITScript VM bytecode representation with the following MITScript program snippet:
```
x = 1;
y = 2;
z = x + y;
```

One possible translation of this MITScript program snippet to an MITScript bytecode representation is the following:
```
function
{
  functions = [],
  constants = [1, 2],
  parameter_count = 0,
  local_vars = [x, y, z],
  local_ref_vars = [],
  free_vars = [],
  names = [ ], 
  instructions =
  [
    load_const  0
    store_local 0
    load_const  1
    store_local 1
    load_local  0
    load_local  1
    add
    store_local 2
  ]
}
```
### Functions

A *function* consists of a sequence of instructions as well as additional supporting metadata for the function. In the abbreviated presentation of this example, this additional metadata includes the function's *constant pool* (the list of constants used by the function's instructions) and the function's *locals* (the list of local variables in the function).

### Instructions

An MITScript instruction consists of an operation code and an optional operand (which we refer to as *operand 0*).  For example, the instruction `load_const 0` loads the constant at index `0` of the function's constants. The *mnemonic* or name for the operation code is `load_const` and operand 0 is the integer value `0`. The meaning of `load_const 0` for this example is to load the value `1` and push that value onto the function's *operand stack*.

### Operand Stack

The MITScript VM maintains an operand stack while executing the sequence of instructions within a function. The operand stack is a standard stack (or LIFO queue) that stores intermediate values during the execution of the function. Instructions receive their input operands (excluding *operand 0*) by popping values off of the operand stack. Instructions produce values by pushing their results onto the stack.

As a note, each function execution maintains its own operand stack. Therefore, if a function calls another function, the virtual machine creates a new, empty operand stack for the second function that only the second function operates on. Once invocation of the second function returns, the virtual machine deallocates its operand stack.

### Step-by-Step Example

We next illustrate the execution of the above function step-by-step. We present the contents of the function's frame and operand stack both before and after each instruction with their values specified in braces.

```
{ Frame: {x : uninit , y : uninit, z : uninit}, Stack : { } }
0: load_const 0
{ Frame: {x : uninit , y : uninit, z : uninit}, Stack : { 1 } }
```

The `load_const i` instruction pushes the `i`th constant in the function's constant pool onto the operand stack.  In this case, the `0`th constant is the integer value 1.

```
{ Frame: {x : uninit , y : uninit, z : uninit}, Stack : { } }
1: store_local 0
{ Frame: {x : 1 , y : uninit, z : uninit}, Stack : { } }
```

The `store_local i` instruction stores the top of the operand stack into the function's `i`th local variable as determined by the function's locals array. The instruction also pops or removes the top value of the operand stack. In this case the `0`th variable is the variable `x`.

```
{ Frame: {x : 1 , y : uninit, z : uninit}, Stack : { } }
2: load_const 1
{ Frame: {x : 1 , y : uninit, z : uninit}, Stack : { 2 } }
```

As above, `load_const i`  loads the `i`th constant onto the operand stack. In this case the constant is the integer value 2.

```
{ Frame: {x : 1 , y : uninit, z : uninit}, Stack : { 2 } }
3: store_local 1
{ Frame: {x : 1 , y : 2, z : uninit}, Stack : {  } }
```
As above, `store_local i` stores the top of the operand stack into the function's `i`th local variable. In this case, the 1st variable is the variable `y`.

```
{ Frame: {x : 1 , y : 2, z : uninit}, Stack : {  } }
4: load_local  0
{ Frame: {x : 1 , y : 2, z : uninit}, Stack : { 1 } }
```
  
The `load_local i` instruction reads the value of the function's `i`th local variable and pushes onto the top of the operand stack. Because the function's `0`th local is the variable `x` and its value is the integer value `1`, the instruction pushes 1 onto the operand stack.

```
{ Frame: {x : 1 , y : 2, z : uninit}, Stack : { 1 } }
5: load_local  1
{ Frame: {x : 1 , y : 2, z : uninit}, Stack : { 1, 2 } }
```

As per the reasoning above, this instruction pushes the integer value 2 onto the operand stack. Note that the top of the stack is the rightmost element.

```
{ Frame: {x : 1 , y : 2, z : uninit}, Stack : { 1, 2 } }
6: add
{ Frame: {x : 1 , y : 2, z : uninit}, Stack : { 3 } }
```

The `add` instruction pops two elements off of the top of the stack, performs the addition of the two elements, and pushes the result back onto the top of the stack. Note that unlike the other instructions, this instruction does not have an operand 0.

```
{ Frame: {x : 1 , y : 2, z : uninit}, Stack : { 3 } }
7: store_local 2
{ Frame: {x : 1 , y : 2, z : 3}, Stack : { } }
```
As described previously, the `store_local i` instruction stores the top of operand stack into the specified local variable.

## MITScript Virtual Machine

The MITScript VM refines the values, program state, and errors of MITScript as seen in Assignment #2.

### Values

As in Assignment #2,  the MITScript VM includes the following values:

  - None
  - Boolean
  - Integer
  - String
  - Record

The MITScript VM also includes the following values:

  - Function: a list of instructions as well as the function's additional supporting metadata.
  
  - Variable References:  a reference to a local variable that has been defined in a (possibly different) scope.
  
  -  Closure: a tuple consisting of a function to execute and the local variable references that the instructions within the function use.
 
To illustrate these values, consider the following MITScript program:

```
x = 1;
f = fun(y) 
{
  g = fun(z)
  {
     return x + y + z;
  };
};
```


The most important aspect to this example is the code inside `g` in that it accesses the *global* variable `x`, the *reference* variable `y`, and the *local* variable `z`. In the MITScript VM, each of these variables have different access instructions as well as different runtime machinery to support their execution. To understand these differing implementations, consider the following bytecode translation for this program:

```
function 
{
  functions =
  [
    function
    {
      functions =[
        function
         {
          functions  = [],
          constants  = [],
          parameter_count = 1,
          local_vars = [z],
          local_ref_vars = [],
          free_vars  = [y],
          names = [x],
          instructions = [
             load_global 0
             push_ref    0
             load_ref   
             add
             load_local  0
             add
             return
          ]
         }
      ],
      constants = [None, 1],
      parameter_count = 1,
      local_vars = [y, g],
      local_ref_vars = [y],
      free_vars = [],
      names = [],
      instructions = [
        push_ref      0
        load_const    1
        load_func     0
        alloc_closure
        store_local   1
        load_const    0
        return
       ] 
    }
  ],
  constants = [1, None],
  parameter_count = 1,
  local_vars = [], 
  local_ref_vars = [],
  free_vars = [],
  names = [x, f],
  instructions = 
  [
    load_const   1
    store_global 0
    load_func    0
    store_global 1
  ] 
}
```

**Functions**: In the MITScript VM, each function includes an expanded set of metadata that capture the semantics of managing local variables (including parameters).

- parameter_count : the number of parameters for the function
- local_vars : the list of local variables of the function
- local_ref_vars : the list of local variables that are accessed by reference
- names: the list of global variable and field names accessed inside the function 
- free_vars : the list of non-global and non-local variables (variables from a parent scope) that are access within this function's instructions
- constants : a list of constants (e.g., None, 1, "hello") used within this function
- functions : a list of definitions of the functions nested within this function

**Variables**: Given this metadata, we can now distinguish and give a semantics to the machinery of variable access.

- *Global Variable*: global variables are accessed by the `load_global i` instruction. The instruction reads the value of a global variable using the index `i`. In this case, `i` refers is the index into the `names` list of the enclosing function. The instruction therefore accesses the variable named `names[i]`.

- *Reference Variable*: To support accessing variables that are allocated in other scopes, the MITScript VM supports reference variables. Reference variables are variables that are accessed via an address that has been passed to the function when the function's closure was created. 

 The `push_ref i` instruction loads the reference to a variable onto the stack.  In this case `i` refers is the index into either the enclosing function's `local_ref_vars` or `free_vars` array (if `i` is greater than the length of `local_ref_vars`).   

 The `load_ref` instruction dereferences the address associated with the reference at the top of the stack to load a new value onto the stack.

- *Local Variable*: As discussed in the previous section, local variables are accessed by the `load_local i` instruction.

**Closures**: To support reference variables, creating closures on the MITScript VM is different than the corresponding implementation in Assignment #2. In Assignment #2, a closure consisted of a function to execute, as well as a pointer to the frame to be used as a parent frame when executing the function. 

In the MITScript VM, instead of taking a pointer to a frame, a closure takes a list of references to the free variables in the nested function. This design is more efficent in that each access to variable inside the function need not recursively search its ancestry of frames to find the appropriate value for the variable.

The instructions for the function`f` above illustrates how `f` creates a closure for variable `g`. The instruction sequence first pushes a reference to `y` onto the stack (via the `push_ref 0` instruction), then pushes the number of variable references that it will pass on the operand stack, and then pushes the function that corresponds to `g` (`load_func 0`). 

The `alloc_closure` instruction, consumes and stores the function and variable references and then asserts that the number of passed variable references matches the number of free variables in the function.

The files VirtualMachine/types.h and VirtualMachine/instructions.h give a more precise explanation of a function's instructions and metadata.

### Program State

**Frames**: A stack frame in MITScript includes 1) dedicated storage for the value of local variables,  2) dedicated storage for local variable references that are accessed/passed in/passed by reference,  and 3) an operand stack.

**Heap**:  Some of your runtime structures will be allocated in a heap. It is up to your discretion as to how you store an MITScript's data and runtime structures for this assignment. However,  your submission for the written portion of this project should match the implementation strategy your group has chosen.

### Errors

The MITScript VM reports the same exceptions for the error conditions as in Assignment #2.  At the bytecode level, however, there are also new opportunities for execution to encounter an error.  For example, consider this alternative translation of our first  MITScript program:

```
function:
  local_vars = [x, y, z],
  constants = [1, 2],

  instructions = 
  [
    0: load_const  0
    1: store_local 0
    2: load_const  1
    3: store_local 1
    4: load_local  0
    5: add
    6: store_local 2
  ]
```

In this case, the bytecode compiler has failed to generate the code to load the value of `y` prior to the `add` instructions on `line 5`. The height of the operand stack prior to `line 5` is `1` and,  therefore,  the `add` instruction will fail because it cannot pop its two necessary operands from the stack. 

Your bytecode interpreter should generate the following additional exceptions:

 - InsufficentStackException: when an instruction cannot pop a sufficient number of arguments from the operand stack
 - RuntimeException: for all other VM error conditions not covered by Assignment #2.
 
### Native Functions

For this assignment, we will use the convention that the first three functions in the functions list of the outermost function (the function for the whole program) correspond to the `print`,  `input`, and `intcast` functions, respectively.

Your bytecode interpreter should detect when one of these functions has been called
and perform the correct action.

## Deliverables

You will deliver a compiler, an interpreter,  test cases, and a written assignment.

### Compiler

Your compiler should translate the AST representation of an MITScript program to a single bytecode function. As explained above, the definition of a function within the program should be located within the "functions" field of that function's parent function.

If the function does not syntatically have a parent function (it is defined at the top level), then its parent should be the single bytecode function that you generate for the whole program.

We have included a pretty printer class as part of the skeleton that may use to pretty print your bytecode representation for output.

### Interpreter

Your interpeter should interpret your compiled bytecode representation and produce its results. Your intereter should support two modes of operation:

1. **MITScript Source**: if passed the option `-s` on the command, then it should parse the input as MITScript source code.

2.  **MITScript Bytecode**: if passed the option `-b` on the command line, then it should treat the input as MITScript VM bytecode. We have provided a bytecode parser in the skeleton to help support this mode of operation.

### Test Cases

In addition to your compiler and interpreter, you should submit 5 tests named bytecodetest[1-5].mit. Your tests should not use the "input" function, but should use print. 

### Written Assignment

The file VirtualMachine/instructions.h provides an English language specification of the semantics of each instruction. For the written portion of the assignment, your task is write a semantics using the notation we have covered in class as well as was presented in Assignment #2.

For the written assignment, you need only formalize the following MITScript VM instructions and the runtime structures required to support their operation.

- LoadConst
- LoadFunc
- LoadLocal
- StoreLocal
- LoadGlobal
- StoreGlobal
- PushReference
- LoadReference
- StoreReference
- AllocClosure 
- Call
- Add
- Dup
- Swap
- Pop

For simplicity,  you need only support Integer, Function, Variable References, and Closure values.
 
**Collaboration Policy**: Collaboration with your classmates is allowed. However,  you are expected to independently write and submit your own solution.

### Submission Instructions

As before, submission of your assigment should be accomplished by pushing your code to a branch with the name a3-submission.

The last assignment pushed to this branch before the assignment due date is the one we will grade unless you notify us otherwise.

## Implementation Notes

There are two main programming patterns that you will see in this assignment: the visitor pattern and the interpreter loop.

- Visitor Pattern. As with your interpreter in Assignment #2,  your compiler is best structured with the Visitor pattern. Your compiler will recursively traverse the AST to generate code and combine small sequences of code into larger sequences.

- Interpreter Loop. The instructions for a function is a flat sequence of instructions. The interpreter for a flat sequence is best structured as an *interpreter loop*. An interpreter loop maintains an instruction pointer that points to the current instruction to be executed. 

 When the interpreter finishes performing an instruction, it increments the instruction pointer to point to the next instruction. For control flow instructions, the next pointee of the instruction pointer may not be the next immediate instruction. Instead, it may a different instruction as determined by the jump destination of the control flow instruction.
