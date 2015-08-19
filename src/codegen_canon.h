#ifndef CODEGEN_CANON
#define CODEGEN_CANON

#include <stack>

#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

#include "canon_ir.h"

class CNodeCodeGenVisitor : public CNodeVisitor {
 public:
  CNodeCodeGenVisitor(llvm::Module* module, int store_size);
  void Visit(CNode& n);
  void Visit(CPtrMov& n);
  void Visit(CAdd& n);
  void Visit(CMul& n);
  void Visit(CSet& n);
  void Visit(CInput& n);
  void Visit(COutput& n);
  void Visit(CLoop& n);

  llvm::Function* GetMain() { return _main; }
  llvm::IRBuilder<> GetLastBuilder() { return _builders.top(); }

 private:
  void VisitNextCNode(CNode& s);
  llvm::Value* GetPtrOffset(int offset);
  llvm::Value* GetDataOffset(int offset);
  llvm::Module* _module;
  llvm::Value* _ptr;
  llvm::Function* _get_char;
  llvm::Function* _put_char;
  llvm::Function* _main;
  std::stack<llvm::IRBuilder<>> _builders;
};

llvm::Function* BuildProgramFromCanon(CNode* s, llvm::Module* module,
                                      int store_size);
#endif  // CODEGEN_CANON
