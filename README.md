# brainfuck-llvm
A brainfuck-to-llvm interpreter and optimizing compiler

Usage: ./bf [options] file
Compiles or interprets brainfuck file
Options:
  -i          JIT compiles and runs the input file
  -O          Apply optimizations (not implemented)
  -o outfile  Outputs llvm code to outfile
  -h          Displays this help message

TODO:
  Add optimization passes
  Allocate heap memory as needed
