# About this project...
"*What is this thing supposed to be?*", some of you might ask.\
Well, it's a compiler I wrote in pure C myself, at the pristine age of *16.*\
But! Before you start questioning if I am really *the reincarnation of Terry A. Davis in the flesh*, consider this.
I was heavily inspired by Matthew Oros on YouTube who published his coding journey for all to see.
Here is his github repository holding the project I followed closely, but without copying:\
[orosmatthew/hydrogen-cpp: A hobby programming language](https://github.com/orosmatthew/hydrogen-cpp)\
Ok, well now you know I kinda cheated... But wait before thy judgment bestows damage upon my pride, for I have indeed surpassed my Master.\
Well, I indeed surpassed the one who started my project, because I wrote my own version of the compiler structure he used, except in C.\
If that ain't convincing enough, if you take a look at his language, it still doesnt have a while loop, printing, input, etc.

# The Exodia (The Forbidden One) Programming Language
, or the Exo programming language for short.\
This programming language does not have a `main()` function, but does use the same logic as python, where the code in the source is ran automatically.\
Now what features does this programming language have? Or what features does the compiler have?
- 64-bit Integer values and variables
- Character values and variables
- Scopes, Control flow and Loops
- Conditions and conditional operators (AND and OR as && and ||)
- Standard functions such as `println()`, `printnum()`, `printchar()`, `getnum()`, `getchar()` and `obliterate()`
- Mathematical operations such as +, -, \*, \/ and %
- Mathematical operator precedence

# Support
- Linux only for now
- Needs to have the *nasm* Assembler installed
- Needs to have the *GNU Linker* (*aka `ld`*)

# How to build and use
You will need to use the CMake buider.\
The executable will be built as `./build/exodia`.
```
git clone https://github.com/DioDogHater/ExodiaCompiler
cd ExodiaCompiler
mkdir build
cmake -S . -B ./build
cmake --build ./build
```
