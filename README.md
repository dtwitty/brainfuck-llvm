# brainfuck-llvm
A brainfuck-to-llvm interpreter and optimizing compiler  

./bf -h for usage  

Features
========
* Several Optimizations, including block-level constant folding,
offset instructions, and loop elimination
* Prioritizes maintainability, with a modular design dominated by visitors
* Outputs valid LLVM code, which can be run with the JIT (included) or 
compiled even further to machine code.  
