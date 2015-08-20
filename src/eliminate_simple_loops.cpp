#include <stack>
#include <unordered_map>

#include "canon_ir.h"
#include "eliminate_simple_loops.h"

SimpleLoopElimVisitor::SimpleLoopElimVisitor() {
  _start_node = new CNode();
  _blocks.push(_start_node);
  _is_simple = false;
  _ptr_mov = 0;
}

void SimpleLoopElimVisitor::VisitNextCNode(CNode* n) {
  CNode* next = n->GetNextCNode();
  if (next) {
    next->Accept(*this);
  }
}

void SimpleLoopElimVisitor::AddSimpleStatement(CNode* n) {
  CNode* block = _blocks.top();
  block->SetNextCNode(n);
  _blocks.top() = n;
}

void SimpleLoopElimVisitor::StartSimpleLoop() {
  _is_simple = true;
  _mult_map.clear();
  _ptr_mov = 0;
}

void SimpleLoopElimVisitor::Visit(CNode* n) { VisitNextCNode(n); }

void SimpleLoopElimVisitor::Visit(CPtrMov* n) {
  int amt = n->GetAmt();
  if (_is_simple) {
    _ptr_mov += amt;
  }
  AddSimpleStatement(new CPtrMov(amt));
  VisitNextCNode(n);
}

void SimpleLoopElimVisitor::Visit(CAdd* n) {
  int offset = n->GetOffset();
  int amt = n->GetAmt();
  if (_is_simple) {
    _mult_map[offset + _ptr_mov] += amt;
  }
  AddSimpleStatement(new CAdd(offset, amt));
  VisitNextCNode(n);
}

void SimpleLoopElimVisitor::Visit(CMul* n) {
  int op_offset = n->GetOpOffset();
  int target_offset = n->GetTargetOffset();
  int amt = n->GetAmt();
  _is_simple = false;
  AddSimpleStatement(new CMul(op_offset, target_offset, amt));
  VisitNextCNode(n);
}

void SimpleLoopElimVisitor::Visit(CSet* n) {
  int offset = n->GetOffset();
  int amt = n->GetAmt();
  _is_simple = false;
  AddSimpleStatement(new CSet(offset, amt));
  VisitNextCNode(n);
}

void SimpleLoopElimVisitor::Visit(CInput* n) {
  int offset = n->GetOffset();
  _is_simple = false;
  AddSimpleStatement(new CInput(offset));
  VisitNextCNode(n);
}

void SimpleLoopElimVisitor::Visit(COutput* n) {
  int offset = n->GetOffset();
  _is_simple = false;
  AddSimpleStatement(new COutput(offset));
  VisitNextCNode(n);
}

void SimpleLoopElimVisitor::Visit(CLoop* n) {
  CNode* body_node = new CNode();

  StartSimpleLoop();
  _blocks.push(body_node);
  n->GetBody()->Accept(*this);
  _blocks.pop();

  if (_is_simple && _ptr_mov == 0 && _mult_map[0] == -1) {
    delete body_node;
    for (auto& pair : _mult_map) {
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
  _is_simple = false;
  VisitNextCNode(n);
}

CNode* EliminateSimpleLoops(CNode* n) {
  SimpleLoopElimVisitor visitor;
  if (n) {
    n->Accept(visitor);
  }
  return visitor.GetProgram();
}
