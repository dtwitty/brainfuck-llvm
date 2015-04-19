#include <iostream>
#include <stack>
#include <fstream>
#include <getopt.h>
#include <fcntl.h>

#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/PassManager.h>
#include <llvm/LinkAllPasses.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/JIT.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Support/FileSystem.h>

#include "parser.h"
#include "codegen.h"

using namespace std;
using namespace llvm;

void help(char* argv[]) {
  cerr << "Usage: " << argv[0] << " [options] file" << endl;
  cerr << "Compiles or interprets brainfuck file" << endl;
  cerr << "Options:" << endl;
  cerr << "  -i          JIT compiles and runs the input file" << endl;
  cerr << "  -O          Apply optimizations (not implemented)" << endl;
  cerr << "  -o outfile  Outputs llvm code to outfile" << endl;
  cerr << "  -h          Displays this help message" << endl;
}

int main(int argc, char* argv[]) {
  bool interpret_flag = false;
  bool output_flag = false;
  bool optimize_flag = false;
  char* output_file;

  char option_char;
  while ((option_char = getopt(argc, argv, "iho:O")) != EOF) {
    switch (option_char) {
      case 'i':
        interpret_flag = true;
        break;
      case 'o':
        output_flag = true;
        output_file = optarg;
        break;
      case 'h':
        help(argv);
        return 0;
      case 'O':
        optimize_flag = true;
        break;
      default:
        help(argv);
        return -1;
    }
  }

  if (optind == argc) {
    help(argv);
    return -1;
  }

  ifstream source_file(argv[optind]);
  Module* module = new Module("bfcode", getGlobalContext());
  Statement* prog = Parse(source_file);
  // TODO: better memory management - preferably allocate vector
  Function* func = BuildProgram(prog, module, 10000);

  if (optimize_flag) {
    legacy::FunctionPassManager pm(module);
    // TODO: Add optimization passes
  }

  if (output_flag) {
    string error_str;
    llvm::sys::fs::OpenFlags flag(llvm::sys::fs::F_RW);
    raw_fd_ostream out_stream(output_file, error_str, flag);
    module->print(out_stream, NULL);
  }

  if (interpret_flag) {
    InitializeNativeTarget();
    std::string error;
    ExecutionEngine* engine =
        EngineBuilder(module).setErrorStr(&error).create();

    if (!engine) {
      cout << "Engine not created: " << error << endl;
      return -1;
    }

    void (*bf)() = (void (*)())engine->getPointerToFunction(func);
    bf();
  }
  return 0;
}
