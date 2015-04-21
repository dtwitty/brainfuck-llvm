#include "canon_translate.h"
#include "canon_ir.h"
#include "parser.h"

CanonTranslateVisitor::CanonTranslateVisitor() {
  _start_node = new CNode();
  _blocks.push(_start_node);
}

void CanonTranslateVisitor::VisitNextASTNode(ASTNode& n) {
  ASTNode* next = n.GetNextASTNode();
  if (next) {
    next->Accept(*this);
  }
}

void CanonTranslateVisitor::Visit(ASTNode& n) { VisitNextASTNode(n); }

void CanonTranslateVisitor::AddSimpleStatement(CNode* n) {
  CNode* block = _blocks.top();
  block->SetNextCNode(n);
  _blocks.top() = n;
}

CNode* CanonTranslateVisitor::GetProgram() { return _start_node; }

void CanonTranslateVisitor::Visit(IncrPtr& n) {
  AddSimpleStatement(new CPtrMov(1));
  VisitNextASTNode(n);
}

void CanonTranslateVisitor::Visit(DecrPtr& n) {
  AddSimpleStatement(new CPtrMov(-1));
  VisitNextASTNode(n);
}

void CanonTranslateVisitor::Visit(IncrData& n) {
  AddSimpleStatement(new CAdd(0, 1));
  VisitNextASTNode(n);
}

void CanonTranslateVisitor::Visit(DecrData& n) {
  AddSimpleStatement(new CAdd(0, -1));
  VisitNextASTNode(n);
}

void CanonTranslateVisitor::Visit(GetInput& n) {
  AddSimpleStatement(new CInput(0));
  VisitNextASTNode(n);
}

void CanonTranslateVisitor::Visit(Output& n) {
  AddSimpleStatement(new COutput(0));
  VisitNextASTNode(n);
}

void CanonTranslateVisitor::Visit(BFLoop& n) {
  CNode* body_node = new CNode();

  _blocks.push(body_node);
  n.GetBody()->Accept(*this);
  _blocks.pop();

  CLoop* loop = new CLoop();
  loop->SetBody(body_node);
  AddSimpleStatement(loop);
  VisitNextASTNode(n);
}

CNode* TranslateASTToCanonIR(ASTNode* s) {
  CanonTranslateVisitor visitor;
  if (s) {
    s->Accept(visitor);
  }
  return visitor.GetProgram();
}
