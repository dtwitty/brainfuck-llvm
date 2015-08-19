#include <iostream>
#include <stack>
#include <fstream>
#include <getopt.h>
#include <fcntl.h>
#include <memory>
#include <system_error>

#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/Support/TargetSelect.h>
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include <llvm/Support/raw_ostream.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Support/FileSystem.h>

#include "parser.h"
#include "canon_ir.h"
#include "canon_translate.h"
#include "codegen_ast.h"
#include "codegen_canon.h"
#include "canonicalize_basic_blocks.h"
#include "eliminate_simple_loops.h"
#include "print_canon.h"

using namespace std;
using namespace llvm;

void help(char* argv[]) {
  cerr << "Usage: " << argv[0] << " [options] file" << endl;
  cerr << "Compiles or interprets brainfuck file" << endl;
  cerr << "Options:" << endl;
  cerr << "  -i          JIT compiles and runs the input file" << endl;
  cerr << "  -O          Apply optimizations" << endl;
  cerr << "  -p          Print new program to stderr" << endl;
  cerr << "  -o outfile  Outputs llvm code to outfile" << endl;
  cerr << "  -s size     Set the size of the bf tape (default 10000)" << endl;
  cerr << "  -h          Displays this help message" << endl;
}

int main(int argc, char* argv[]) {
  bool interpret_flag = false;
  bool output_flag = false;
  bool optimize_flag = false;
  bool print_flag = false;
  char* output_file;
  unsigned store_size = 10000;

  char option_char;
  while ((option_char = getopt(argc, argv, "ps:iho:O")) != EOF) {
    switch (option_char) {
      case 'p':
        print_flag = true;
        break;
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
  Function* func;

  if (optimize_flag) {
    // TODO this leaks the old program
    CNode* canon_prog = TranslateASTToCanonIR(prog);
    canon_prog = CanonicalizeBasicBlocks(canon_prog);
    canon_prog = EliminateSimpleLoops(canon_prog);
    if (print_flag) {
      PrintCanonIR(canon_prog);
    }
    func = BuildProgramFromCanon(canon_prog, module, store_size);
  } else {
    if (print_flag) {
      CNode* canon_prog = TranslateASTToCanonIR(prog);
      PrintCanonIR(canon_prog);
    }
    func = BuildProgramFromAST(prog, module, store_size);
  }

  if (output_flag) {
    std::error_code ec;
    raw_fd_ostream out_stream(output_file, ec, llvm::sys::fs::F_RW);
    module->print(out_stream, NULL);
  }


  if (interpret_flag) {
    InitializeNativeTarget();
    std::string error;
    // Module must be unique
    std::unique_ptr<Module> mod_ptr(module);
    ExecutionEngine* engine =
      EngineBuilder(std::move(mod_ptr)).setErrorStr(&error).create();

    if (!engine) {
      cout << "Engine not created: " << error << endl;
      return -1;
    }

    void (*bf)() = (void (*)())engine->getPointerToFunction(func);
    bf();
  }
  return 0;
}
