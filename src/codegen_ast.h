#ifndef CODEGEN_AST
#define CODEGEN_AST

#include <stack>

#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

#include "parser.h"

class ASTCodeGenVisitor : public ASTNodeVisitor {
 public:
  ASTCodeGenVisitor(llvm::Module* module, int store_size);
  void Visit(ASTNode* s);
  void Visit(IncrPtr* s);
  void Visit(DecrPtr* s);
  void Visit(IncrData* s);
  void Visit(DecrData* s);
  void Visit(GetInput* s);
  void Visit(Output* s);
  void Visit(BFLoop* s);

  llvm::Function* GetMain() { return _main; }
  llvm::IRBuilder<> GetLastBuilder() { return _builders.top(); }

 private:
  void VisitNextASTNode(ASTNode* s);
  llvm::Module* _module;
  llvm::Value* _ptr;
  llvm::Function* _get_char;
  llvm::Function* _put_char;
  llvm::Function* _main;
  std::stack<llvm::IRBuilder<>> _builders;
};

llvm::Function* BuildProgramFromAST(ASTNode* s, llvm::Module* module,
                                    int store_size);

#endif  // CODEGEN_AST
