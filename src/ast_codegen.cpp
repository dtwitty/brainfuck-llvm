#include <stack>
#include <cassert>

#include <llvm/IR/Verifier.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include "parser.h"
#include "ast_codegen.h"

using namespace llvm;

static Type* VOID_TYPE = Type::getVoidTy(getGlobalContext());
static IntegerType* CELL_TYPE = IntegerType::get(getGlobalContext(), 8);
static IntegerType* INDEX_TYPE = IntegerType::get(getGlobalContext(), 32);
static PointerType* STORE_TYPE = PointerType::get(CELL_TYPE, 0);
static Value* zero = ConstantInt::get(CELL_TYPE, 0);
static Value* one = ConstantInt::get(CELL_TYPE, 1);
static Value* neg_one = ConstantInt::get(CELL_TYPE, -1);

CodeGenVisitor::CodeGenVisitor(Module* module, int store_size) {
  _module = module;

  // Define function for getting input
  _get_char =
    cast<Function>(_module->getOrInsertFunction("getchar", CELL_TYPE, NULL));
  _get_char->setCallingConv(CallingConv::C);

  // Define function for printing
  _put_char = cast<Function>(
      _module->getOrInsertFunction("putchar", VOID_TYPE, CELL_TYPE, NULL));
  _put_char->setCallingConv(CallingConv::C);

  // Define main function
  _main =
    cast<Function>(_module->getOrInsertFunction("main", VOID_TYPE, NULL));
  _main->setCallingConv(CallingConv::C);

  // Push the main block onto a stack of loops
  IRBuilder<> builder(BasicBlock::Create(getGlobalContext(), "code", _main));
  _builders.push(builder);

  Value* store_size_v = ConstantInt::get(INDEX_TYPE, store_size);

  // Allocate the data pointer, an array of size store_size
  _ptr =
    builder.CreateAlloca(CELL_TYPE, store_size_v);

  // Zero-out the data array
  builder.CreateMemSet(_ptr, zero, store_size, 0);
}

void CodeGenVisitor::VisitNextASTNode(ASTNode& s) {
  ASTNode* next = s.GetNextASTNode();
  if (next) {
    next->Accept(*this);
  }
}

void CodeGenVisitor::Visit(ASTNode& s) { VisitNextASTNode(s); }

void CodeGenVisitor::Visit(IncrPtr& s) {
  IRBuilder<> builder = _builders.top();
  _ptr = builder.CreateGEP(_ptr, one);
  VisitNextASTNode(s);
}

void CodeGenVisitor::Visit(DecrPtr& s) {
  IRBuilder<> builder = _builders.top();
  _ptr = builder.CreateGEP(_ptr, neg_one);
  VisitNextASTNode(s);
}

void CodeGenVisitor::Visit(IncrData& s) {
  IRBuilder<> builder = _builders.top();
  Value* ptr_val = builder.CreateLoad(_ptr);
  Value* result = builder.CreateAdd(ptr_val, one);
  builder.CreateStore(result, _ptr);
  VisitNextASTNode(s);
}

void CodeGenVisitor::Visit(DecrData& s) {
  IRBuilder<> builder = _builders.top();
  Value* ptr_val = builder.CreateLoad(_ptr);
  Value* result = builder.CreateAdd(ptr_val, neg_one);
  builder.CreateStore(result, _ptr);
  VisitNextASTNode(s);
}

void CodeGenVisitor::Visit(GetInput& s) {
  IRBuilder<> builder = _builders.top();
  Value* input = builder.CreateCall(_get_char);
  builder.CreateStore(input, _ptr);
  VisitNextASTNode(s);
}

void CodeGenVisitor::Visit(Output& s) {
  IRBuilder<> builder = _builders.top();
  Value* output = builder.CreateLoad(_ptr);
  builder.CreateCall(_put_char, output);
  VisitNextASTNode(s);
}

void CodeGenVisitor::Visit(BFLoop& s) {
  // Create basic blocks for condition, body, and after
  BasicBlock* body_block = BasicBlock::Create(getGlobalContext(), "", _main);
  BasicBlock* post_block = BasicBlock::Create(getGlobalContext(), "", _main);

  // Make builders for each block
  IRBuilder<> curr_builder = _builders.top();
  IRBuilder<> body_builder(body_block);
  IRBuilder<> post_builder(post_block);

  BasicBlock* curr_block = curr_builder.GetInsertBlock();

  // Conditionally jump into the body or to the post block
  Value* ptr_value = curr_builder.CreateLoad(_ptr);
  Value* cond = curr_builder.CreateIsNotNull(ptr_value);
  curr_builder.CreateCondBr(cond, body_block, post_block);

  // Current block is now done
  _builders.pop();

  // Create a phi node in the body for the ptr
  PHINode* body_phi = body_builder.CreatePHI(STORE_TYPE, 2);
  body_phi->addIncoming(_ptr, curr_block);

  // Create a phi node in the post block for the ptr
  PHINode* post_phi = post_builder.CreatePHI(STORE_TYPE, 2);
  post_phi->addIncoming(_ptr, curr_block);

  // Set the loop body as our current block
  _builders.push(body_builder);

  // Set the pointer in the body to the phi node
  _ptr = body_phi;

  // Process the loop body
  s.GetBody()->Accept(*this);

  // Body could have progressed to a new block
  IRBuilder<> new_body_builder = _builders.top();
  BasicBlock* new_body_block = new_body_builder.GetInsertBlock();

  // Create a conditional branch to restart the loop
  ptr_value = new_body_builder.CreateLoad(_ptr);
  cond = new_body_builder.CreateIsNotNull(ptr_value);
  new_body_builder.CreateCondBr(cond, body_block, post_block);

  // Update phi nodes
  body_phi->addIncoming(_ptr, new_body_block);
  post_phi->addIncoming(_ptr, new_body_block); 

  // Body block is now done
  _builders.pop();

  // Set the block after the loop as the current block
  _builders.push(post_builder);

  // Set the pointer to the phi node
  _ptr = post_phi; 

  // Visist the rest of the program
  VisitNextASTNode(s);
}

Function* BuildProgram(ASTNode* s, llvm::Module* module, int store_size) {
  CodeGenVisitor visitor(module, store_size);
  s->Accept(visitor);
  IRBuilder<> builder = visitor.GetLastBuilder();
  builder.CreateRetVoid();
  Function* func = visitor.GetMain();
  return func;
}
