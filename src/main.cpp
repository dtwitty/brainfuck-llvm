#include <iostream>
#include <stack>
#include <fstream>
#include <getopt.h>
#include <fcntl.h>
#include <memory>
#include <system_error>

#include "llvm/Analysis/Passes.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/PassManager.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Scalar.h"

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
  cerr << "  -O          Apply BF-specific optimizations" << endl;
  cerr << "  -L          Apply LLVM optimizations" << endl;
  cerr << "  -p          Print new program to stderr" << endl;
  cerr << "  -o outfile  Outputs llvm code to outfile" << endl;
  cerr << "  -s size     Set the size of the bf tape (default 10000)" << endl;
  cerr << "  -h          Displays this help message" << endl;
}

int main(int argc, char* argv[]) {
  bool interpret_flag = false;
  bool output_flag = false;
  bool optimize_bf_flag = false;
  bool optimize_llvm_flag = false;
  bool print_flag = false;
  char* output_file;
  unsigned store_size = 10000;

  char option_char;
  while ((option_char = getopt(argc, argv, "ps:iho:OL")) != EOF) {
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
        optimize_bf_flag = true;
        break;
      case 'L':
        optimize_llvm_flag = true;
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
  std::unique_ptr<Module> module(new Module("bfcode", getGlobalContext()));
  std::unique_ptr<ASTNode> prog(Parse(source_file));
  // This function belongs to the module
  Function* func;

  if (optimize_bf_flag) {
    std::unique_ptr<CNode> canon_prog(TranslateASTToCanonIR(prog.get()));
    canon_prog.reset(CanonicalizeBasicBlocks(canon_prog.get()));
    canon_prog.reset(EliminateSimpleLoops(canon_prog.get()));
    if (print_flag) {
      PrintCanonIR(canon_prog.get());
    }
    func = BuildProgramFromCanon(canon_prog.get(), module.get(), store_size);

  } else {
    if (print_flag) {
      std::unique_ptr<CNode> canon_prog(TranslateASTToCanonIR(prog.get()));
      PrintCanonIR(canon_prog.get());
    }
    func = BuildProgramFromAST(prog.get(), module.get(), store_size);
  }

  if (optimize_llvm_flag) {
    FunctionPassManager pass_manager(module.get());
    pass_manager.add(createVerifierPass());
    pass_manager.add(new DataLayoutPass());
    for (int repeat = 0; repeat < 5; repeat++) {
      pass_manager.add(
          createInstructionCombiningPass());  // Cleanup for scalarrepl.
      pass_manager.add(createLICMPass());     // Hoist loop invariants
      pass_manager.add(createLoopStrengthReducePass());  // Reduce strength
      pass_manager.add(createIndVarSimplifyPass());      // Canonicalize indvars
      pass_manager.add(createLoopDeletionPass());        // Delete dead loops
      pass_manager.add(createGVNPass());                 // Remove redundancies
      pass_manager.add(createSCCPPass());  // Constant prop with SCCP
      pass_manager.add(createCFGSimplificationPass());  // Merge & remove BBs
      pass_manager.add(createInstructionCombiningPass());
      pass_manager.add(
          createConstantPropagationPass());         // Propagate conditionals
      pass_manager.add(createGVNPass());            // Remove redundancies
      pass_manager.add(createAggressiveDCEPass());  // Delete dead instructions
      pass_manager.add(createCFGSimplificationPass());     // Merge & remove BBs
      pass_manager.add(createDeadStoreEliminationPass());  // Delete dead stores
    }

    pass_manager.doInitialization();
    pass_manager.run(*func);
  }

  if (output_flag) {
    std::error_code ec;
    raw_fd_ostream out_stream(output_file, ec, llvm::sys::fs::F_RW);
    module->print(out_stream, NULL);
  }

  if (interpret_flag) {
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    InitializeNativeTargetAsmParser();
    std::string error;
    std::unique_ptr<ExecutionEngine> engine(
        EngineBuilder(std::move(module))
            .setErrorStr(&error)
            .setMCJITMemoryManager(llvm::make_unique<SectionMemoryManager>())
            .create());
    engine->finalizeObject();

    if (!engine.get()) {
      cout << "Engine not created: " << error << endl;
      return -1;
    }

    void (*bf)() = (void (*)())engine->getPointerToFunction(func);
    bf();
  }
  return 0;
}
