#include <stack>
#include <cassert>

#include "llvm/IR/Verifier.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

#include "parser.h"
#include "codegen.h"

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

  // Allocate the data pointer, an array of size store_size
  _ptr =
      builder.CreateAlloca(CELL_TYPE, ConstantInt::get(INDEX_TYPE, store_size));

  // Zero-out the data array
  Value* iter = _ptr;
  for (int i = 0; i < store_size; i++) {
    builder.CreateStore(zero, iter);
    iter = builder.CreateGEP(iter, one);
  }
}

void CodeGenVisitor::VisitNextStatement(Statement& s) {
  Statement* next = s.GetNextStatement();
  if (next) {
    next->Accept(*this);
  }
}

void CodeGenVisitor::Visit(Statement& s) { VisitNextStatement(s); }

void CodeGenVisitor::Visit(IncrPtr& s) {
  IRBuilder<> builder = _builders.top();
  _ptr = builder.CreateGEP(_ptr, one);
  VisitNextStatement(s);
}

void CodeGenVisitor::Visit(DecrPtr& s) {
  IRBuilder<> builder = _builders.top();
  _ptr = builder.CreateGEP(_ptr, neg_one);
  VisitNextStatement(s);
}

void CodeGenVisitor::Visit(IncrData& s) {
  IRBuilder<> builder = _builders.top();
  Value* ptr_val = builder.CreateLoad(_ptr);
  Value* result = builder.CreateAdd(ptr_val, one);
  builder.CreateStore(result, _ptr);
  VisitNextStatement(s);
}

void CodeGenVisitor::Visit(DecrData& s) {
  IRBuilder<> builder = _builders.top();
  Value* ptr_val = builder.CreateLoad(_ptr);
  Value* result = builder.CreateAdd(ptr_val, neg_one);
  builder.CreateStore(result, _ptr);
  VisitNextStatement(s);
}

void CodeGenVisitor::Visit(GetInput& s) {
  IRBuilder<> builder = _builders.top();
  Value* input = builder.CreateCall(_get_char);
  builder.CreateStore(input, _ptr);
  VisitNextStatement(s);
}

void CodeGenVisitor::Visit(Output& s) {
  IRBuilder<> builder = _builders.top();
  Value* output = builder.CreateLoad(_ptr);
  builder.CreateCall(_put_char, output);
  VisitNextStatement(s);
}

void CodeGenVisitor::Visit(BFLoop& s) {
  // Create basic blocks for condition, body, and after
  BasicBlock* cond_block = BasicBlock::Create(getGlobalContext(), "", _main);
  BasicBlock* body_block = BasicBlock::Create(getGlobalContext(), "", _main);
  BasicBlock* next_block = BasicBlock::Create(getGlobalContext(), "", _main);

  // Make builders for each block
  IRBuilder<> curr_builder = _builders.top();
  IRBuilder<> cond_builder(cond_block);
  IRBuilder<> body_builder(body_block);
  IRBuilder<> next_builder(next_block);

  BasicBlock* curr_block = curr_builder.GetInsertBlock();

  // Unconditionally jump to the condition block
  curr_builder.CreateBr(cond_block);

  // Current block is now done
  _builders.pop();

  // Create a phi node in the condition
  PHINode* cond_phi = cond_builder.CreatePHI(STORE_TYPE, 2);
  cond_phi->addIncoming(_ptr, curr_block);

  // Create a branch at the condition block
  Value* ptr_value = cond_builder.CreateLoad(cond_phi);
  Value* cond = cond_builder.CreateIsNotNull(ptr_value);
  cond_builder.CreateCondBr(cond, body_block, next_block);

  // Set the loop body as our current block
  _builders.push(body_builder);

  // Set the pointer in the body to the phi node
  _ptr = body_builder.CreateGEP(cond_phi, zero);

  // Process the loop body
  s.GetBody()->Accept(*this);

  // Body could have progressed to a new block
  IRBuilder<> new_body_builder = _builders.top();
  BasicBlock* new_body_block = new_body_builder.GetInsertBlock();

  // Add an unconditional jump at the end of the body back to the condition
  new_body_builder.CreateBr(cond_block);

  // Update the conditional phi node
  cond_phi->addIncoming(_ptr, new_body_block);

  // Body block is now done
  _builders.pop();

  // Set the block after the loop as the current block
  _builders.push(next_builder);

  // Set the pointer to the phi node
  _ptr = next_builder.CreateGEP(cond_phi, zero);

  // Visist the rest of the program
  VisitNextStatement(s);
}

Function* BuildProgram(Statement* s, llvm::Module* module, int store_size) {
  CodeGenVisitor visitor(module, store_size);
  s->Accept(visitor);
  IRBuilder<> builder = visitor.GetLastBuilder();
  builder.CreateRetVoid();
  Function* func = visitor.GetMain();
  return func;
}
