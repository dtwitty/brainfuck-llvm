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
#include "ast_codegen.h"

using namespace std;
using namespace llvm;

void help(char* argv[]) {
  cerr << "Usage: " << argv[0] << " [options] file" << endl;
  cerr << "Compiles or interprets brainfuck file" << endl;
  cerr << "Options:" << endl;
  cerr << "  -i          JIT compiles and runs the input file" << endl;
  cerr << "  -O          Apply optimizations (not implemented)" << endl;
  cerr << "  -o outfile  Outputs llvm code to outfile" << endl;
  cerr << "  -s size     Set the size of the bf tape (default 10000)" << endl;
  cerr << "  -h          Displays this help message" << endl;
}

int main(int argc, char* argv[]) {
  bool interpret_flag = false;
  bool output_flag = false;
  bool optimize_flag = false;
  char* output_file;
  unsigned store_size = 10000;

  char option_char;
  while ((option_char = getopt(argc, argv, "s:iho:O")) != EOF) {
    switch (option_char) {
      case 's':
        store_size = atoi(optarg);
        break;
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
  ASTNode* prog = Parse(source_file);
  // TODO: better memory management - preferably allocate vector
  Function* func = BuildProgram(prog, module, store_size);

  if (optimize_flag) {
    legacy::FunctionPassManager pm(module);

    // Eliminate simple loops such as [>>++<<-]
    pm.add(createInstructionCombiningPass()); // Cleanup for scalarrepl.
    pm.add(createLICMPass());                 // Hoist loop invariants
    pm.add(createIndVarSimplifyPass());       // Canonicalize indvars
    pm.add(createLoopDeletionPass());         // Delete dead loops

    // Simplify code
    for(int repeat=0; repeat < 3; repeat++)
    {
      pm.add(createGVNPass());                  // Remove redundancies
      pm.add(createSCCPPass());                 // Constant prop with SCCP
      pm.add(createCFGSimplificationPass());    // Merge & remove BBs
      pm.add(createInstructionCombiningPass());
      pm.add(createAggressiveDCEPass());        // Delete dead instructions
      pm.add(createCFGSimplificationPass());    // Merge & remove BBs
      pm.add(createDeadStoreEliminationPass()); // Delete dead stores
    }
    pm.run(*func);
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
