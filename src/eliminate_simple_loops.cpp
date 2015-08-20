#include <stack>
#include <unordered_map>

#include "canon_ir.h"
#include "eliminate_simple_loops.h"

SimpleLoopElimVisitor::SimpleLoopElimVisitor() {
  start_node_ = new CNode();
  blocks_.push(start_node_);
  is_simple_ = false;
  ptr_mov_ = 0;
}

void SimpleLoopElimVisitor::VisitNextCNode(CNode* n) {
  CNode* next = n->GetNextCNode();
  if (next) {
    next->Accept(*this);
  }
}

void SimpleLoopElimVisitor::AddSimpleStatement(CNode* n) {
  CNode* block = blocks_.top();
  block->SetNextCNode(n);
  blocks_.top() = n;
}

void SimpleLoopElimVisitor::StartSimpleLoop() {
  is_simple_ = true;
  mult_map_.clear();
  ptr_mov_ = 0;
}

void SimpleLoopElimVisitor::Visit(CNode* n) { VisitNextCNode(n); }

void SimpleLoopElimVisitor::Visit(CPtrMov* n) {
  int amt = n->GetAmt();
  if (is_simple_) {
    ptr_mov_ += amt;
  }
  AddSimpleStatement(new CPtrMov(amt));
  VisitNextCNode(n);
}

void SimpleLoopElimVisitor::Visit(CAdd* n) {
  int offset = n->GetOffset();
  int amt = n->GetAmt();
  if (is_simple_) {
    mult_map_[offset + ptr_mov_] += amt;
  }
  AddSimpleStatement(new CAdd(offset, amt));
  VisitNextCNode(n);
}

void SimpleLoopElimVisitor::Visit(CMul* n) {
  int op_offset = n->GetOpOffset();
  int target_offset = n->GetTargetOffset();
  int amt = n->GetAmt();
  is_simple_ = false;
  AddSimpleStatement(new CMul(op_offset, target_offset, amt));
  VisitNextCNode(n);
}

void SimpleLoopElimVisitor::Visit(CSet* n) {
  int offset = n->GetOffset();
  int amt = n->GetAmt();
  is_simple_ = false;
  AddSimpleStatement(new CSet(offset, amt));
  VisitNextCNode(n);
}

void SimpleLoopElimVisitor::Visit(CInput* n) {
  int offset = n->GetOffset();
  is_simple_ = false;
  AddSimpleStatement(new CInput(offset));
  VisitNextCNode(n);
}

void SimpleLoopElimVisitor::Visit(COutput* n) {
  int offset = n->GetOffset();
  is_simple_ = false;
  AddSimpleStatement(new COutput(offset));
  VisitNextCNode(n);
}

void SimpleLoopElimVisitor::Visit(CLoop* n) {
  CNode* body_node = new CNode();

  StartSimpleLoop();
  blocks_.push(body_node);
  n->GetBody()->Accept(*this);
  blocks_.pop();

  if (is_simple_ && ptr_mov_ == 0 && mult_map_[0] == -1) {
    delete body_node;
    for (auto& pair : mult_map_) {
      int target_offset = pair.first;
      int amt = pair.second;
      if (target_offset != 0) {
        AddSimpleStatement(new CMul(0, target_offset, amt));
      }
    }
    AddSimpleStatement(new CSet(0, 0));
  } else {
    CLoop* loop = new CLoop();
    loop->SetBody(body_node);
    AddSimpleStatement(loop);
  }
  is_simple_ = false;
  VisitNextCNode(n);
}

CNode* EliminateSimpleLoops(CNode* n) {
  SimpleLoopElimVisitor visitor;
  if (n) {
    n->Accept(visitor);
  }
  return visitor.GetProgram();
}
