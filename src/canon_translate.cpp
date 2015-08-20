#include "canon_translate.h"
#include "canon_ir.h"
#include "parser.h"

CanonTranslateVisitor::CanonTranslateVisitor() {
  start_node_ = new CNode();
  blocks_.push(start_node_);
}

void CanonTranslateVisitor::VisitNextASTNode(ASTNode* n) {
  ASTNode* next = n->GetNextASTNode();
  if (next) {
    next->Accept(*this);
  }
}

void CanonTranslateVisitor::Visit(ASTNode* n) { VisitNextASTNode(n); }

void CanonTranslateVisitor::AddSimpleStatement(CNode* n) {
  CNode* block = blocks_.top();
  block->SetNextCNode(n);
  blocks_.top() = n;
}

CNode* CanonTranslateVisitor::GetProgram() { return start_node_; }

void CanonTranslateVisitor::Visit(IncrPtr* n) {
  AddSimpleStatement(new CPtrMov(1));
  VisitNextASTNode(n);
}

void CanonTranslateVisitor::Visit(DecrPtr* n) {
  AddSimpleStatement(new CPtrMov(-1));
  VisitNextASTNode(n);
}

void CanonTranslateVisitor::Visit(IncrData* n) {
  AddSimpleStatement(new CAdd(0, 1));
  VisitNextASTNode(n);
}

void CanonTranslateVisitor::Visit(DecrData* n) {
  AddSimpleStatement(new CAdd(0, -1));
  VisitNextASTNode(n);
}

void CanonTranslateVisitor::Visit(GetInput* n) {
  AddSimpleStatement(new CInput(0));
  VisitNextASTNode(n);
}

void CanonTranslateVisitor::Visit(Output* n) {
  AddSimpleStatement(new COutput(0));
  VisitNextASTNode(n);
}

void CanonTranslateVisitor::Visit(BFLoop* n) {
  CNode* body_node = new CNode();

  blocks_.push(body_node);
  n->GetBody()->Accept(*this);
  blocks_.pop();

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
