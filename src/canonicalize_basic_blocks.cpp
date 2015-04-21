#include <stack>
#include <unordered_map>
#include <utility>

#include "canon_ir.h"
#include "canonicalize_basic_blocks.h"

CanonicalizeVisitor::CanonicalizeVisitor() {
  _start_node = new CNode();
  _blocks.push(_start_node);
  StartBB();
}

void CanonicalizeVisitor::StartBB() {
  _current_bb = {};
}

void CanonicalizeVisitor::FinishBB() {
  // Add instrcutions for additions
  // Add instructions for multiplications
  // Add pointer move instruction
}

void CanonicalizeVisitor::VisitNextCNode(CNode& s) {
  CNode* next = s.GetNextCNode();
  if (next) {
    next->Accept(*this);
  } else {
    FinishBB();
  }
}

void CanonicalizeVisitor::AddSimpleStatement(CNode* n) {
  CNode* block = _blocks.top();
  block->SetNextCNode(n);
  _blocks.top() = n;
}

void CanonicalizeVisitor::Visit(CNode& n) {
  VisitNextCNode(n);
}

void CanonicalizeVisitor::Visit(CPtrMov& n) {
  _current_bb.ptr_mov += n.GetAmt();
  VisitNextCNode(n);
}



