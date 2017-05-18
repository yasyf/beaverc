# Beaver Compiler

A dynamic interpreter and 6-stage JIT for MITScript, the language used for 6.035.

## Usage

Inside the `mitscript` directory, on a Ubuntu x64 machine:

```
make
./bin/vm [--opt=all] <mitscript source file>
```

## Overview

Over the past semester, we’ve put considerable effort into designing and implementing an interpreter for the MITScript language. This document will outline the design of our interpreter, as well as go into detail about the optimizations we have employed to make given source files execute as quickly as possible.

## Interpreter Design

Our interpreter has six main components:

1. MITScript-to-bytecode compiler (referred to as BCC or Bytecode Compiler), designed to take in MITScript source files, and compile them to a bytecode representation.
2. The bytecode virtual machine, designed to execute compiled bytecode functions directly and produce a program’s output.
3. The heap manager, used to allocate any objects and collect any garbage a program creates.
4. The bytecode-to-IR compiler, designed to take in compiled bytecode functions and produce an internal representation useful for optimization and producing machine code.
5. The IR optimizer, designed to optimize and remove any redundancy or inefficiency in the IR representation
6. The IR-to-machine-code compiler, used to produce machine executable code, directly executed on the host machine.

When running in unoptimized mode, the interpreter will only use components 1, 2, and 3. A source file will be compiled to bytecode by 1, and then will be executed by 2 to produce the desired result, while having its heap managed by 3. Note this will happen all at once, without further serialization/deserialization of the code.

In fully optimized mode, all six components are used. First, a source file will be compiled to bytecode by 1, and then each bytecode function will be individually compiler to the IR representation by 4. This IR will be optimized by 5 and passed to 6, which will generate executable machine code. To run the compiled assembly, a helper implemented in 2 will setup the correct call frame and jump directly into the machine code, beginning execution. When things need to be allocated, 3 will be periodically called for allocations and garbage collection.

Currently, we have made the decision that every function is compiled to machine code just-in-time. However, the VM is designed such that we could use an arbitrary predicate for making this choice.

## General Optimizations

### Generational Garbage Collection

One problem with a naive mark and sweep algorithm is that both phases take a long time, since you have to iterate through every live object in the entire set. A generational garbage collector solves this problem by taking advantage of the fact that most objects that are allocated are very short lived, which means they are allocated and freed all in one cycle of the collector. If this is the case, we can save time in our collector by doing a “good enough” job just collecting all of these low hanging fruits, only stopping to collect everything once enough objects pile-up.

We implemented a 2-generation garbage collection algorithm, and saw decent speedups in our program execution time. Our collector works as follows. Whenever an object is allocated, it’s added to the “New objects” heap. When objects in the the “old objects” heap, change to point to an object in the new object’s heap, we add that object to a set of temporary cross-generation roots. To perform a collection, we just perform mark and sweep on the new objects starting from the initial set of roots and the set of temporary roots, collecting objects from the new generation heap that aren’t marked. Finally, we "move" objects from the new heap to the old heap. By doing it with generations, we save us a lot of time since we only have to mark and sweep over the new objects.

### Hybrid Linear Scan Register Allocator

To improve the runtime of our assembly code, we worked to keep objects in registers as often as possible. To accomplish this, we implemented a two-stage register allocator which first uses the [Linear Scan technique](https://pdos.csail.mit.edu/papers/toplas-linearscan.ps) to effeciently distribute the bulk of the registers (r12 - r15, r8 - r11), followed by a greedy allocator to distribute the remaining registers as required operands and scratch space.

Each Temp and Var is tagged with the register (if any) it has been allocated by the first phase, so that the second phase can spill these operands to main memory as infrequently as possible. As of right now, for simplicity, we still allocate enough space for each such operand to be spilled to a static location on the function’s stack.

We made the decision to only have the Linear Scan allocator tag operands with registers that are never required arguments to x64 operations, to avoid conflicts and the complexity of shuffling registers around at runtime. While we explored graph-colouring-based approaches to optimal register allocation, we settled on the solution that gave us the best tradeoff between optimality, memory efficiency, and performance. Given that we are building a JIT compiler, a high fixed cost at compilation time is undesirable and we sought to avoid extraneous overhead at all costs.

### Tagged Pointers

A huge performance optimization that we made early on was the addition of Tagged Pointers. We noticed that all of the pointers to objects in our Value class where 8-byte aligned, and that meant that the last three bits of any pointer was free to store extra data. The end result of our tagged pointers implementation meant that all integers, booleans, and nones never had to be directly allocated, since they would always be resting inside our value class either on the stack or inside a ReferenceValue. This gave an immense speedup since we no longer had to allocate tiny amounts of memory to do a simple addition, or call a helper to do a type check.

## IR and Machine Code Optimizations

We have the luxury of working with an IR representation in which every operand is a Temp which is used once and discarded. We exploit this to be able to do analysis for optimizations on the linear representation of the IR instructions, instead of building a CFG. We traverse the lattice formed by the flow of data into and out of temps, iteratively transferring properties from source to destination until convergence or a static halting point. Many of the optimizations below rely on this process.

### Constant Folding

We propagate the property of being derived from a constant value or expression through our lattice. This starts with constant literals and is transferred through any arithmetic or logical operation, as well as loads and stores involving local Vars. At every pass, we statically evaluate any expression whose operands are all tagged as constant, replacing the instruction with the result of the expression.

This optimization resulted in many fewer IR instructions for constant literal-heavy programs.

### Type Propagation

We propagate hints about the type of an operand, limited in scope to Ints, Bools, and Strings. This allows us to make several optimizations down the line, including removing unnecessary runtime type assertions. We use a bitvector to store these type hints, ORing new hints in each time.

### Type Specialization

Once we are certain of the type of a Temp (it only has one option that has been hinted), we can attempt to specialize certain operations (such as Add for Ints or Eq for Ints/Bools) so that they can be executed in assembly without calling a helper.

### Copy-On-Write

We included an optimization which retained values which were written to a variable and then subsequently read back out. Instead of potentially reading and writing from main memory, the values are kept in temps which are ideally register-allocated. This optimization is currently disabled as it conflicts with our register allocation scheme.

### Temp and Var Liveness

In order for subsequent optimizations, including register allocation to occur, we must have an accurate sense of the “live range” for each Temp and Var. Thus each optimization pass includes one pass through the code, examining where each Temp and Var is used and setting the start and end of the live range for each.

### Dead Temps

After passes through all the other optimizations, we replace assignments from Temps to Temps by simply resolving the aliases. We then find any unused temps and add them to an obsolete list, which is later processed by deleting all checks and references to said Temp.

### Dead Variable Assignment

Local Vars which are written to but never read from are simply removed from the IR. This is one example of several instances where we remove an operation that has no observable effect on the final output of a program.

### Noop Generation and Removal

Resizing vectors of instructions is an O(n) operation in C++. Thus, to avoid this overhead, optimizations simply replace instructions they would like to delete with a Noop singleton. We then occasionally scan the instruction list between passes, copying all existing instructions to a new list but excluding any Noops.

### Short Jumps

In order to maintain conservative correctness, our IR compiler by default outputs Jump instructions that get translated to x64 near jumps. However, in specific cases, we can make do with the more efficient short jump instead. Thus one of our optimization passes goes through and makes this replacement as conservatively as possible. This optimization provided a negligible speedup.
