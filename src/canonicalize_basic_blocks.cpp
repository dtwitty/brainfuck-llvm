#include <stack>
#include <unordered_map>
#include <utility>

#include "canon_ir.h"
#include "canonicalize_basic_blocks.h"

CanonicalizeVisitor::CanonicalizeVisitor() {
  start_node_ = new CNode();
  blocks_.push(start_node_);
  StartBB();
}

void CanonicalizeVisitor::StartBB() { current_bb_ = {}; }

void CanonicalizeVisitor::FinishBB() {
  // Add instrcutions for additions
  for (auto& pair : current_bb_.additions) {
    int offset = pair.first;
    int amt = pair.second;
    AddSimpleStatement(new CAdd(offset, amt));
  }

  // Add pointer move instruction
  if (current_bb_.ptr_mov != 0) {
    AddSimpleStatement(new CPtrMov(current_bb_.ptr_mov));
  }
}

void CanonicalizeVisitor::VisitNextCNode(CNode* s) {
  CNode* next = s->GetNextCNode();
  if (next) {
    next->Accept(*this);
  } else {
    FinishBB();
  }
}

void CanonicalizeVisitor::AddSimpleStatement(CNode* n) {
  CNode* block = blocks_.top();
  block->SetNextCNode(n);
  blocks_.top() = n;
}

void CanonicalizeVisitor::Visit(CNode* n) { VisitNextCNode(n); }

void CanonicalizeVisitor::Visit(CPtrMov* n) {
  current_bb_.ptr_mov += n->GetAmt();
  VisitNextCNode(n);
}

void CanonicalizeVisitor::Visit(CAdd* n) {
  current_bb_.additions[n->GetOffset() + current_bb_.ptr_mov] += n->GetAmt();
  VisitNextCNode(n);
}

void CanonicalizeVisitor::Visit(CMul* n) {
  FinishBB();
  AddSimpleStatement(
      new CMul(n->GetOpOffset(), n->GetTargetOffset(), n->GetAmt()));
  StartBB();
  VisitNextCNode(n);
}

void CanonicalizeVisitor::Visit(CSet* n) {
  FinishBB();
  AddSimpleStatement(new CSet(n->GetOffset(), n->GetAmt()));
  StartBB();
  VisitNextCNode(n);
}

void CanonicalizeVisitor::Visit(CInput* n) {
  FinishBB();
  AddSimpleStatement(new CInput(n->GetOffset()));
  StartBB();
  VisitNextCNode(n);
}

void CanonicalizeVisitor::Visit(COutput* n) {
  FinishBB();
  AddSimpleStatement(new COutput(n->GetOffset()));
  StartBB();
  VisitNextCNode(n);
}

void CanonicalizeVisitor::Visit(CLoop* n) {
  FinishBB();
  CNode* body_node = new CNode();

  StartBB();
  blocks_.push(body_node);
  n->GetBody()->Accept(*this);
  blocks_.pop();

  CLoop* loop = new CLoop();
  loop->SetBody(body_node);
  AddSimpleStatement(loop);
  StartBB();
  VisitNextCNode(n);
}

CNode* CanonicalizeBasicBlocks(CNode* n) {
  CanonicalizeVisitor visitor;
  if (n) {
    n->Accept(visitor);
  }
  return visitor.GetProgram();
}
