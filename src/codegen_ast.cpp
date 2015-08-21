#include <stack>
#include <cassert>

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include "parser.h"
#include "codegen_ast.h"

using namespace llvm;

static Type* VOID_TYPE = Type::getVoidTy(getGlobalContext());
static IntegerType* CELL_TYPE = IntegerType::get(getGlobalContext(), 8);
static IntegerType* INDEX_TYPE = IntegerType::get(getGlobalContext(), 32);
static PointerType* STORE_TYPE = PointerType::get(CELL_TYPE, 0);
static Value* zero = ConstantInt::get(CELL_TYPE, 0);
static Value* one = ConstantInt::get(CELL_TYPE, 1);
static Value* neg_one = ConstantInt::get(CELL_TYPE, -1);

ASTCodeGenVisitor::ASTCodeGenVisitor(Module* module, int store_size) {
  module_ = module;

  // Define function for getting input
  get_char_ =
      cast<Function>(module_->getOrInsertFunction("getchar", CELL_TYPE, NULL));
  get_char_->setCallingConv(CallingConv::C);

  // Define function for printing
  put_char_ = cast<Function>(
      module_->getOrInsertFunction("putchar", VOID_TYPE, CELL_TYPE, NULL));
  put_char_->setCallingConv(CallingConv::C);

  // Define main function
  main_ = cast<Function>(module_->getOrInsertFunction("main", VOID_TYPE, NULL));
  main_->setCallingConv(CallingConv::C);

  // Push the main block onto a stack of loops
  IRBuilder<> builder(BasicBlock::Create(getGlobalContext(), "code", main_));
  builders_.push(builder);

  Value* store_size_v = ConstantInt::get(INDEX_TYPE, store_size);

  // Allocate the data pointer, an array of size store_size
  ptr_ = builder.CreateAlloca(CELL_TYPE, store_size_v);

  // Zero-out the data array
  builder.CreateMemSet(ptr_, zero, store_size, 0);
}

void ASTCodeGenVisitor::VisitNextASTNode(ASTNode* s) {
  ASTNode* next = s->GetNextASTNode();
  if (next) {
    next->Accept(*this);
  }
}

void ASTCodeGenVisitor::Visit(ASTNode* s) { VisitNextASTNode(s); }

void ASTCodeGenVisitor::Visit(IncrPtr* s) {
  IRBuilder<> builder = builders_.top();
  ptr_ = builder.CreateGEP(ptr_, one);
  VisitNextASTNode(s);
}

void ASTCodeGenVisitor::Visit(DecrPtr* s) {
  IRBuilder<> builder = builders_.top();
  ptr_ = builder.CreateGEP(ptr_, neg_one);
  VisitNextASTNode(s);
}

void ASTCodeGenVisitor::Visit(IncrData* s) {
  IRBuilder<> builder = builders_.top();
  Value* ptr_val = builder.CreateLoad(ptr_);
  Value* result = builder.CreateAdd(ptr_val, one);
  builder.CreateStore(result, ptr_);
  VisitNextASTNode(s);
}

void ASTCodeGenVisitor::Visit(DecrData* s) {
  IRBuilder<> builder = builders_.top();
  Value* ptr_val = builder.CreateLoad(ptr_);
  Value* result = builder.CreateAdd(ptr_val, neg_one);
  builder.CreateStore(result, ptr_);
  VisitNextASTNode(s);
}

void ASTCodeGenVisitor::Visit(GetInput* s) {
  IRBuilder<> builder = builders_.top();
  Value* input = builder.CreateCall(get_char_);
  builder.CreateStore(input, ptr_);
  VisitNextASTNode(s);
}

void ASTCodeGenVisitor::Visit(Output* s) {
  IRBuilder<> builder = builders_.top();
  Value* output = builder.CreateLoad(ptr_);
  builder.CreateCall(put_char_, output);
  VisitNextASTNode(s);
}

void ASTCodeGenVisitor::Visit(BFLoop* s) {
  // Create basic blocks for condition, body, and after
  BasicBlock* body_block = BasicBlock::Create(getGlobalContext(), "", main_);
  BasicBlock* post_block = BasicBlock::Create(getGlobalContext(), "", main_);

  // Make builders for each block
  IRBuilder<> curr_builder = builders_.top();
  IRBuilder<> body_builder(body_block);
  IRBuilder<> post_builder(post_block);

  BasicBlock* curr_block = curr_builder.GetInsertBlock();

  // Conditionally jump into the body or to the post block
  Value* ptr_value = curr_builder.CreateLoad(ptr_);
  Value* cond = curr_builder.CreateIsNotNull(ptr_value);
  curr_builder.CreateCondBr(cond, body_block, post_block);

  // Current block is now done
  builders_.pop();

  // Create a phi node in the body for the ptr
  PHINode* body_phi = body_builder.CreatePHI(STORE_TYPE, 2);
  body_phi->addIncoming(ptr_, curr_block);

  // Create a phi node in the post block for the ptr
  PHINode* post_phi = post_builder.CreatePHI(STORE_TYPE, 2);
  post_phi->addIncoming(ptr_, curr_block);

  // Set the loop body as our current block
  builders_.push(body_builder);

  // Set the pointer in the body to the phi node
  ptr_ = body_phi;

  // Process the loop body
  s->GetBody()->Accept(*this);

  // Body could have progressed to a new block
  IRBuilder<> new_body_builder = builders_.top();
  BasicBlock* new_body_block = new_body_builder.GetInsertBlock();

  // Create a conditional branch to restart the loop
  ptr_value = new_body_builder.CreateLoad(ptr_);
  cond = new_body_builder.CreateIsNotNull(ptr_value);
  new_body_builder.CreateCondBr(cond, body_block, post_block);

  // Update phi nodes
  body_phi->addIncoming(ptr_, new_body_block);
  post_phi->addIncoming(ptr_, new_body_block);

  // Body block is now done
  builders_.pop();

  // Set the block after the loop as the current block
  builders_.push(post_builder);

  // Set the pointer to the phi node
  ptr_ = post_phi;

  // Visist the rest of the program
  VisitNextASTNode(s);
}

Function* BuildProgramFromAST(ASTNode* s, llvm::Module* module,
                              int store_size) {
  ASTCodeGenVisitor visitor(module, store_size);
  s->Accept(visitor);
  IRBuilder<> builder = visitor.GetLastBuilder();
  builder.CreateRetVoid();
  Function* func = visitor.GetMain();
  return func;
}
