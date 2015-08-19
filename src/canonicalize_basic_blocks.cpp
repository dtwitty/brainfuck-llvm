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

void CanonicalizeVisitor::StartBB() { _current_bb = {}; }

void CanonicalizeVisitor::FinishBB() {
  // Add instrcutions for additions
  for (auto& pair : _current_bb.additions) {
    int offset = pair.first;
    int amt = pair.second;
    AddSimpleStatement(new CAdd(offset, amt));
  }

  // Add pointer move instruction
  if (_current_bb.ptr_mov != 0) {
    AddSimpleStatement(new CPtrMov(_current_bb.ptr_mov));
  }
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

void CanonicalizeVisitor::Visit(CNode& n) { VisitNextCNode(n); }

void CanonicalizeVisitor::Visit(CPtrMov& n) {
  _current_bb.ptr_mov += n.GetAmt();
  VisitNextCNode(n);
}

void CanonicalizeVisitor::Visit(CAdd& n) {
  _current_bb.additions[n.GetOffset() + _current_bb.ptr_mov] += n.GetAmt();
  VisitNextCNode(n);
}

void CanonicalizeVisitor::Visit(CMul& n) {
  FinishBB();
  AddSimpleStatement(
      new CMul(n.GetOpOffset(), n.GetTargetOffset(), n.GetAmt()));
  StartBB();
  VisitNextCNode(n);
}

void CanonicalizeVisitor::Visit(CSet& n) {
  FinishBB();
  AddSimpleStatement(new CSet(n.GetOffset(), n.GetAmt()));
  StartBB();
  VisitNextCNode(n);
}

void CanonicalizeVisitor::Visit(CInput& n) {
  FinishBB();
  AddSimpleStatement(new CInput(n.GetOffset()));
  StartBB();
  VisitNextCNode(n);
}

void CanonicalizeVisitor::Visit(COutput& n) {
  FinishBB();
  AddSimpleStatement(new COutput(n.GetOffset()));
  StartBB();
  VisitNextCNode(n);
}

void CanonicalizeVisitor::Visit(CLoop& n) {
  FinishBB();
  CNode* body_node = new CNode();

  StartBB();
  _blocks.push(body_node);
  n.GetBody()->Accept(*this);
  _blocks.pop();

  CLoop* loop = new CLoop();
  loop->SetBody(body_node);
  // Skip the dummy node if possible
  if (body_node->GetNextCNode()) {
    loop->SetBody(body_node->GetNextCNode());
  }
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
